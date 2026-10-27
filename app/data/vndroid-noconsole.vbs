strCommand = "cmd /c vndroid.exe"

For Each Arg In WScript.Arguments
    strCommand = strCommand & " """ & replace(Arg, """", """""""""") & """"
Next

' vndroid-update.bat restarts VNdroid through this script when this is set
CreateObject("Wscript.Shell").Environment("Process")("VNDROID_NOCONSOLE") = "1"
CreateObject("Wscript.Shell").Run strCommand, 0, false
