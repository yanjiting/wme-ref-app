@echo off
rem *************************************************************************************************
rem   usage: DolphinUTBuildAndTest.bat  Action-BuildOption
rem          Action:
rem             Clean-All     clean DolphinTest solution (Debug and Release)
rem             Clean-Debug   clean DolphinTest solution (Debug)
rem             Clean-Release clean DolphinTest solution (Release)
rem             Build-All     build MediaEngine SDK solution (Debug and Release)
rem             Build-Debug   build MediaEngine SDK solution (Debug)
rem             Build-Release build MediaEngine SDK solution (Release)
rem             Test-All      build  DolphinTest solution (Debug and Release)
rem                           launch DophinTest App and get test result(Debug and Release)
rem             Test-Debug    build  DolphinTest solution (Debug)
rem                           launch DophinTest App and get test result(Debug)
rem             Test-Release  build  DolphinTest solution (Release)
rem                           launch DophinTest App and get test result(Release)
rem             Help          show usage info
rem   2015/02/12 huashi@cisco.com
rem *************************************************************************************************

rem default setting
setlocal EnableDelayedExpansion
set CurrentWorkingDir=%cd%
set Platform=ARM
set Configuration=Debug
set aConfigurationList= Debug Release
set Action=Test-All
rem time in millisecond
set MaxUTRunTime=30000

rem basic setting 
call :BasicSetting
call :EnvironmentPathSetting
call :BasicPathSetting
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
call :PathInfoOutput
call :OutputResult
echo  ReturnCode is %ReturnCode%
goto :EOF

rem function for setting 
rem **********************************************************************************************************
:BasicSetting
	rem product ID can be found under APP project's property->WMAppManifest.xml->packaging
	set APPProductID=202a6815-ffd9-4a6f-a4e2-085198d360e7
	Set MediaEngineSolution=MediaSDKClient.sln
	set DolphinTestSolution=DolphinTest.sln
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

:BasicPathSetting
	rem relative path setting
	set MediaEngineRelativePath=..\..\..\..\mediaengine\bld\WP8
	set SDKBinDirRelativePath=..\..\..\..\mediaengine\bin\WP8
	set DolpinAppRelativePath=.\DolphinTestApp
	set DolphinTestWorkingDir=%CurrentWorkingDir%
	cd  %CurrentWorkingDir%
	cd  %MediaEngineRelativePath%
	set MediaEngineWorkingDir=%cd%
	cd  %CurrentWorkingDir%
	cd  %DolpinAppRelativePath%
	set DolpinAppWorkingDir=%cd%
	cd  %CurrentWorkingDir%
	cd  %SDKBinDirRelativePath%
	set SDKBinDir=%cd%
	cd  %CurrentWorkingDir%
	echo CurrentWorkingDir is %CurrentWorkingDir%
	echo MediaEngineWorkingDir is %MediaEngineWorkingDir%
	echo DolpinAppWorkingDir is %DolpinAppWorkingDir%
goto :EOF

:APPSetting
	rem APP setting
	set vConfiguration=%1
	set vPlatform=%2
	set APPPackageFileName=DolphinTestApp_%vConfiguration%_%vPlatform%.xap
	cd  %CurrentWorkingDir%
	cd  %DolpinAppWorkingDir%\Bin\%vPlatform%\%vConfiguration%
	set APPPackageDir=%cd%
	set APPPackageFile=%APPPackageDir%\%APPPackageFileName%
	echo APPPackageFile is %APPPackageFile%
	cd  %CurrentWorkingDir%
	
	rem xml file for test result
	set UTResultDir=%CurrentWorkingDir%\XML_%vConfiguration%
	set XMLFileRelativePath=IsolatedStore\Shared
	set TemDirName=IsolatedStore
	set TemDir=%UTResultDir%\%TemDirName%
	set XMLFileName=dolphin_unittest_main.xml
	set XMLFilePath=%UTResultDir%\%XMLFileRelativePath%\%XMLFileName%
	
	rem the same function with sleep command on linux
	set PingIP=192.0.2.2
goto :EOF


