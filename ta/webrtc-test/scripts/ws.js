(function(){
	"use strict";
	var logger = yams.logger;
	
	function Signal()
	{
		this.venueUrl = yams.uuid();
		this.websocket = null;
		this.callbacks = null;
		this.roomId = "0";
		this.users = {};
		this.rtc = null;
		this.myId = 0;
		this.recvonly = false;
	}
	
	$.extend(Signal.prototype, {
		onOpen: function(evt){
		},
		
		onClose: function(evt){
		},
		
		onMessage: function(evt){
			var response = null;
			try{
				response = JSON.parse(evt.data);
			}catch(ex){
				logger.error(ex);
			}
			logger.info("onMessage" + evt.data);
			if(!response)
				return;
			if(typeof(this["on_" + response.type]) !== "undefined"){
				this["on_" + response.type](response);
			}else{
				logger.error("no handler was found.");
			}
		},
		
		on_join: function(resp){
			if(resp["count"] == 1){
				var data = {"type": "venue", "sdp":this.venueUrl};
				this.websocket.send(JSON.stringify(data)); 
			}
			if(this.callbacks && this.callbacks.onConnect)
				this.callbacks.onConnect(resp["count"]);
		},
		
		on_venue: function(resp){
			logger.info(resp);
			this.venueUrl = resp["sdp"];
			//
		},
		
		on_JOIN_N: function(resp){
			this.users[resp.id] = {name: resp.name, id: resp.id};
			this.startOffer();
		},
		
		on_LEAVE_N: function(resp){
			var user = this.users[resp.id];
			$("#" + user.id).remove();
			if(user && user.pc)
				delete user.pc;
			if(user && user.pc_s)
				delete user.pc_s;
			delete this.users[resp.id];
		},
		
		on_offer: function(resp){
			var remoteSdp = atob(resp.sdp);
			logger.info("on_offer,  " + "\r\n" + remoteSdp);
			
			if(this.pc == null) {
				var remoteVideo = this.createVideo(this.myId, "no name" + "(" + this.myId + ")", S_WIDTH, S_HEIGHT);
				$('#smallview').append(remoteVideo);
				var tag = remoteVideo.attr("videotag");
				this.pc = this.rtc.createPeerConnection("no name", this.myId, tag, this, false);
				this.pc.setRemote("offer", remoteSdp);
				this.pc.getAnswer();
			}
			else{
				throw "offer received, this user should not create offer.";
			}
		},
		
		on_answer: function(resp){
			var remoteSdp = atob(resp.sdp);
			logger.info("on_answer, \r\n" + remoteSdp);
			if(this.pc != null){
				this.pc.setRemote("answer", remoteSdp);
			}
			else{
				throw "user should have existed and have created the peer connection.(pc)"
			}
		},
		
		on_CANDIDATE: function(resp){
			var candidate = atob(resp.candidate);
			logger.info("on_CANDIDATE, from: " + resp.from + "\r\n" + candidate);
			if(this.pc){
				this.pc.addCandidate(resp.label, candidate);
			}
			else{
				throw "user should have existed and have created the peer connection."
			}
		},
		
		onError: function(evt){
		},
		
		connect: function(wsUri, roomId, name, callbacks){
			this.name = name;
			this.roomId = roomId;
			this.callbacks = callbacks;
			this.websocket = new WebSocket(wsUri + roomId); 
			this.websocket.onopen = $.proxy(this.onOpen, this);
			this.websocket.onclose = $.proxy(this.onClose, this);
			this.websocket.onmessage = $.proxy(this.onMessage, this);
			this.websocket.onerror = $.proxy(this.onError, this);
			logger.info("WebSocket is inited.");
		},
		
		startOffer: function(rtc, cnt){
			if(rtc)
				this.rtc = rtc;
			
			if(cnt == 1) {
				rtc.isSender = true;
				return;
			}

			var remoteVideo = this.createVideo("remote", "remote", S_WIDTH, S_HEIGHT);
			$('#smallview').append(remoteVideo);
			var tag = remoteVideo.attr("videotag");
			this.pc = this.rtc.createPeerConnection("remote", "remote", tag, this, false);
			this.pc.getOffer();
		},
		
		sendMessage: function(data){
			if(typeof data == 'object'){
				data = JSON.stringify(data);
			}
			console.log("sendMessage, need to get linus answer");
			this.websocket.send(data); 
		},
		
		close: function(){
			this.websocket.close();
		},
		
		createVideo: function(id, name, w, h){
			var view =  $('<div class="ui-resizable view ui-widget-content" id="' + id + '">' +
						  '<video id="video_' + id + '" autoplay="autoplay" muted="true"/>' + 
						  '<div class="viewname"></div></div>');
			view.attr("videotag", "video_" + id);
			view.find(".viewname").text(name);
			view.find("video").width(w).height(h);
			return view;			  
		}
	});
	
	yams.Signal = Signal;
}(yams));
