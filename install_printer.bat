: Delete a previously installed printer 
printui.exe /q /dl /n "kuroko-printer"

: Install a new printer
printui.exe /if /f "%WINDIR%\inf\ntprint.inf" /b "kuroko-printer" /r "FILE:" /m "Microsoft Print to PDF"

:printui.exe /Ss /n "kuroko-printer" /a "kuroko-printer.dat"
:printui.exe /Sr /n "kuroko-printer" /a "kuroko-printer.dat"
