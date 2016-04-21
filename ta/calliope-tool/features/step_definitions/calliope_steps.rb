When /^I call backdoor$/ do
    put "hello\nddd"
	p "trying backdoor..."
	p backdoor("backdoorStart", "Nil")
	sleep 1
	p backdoor("backdoorGetSDP", "local")
	sleep 60
end	

Given /^the Linus server is (.*)$/ do |linus|
	ENV["LINUS_SERVER"] = nil
	ENV["LINUS_SERVER"] = linus if linus
	@client = CalliopeClient.new(true)
	@venueurl = SecureRandom.uuid()
	@urls = []
end

Given /^I create a venue$/ do
	@client = CalliopeClient.new(false)
	@venueurl = @client.createVenue()
end

When /^I create confluence$/ do
	sdpOffer = "v=0\r\no=wme-mac-3.6.0 0 1 IN IP4 127.0.0.1\r\ns=-\r\nb=AS:0\r\nt=0 0\r\na=cisco-mari:v0\r\na=cisco-mari-rate\r\nm=audio 20538 RTP/AVP 101 0 8 9 102\r\nc=IN IP4 10.35.132.106\r\nb=TIAS:64000\r\na=content:main\r\na=sendrecv\r\na=rtpmap:101 opus/48000/2\r\na=fmtp:101 maxplaybackrate=48000;maxaveragebitrate=64000;stereo=1\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 iLBC/8000\r\na=extmap:1/sendrecv http://protocols.cisco.com/virtualid\r\na=extmap:2/sendrecv urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=rtcp-mux\r\na=sprop-source:0 csi=315201793;count=1\r\na=sprop-simul:0 100 *\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:mp9g\r\na=ice-pwd:Dj5dHJ4iz8PAHMEvrODIKU\r\na=candidate:1 1 UDP 2113933823 10.35.132.106 20538 typ HOST\r\na=candidate:1 2 UDP 2113933822 10.35.132.106 20539 typ HOST\r\na=candidate:2 1 TCP 2113933567 10.35.132.106 20930 typ HOST\r\na=candidate:2 2 TCP 2113933566 10.35.132.106 20931 typ HOST\r\na=candidate:3 1 UDP 2113933311 192.168.0.106 20116 typ HOST\r\na=candidate:3 2 UDP 2113933310 192.168.0.106 20117 typ HOST\r\na=candidate:4 1 TCP 2113933055 192.168.0.106 20144 typ HOST\r\na=candidate:4 2 TCP 2113933054 192.168.0.106 20145 typ HOST\r\na=candidate:5 1 UDP 2113932799 10.35.132.21 20908 typ HOST\r\na=candidate:5 2 UDP 2113932798 10.35.132.21 20909 typ HOST\r\na=candidate:6 1 TCP 2113932543 10.35.132.21 20088 typ HOST\r\na=candidate:6 2 TCP 2113932542 10.35.132.21 20089 typ HOST\r\nm=video 20986 RTP/AVP 118 98 117 97 111\r\nc=IN IP4 10.35.132.106\r\nb=TIAS:4000000\r\na=content:main\r\na=sendrecv\r\na=rtpmap:118 H264-SVC/90000\r\na=fmtp:118 profile-level-id=420016;packetization-mode=1;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520;uc-mode=1\r\na=rtpmap:98 H264-SVC/90000\r\na=fmtp:98 profile-level-id=420016;packetization-mode=0;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520;uc-mode=1\r\na=rtpmap:117 H264/90000\r\na=fmtp:117 profile-level-id=420016;packetization-mode=1;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520\r\na=rtpmap:97 H264/90000\r\na=fmtp:97 profile-level-id=420016;packetization-mode=0;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520\r\na=rtpmap:111 x-ulpfecuc/8000\r\na=fmtp:111 max_esel=1400;max_n=255;m=8;multi_ssrc=1\r\na=rtcp-fb:* nack pli\r\na=rtcp-fb:* ccm tmmbr\r\na=extmap:1/sendrecv http://protocols.cisco.com/framemarking\r\na=extmap:2/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:3/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=extmap:4/sendrecv http://protocols.cisco.com/priority\r\na=rtcp-mux\r\na=sprop-source:0 csi=315201792;count=1\r\na=sprop-simul:0 100 118 profile-level-id=42e00b;max-mbps=1800;max-fs=60;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 101 118 profile-level-id=42e014;max-mbps=7200;max-fs=240;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 102 118 profile-level-id=42e01e;max-mbps=27600;max-fs=920;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 103 118 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 104 98 profile-level-id=42e00b;max-mbps=1800;max-fs=60;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 105 98 profile-level-id=42e014;max-mbps=7200;max-fs=240;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 106 98 profile-level-id=42e01e;max-mbps=27600;max-fs=920;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 107 98 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;fr-layers=750,1500,3000\r\na=sprop-simul:0 108 117 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;\r\na=sprop-simul:0 109 97 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:qeGP\r\na=ice-pwd:hYfuXyyfzxpOSiK5JFhxoA\r\na=candidate:1 1 UDP 2113933823 10.35.132.106 20986 typ HOST\r\na=candidate:1 2 UDP 2113933822 10.35.132.106 20987 typ HOST\r\na=candidate:2 1 TCP 2113933567 10.35.132.106 20150 typ HOST\r\na=candidate:2 2 TCP 2113933566 10.35.132.106 20151 typ HOST\r\na=candidate:3 1 UDP 2113933311 192.168.0.106 20788 typ HOST\r\na=candidate:3 2 UDP 2113933310 192.168.0.106 20789 typ HOST\r\na=candidate:4 1 TCP 2113933055 192.168.0.106 20634 typ HOST\r\na=candidate:4 2 TCP 2113933054 192.168.0.106 20635 typ HOST\r\na=candidate:5 1 UDP 2113932799 10.35.132.21 20780 typ HOST\r\na=candidate:5 2 UDP 2113932798 10.35.132.21 20781 typ HOST\r\na=candidate:6 1 TCP 2113932543 10.35.132.21 20498 typ HOST\r\na=candidate:6 2 TCP 2113932542 10.35.132.21 20499 typ HOST";
    puts sdpOffer
	if(ENV["LINUS_SERVER"])
		@answer = @client.createLinusConfluence(@venueurl, sdpOffer)
	else	
		@answer = @client.createConfluence(@venueurl, sdpOffer)
	end
    @urls = [] unless(@urls)
	@urls.push(@answer[:confluenceUrl])
	puts @answer
