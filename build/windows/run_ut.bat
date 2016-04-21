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
 
copy ..\..\mediaengine\bin\win32\!ConfigurationName!\*.dll ..\..\unittest\bin\win32\!ConfigurationName!
copy ..\..\vendor\openh264\libs\win32\!ConfigurationName!\*.dll ..\..\unittest\bin\win32\!ConfigurationName!
 
cd ..\..\unittest\bin\win32\!ConfigurationName!
set ExeFiles=DolphinTest.exe SharkTest.exe
:: tpTest.exe utilTest.exe WmeTest.exe wqosTest.exe wrtpTest.exe
set /a failedNum=0
set /a totalNum=0
set /a successNum=0
set passedUTName=
set failedUTName=
::
:: dolphintest
::
set exefile=DolphinTest.exe
set resultfile=wbxaudioengineTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: SharkTest.exe
::
set exefile=SharkTest.exe
set resultfile=SharkSDKTest.xml
call !exefile!
echo SharkTest ERRORLEVEL=%ERRORLEVEL%
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: tpTest.exe
::
set exefile=tpTest.exe
set resultfile=tpTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: utilTest.exe
::
set exefile=utilTest.exe
set resultfile=utilTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: WmeTest.exe
::
set exefile=WmeTest.exe
set resultfile=WmeTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: wqosTest.exe
::
set exefile=wqosTest.exe
set resultfile=wqosTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

::
:: wrtpTest.exe
::
set exefile=wrtpTest.exe
set resultfile=wrtpTest_windows.xml
call !exefile!
if %ERRORLEVEL%==0 (
	echo execute !exefile!, all cases are passed!
	set passedUTName=!passedUTName!,!exefile!
	set /a successNum=!successNum!+1
) else (
	echo execute !exefile!, some cases are failed!
	set /a failedNum=!failedNum!+1
	set failedUTName=!failedUTName!,!exefile!
)
set /a totalNum=!totalNum!+1

echo Finish to run !totalNum! unit tests!

echo !successNum! unit tests are passed:!passedUTName!

if !failedNum! GTR 0 (

	echo !failedNum! unit tests are failed:!failedUTName!
	goto :failed
) else (
	echo No unit test is failed!
	goto :end
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