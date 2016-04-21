(function(){
    "use strict";
    var logger = yams.logger;
    var WebRTC = yams.WebRTC;
    var Signal = yams.Signal;
    var rtc;
    var cmd = new Signal();
    var roomId = 0, userName = "";
    var signalReady = false, localReady = false;
    window.S_WIDTH = 320;
    window.S_HEIGHT = 240;

    var currHost = $(location).attr('host');
    var wsUri = "ws://10.224.166.110:8080/echo?r=";
    logger.info("Websocket URI = " + wsUri);
    logger.info("Parameters = " + yams.urlParams);
    if(yams.urlParams["name"])
        userName = yams.urlParams["name"];
    if(yams.urlParams["r"])
        roomId = yams.urlParams["r"];

    $(window).unload(function() {
        cmd.close();
    });

    function createAudioContext() {
        try {
            // Fix up for prefixing
            window.AudioContext = window.AudioContext||window.webkitAudioContext;
            yams.context = new AudioContext();
        }
        catch(e) {
            alert('Web Audio API is not supported in this browser');
        }
    }

    function initialize(){
        createAudioContext();
        rtc = new WebRTC(); 
        cmd.connect(wsUri, 
                    roomId, 
                    userName, 
                    {
            onConnect: function(cnt){
                signalReady = true;
                yams.join_cnt = cnt;
                if(localReady)
                    cmd.startOffer(rtc, cnt);
                logger.info("After start offer, RTC.isSender = " + rtc.isSender);
            },

            onClose: function(){
            }
        });
        var localVideo = cmd.createVideo("local", "me", S_WIDTH, S_HEIGHT);
        $('#smallview').append(localVideo);
        rtc.init(localVideo.attr('videotag'), 
                 {
            onLocalReady: function() {
                localReady = true;
                if(signalReady)
                    cmd.startOffer(rtc, yams.join_cnt);
            }
        });
        yams.rtc = rtc;
        yams.cmd = cmd;
        $("#close").click(function() {
            rtc.localStream.stop();
        });
    }
    setTimeout(initialize, 1);
}());
