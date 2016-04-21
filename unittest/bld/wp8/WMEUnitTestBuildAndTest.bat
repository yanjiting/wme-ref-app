@echo off
rem *************************************************************************************************
rem   usage: DolphinUTBuildAndTest.bat  Action-BuildOption
rem          Action:
rem             Clean-All     clean  WMEUnitTest solution (Debug and Release)
rem             Clean-Debug   clean  WMEUnitTest solution (Debug)
rem             Clean-Release clean  WMEUnitTest solution (Release)
rem             Build-All     build  WMEUnitTest solution (Debug and Release)
rem             Build-Debug   build  WMEUnitTest solution (Debug)
rem             Build-Release build  WMEUnitTest solution (Release)
rem             Test-All      launch WMEUnitTest App and get test result(Debug and Release)
rem             Test-Debug    launch WMEUnitTest App and get test result(Debug)
rem             Test-Release  launch WMEUnitTest App and get test result(Release)
rem
rem             Build-SDK-All     build MediaEngine SDK solution (Debug and Release)
rem             Build-SDK-Debug   build MediaEngine SDK solution (Debug)
rem             Build-SDK-Release build MediaEngine SDK solution (Release)
rem             Help          show usage info
rem   2015/02/12 huashi@cisco.com
rem *************************************************************************************************

rem default setting
setlocal EnableDelayedExpansion
set CurrentWorkingDir=%cd%
set Platform=ARM
set aConfigurationList= Debug Release
set Action=Test-All

rem basic setting 
call :BasicSetting
call :EnvironmentPathSetting
call :BasicCheck
if not %ERRORLEVEL%==0 (
		goto :ErrorReturn
)

call :SetAction %1
if not %ERRORLEVEL%==0 (
		echo not suppot option!
		goto :ErrorReturn
)

call :TestResultInitial
call :Action
if "%Action%" == "Help" (
	goto :EOF
)
call :GetFinalTestReturnCode
call :OutputResult
echo  ReturnCode is %ReturnCode%
goto :EOF

rem function for setting 
rem **********************************************************************************************************
:BasicSetting
	Set MediaEngineSolution=MediaSDKClient.sln
	set WMEUnitTestSolution=WMEUnitTest.sln
	set WMEUnitTestWorkingDir=%CurrentWorkingDir%
goto :EOF

:EnvironmentPathSetting
	rem VC2013 setting
	set VC2013Path=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE
	set VC2013BinPath=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin
	set MSBuildPath=C:\Program Files (x86)\MSBuild\12.0\Bin
	rem tools setting
	set ISEToolPath=C:\Program Files (x86)\Microsoft SDKs\Windows Phone\v8.1\Tools\IsolatedStorageExplorerTool
	set APPDeployToolPath=C:\Program Files (x86)\Microsoft SDKs\Windows Phone\v8.1\Tools\AppDeploy
	rem set environment path
	set PATH=%PATH%;%VC2013Path%;%ISEToolPath%;%APPDeployToolPath%;%VC2013BinPath%;%MSBuildPath%
goto :EOF

:SDKSetting
	rem relative path setting
	set MediaEngineRelativePath=..\..\..\mediaengine\bld
	set MediaEngineBinRelativePath=..\..\..\mediaengine\bin
	set MediaEngineLibsRelativePath=..\..\..\mediaengine\libs

	cd  %CurrentWorkingDir%
	cd  %MediaEngineRelativePath%
	set MediaEngineWorkingDir=%cd%\WP8

	cd  %CurrentWorkingDir%
	cd  %MediaEngineBinRelativePath%
	set MediaEngineBinDir=%cd%\WP8
	
	cd  %CurrentWorkingDir%	
	cd  %MediaEngineLibsRelativePath%
	set MediaEngineLibsDir=%cd%\WP8
	cd  %CurrentWorkingDir%
	
	echo ********************************************************
	echo       setting for SDK
	echo ********************************************************	
	echo CurrentWorkingDir     is %CurrentWorkingDir%
	echo MediaEngineWorkingDir is %MediaEngineWorkingDir%
	echo MediaEngineBinDir     is %MediaEngineBinDir%
	echo MediaEngineLibsDir    is %MediaEngineLibsDir%
goto :EOF

