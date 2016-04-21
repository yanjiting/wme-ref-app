wme ta
============

See more details about how to do autoamtion test with Cucumber in 

    ref-app/README.md

    
What ref-app basic features for manual test (command line)
----------------------------------------------------------

1.  Loopback through Linus server mode
    You need to fill the linus server, and it will build caller/callee in the same device, caller send media out and callee render the media received. So the media data will go through Linus but the test is on the one device. It is the most recommended test method. The default Linus address is "10.224.166.47:5000" -- located in Hangzhou.

    **desktop command line**: `--loopback --calliope --linus http://10.224.166.47:5000/`

2.  Loopback through Calliope(Orpheus) mode
    This mode is similar to 1, you just need to leave the Linus server address empty in the UI. It will create venue through Orpheus server, and data go through the Linus allocated by Orpheus. It now by default connect to "https://calliope-integration.wbx2.com/" with Bearer authentication. (built-in test username and password)
    **desktop command line**: `--loopback --calliope`

3.  Loopback peer to peer mode
    This mode is similar to 1, you need to turn "Calliope" button to OFF to enter this mode. This mode the data will use loopback address (127.0.0.1) to send data, so the data is all in local.
    
    **desktop command line**: `--loopback`

4.  Peer to peer call through Linus server mode
    This mode needs you have 2 devices, they will connect to the same websocket server to exchange SDP like apprtc demo of WebRTC. The websocket server is under ref-app folder: `wsserver.js` it is written in Node.js. You can setup it easily by yourself as following steps:


        1) Install node.js and add it to the path (both windows/linux/mac are supported)
        2) run `npm install websocket` in ref-app folder to install websocket module
        3) run `node wsserver.js` (you can change the file to alter the listen port, default is 8080).

    We also setup a default server `10.224.166.31:8080/echo?r=<roomID>` in Hangzhou, so you can use it if you can access it. One important tip is: **you MUST set the roomID to the same for 2 devices and distinct from others.** Otherwise it will get messed up. So the test steps are:

        1) Turn on "Loopback" button to "OFF" for both devices
        2) Set the WS to the same distinct URL for 2 devices
        3) Click "Start" button on both devices
    
    The media data will go through Linus server you set.
    
    **desktop command line**: `--calliope --linus http://10.224.166.47:5000/ --server 10.224.166.31:8080/?r=419`

5.  Peer to peer call through Calliope mode

    In addition to mode 4, you clear the Linus address like you have done for mode 2. The data will go through integration test Linus server allocated by Orpheus.
    
    **desktop command line**: `--calliope --server 10.224.166.31:8080/?r=419`

6.  Pure Peer to peer call

    In addition to mode 4, you turn "Calliope" to "OFF". The data will go directly from device A to device B. The SDP is exchanged through websocket server.
    
    **desktop command line**: `--server 10.224.166.31:8080/?r=419`

7.  Start in TA mode with embedded HTTP server (except for android)

    **desktop command line**: `--ta <port>`
    
    In this mode, you can query statistics, or command our ref-app to do something with HTTP restful interface. It is compatible with calabash restful API.

8.  We have a lot of other command line options, you need to read the code to use it.

How to run command line for different platforms:
-----------------------------------------------
I will use loopback + calliope as the command line example, you can extend the command line.

  - Windows: `ClickCall.exe --loopback --calliope`
  - MAC: `open -n MediaSessionTATest.app --args [command lines]`
  - Android: Android command line is a little bit different from other platforms.    
        `adb shell am force-stop com.wx2.clickcall; adb shell am start -n com.wx2.clickcall/com.wx2.clickcall.MainActivity -e loopback 1 -e calliope 1`
  - iOS: `./ios-deploy -d -b ../build/Release-iphoneos/MediaSessionTest.app --args "--loopback --calliope"` It will install the app and start it as xcode.