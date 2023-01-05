@echo off
echo.
echo.
echo Available COM ports
echo -------------------
powershell.exe [System.IO.Ports.SerialPort]::getportnames()
echo.
echo.
SetLocal EnableDelayedExpansion

set port=
for /F "delims=" %%i in (spank_port.txt) do set port=%port%%%i
call :TRIM %port%
set port=%TRIMRESULT%

echo Using: COM%port%
set /p port=What's the COM port? (please enter a number or press ENTER to use COM%port%) 'x' to exit 

if %port% == x exit

echo %port% > spank_port.txt

@mode com%port% baud=1200
timeout /t 2

.\updater\bossac.exe -d -U true -i -e -w -v -b .\out\Spankulator.ino.bin -R
EndLocal

:TRIM
  rem echo "%1"
  rem echo "%2"
  rem echo "%3"
  rem echo "%4"
  SET TRIMRESULT=%*
GOTO :EOF