:DolphinTestSetting
	set vConfiguration=%1
	set vPlatform=%2
	
	set TestAppName=DolphinTestApp
	rem product ID can be found under APP project's property->WMAppManifest.xml->packaging
	set APPProductID=202a6815-ffd9-4a6f-a4e2-085198d360e7
	set APPPackageFileName=%TestAppName%_%vConfiguration%_%vPlatform%.xap	
	set AppRelativePath=..\..\dolphin\bld\wp8\DolphinTestApp
	
	set XMLFileName=dolphin_unittest_main.xml
	rem set TraceFileName=DolphinTest_Trace.log

	rem time in millisecond for ut runtime setting
	set MaxUTRunTime=30000
goto :EOF

:SharkTestSetting
	set vConfiguration=%1
	set vPlatform=%2
	set TestAppName=SharkTestApp
	rem product ID can be found under APP project's property->WMAppManifest.xml->packaging
	set APPProductID=08d47b9f-84de-4451-b330-701f9da16aa6
	set APPPackageFileName=%TestAppName%_%vConfiguration%_%vPlatform%.xap
	set AppRelativePath=..\..\shark\bld\wp8\SharkTestApp

	rem xml file for test result
	set XMLFileName=shark_unittest_main.xml
	rem set TraceFileName=SharkTest_Trace.log

	rem time in millisecond for ut runtime setting
	set MaxUTRunTime=40000	
goto :EOF


rem add more test app setting below used template like DolphinTestSetting
rem *********************

rem *********************
:CommonAppPostSetting

	cd  %CurrentWorkingDir%
	cd  %AppRelativePath%
	set AppWorkingDir=%cd%
	cd  %CurrentWorkingDir%

	cd  %AppWorkingDir%\Bin\%vPlatform%\%vConfiguration%
	set APPPackageDir=%cd%
	cd  %CurrentWorkingDir%
	set APPPackageFile=%APPPackageDir%\%APPPackageFileName%
	
	rem UT trace log file and test result xml file setting
	set UTResultDir=%CurrentWorkingDir%\XML_%vConfiguration%_%TestAppName%
	set TempXMLDir=%UTResultDir%\IsolatedStore
	set XMLFileRelativePath=IsolatedStore\Shared
	set XMLFilePath=%UTResultDir%\%XMLFileRelativePath%\%XMLFileName%
	set UTResultFile=%UTResultDir%\%XMLFileName%

	set UTTraceDir=%CurrentWorkingDir%\Trace_%vConfiguration%_%TestAppName%
	set TempTraceDir=%UTTraceDir%\IsolatedStore
	set TraceFileRelativePath=IsolatedStore\Shared\logs
	rem set TraeFilePath=%UTTraceDir%\%TraceFileRelativePath%\%TraceFileName%
	set TraeFilePath=%UTTraceDir%\%TraceFileRelativePath%
	
	rem the same function with sleep command on linux
	set PingIP=192.0.2.2
goto :EOF

:OutputAppSetting	
	echo ********************************************************
	echo       setting for %TestAppName%
	echo ********************************************************
	echo AppWorkingDir        is %AppWorkingDir%
	echo APPPackageFileName   is %APPPackageFileName%
	echo APPPackageFile       is %APPPackageFile%
	echo UT maximum runtime   is %MaxUTRunTime%(ms)
	echo UTResultDir          is %UTResultDir%
	echo Test result xml file is %UTResultFile%
	echo UTTraceDir           is %UTTraceDir%
	echo Test trace file      is %TraeFilePath%
	echo ********************************************************
goto :EOF

rem function for setting 
rem **********************************************************************************************************
:BasicCheck
	if not exist  "%VC2013Path%" (
		echo VC2013 has not been installed yet, please prepare your test environment first!	
		goto :ErrorReturn
	)
goto :EOF

rem function for help 
rem **********************************************************************************************************
:help
	echo *******************************************************************************
	echo ********************  help   **************************************************
	echo   usage: DolphinUTBuildAndTest.bat  Action-BuildOption
	echo          Action:
	echo             Clean-All     clean  WMEUnitTest solution (Debug and Release)
	echo             Clean-Debug   clean  WMEUnitTest solution (Debug)
	echo             Clean-Release clean  WMEUnitTest solution (Release)
	echo             Build-All     build  WMEUnitTest solution (Debug and Release)
	echo             Build-Debug   build  WMEUnitTest solution (Debug)
	echo             Build-Release build  WMEUnitTest solution (Release)
	echo             Test-All      launch WMEUnitTest App and get test result(Debug and Release)
	echo             Test-Debug    launch WMEUnitTest App and get test result(Debug)
	echo             Test-Release  launch WMEUnitTest App and get test result(Release)
	echo
	echo             Build-SDK-All     build MediaEngine SDK solution (Debug and Release)
	echo             Build-SDK-Debug   build MediaEngine SDK solution (Debug)
	echo             Build-SDK-Release build MediaEngine SDK solution (Release)
	echo             Help          show usage info
	echo ******************************************************************************
	echo ******************************************************************************
