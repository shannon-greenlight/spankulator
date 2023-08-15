@echo off

set new_make=true
set build_path=.\out
set verbose=
rem set verbose=-verbose
set action=-compile

rem tool_root C:\Program Files\WindowsApps
set tool_root=.
set tool_dir=arduino_bin
set data_dir=C:\Users\shann\OneDrive\Documents\ArduinoData\packages
set code_dir=C:\Users\shann\Dropbox\moog\spankulator\Spankulator-Code\Spankulator
set library_dir=C:\Users\shann\Dropbox\moog\Arduino-Libs
set cache_dir=C:\Users\shann\AppData\Local\Temp\arduino_cache_aug2033
set secret_dir=%code_dir%\secrets

rem echo Arg: %1

if %new_make%==true (
	echo "New Make" %cache_dir%


	"%tool_root%\%tool_dir%\arduino-builder" %action% -logger=machine -hardware %tool_root%\%tool_dir%\hardware -hardware %data_dir% -tools %tool_root%\%tool_dir%\tools-builder -tools %tool_root%\%tool_dir%\hardware\tools\avr -tools %data_dir% -built-in-libraries %tool_root%\%tool_dir%\libraries -libraries %library_dir% -libraries %secret_dir% -fqbn=arduino:samd:nano_33_iot -vid-pid=2341_8057 -ide-version=10816 -build-path .\out -warnings=default -build-cache %cache_dir% -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.arm-none-eabi-gcc.path=%data_dir%\arduino\tools\arm-none-eabi-gcc\7-2017q4 -prefs=runtime.tools.arm-none-eabi-gcc-7-2017q4.path=%data_dir%\arduino\tools\arm-none-eabi-gcc\7-2017q4 -prefs=runtime.tools.CMSIS.path=%data_dir%\arduino\tools\CMSIS\4.5.0 -prefs=runtime.tools.CMSIS-4.5.0.path=%data_dir%\arduino\tools\CMSIS\4.5.0 -prefs=runtime.tools.openocd.path=%data_dir%\arduino\tools\openocd\0.10.0-arduino7 -prefs=runtime.tools.openocd-0.10.0-arduino7.path=%data_dir%\arduino\tools\openocd\0.10.0-arduino7 -prefs=runtime.tools.bossac.path=%data_dir%\arduino\tools\bossac\1.7.0-arduino3 -prefs=runtime.tools.bossac-1.7.0-arduino3.path=%data_dir%\arduino\tools\bossac\1.7.0-arduino3 -prefs=runtime.tools.arduinoOTA.path=%data_dir%\arduino\tools\arduinoOTA\1.2.1 -prefs=runtime.tools.arduinoOTA-1.2.1.path=%data_dir%\arduino\tools\arduinoOTA\1.2.1 -prefs=runtime.tools.CMSIS-Atmel.path=%data_dir%\arduino\tools\CMSIS-Atmel\1.2.0 -prefs=runtime.tools.CMSIS-Atmel-1.2.0.path=%data_dir%\arduino\tools\CMSIS-Atmel\1.2.0 %verbose% %code_dir%\Spankulator.ino
)
if %new_make%==false  (
	echo "Old Make"
	"C:\Program Files (x86)\Arduino\arduino-builder" -compile -logger=machine -hardware "C:\Program Files (x86)\Arduino\hardware" -hardware C:\Users\Shannon\AppData\Local\Arduino15\packages -tools "C:\Program Files (x86)\Arduino\tools-builder" -tools "C:\Program Files (x86)\Arduino\hardware\tools\avr" -tools C:\Users\Shannon\AppData\Local\Arduino15\packages -built-in-libraries "C:\Program Files (x86)\Arduino\libraries" -libraries .\Spankulator\greenface_libs -libraries C:\Users\Shannon\Documents\Arduino\libraries -fqbn=arduino:samd:nano_33_iot -vid-pid=0X2341_0X8057 -ide-version=10810 -build-path %build_path% -warnings=default -build-cache C:\Users\Shannon\AppData\Local\Temp\arduino_cache_217360873_bluemaxumu -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.openocd.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\openocd\0.10.0-arduino7 -prefs=runtime.tools.openocd-0.10.0-arduino7.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\openocd\0.10.0-arduino7 -prefs=runtime.tools.CMSIS-Atmel.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\CMSIS-Atmel\1.2.0 -prefs=runtime.tools.CMSIS-Atmel-1.2.0.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\CMSIS-Atmel\1.2.0 -prefs=runtime.tools.arm-none-eabi-gcc.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\7-2017q4 -prefs=runtime.tools.arm-none-eabi-gcc-7-2017q4.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\7-2017q4 -prefs=runtime.tools.arduinoOTA.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\arduinoOTA\1.2.1 -prefs=runtime.tools.arduinoOTA-1.2.1.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\arduinoOTA\1.2.1 -prefs=runtime.tools.CMSIS.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\CMSIS\4.5.0 -prefs=runtime.tools.CMSIS-4.5.0.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\CMSIS\4.5.0 -prefs=runtime.tools.openocd.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\openocd\0.10.0-arduino7 -prefs=runtime.tools.openocd-0.10.0-arduino7.path=C:\Users\Shannon\AppData\Local\Arduino15\packages\arduino\tools\openocd\0.10.0-arduino7 %verbose% Spankulator\Spankulator.ino
)	

if errorlevel 1 (
   if [%1]==[] echo Compiler Error: %errorlevel%
   exit /b %errorlevel%
)
rem set /p exitkey= "Press any key to continue..."
exit /b 0