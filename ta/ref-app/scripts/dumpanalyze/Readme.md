How to analyze minidump of Android app
==================================================

   1. You need to find the maps of crashed build. You can first find the WMEVERSION and then try to find the maps in the Jenkins from the build number.
   2. Download the maps from Jenkins and unzip it to folder a folder
   3. run `sh analyze_dumpfile.sh <maps dir> <id.dmp> <output dir>
   4. If you cannot find the line information with only address, you can try to do it with:


	addr2line -C -f -e libxxxx.so 0xFFFFF
	
	In windows it is:
	
	%ANDROID_NDK_HOME%\toolchains\arm-linux-androideabi-4.8\prebuilt\windows-x86_64\bin\arm-linux-androideabi-addr2line.exe
	