goto :EOF

rem  function for option
rem **********************************************************************************************************
:SetAction
	rem default setting
	if "aaa%1"=="aaa" (
		set aConfigurationList=Debug Release
		set Action=Test
	) else if "%1"=="Clean-All" (
		set aConfigurationList=Debug Release
		set Action=Clean
	) else if "%1"=="Clean-Debug" (
		set aConfigurationList=Debug
		set Action=Clean
	)  else if "%1"=="Clean-Release" (
		set aConfigurationList=Release
		set Action=Clean
	) else if "%1"=="Build-All" (
		set aConfigurationList=Debug Release
		set Action=Build
	) else if "%1"=="Build-Debug" (
		set aConfigurationList=Debug
		set Action=Build
	) else if "%1"=="Build-Release" (
		set aConfigurationList=Release
		set Action=Build
	) else if "%1"=="Test-All" (
		set aConfigurationList=Debug Release
		set Action=Test
	) else if "%1"=="Test-Debug" (
		set aConfigurationList=Debug
		set Action=Test
	) else if "%1"=="Test-Release" (
		set aConfigurationList=Release
		set Action=Test
	) else if "%1"=="Build-SDK-All" (
		set aConfigurationList=Debug Release
		set Action=BuildSDK
	) else if "%1"=="Build-SDK-Debug" (
		set aConfigurationList=Debug
		set Action=BuildSDK
	)  else if "%1"=="Build-SDK-Release" (
		set aConfigurationList=Release
		set Action=BuildSDK
	) else if "%1"=="Help" (
		set aConfigurationList=Debug Release
		set Action=Help
	)else (
		call :help
		goto :ErrorReturn
	)
goto :EOF

rem function for test action
rem **********************************************************************************************************
:Action
	echo aConfigurationList is %aConfigurationList%
	echo Action is %Action%
	set  ReturnCode=0
	set TestDebugInfo=Test summary
	set TestReleaseInfo=Test summary
	set TestCasesStatusInfo=
	set AllTestCasesInfo=

	for %%j in ( %aConfigurationList% ) do (
		echo  Configuration is %%j

		if "%Action%" == "Clean" (
			echo  Clean  action --%%j
			call :Action-Clean %%j
		)
		if "%Action%" == "Build" (
			echo  Build action --%%j
			call :Action-Build %%j
		)
		if "%Action%" == "Test" (
			echo  Test action --%%j
			call :Action-Test %%j
		)
		if "%Action%" == "BuildSDK" (
			echo  BuildSDK action --%%j
			call :Action-BuildSDK %%j
		)
		if "%Action%" == "Help" (
			echo  Help
			call :Help
		)
	)
goto :EOF

:Action-Clean
	set ActionConfiguration=%1
	set CleanFlag=0
	call :Clean  "%WMEUnitTestWorkingDir%\%WMEUnitTestSolution%"  %ActionConfiguration%  %Platform%
	call :CleanCheck
	
goto :EOF

:CleanCheck
	if not %CleanFlag%==0 (
		if "%ActionConfiguration%"=="Debug" (
			set CleanDebugFlag=1
			set CleanDebugInfo="clean debug--failed"
		) else (
			set CleanReleaseFlag=1
			set CleanReleaseInfo="clean release--failed"
		)
	) else (
		if "%ActionConfiguration%"=="Debug" (
			set CleanDebugFlag=0
			set CleanDebugInfo="clean debug--succeed"
		) else (
			set CleanReleaseFlag=0
			set CleanReleaseInfo="clean release--succeed"
		)	
	)
goto :EOF

:Action-Build
	set CleanFlag=0
	set BuildFlag=0 
	call :Action-Clean %1

	set ActionConfiguration=%1
	call :Build  "%WMEUnitTestWorkingDir%\%WMEUnitTestSolution%"  %ActionConfiguration%  %Platform%
    call :BuildCheck
