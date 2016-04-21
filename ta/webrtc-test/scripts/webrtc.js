(function(){
    "use strict";
    var logger = yams.logger;
    var mediaConstraints = {"video": true, "audio": true};
    var pcConfig = {"iceServers": [{"url": "stun:www.webex.com:19302"}]};
    var pcConstraints = {"optional": [{"DtlsSrtpKeyAgreement": true}]};
    var sdpConstraints = {'mandatory': {
        'offerToReceiveAudio': true,
        'offerToReceiveVideo': true }};

    var sendProgress = null;
    var receiveProgress = null;
    var dataChannel = null;
    var recvChannel = null;
    var receivedSize = 0;
    var sendCount = 0;
    var recePkgCount = 0;
    var bitrateMax = 0;

    var onMsgCallback = null;

    function WebRTC()
    {
        this.localVideo = null;
        this.localStream = null;
        this.callbacks = null;
        this.isSender = false;
    };

    $.extend(WebRTC.prototype, {
        onUserMediaSuccess: function(stream) {
            logger.info('User has granted access to local media.');
            // Call the polyfill wrapper to attach the media stream to this element.
            attachMediaStream(this.localVideo, stream);
            this.localVideo.style.opacity = 1;
            this.localStream = stream;
            if(this.callbacks && this.callbacks.onLocalReady){
                this.callbacks.onLocalReady(this);
            }
        },

        onUserMediaError: function(error) {
            logger.info('Failed to get access to local media. Error code was ' + error.code);
            alert('Failed to get access to local media. Error code was ' + error.code + '.');
        },

        doGetUserMedia: function() {
            // Call into getUserMedia via the polyfill (adapter.js).
            try {
                getUserMedia(mediaConstraints, $.proxy(this.onUserMediaSuccess, this), $.proxy(this.onUserMediaError, this));
                logger.info('Requested access to local media with mediaConstraints:\n' +
                            '  \'' + JSON.stringify(mediaConstraints) + '\'');
            } catch (e) {
                alert('getUserMedia() failed. Is this a WebRTC capable browser?');
                logger.warn('getUserMedia failed with exception: ' + e.message);
            }
        },

        doGetSourceMedia: function() {
            var audioSource = yams.context.createMediaElementSource($("#videosrc")[0]);
            var mixedOutput = yams.context.createMediaStreamDestination();
            audioSource.connect(mixedOutput);
            setTimeout($.proxy(this.onUserMediaSuccess, this), 50, mixedOutput.stream);
        },

        init: function(localTag, callbacks){
            logger.info("Initializing webrtc.");
            sendProgress = document.querySelector('progress#sendProgress');
            receiveProgress = document.querySelector('progress#receiveProgress');
            // Reset localVideo display to center.
            this.localVideo = document.getElementById(localTag);
            this.localVideo.addEventListener('loadedmetadata', function(){
                logger.info("local media loaded, need to resize the window.");
                //window.onresize();
            });
            this.doGetUserMedia();
            //this.doGetSourceMedia();
            this.callbacks = callbacks;
            onMsgCallback = this.onReceiveMessageCallback;
            $("#sendTheData").click(function() {
                if(dataChannel == null) {
                    logger.info("dataChannel is null, please check the peerconnection.");
                    return;
                }
                var bytesToSend = 1 * 20 * 1024;
                sendProgress.max = bytesToSend;
                receiveProgress.max = sendProgress.max;
                sendProgress.value = 0;
                receiveProgress.value = 0;

                var chunkSize = 1024; 
                var stringToSendRepeatedly = '';
                for (var i = 0; i < chunkSize; i++) {
                    // Visible ASCII chars are between 33 and 126.
                    stringToSendRepeatedly += String.fromCharCode(33 + Math.random() * 93);
                }

                var sendAllData = function() {
                    while (sendProgress.value < sendProgress.max) {
                        if (dataChannel.bufferedAmount > 5 * chunkSize) {
                            setTimeout(sendAllData(), 250);
                            return;
                        }
                        sendProgress.value += chunkSize;
                        sendCount ++;
                        dataChannel.send(stringToSendRepeatedly);
                        logger.info("data Channel send data, send packge count=" + sendCount + ', sendProgress.value=' + sendProgress.value);
                    }
                };

                setTimeout(sendAllData(), 0);
            });
        },

        onSendChannelStateChange: function() {
            var readyState = this.readyState;
            logger.info('Send channel state is: ' + readyState);
        },

        onReceiveMessageCallback: function (event) {
            recePkgCount ++;
            receivedSize += event.data.length;
            receiveProgress.value = receivedSize;
            logger.info('onReceiveMessageCallback, recePkgCount=' + recePkgCount + ', receivedSize=' + receivedSize);
        },

        receiveChannelCallback: function(event) {
            trace('Receive Channel Callback');
            recvChannel = event.channel;
            recvChannel.binaryType = 'arraybuffer';
            recvChannel.onmessage = onMsgCallback;
            receivedSize = 0;
        },

        createPeerConnection: function(name, id, videotag, cmd, recvonly) {
            var usr = new RTCUser(name, id, videotag, cmd);
            try{
                // Create an RTCPeerConnection via the polyfill (adapter.js).
                usr.pc = new RTCPeerConnection(pcConfig, pcConstraints);
                logger.info('Created RTCPeerConnnection with:\n' +
                            '  config: \'' + JSON.stringify(pcConfig) + '\';\n' +
                            '  constraints: \'' + JSON.stringify(pcConstraints) + '\'.');
            }catch (e) {
                logger.warn('Failed to create PeerConnection, exception: ' + e.message);
                alert('Cannot create RTCPeerConnection object; \
WebRTC is not supported by this browser.');
                return;
            }

            usr.video = document.getElementById(videotag);
            usr.pc.onicecandidate = $.proxy(usr.onIceCandidate, usr);
            usr.pc.onaddstream = $.proxy(usr.onRemoteStreamAdded, usr);
            usr.pc.onremovestream = $.proxy(usr.onRemoteStreamRemoved, usr);
            usr.pc.onsignalingstatechange = $.proxy(usr.onSignalingStateChanged, usr);
            usr.pc.oniceconnectionstatechange = $.proxy(usr.onIceConnectionStateChanged, usr);
            usr.recvonly = recvonly;
            if(!recvonly && this.localStream)
                usr.pc.addStream(this.localStream);

            //var dataChannelParams = {ordered: false, reliable: false, maxRetransmitTime: 0};
            var dataChannelParams = {ordered: false, maxRetransmits: 0};
            dataChannel = usr.pc.createDataChannel('sendDataChannel', dataChannelParams);
            dataChannel.binaryType = 'arraybuffer';
            logger.info('Created send data channel');
            dataChannel.onopen = this.onSendChannelStateChange;
            dataChannel.onclose = this.onSendChannelStateChange;
            //new datachannel to test SCTP
            if(this.isSender == false) {
                usr.pc.ondatachannel = this.receiveChannelCallback;
            }
            return usr;	
        }
    });

    function RTCUser(name, id, videotag, cmd)
    {
        this.pc = null;
        this.name = name;
        this.id = id;
        this.videotag = videotag;
        this.cmd = cmd;
        this.recvonly = true;
    }

    $.extend(RTCUser.prototype, {
        setFor: function(forId){
            this.forId = forId;
        },

        setRemote: function(type, sdp) {
            var message = {"type": type, "sdp": sdp};
            this.pc.setRemoteDescription(new RTCSessionDescription(message),
                                         $.proxy(this.onSetSDPSuccess, this),
                                         $.proxy(this.onSetSDPFailure, this));
        },

        getOffer: function(){
            this.pc.createOffer($.proxy(this.onLocalSDP, this), 
                                $.proxy(this.onCreateSessionDescriptionError, this), 
                                sdpConstraints);
        },

        getAnswer: function(){
            this.pc.createAnswer($.proxy(this.onLocalSDP, this),
                                 $.proxy(this.onCreateSessionDescriptionError, this), 
                                 sdpConstraints);
        },

        addCandidate: function(label, candidate){
            var candidate = new RTCIceCandidate({sdpMLineIndex: label,
                                                 candidate: candidate});
            this.pc.addIceCandidate(candidate);
        },

        onCreateSessionDescriptionError: function(error){
            logger.error("onCreateSessionDescriptionError,", error);
        },

        onSetSDPSuccess: function(){
            logger.info("onSetSessionDescriptionSuccess");
        },

        onSetSDPFailure: function(error){
            logger.error("onSetSessionDescriptionError ", error);
        },

        onLocalSDP: function(sessionDescription){
            var sdpStr = sessionDescription.sdp;

            sdpStr = sdpStr.replace(/(RTP\/SAVPF\s)100/g, "$1120");
            sdpStr = sdpStr.replace(/(rtcp-fb:)100/g, "$1120");
            sdpStr = sdpStr.replace(/(rtpmap:)100/g, "$1120");
            sessionDescription.sdp = sdpStr;
            this.pc.setLocalDescription(sessionDescription, 
                                        $.proxy(this.onSetSDPSuccess, this),
                                        $.proxy(this.onSetSDPFailure, this)
                                       );
            var offer = {type: sessionDescription.type.toLowerCase(),
                         sdp: btoa(sessionDescription.sdp)
                        };
            this.cmd.sendMessage(offer);
            logger.info("onLocalSDP, type=" + sessionDescription.type + ", local sdp=\r\n" + sessionDescription.sdp);
        },

        onIceCandidate: function(event) {
            logger.info("onIceCandidate, candidate gathering state = " + this.pc.iceGatheringState);
            if (event.candidate) {
                logger.info(event.candidate);
                var candidates = {type: "CANDIDATE",
                                  label: event.candidate.sdpMLineIndex,
                                  mid: event.candidate.sdpMid,
                                  candidate: btoa(event.candidate.candidate),
                                  from: this.cmd.myId,
                                  to: this.id
                                 };
                if(this.forId){
                    candidates.to = 0;
                    candidates["for"] = this.forId;
                }			
                this.cmd.sendMessage(candidates);
                logger.info("onIceCandidate, candidate=" + event.candidate.candidate);
            }else{
                logger.info('End of candidates.');
            }
        },

        onRemoteStreamAdded: function(event) {
            logger.info('Remote stream added.', event);
            attachMediaStream(this.video, event.stream);
            this.videostream = event.stream;
        },

        onRemoteStreamRemoved: function(event){
            logger.info('Remote stream removed.', event);
        },

        onSignalingStateChanged: function(event){
            logger.info("onSignalingStateChanged, signal state=" + this.pc.signalingState );
        },

        onIceConnectionStateChanged: function(event){
            logger.info("onIceConnectionStateChanged, now = " + this.pc.iceGatheringState);
        }
    });

    yams.WebRTC = WebRTC;
}());
