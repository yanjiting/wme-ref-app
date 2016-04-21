setlocal enabledelayedexpansion
@echo off

set /a retValue=0
set VS2008="C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE"
if not exist "C:\Program Files (x86)" (
	set VS2008="C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE"
)

set PATH=%PATH%;!VS2008!

set CURPATH=.
set WMEROOT=!CURPATH!\..\..
set EngineDemoProj=!WMEROOT!\ref-app\windows\EngineDemo\bld\EngineDemo.sln

set build_log=!CURPATH!\build_refapp_log.txt

if exist !build_log! del !build_log!

:: check the parameter
:: debug - only build the debug version
:: release - only build the release version
:: clean - clean the solution, only clean the UT projects, not clean gtest and gmock
:: null - if no parameter, will build both debug and release version
set configuration=null
set action=null

if "aaa%1"=="aaa" (
	set configuration=debug release
	set action=build
)

if "%1"=="debug" (
	set configuration=debug
	set action=build
)

if "%1"=="release" (
	set configuration=release
	set action=build
)

if "%1"=="clean" (
	set configuration=release debug
	set action=clean

	for %%j in (!configuration!) do (
		call :Build_project !action! !MediaEngineTestProj! "%%j|Win32" !build_log!
	)
	goto :end
)

:: if the parameter is wrong, output the error messages
if "!configuration!"=="null" (
echo Unsupported parameter! >> !build_log!
echo Parameter can only be debug/release/clean or not use parameter >> !build_log!
echo Stop build!>> !build_log!
echo Unsupported parameter!
echo Parameter can only be debug/release/clean or not use parameter
echo Stop build!
::@pause
goto :fail
)

for %%j in (!configuration!) do (
	
	call :Build_project !action! !EngineDemoProj! "%%j|Win32" !build_log!	
	
 	if ERRORLEVEL 1 (
		set /a retValue=1
	)
)

if !retValue! EQU 0 (
	goto :end
) else (
	goto :fail_with_log
)

:Build_project
setlocal
echo %1 %2 %3
echo on
devenv %2 /%1 %3
@echo off
if not %ERRORLEVEL%==0 (
		echo failed to %1 %2
		goto :fail
) 
echo completed %1 %2
goto :eof


:fail_with_log
echo failed, please check the log...
:fail
endlocal
exit /b 1

:end 
endlocal
goto :eof