goto :EOF

:Action-BuildSDK
	set ActionConfiguration=%1
	set CleanFlag=0
	set BuildFlag=0 
	call :SDKSetting
	call :Clean  "%MediaEngineWorkingDir%\%MediaEngineSolution%"  %ActionConfiguration%  %Platform%
	call :Build  "%MediaEngineWorkingDir%\%MediaEngineSolution%"  %ActionConfiguration%  %Platform%
	call :CleanCheck
    call :BuildCheck
goto :EOF

:BuildCheck
	if not %BuildFlag%==0 (
		if "%ActionConfiguration%"=="Debug" (
			set BuildDebugFlag=1
			set BuildDebugInfo="build debug--failed"
		) else (
			set BuildReleaseFlag=1
			set BuildReleaseInfo="build release--failed"
		)
	) else (
		if "%ActionConfiguration%"=="Debug" (
			set BuildDebugFlag=0
			set BuildDebugInfo="build debug--succeed"
		) else (
			set BuildReleaseFlag=0
			set BuildReleaseInfo="build release--succeed"
		)
	)
goto :EOF

:Action-Test	
	rem call :Action-Build %1
	set ActionConfiguration=%1

	rem APP setting based on configuration
	call :DolphinTestSetting %ActionConfiguration%  %Platform%
	call :CommonAppPostSetting
	call :OutputAppSetting
	set TestFlag=0
	call :Test  %ActionConfiguration%  %Platform%
	call :TestCasesStatusCheck
	call :TestSummary

	rem shark test
	call :SharkTestSetting %ActionConfiguration%  %Platform%
	call :CommonAppPostSetting
	call :OutputAppSetting
	set TestFlag=0
    call :Test  %ActionConfiguration%  %Platform%
	call :TestCasesStatusCheck
	call :TestSummary

goto :EOF

:TestSummary
	if not %TestFlag%==0 (
		if "%ActionConfiguration%"=="Debug" (
			set TestDebugFlag=1
			set TestDebugInfo=%TestDebugInfo%----%TestAppName%--test--debug--failed--%TestFailedLog%
		) else (
			set TestReleaseFlag=1
			set TestReleaseInfo=%TestReleaseInfo%----%TestAppName%--test--release--failed--%TestFailedLog%
		) 
	) else (
		if "%ActionConfiguration%"=="Debug" (
			set TestDebugInfo=%TestDebugInfo%----%TestAppName%--test--debug--succeed
		) else (
			set TestReleaseInfo=%TestReleaseInfo%----%TestAppName%--test--release--succeed
		)
	)
	set AllTestCasesInfo=!AllTestCasesInfo!----------!TestAppName!--!ActionConfiguration!--!TestCasesStatusInfo!

goto :EOF

:TestCasesStatusCheck
	set /a iLineIndex=0
    for /f  "delims=" %%i in (%UTResultFile%) do (
	    set /a iLineIndex = !iLineIndex! + 1
		if !iLineIndex! EQU 2 (
			set TestCasesStatusInfo=%%i
		    call :TestCasesStatusCheck
		)
	)
goto :EOF

:TestCasesStatusCheck
	set PassPatern=failures="0"
	for /f "tokens=3 delims= " %%j in ("!TestCasesStatusInfo!") do (
		if not "%%j" == "!PassPatern!" (
			set TestFlag=1
			set TestFailedLog=Not all cases passed!
			echo not all cases pase the test
		)
	)
goto :EOF

rem function for test result 
rem **********************************************************************************************************
:TestResultInitial

	set CleanDebugFlag=0
	set CleanReleaseFlag=0
	set BuildDebugFlag=0
	set BuildReleaseFlag=0
	set TestDebugFlag=0 
	set TestReleaseFlag=0
	
	set CleanDebugInfo=NULL
	set CleanReleaseInfo=NULL
	set BuildDebugInfo=NULL
	set BuildReleaseInfo=NULL
	set TestDebugInfo=NULL
	set TestReleaseInfo=NULL

	Set TestCasesStatusInfo=NULL
goto :EOF

:GetFinalTestReturnCode
	set aActionFlagList=%CleanDebugFlag%  %CleanReleaseFlag% %BuildDebugFlag%  %BuildReleaseFlag% %TestDebugFlag%   %TestReleaseFlag%
	echo  aActionFlagList is %aActionFlagList%
	set ReturnCode=0
	for %%k in (%aActionFlagList%) do  (
		if not %%k == 0 (
			set ReturnCode=1
		)	
	)