end

When /^I create confluence with multiple sdps$/ do
	sdpOffer = ["v=0\r\no=wme-mac-3.9.0 0 1 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=cisco-mari:v1\r\na=cisco-mari-rate\r\nm=audio 20764 RTP/SAVPF 101 0 8 9 102\r\nc=IN IP4 10.224.204.44\r\nb=TIAS:64000\r\na=content:main\r\na=recvonly\r\na=rtpmap:101 opus/48000/2\r\na=fmtp:101 maxplaybackrate=48000;maxaveragebitrate=64000;stereo=1\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:9 G722/8000\r\na=rtpmap:102 iLBC/8000\r\na=extmap:1/sendrecv http://protocols.cisco.com/virtualid\r\na=extmap:2/sendrecv urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:5zJDipDZVPCpbVKMWpVoRqYVjKwYqkB/06O/beVn|2^31\r\na=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:5zJDipDZVPCpbVKMWpVoRqYVjKwYqkB/06O/beVn|2^31\r\na=crypto:3 AES_CM_256_HMAC_SHA1_80 inline:5zJDipDZVPCpbVKMWpVoRqYVjKwYqkB/06O/beVnXfZbNiJPlI0wNmUHMELU9g==|2^31\r\na=rtcp-mux\r\na=sprop-source:0 csi=638512896;count=1\r\na=sprop-simul:0 100 *\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:ncWk\r\na=ice-pwd:eE0CcHfE4mV6IiOn+NSnXE\r\na=candidate:1 1 UDP 2113933823 10.224.204.44 20764 typ HOST\r\na=candidate:1 2 UDP 2113933822 10.224.204.44 20765 typ HOST\r\na=candidate:2 1 TCP 2113933567 10.224.204.44 20158 typ HOST\r\na=candidate:2 2 TCP 2113933566 10.224.204.44 20159 typ HOST\r\na=candidate:3 1 UDP 2113933311 192.168.178.1 20988 typ HOST\r\na=candidate:3 2 UDP 2113933310 192.168.178.1 20989 typ HOST\r\na=candidate:4 1 TCP 2113933055 192.168.178.1 20770 typ HOST\r\na=candidate:4 2 TCP 2113933054 192.168.178.1 20771 typ HOST\r\na=candidate:5 1 UDP 2113932799 172.16.82.1 20018 typ HOST\r\na=candidate:5 2 UDP 2113932798 172.16.82.1 20019 typ HOST\r\na=candidate:6 1 TCP 2113932543 172.16.82.1 20604 typ HOST\r\na=candidate:6 2 TCP 2113932542 172.16.82.1 20605 typ HOST\r\na=candidate:7 1 UDP 2113932287 10.140.48.95 20058 typ HOST\r\na=candidate:7 2 UDP 2113932286 10.140.48.95 20059 typ HOST\r\na=candidate:8 1 TCP 2113932031 10.140.48.95 20990 typ HOST\r\na=candidate:8 2 TCP 2113932030 10.140.48.95 20991 typ HOST\r\n",
        "v=0\r\no=wme-mac-3.9.0 0 1 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=cisco-mari:v1\r\na=cisco-mari-rate\r\nm=video 20904 RTP/SAVPF 117 97 111\r\nc=IN IP4 10.224.204.44\r\nb=TIAS:4000000\r\na=content:main\r\na=recvonly\r\na=rtpmap:117 H264/90000\r\na=fmtp:117 profile-level-id=420016;packetization-mode=1;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520\r\na=rtpmap:97 H264/90000\r\na=fmtp:97 profile-level-id=420016;packetization-mode=0;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=4000;max-dpb=11520\r\na=rtpmap:111 x-ulpfecuc/8000\r\na=fmtp:111 max_esel=1400;max_n=255;m=8;multi_ssrc=1;FEC_ORDER=FEC_SRTP\r\na=rtcp-fb:* nack pli\r\na=rtcp-fb:* ccm tmmbr\r\na=extmap:1/sendrecv http://protocols.cisco.com/virtualid\r\na=extmap:2/sendrecv http://protocols.cisco.com/framemarking\r\na=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:yrkAP26oBXjW0b64yg29Qp6w0di9qp+Bmf0RTVI+|2^31\r\na=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:yrkAP26oBXjW0b64yg29Qp6w0di9qp+Bmf0RTVI+|2^31\r\na=crypto:3 AES_CM_256_HMAC_SHA1_80 inline:yrkAP26oBXjW0b64yg29Qp6w0di9qp+Bmf0RTVI+p2NutXKWoHasv/HxGoK9jQ==|2^31\r\na=rtcp-mux\r\na=sprop-source:0 csi=677681153;count=1\r\na=sprop-simul:0 100 117 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;\r\na=sprop-simul:0 101 97 profile-level-id=42e01f;max-mbps=108000;max-fs=3600;max-fps=3000;\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:VyUd\r\na=ice-pwd:52v9Q2Y6iHYcSThQqcQQGx\r\na=candidate:1 1 UDP 2113933823 10.224.204.44 20904 typ HOST\r\na=candidate:1 2 UDP 2113933822 10.224.204.44 20905 typ HOST\r\na=candidate:2 1 TCP 2113933567 10.224.204.44 20392 typ HOST\r\na=candidate:2 2 TCP 2113933566 10.224.204.44 20393 typ HOST\r\na=candidate:3 1 UDP 2113933311 192.168.178.1 20192 typ HOST\r\na=candidate:3 2 UDP 2113933310 192.168.178.1 20193 typ HOST\r\na=candidate:4 1 TCP 2113933055 192.168.178.1 20880 typ HOST\r\na=candidate:4 2 TCP 2113933054 192.168.178.1 20881 typ HOST\r\na=candidate:5 1 UDP 2113932799 172.16.82.1 20288 typ HOST\r\na=candidate:5 2 UDP 2113932798 172.16.82.1 20289 typ HOST\r\na=candidate:6 1 TCP 2113932543 172.16.82.1 20946 typ HOST\r\na=candidate:6 2 TCP 2113932542 172.16.82.1 20947 typ HOST\r\na=candidate:7 1 UDP 2113932287 10.140.48.95 20240 typ HOST\r\na=candidate:7 2 UDP 2113932286 10.140.48.95 20241 typ HOST\r\na=candidate:8 1 TCP 2113932031 10.140.48.95 20302 typ HOST\r\na=candidate:8 2 TCP 2113932030 10.140.48.95 20303 typ HOST\r\n"];
    puts sdpOffer
	if(ENV["LINUS_SERVER"])
		@answer = @client.createLinusConfluence(@venueurl, sdpOffer)
	else	
		@answer = @client.createConfluence(@venueurl, sdpOffer)
	end
    @urls = [] unless(@urls)
	@urls.push(@answer[:confluenceUrl])
	puts @answer
end

Then /^I should get the SDP$/ do
	assert @answer[:sdpAnswer]
    p @answer[:sdpAnswers]
	@urls.each do |url|
	    p "delete url:" + url
	    @client.deleteLinusConfluence(url)
	end
	@client.deleteVenue(@venueurl) unless (ENV["LINUS_SERVER"])
end
