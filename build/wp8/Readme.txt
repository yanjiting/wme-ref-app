How to build WME ?

1. Prepare build environment
   1) install python 2.7
   2) install 7zip
   3) add python and 7zip to $PATH

2. Get code form git
   command:
   git clone git@sqbu-github.cisco.com:WebExSquared/wme.git c:\git\wme
   git submodule update --init --recursive

3. build WME via VS2013 update 4
   reminder: build platform is "ARM", you need to set it before build the solution
   1) build solution C:\git\wme\mediaengine\bld\WP8\MediaSDKClient.sln  (top module is mediasession)

4. build WME via python script
    1) Open command windows under folder c:\git\wme\build\wp8
       command: python build_2013_wp8.py

5. package WME distribution
   1) Open command windows under folder c:\git\wme\build\wp8
      command: python package.py [release | debug | clean]        //the default parameter is release