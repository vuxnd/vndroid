@echo off
rem Usage: vndroid-update.bat <zip> <install-dir> <pid> [<arg>...]
rem Started by VNdroid when the user accepts an update: waits for it to exit,
rem copies the new files over the install dir, then restarts VNdroid with
rem <arg>..., through vndroid-noconsole.vbs if that set VNDROID_NOCONSOLE.
setlocal
set "ZIP=%~1"
set "DIR=%~2"
set "PID=%~3"
set "TMP_DIR=%TEMP%\vndroid-update-%PID%"
set "BACKUP=%TEMP%\vndroid-backup-%PID%"
set "REASON="

rem Keep each <arg> as VNdroid quoted it. Test it with set, not if: a lone
rem quote in an <arg> leaves the if line unterminated.
set "ARGS="
:args
set ARG=%4
if not defined ARG goto args_done
set ARGS=%ARGS% %4
shift /4
goto args
:args_done

:wait
tasklist /FI "PID eq %PID%" /FO CSV /NH | find """%PID%""" >nul
if not errorlevel 1 (
    ping -n 2 127.0.0.1 >nul
    goto wait
)

rmdir /S /Q "%TMP_DIR%" 2>nul
rmdir /S /Q "%BACKUP%" 2>nul
mkdir "%TMP_DIR%" || goto fail
tar -xf "%ZIP%" -C "%TMP_DIR%" || goto fail
rem The zip must hold exactly one vndroid-* dir, containing vndroid.exe
set "SRC="
for /D %%D in ("%TMP_DIR%\vndroid-*") do (
    if defined SRC goto fail
    set "SRC=%%D"
)
if not defined SRC goto fail
if not exist "%SRC%\vndroid.exe" goto fail

rem Wait up to 10 s for another VNdroid running from the install dir to exit
rem (it locks vndroid.exe), then for the adb server to exit (it locks adb.exe).
rem Opening a file to append nothing fails while it is locked.
set "TRIES=10"
:unlock
set "LOCKED="
if exist "%DIR%\vndroid.exe" (>>"%DIR%\vndroid.exe" (call )) 2>nul || set "LOCKED=vndroid.exe"
if not defined LOCKED "%DIR%\adb.exe" kill-server
if not defined LOCKED if exist "%DIR%\adb.exe" (>>"%DIR%\adb.exe" (call )) 2>nul || set "LOCKED=adb.exe"
if defined LOCKED (
    if %TRIES%==0 (
        set "REASON=%LOCKED% is in use"
        goto fail
    )
    set /a TRIES-=1
    ping -n 2 127.0.0.1 >nul
    goto unlock
)

rem Back up the install dir, to restore it if the copy fails
xcopy "%DIR%" "%BACKUP%\" /E /Y /Q /I <nul || goto fail

rem One block: xcopy overwrites this file and cmd reads a .bat line by line
(
    xcopy "%SRC%" "%DIR%\" /E /Y /Q <nul || goto fail
    rmdir /S /Q "%TMP_DIR%"
    rmdir /S /Q "%BACKUP%"
    del "%ZIP%"
    if defined VNDROID_NOCONSOLE start "" /D "%DIR%" wscript.exe "%DIR%\vndroid-noconsole.vbs"%ARGS%
    if not defined VNDROID_NOCONSOLE start "" /D "%DIR%" "%DIR%\vndroid.exe"%ARGS%
    exit /b 0
)

:fail
if exist "%BACKUP%" xcopy "%BACKUP%" "%DIR%\" /E /Y /Q <nul
rmdir /S /Q "%BACKUP%" 2>nul
rmdir /S /Q "%TMP_DIR%" 2>nul
del "%ZIP%" 2>nul
echo VNdroid update failed: zip="%ZIP%" dir="%DIR%" reason="%REASON%" > "%TEMP%\vndroid-update.log"
if defined VNDROID_NOCONSOLE start "" /D "%DIR%" wscript.exe "%DIR%\vndroid-noconsole.vbs"%ARGS%
if not defined VNDROID_NOCONSOLE start "" /D "%DIR%" "%DIR%\vndroid.exe"%ARGS%
exit /b 1
