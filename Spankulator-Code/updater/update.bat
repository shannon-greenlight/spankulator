@echo off
echo.
echo.
echo Available COM ports
echo -------------------
REM Get a list of connected boards
powershell -Command "$boardList = .\arduino-cli board list; $boardList -split \"`n\" | ForEach-Object { if ($_ -match '^(COM\d+)\s') { $port = $Matches[1]; if ($_ -match 'Arduino Nano RP2040 Connect') { Write-Host \"Bonkulator found on $port\" } elseif ($_ -match 'Arduino NANO 33 IoT') { Write-Host \"Spankulator found on $port\" } } }"
echo.

set /p port=What's the COM port? (please enter a number)

if %port% == x exit

arduino-cli upload -p COM%port% -b arduino:samd:nano_33_iot -i ./Spankulator.ino.bin
pause
