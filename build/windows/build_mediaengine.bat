@echo off
setlocal
set CurrentPath=%cd%
cd ..\..\mediaengine\bld\windows
call build_MediaEngine_vs2008.bat %1
set retValue=%errorlevel%
cd %CurrentPath%

if %retValue%==0 (
	echo sucessfull to build mediaengine!
	goto :success
) else (
	echo failed to build mediaengine!
	goto :failed
)

:success
endlocal & goto :eof

:failed
endlocal
exit /b 1
