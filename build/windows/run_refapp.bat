@echo off
setlocal enabledelayedexpansion
set CurrentPath=%cd%
if "%1aaa"=="aaa" (
set ConfigurationName=release
) else (
set ConfigurationName=%1
)

if /i not "!ConfigurationName!"=="debug" (
	if /i not "!ConfigurationName!"=="release" (
		echo Parameter can only be debug or release, or run this script without parameter!
		goto :end
	)
)
set RefAppBinDir=..\..\ref-app\windows\EngineDemo\bin\!ConfigurationName!
copy ..\..\mediaengine\bin\win32\!ConfigurationName!\*.dll !RefAppBinDir!
copy ..\..\vendor\openh264\libs\win32\!ConfigurationName!\*.dll !RefAppBinDir!

cd !RefAppBinDir!
set exefile=EngineDemo.exe

call !exefile!
if %ERRORLEVEL%==0 (
	echo !exefile!, OK!
	goto :end	
) else (
	echo failed to execute !exefile!
	goto :failed
)

goto :end

:end 
cd %CurrentPath%
endlocal
goto :eof

:failed
cd %CurrentPath%
endlocal
exit /b 1