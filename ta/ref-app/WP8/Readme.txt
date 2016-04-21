How to build and install Refapp ?

1. Prepare build environment
   1) install VS2013 update 4
   2) install python 2.7
   3) install 7zip
   4) add python and 7zip to $PATH

2. Get code from git
   command:
   git clone git@sqbu-github.cisco.com:WebExSquared/wme.git c:\git\wme
   git submodule update --init --recursive

3. build Refapp via python script (Recommended)
    1) Open command windows under folder c:\git\wme\build\wp8
       command: python build_2013_wp8.py
    2) Open command windows under folder C:\git\wme\ta\ref-app\WP8\buildscript
       command: python refapp_build_wp8.py
	3) Get file Refapp_wp_Release_ARM.xap at C:\git\wme\ta\ref-app\WP8\bin\release\
	
4. build Refapp via VS2013 update 4, (Complex than the python script method)
   reminder: build platform is "ARM", you need to set it before build the solution
   1) build solution C:\git\wme\mediaengine\bld\WP8\MediaSDKClient.sln  (top module is mediasession)
   2) build solution C:\git\wme\ta\ref-app\WP8\Refapp-wp\Refapp-wp.sln
   3) Get file Refapp_wp_Release_ARM.xap under C:\git\wme\ta\ref-app\WP8\Refapp-wp\Refapp-wp\Bin\ARM\Release

5. Install Refapp.
   1) Download WP Power Tool at https://wptools.codeplex.com/
   2) Install Refapp via the Power tool.