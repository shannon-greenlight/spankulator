@echo off

set product_name=Spankulator
echo "Make" %product_name%

.\arduino-cli compile -b arduino:samd:nano_33_iot --output-dir .\out --libraries .\%product_name%\libraries %product_name%

if errorlevel 1 (
   if [%1]==[] echo Compiler Error: %errorlevel%
   set /p exitkey= "Press any key to continue..."
   exit /b %errorlevel%
)

exit /b 0