goto :EOF


:OutputResult
	echo **********************************************************
	echo        %Action% summary list as below:
	echo **********************************************************
	echo CleanDebugFlag   is %CleanDebugFlag%
	echo CleanReleaseFlag is %CleanReleaseFlag%
	echo BuildDebugFlag   is %BuildDebugFlag%
	echo BuildReleaseFlag is %BuildReleaseFlag%
	echo TestDebugFlag    is %TestDebugFlag%
	echo TestReleaseFlag  is %TestReleaseFlag%
	
	echo CleanDebugInfo   is %CleanDebugInfo%
	echo CleanReleaseInfo is %CleanReleaseInfo%
	echo BuildDebugInfo   is %BuildDebugInfo%
	echo BuildReleaseInfo is %BuildReleaseInfo%
	echo TestDebugInfo    is %TestDebugInfo%
	echo TestReleaseInfo  is %TestReleaseInfo%
	echo **********************************************************
	if "%Action%" == "Test" (
		echo     Test cases pass status info list as below:
		echo **********************************************************
		echo !AllTestCasesInfo!
		echo **********************************************************
	)
goto :EOF

rem function for action
rem **********************************************************************************************************
:Clean
	set vSolutionFile=%1
	set vConfiguration=%2
	set vPlatform=%3
	set vAction=clean
	echo devenv  %vSolutionFile%  /%vAction%   "%vConfiguration%|%vPlatform%"
	devenv  %vSolutionFile%  /%vAction%   "%vConfiguration%|%vPlatform%"
	if not %ERRORLEVEL%==0 (
	 	set CleanFlag=1
	)
goto :EOF

:Build
	set vSolutionFile=%1
	set vConfiguration=%2
	set vPlatform=%3
	set vAction=build
    echo devenv  %vSolutionFile%  /%vAction%   "%vConfiguration%|%vPlatform%"
    devenv  %vSolutionFile%  /%vAction%   "%vConfiguration%|%vPlatform%"
	if not %ERRORLEVEL%==0 (
	 	set BuildFlag=1
	)
goto :EOF

:Test
	set vConfiguration=%1
	set vPlatform=%2

	rem remove previous APP, install latest App and run UT
	echo Removing previous App and install/launch latest UT App....
	AppDeployCmd   /uninstall %APPProductID% /targetdevice:de
	AppDeployCmd   /installlaunch %APPPackageFile% /targetdevice:de
	if not %ERRORLEVEL%==0 (
	 	set TestFlag=1
		set TestFailedLog=launch APP  and run UT failed
		echo %TestFailedLog%
		echo APPPackageFileName %APPPackageFile%
		goto :ErrorReturn
	)
	
	rem remove previous xml file for test result
	if exist %UTResultDir%  rd /s /q %UTResultDir%
	if exist %UTTraceDir%   rd /s /q %UTTraceDir%
	md %UTResultDir%
	md %UTTraceDir%

	rem waiting for UT test
	echo waiting for all cases in UT .....
	ping %PingIP%  -n 1  -w %MaxUTRunTime% >nul
	
	rem copy xml file from device
	ISETool ts de %APPProductID% %UTResultDir%
	copy %XMLFilePath%  %UTResultDir%
	if not %ERRORLEVEL%==0 (
        set TestFlag=1
		set TestFailedLog=copy UT output xml file failed
		goto :ErrorReturn
	)
	ISETool ts de %APPProductID% %UTTraceDir%
	copy %TraeFilePath%\*  %UTTraceDir%
	if not %ERRORLEVEL%==0 (
		set TestFlag=1
		set TestFailedLog=copy UT trace file failed
		goto :ErrorReturn
	)

	dir %TempXMLDir%
	dir %TempTraceDir%
	echo TempXMLDir   is %TempXMLDir%
	echo TempTraceDir is %TempTraceDir%
	if exist %TempXMLDir%   rd /s /q  %TempXMLDir%
	if exist %TempTraceDir% rd /s /q  %TempTraceDir%

	rem remove APP after test
	echo "remove APP after test......"
	AppDeployCmd   /uninstall %APPProductID% /targetdevice:de
	
goto :EOF

:ErrorReturn
endlocal
exit /b 2

:End
endlocal
exit /b %ReturnCode%
