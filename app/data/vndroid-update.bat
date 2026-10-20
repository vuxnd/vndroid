@echo off
rem Usage: vndroid-update.bat <zip> <install-dir> <pid>
rem Started by VNdroid when the user accepts an update: waits for it to exit,
rem copies the new files over the install dir, then restarts VNdroid.
setlocal
set "ZIP=%~1"
set "DIR=%~2"
set "PID=%~3"
set "TMP_DIR=%TEMP%\vndroid-update-%PID%"

:wait
tasklist /FI "PID eq %PID%" /FO CSV /NH | find """%PID%""" >nul
if not errorlevel 1 (
    ping -n 2 127.0.0.1 >nul
    goto wait
)

rmdir /S /Q "%TMP_DIR%" 2>nul
mkdir "%TMP_DIR%" || goto fail
tar -xf "%ZIP%" -C "%TMP_DIR%" || goto fail
rem A running adb server locks adb.exe
"%DIR%\adb.exe" kill-server

rem One block: xcopy overwrites this file and cmd reads a .bat line by line
(
    for /D %%D in ("%TMP_DIR%\vndroid-*") do xcopy "%%D" "%DIR%\" /E /Y /Q || goto fail
    rmdir /S /Q "%TMP_DIR%"
    del "%ZIP%"
    start "" /D "%DIR%" "%DIR%\vndroid.exe"
    exit /b 0
)

:fail
echo VNdroid update failed: zip="%ZIP%" dir="%DIR%" > "%TEMP%\vndroid-update.log"
start "" /D "%DIR%" "%DIR%\vndroid.exe"
exit /b 1
