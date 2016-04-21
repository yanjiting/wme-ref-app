Build instructions for Windows
===============================
 
Pre-requisition
-------------------------------

1.  Have VS2008 installed;
2.  Check the VS2008 IDE folder. If it is not "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE" as default.
please modify mediaengine\bld\windows\build_MediaEngine_vs2008.bat and unittest\bld\windows build_ut_vs2008.bat and set it to
the right value

If you want to build media session API, you need to start to use VS2013 and build script is not supported yet, so you need to use VS2013 IDE (express version) to compile. (old WME need MFC, so need VS2013 pro/team edition.)
Here is the steps:
1. Download and install VS2013 express for desktop (not update 2); (http://www.visualstudio.com/zh-cn/downloads/download-visual-studio-vs#DownloadFamilies_2)
2. Run `python prepare_vs2013.py` to copy nasm.exe/afxres.h into your IDE, with administrator priviledge.
3. Open solution: mediaengine\bld\windows\MediaSDKClient12.sln, both debug/release are supported (no Unicode)

MSBUILD for vc90: http://www.microsoft.com/en-us/download/details.aspx?id=3138
VSExpress 2008: http://download.microsoft.com/download/8/B/5/8B5804AD-4990-40D0-A6AA-CE894CBBB3DC/VS2008ExpressENUX1397868.iso

Build media engine libs
-------------------------------
Just run build_mediaengine.bat or run it with parameter debug/release/clean
If run build_mediaengine.bat without parameter, it will build both debug and release versions.

The generated dlls are in mediaengine/bin/debug or mediaengine/bin/release.



Build UT projects:
--------------------------------
Just run build_ut.bat or run it with parameter debug/release/clean
If run build_ut.bat without parameter, it will build both debug and release versions.

The generated exe files are in unittest/bin/debug or unittest/bin/release.

