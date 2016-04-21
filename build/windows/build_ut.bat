@echo off
setlocal
set CurrentPath=%cd%
cd ..\..\unittest\bld\windows
call build_ut_vs2008.bat %1
set retValue=%errorlevel%
cd %CurrentPath%
if %retValue%==0 (
	echo sucessfull to build UT of mediaengine!
	goto :success
) else (
	echo failed to build UT of mediaengine!
	goto :failed
)

:success
endlocal & goto :eof

:failed
endlocal
exit /b 1
