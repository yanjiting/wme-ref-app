setlocal enabledelayedexpansion
@echo off

set /a retValue=0
set VS2008="C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE"
if not exist "C:\Program Files (x86)" (
	set VS2008="C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE"
)

set PATH=%PATH%;!VS2008!

set CURPATH=.
set WMEROOT=!CURPATH!\..\..\..
set MediaEngineTestProj=!WMEROOT!\unittest\bld\windows\MediaEngineTest.sln

set build_log=!CURPATH!\build_ut_log.txt

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

set GTestLibsDir=!WMEROOT!\vendor\gtest\libs
set GTestBldDir=!WMEROOT!\vendor\gtest\gtest-1.7.0\msvc2008
set GTestProj=%GTestBldDir%\gtest-md.sln
set GMockBldDir=!WMEROOT!\vendor\gtest\gmock-1.7.0\msvc\2008
set GMockProj=%GMockBldDir%\gmock.sln

if not exist %GTestLibsDir% mkdir %GTestLibsDir%

for %%j in (!configuration!) do (
	if not exist %GTestLibsDir%\%%j mkdir %GTestLibsDir%\%%j
	if "%%j"=="debug" (
		if NOT exist "%GTestLibsDir%\%%j\gtestd.lib" (
			call :Build_project !action! %GTestProj% "%%j|Win32" !build_log!
			copy %GTestBldDir%\gtest-md\%%j\gtestd.lib %GTestLibsDir%\%%j
		)
	) else (
		if NOT exist "%GTestLibsDir%\%%j\gtest.lib" (	
			call :Build_project !action! %GTestProj% "%%j|Win32" !build_log!
			copy %GTestBldDir%\gtest-md\%%j\gtest.lib %GTestLibsDir%\%%j
		)			
	)

	if NOT exist "%GTestLibsDir%\%%j\gmock.lib" (
		call :Build_project !action! %GMockProj% "%%j|Win32" !build_log!
		copy %GMockBldDir%\%%j\gmock.lib %GTestLibsDir%\%%j
	)
	
:: build ut projects	
	call :Build_project !action! !MediaEngineTestProj! "%%j|Win32" !build_log!	
	
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
devenv %2 /%1 %3 /Out %4
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



