call build_mediaengine.bat
@set package=EngineReleasePackage-win-2-8

@call :copy_files %package%
@call :zip_files %package%
@rd /S /Q %package%

@goto :eof

:copy_files
mkdir %1
mkdir %1\doc
mkdir %1\external
mkdir %1\external\include
mkdir %1\external\libs
mkdir %1\external\libs\debug
mkdir %1\external\libs\release
mkdir %1\ref-app
mkdir %1\ref-app\bin
mkdir %1\ref-app\bin\release
mkdir %1\ref-app\bin\debug
mkdir %1\sdk
mkdir %1\sdk\include
mkdir %1\sdk\libs
mkdir %1\sdk\libs\release
mkdir %1\sdk\libs\debug
mkdir %1\sdk\dlls
mkdir %1\sdk\dlls\release
mkdir %1\sdk\dlls\debug
@set RefAppDir=..\..\ref-app
@set EngineDemoDir=%RefAppDir%\windows\EngineDemo
xcopy /E %EngineDemoDir%\* %1\ref-app\
xcopy /E %RefAppDir%\common\src\* %1\ref-app\src\
xcopy /E %RefAppDir%\common\include\* %1\ref-app\include\
del /f /q %1\ref-app\bin\debug\*
del /f /q %1\ref-app\bin\release\*
del /f /q %1\ref-app\bld\EngineDemoWithTA.*
del /f /q %1\ref-app\bld\*.ncb
del /f /q %1\ref-app\src\EngineDemoWithTA.*
del /f /q %1\ref-app\src\EngineDemo.vcproj*
rmdir /q /s %1\ref-app\src\TA
rmdir /q /s %1\ref-app\src\debug
rmdir /q /s %1\ref-app\src\release
rmdir /q /s %1\ref-app\objs
move %1\ref-app\src\EngineDemoForRefApp.vcproj %1\ref-app\src\EngineDemo.vcproj

set sdk_libs1=libsrtp wrtp wqos wbxtra util wbxtrace appshare
set sdk_libs2=MJPGDecoder wbxaecodec wbxaudioengine wmeutil wmeclient wseclient wsertp ciscosrtp
set external_libs=tp mmssl32 ffpref wp2p stunlib libeay32
set other_libs=welsdec welsenc welsvp WelsHEVCDecoder WelsHEVCEncoder
set configs=release debug
@set MediaEngineLibsDir=..\..\mediaengine\libs\win32
@set MediaEngineDllsDir=..\..\mediaengine\bin\win32
@set MediaEngineMapsDir=..\..\mediaengine\maps\win32
@set OpenH264DllsDir=..\..\vendor\openh264\libs\win32
@echo on
@rem copy sdk libs and dlls from the libs and bin directories of whiteney
@for %%i in (%sdk_libs1% %sdk_libs2% %external_libs%) do (
@	for %%j in (%configs%) do (
@		if exist %MediaEngineLibsDir%\%%j\%%i.lib  xcopy %MediaEngineLibsDir%\%%j\%%i.lib %1\sdk\libs\%%j
@		if exist %MediaEngineDllsDir%\%%j\%%i.dll  (
			xcopy %MediaEngineDllsDir%\%%j\%%i.dll %1\sdk\dlls\%%j	
			xcopy %MediaEngineDllsDir%\%%j\%%i.dll %1\ref-app\bin\%%j
		)
@		if exist %MediaEngineDllsDir%\%%j\%%i.pdb  xcopy %MediaEngineDllsDir%\%%j\%%i.pdb %1\sdk\dlls\%%j
@		if exist %MediaEngineMapsDir%\%%j\%%i.pdb  xcopy %MediaEngineMapsDir%\%%j\%%i.pdb %1\sdk\dlls\%%j
		
	)
)

@rem copy other dlls from the bin directory of MediaSDK
@for %%i in (%other_libs%) do (
@	for %%j in (%configs%) do (
@		if exist %OpenH264DllsDir%\%%j\%%i.dll  (
			xcopy %OpenH264DllsDir%\%%j\%%i.dll %1\sdk\dlls\%%j	
			xcopy %OpenH264DllsDir%\%%j\%%i.dll %1\ref-app\bin\%%j
		)		
	)
)

@rem copy wbxtracer.exe
xcopy %MediaEngineDllsDir%\win32\release\wbxtracer.exe %1\sdk\dlls\release

@rem copy header files
@rem external header files
@set UtilHeaderIncDir=..\..\mediaengine\util\include
xcopy /E %UtilHeaderIncDir%\client\* %1\external\include\client\
xcopy /E %UtilHeaderIncDir%\windows\* %1\external\include\windows\
xcopy %UtilHeaderIncDir%\*.h %1\external\include\

@rem SDK header files
@rem MediaSDK
@set MediaSdkHeaderIncDir=..\..\api
xcopy /E %MediaSdkHeaderIncDir%\* %1\sdk\include

@rem wrtp
@set WrtpHeaderIncDir=..\..\medianengine\transmission\include
set wrtp_headers=wrtpapi.h wrtpdefines.h wrtpmediaapi.h wrtppacket.h wrtpclientapi.h  wrtpcommonapi.h
@for %%i in (%wrtp_headers%) do (xcopy /E %WrtpHeaderIncDir%\%%i %1\sdk\include\wrtp\)

@rem wqos
@set WqosHeaderIncDir=..\..\medianengine\wqos\include
@set wqos_headers=qosapi.h qosapidef.h qoscodevapi.h qoscodevdefs.h QoSControlManager.h qosdefines.h
@for %%i in (%wqos_headers%) do (xcopy /E %WqosHeaderIncDir%\%%i %1\sdk\include\wqos\)


@rem copy windowsDemoStep.doc & windowsReadme.txt to doc
xcopy .\windowsReadme.txt %1\doc\
xcopy .\windowsDemoStep.doc %1\doc\

@endlocal & goto :eof

:zip_files
@setlocal
@7z a %1.zip .\%1\*
@endlocal & goto :eof