:PathInfoOutput
	echo ************************************************************
	echo               test check!
	echo ************************************************************
	echo CurrentWorkingDir is %CurrentWorkingDir%
	echo MediaEngineWorkingDir is %MediaEngineWorkingDir%
	echo MediaEngine SDK bin dir is %SDKBinDir%
    echo DolphinTestWorkingDir is %DolphinTestWorkingDir%
    echo DolpinAppWorkingDir is %DolpinAppWorkingDir%
	echo APPPackageDir is %APPPackageDir%
	echo APPPackageFile is %APPPackageFile%
    echo XMLFilePath is %XMLFilePath%
	set UTResultDir=%CurrentWorkingDir%\XML_Debug
	dir %UTResultDir%
	set UTResultDir=%CurrentWorkingDir%\XML_Release
	dir %UTResultDir%
	set SDKDebugBinDir=%SDKBinDir%\Debug
	set SDKReleaseBinDir=%SDKBinDir%\Release
	if exist  "%SDKDebugBinDir%" (
		echo SDK debug bin info:
		dir %SDKDebugBinDir%
	)
	if exist  "%SDKReleaseBinDir%" (
		echo SDK release bin info:
		dir %SDKReleaseBinDir%
	)
	echo ************************************************************
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
	echo usage: DolphinUTBuildAndTest.bat  Action-BuildOption
	echo  Action:
	echo  Clean-All     clean DolphinTest solution (Debug and Release)
	echo  Clean-Debug   clean DolphinTest solution (Debug)
	echo  Clean-Release clean DolphinTest solution (Release)
	echo  Build-All     build MediaEngine SDK solution (Debug and Release)
	echo  Build-Debug   build MediaEngine SDK solution (Debug)
	echo  Build-Release build MediaEngine SDK solution (Release)
	echo  Test-All      build  DolphinTest solution (Debug and Release)
	echo                launch DophinTest App and get test result(Debug and Release)
	echo  Test-Debug    build  DolphinTest solution (Debug)
	echo                launch DophinTest App and get test result(Debug)
	echo  Test-Release  build  DolphinTest solution (Release)
	echo                launch DophinTest App and get test result(Release)
	echo ******************************************************************************
	echo ******************************************************************************
goto :EOF

rem function for option
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
	echo   aConfigurationList is %aConfigurationList%
	echo   Action is %Action%
	set ReturnCode=0
	for %%j in ( %aConfigurationList% ) do (
		echo       Configuration is %%j
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
		if "%Action%" == "Help" (
			echo  Help
			call :Help
		)
	)
goto :EOF

:Action-Clean
	set ActionConfiguration=%1
	set CleanFlag=0
	set BuildFlag=0
	set TestFlag=0
	call :Clean  "%DolphinTestWorkingDir%\%DolphinTestSolution%"  %ActionConfiguration%  %Platform%
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
	set ActionConfiguration=%1
	set CleanFlag=0
	set BuildFlag=0 
	
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
	set CleanFlag=0
	set BuildFlag=0
	set TestFlag=0

	call :Clean  "%DolphinTestWorkingDir%\%DolphinTestSolution%"  %ActionConfiguration%  %Platform%
	call :Build  "%DolphinTestWorkingDir%\%DolphinTestSolution%"  %ActionConfiguration%  %Platform%
	call :CleanCheck
    call :BuildCheck

	call :Test  %ActionConfiguration%  %Platform%	
	call :TestCheck
goto :EOF

:TestCheck
	if not %TestFlag%==0 (
		if "%ActionConfiguration%"=="Debug" (
			set TestDebugFlag=1
			set TestDebugInfo="test debug--failed--%TestFailedLog%"
		) else (
			set TestReleaseFlag=1
			set TestReleaseInfo="test release--failed--%TestFailedLog%"
		) 
	) else (
		if "%ActionConfiguration%"=="Debug" (
			set TestDebugFlag=0
			set TestDebugInfo="test debug--succeed"
		) else (
			set TestReleaseFlag=0
			set TestReleaseInfo="test release--succeed"
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
	echo         test result file  list as below:
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
	
	rem APP setting based on configuration
	call :APPSetting %vConfiguration%  %vPlatform%
	rem delete previous APP, install latest App and run UT
	AppDeployCmd   /uninstall %APPProductID% /targetdevice:de
	
	echo installing and launching UTApp....
	AppDeployCmd   /installlaunch %APPPackageFile% /targetdevice:de
	if not %ERRORLEVEL%==0 (
	 	set TestFlag=1
		set TestFailedLog=launch APP  and run UT failed
		echo error info: APPPackageFileName %APPPackageFile%
		goto :ErrorReturn
	)
	
	rem copy xml file for test result
	if exist %UTResultDir% (
		rd /s /q %UTResultDir%
	)
	md %UTResultDir%
	
	rem wait for UT 
	echo waiting for all caese in UT .....
	ping %PingIP%  -n 1  -w %MaxUTRunTime% >nul

	rem copy xml file from device
	ISETool ts de %APPProductID% %UTResultDir%
	copy %XMLFilePath%  %UTResultDir%
	if not %ERRORLEVEL%==0 (
	 	set TestFlag=1
		set TestFailedLog=copy UT output xml file failed
		goto :ErrorReturn
	)
	
	if exist %TemDir% (
		rd /s /q  %TemDir%
	)
	echo end
	
goto :EOF


:ErrorReturn
endlocal
exit /b 2

:End
endlocal
exit /b %ReturnCode%



