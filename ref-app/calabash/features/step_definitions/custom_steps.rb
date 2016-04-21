def create_venue
    #ENV['LINUS_SERVER'] = 'http://173.39.168.98:5000/'
    if(ENV['LINUS_SERVER'] == nil)
        @calliope_client = CalliopeClient.new(false)
        @venue_url = @calliope_client.createVenue()
    else
        @calliope_client = CalliopeClient.new(true)
        @venue_url = SecureRandom.uuid()
    end
    @sdpOffer = []
    @linusUrl = []
	p "The venue URL is: #{@venue_url}"
end

Given(/^I have (\d+) (android|ios) (?:devices|device)$/) do |count,os|
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @devices = []
    if(os.eql?("android"))
        if(WX2Calabash.deviceman.count_android < count.to_i)
            raise "You MUST attach at least #{count} android devices for this scenario."
        end
        count.to_i.times do |i|
            @devices.push(WX2Calabash.deviceman.get_android_device(i))
            @devices[i].launch
        end
    else
        if(WX2Calabash.deviceman.count_ios < count.to_i)
            raise "You MUST attach at least #{count} ios devices for this scenario."
        end
        count.to_i.times do |i|
            @devices.push(WX2Calabash.deviceman.get_ios_device(i))
            @devices[i].launch
        end
    end
    #create_venue
end
When(/^I select a raw file$/) do
   @devices[0].backdoor("bdEnableExternalVideoTrack", nil)
   @devices[0].backdoor("bdSelectRawVideoFile", nil)
   @devices[0].backdoor("bdApplyExternalVideoTrack", nil)
end
Then(/^I start as the Host$/) do
    @ipaddr = @devices[0].backdoor("bdConnectAsHost", nil)
    p "ip address is:",@ipaddr
end
Then(/^I check the output box on the other device$/) do 
    @devices[1].backdoor("bdApplyVideoOutputFile", nil)
end
Then(/^I connect as a client on the other device$/) do 
    #@devices[1].backdoor("bdConnectAsClient", @ipaddr)
    @devices[1].backdoor("bdConnectAsClient", @ipaddr)
end

When(/^I select a raw audio file$/) do
  # @devices[0].backdoor("bdEnableExternalTrack", nil)
    @devices[0].backdoor("bdSelectAudioTab", nil)
    @devices[0].backdoor("bdSelectRawAudioFile", nil)
    @devices[0].backdoor("bdApplyAudioRawFile", nil)
end

Then(/^I change resolution in encoder param to 90P$/) do
    
    @host.backdoor("bdSelectVideoTab", nil)
    # "1" = 90P
    @host.backdoor("bdSelectResolution","1")
    sleep 3
end

Then(/^I change resolution in encoder param to 180P$/) do
    
    @host.backdoor("bdSelectVideoTab", nil)
     # "2" = 180P
    @host.backdoor("bdSelectResolution","2")
    sleep 3
end

Then(/^I change resolution in encoder param to 360P$/) do
    
    @host.backdoor("bdSelectVideoTab", nil)
     # "3" = 360P
    @host.backdoor("bdSelectResolution","3")
    sleep 3
end

Then(/^I check the audio output box on the other device$/) do 
    @devices[1].backdoor("bdApplyAudioOutputFile", nil)
end
Then(/^I check the audio output file$/) do 
    sleep 10
    ret = @devices[1].backdoor("bdCheckAudioOutputFile", nil)
    while ret == -1
        sleep 10
        ret = @devices[1].backdoor("bdCheckAudioOutputFile", nil)
    end
    if(ret == 0)
        raise "Did not pass the audio test"
    end
end

Given(/^I have 1 android and 1 ios$/) do
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @devices = []
    if(WX2Calabash.deviceman.count_android < 1 || WX2Calabash.deviceman.count_ios < 1)
        raise "You MUST attach at least 1 android and 1 ios device for this scenario."
    end
    
    @devices.push(WX2Calabash.deviceman.get_android_device(0))
    @devices[0].launch
    @devices.push(WX2Calabash.deviceman.get_ios_device(0))
    @devices[1].launch
    
    #create_venue
end

def cleanup_resources
    p "entering cleanup_resources"
    return unless(@linusUrl)
    @linusUrl.each do |url|
        p "delete confluence url: " + url
        @calliope_client.deleteLinusConfluence(url)
    end
    @linusUrl = nil

	@calliope_client.deleteVenue(@venue_url) unless(ENV['LINUS_SERVER'])
    @sdpOffer = nil
	@venue_url = nil
    @devices.each do |device|
        device.exitapp()
    end
    @devices = nil
end

When(/^I create offer on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @devices[idx].backdoor("backdoorStartCall", nil)
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    p "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

Then(/^I should get SDP from device (\d+) and create confluence$/) do |idxstr|
    idx = idxstr.to_i - 1
    raise "I didn't get the SDP from device #{idx}." if !@sdpOffer[idx]
    if(ENV['LINUS_SERVER'] == nil)
	    sdpAnswer = @calliope_client.createConfluence(@venue_url, @sdpOffer[idx])
    else
        sdpAnswer = @calliope_client.createLinusConfluence(@venue_url, @sdpOffer[idx])
    end
    p sdpAnswer
    @linusUrl[idx] = sdpAnswer[:confluenceUrl]
	@devices[idx].backdoor("backdoorSetRemoteSdp", sdpAnswer[:sdpAnswer])
end

Then(/^I keep the call for (\d+) seconds$/) do |time_out|
    p "waiting for #{time_out} seconds..."
    sleep time_out.to_i
end

def get_stats(device)
	statistics = device.backdoor("backdoorGetStatistics", nil)
    stats = JSON.parse(statistics)
    puts "device #{device.deviceID}:"
    puts JSON.pretty_generate(stats)
    return stats
end

def check_stats(stats)
    if(stats["videoLocal"]["fBitRate"] == 0 || 
	   stats["videoLocal"]["fFrameRate"] == 0 || 
	   stats["videoRemote"]["fBitRate"] == 0 || 
	   stats["videoRemote"]["fFrameRate"] == 0 || 
	   stats["audioLocal"]["uBitRate"] == 0 || 
	   stats["audioRemote"]["uBitRate"] == 0)
       return false
    end   
    return true
end

def compare_stats(stats1, stats2)

    if(stats2["videoInNet"]["uPackets"] > stats1["videoInNet"]["uPackets"] ||
	   stats2["audioInNet"]["uPackets"] > stats1["audioInNet"]["uPackets"])
       return true
    end
    return false
end

Then(/^I should have received and decoded some media in (?:devices|device)$/) do
    @devices.each do |device|
        raise "The statistics is abnormal for device #{device.deviceID}" unless(check_stats(get_stats(device)))
        device.backdoor("backdoorStopCall", nil)
    end
end


When(/^I start loopback call$/) do
    @devices[0].backdoor("backdoorStartLoopbackCall", ENV["LINUS_SERVER"])
end

And(/^I (disable|enable) wifi$/) do |op|
    p "#{op} wifi..."
    @devices[0].backdoor("backdoorChangeWifi", op)
end

Then(/^I wait for wifi enable$/) do
    while @devices[0].backdoor("backdoorIsWifiEnabled", nil) != "enabled" do
        p "waiting for wifi enabled"
        sleep 1
    end
    p "wifi is enabled"
    @stats_list = []
    @stats_list.push(get_stats(@devices[0]))
end

Then(/^I should continue to receive some media in device$/) do
    @stats_list.push(get_stats(@devices[0]))
    raise "The statistics is abnormal for device #{@devices[0].deviceID}" unless(check_stats(@stats_list[1]))
    raise "The packets is not increased after reconnect" unless(compare_stats(@stats_list[0], @stats_list[1]))
    @devices[0].backdoor("backdoorStopCall", nil)
end

Then(/^I rotate camera$/) do
    @devices[0].backdoor("backdoorRotateCamera", nil)
end


And (/^one (android|ios) device as the host, the other (android|ios) device as the client$/) do |host_device,client_device|
    
    if(host_device.eql?("android"))
        @host_ios_or_android = 1 #ios 0, android 1
    elsif(host_device.eql?("ios"))
        @host_ios_or_android = 0
    else
        p host_device
        raise "unknown host device"
    end
    
    if(client_device.eql?("android"))
        @client_ios_or_android = 1 #ios 0, android 1
    elsif(host_device.eql?("ios"))
        @client_ios_or_android = 0
    else
        p client_device
        raise "unknown client device"
    end
    
    #host
    if @host_ios_or_android == 0 
        @devices[1].backdoor("bdTouchTabBarItem", "1")
    end

    #client
    if @client_ios_or_android == 0
        @devices[0].backdoor("bdTouchTabBarItem", "1")
        @devices[0].backdoor("bdSetExtendParameter", "1")
        @devices[0].backdoor("bdTouchSegment", "scServerOrClient")
        @devices[0].backdoor("bdSetExtendParameter", @devices[1].ipaddr)
        @devices[0].backdoor("bdSetTextField", "tfServerIP")
        @devices[1].backdoor("bdTouchTabBarItem", "1")
    end 
        
    @host = @devices[1]
    @client = @devices[0]
end

Then(/^I change resolution in encoder param$/) do
    
    @host.backdoor("bdTouchUIControl", "mainView")
    sleep 1
    @host.backdoor("bdTouchBarButtonItem", "btSetting")
    sleep 1
    @host.backdoor("bdPerformSegue", "videoEncodingParametersSegue")
    sleep 1

    @host.backdoor("bdSetExtendParameter", "0")
    @host.backdoor("bdTouchTableRow", "1")
    sleep 1
    @host.backdoor("bdTouchBarButtonItem", "btBack")
    sleep 10

    @host.backdoor("bdPerformSegue", "videoEncodingParametersSegue")
    
    @host.backdoor("bdSetExtendParameter", "0")
    @host.backdoor("bdTouchTableRow", "2")
    sleep 1
    @host.backdoor("bdTouchBarButtonItem", "btBack")
    sleep 10
    
    @host.backdoor("bdPerformSegue", "videoEncodingParametersSegue")
    
    @host.backdoor("bdSetExtendParameter", "0")
    @host.backdoor("bdTouchTableRow", "3")
    sleep 1
    @host.backdoor("bdTouchBarButtonItem", "btBack")
    sleep 10

end


And (/^set the (input|output) (video|audio) file of the (host|client) to (.*)$/) do |inputOrOutput,videoOrAudio,hostOrClient,fileName|
 
    if(hostOrClient.eql?("host"))
        target = @host
    else
        target = @client
    end
    
    if @host_ios_or_android == 0
		target.backdoor("bdPerformSegue", "PreMeetingSetting")
	end
    @inputFileName = fileName
	if(inputOrOutput.eql?("input"))
		if @host_ios_or_android == 0
            selectedFileName = ""
            rowNum = 0
            while selectedFileName != fileName
                if(videoOrAudio.eql?("video"))
                    #select the first video file
                    target.backdoor("bdPerformSegue", "VideoFileExplorer")
                    sleep 1
                    target.backdoor("bdSetExtendParameter", "0")
                    target.backdoor("bdTouchTableRow", rowNum.to_s)
                    selectedFileName = target.backdoor("bdGetTextField", "videoFileName")
                else
                    #select the first audio file
                    target.backdoor("bdPerformSegue", "AudioFileExplorer")
                    sleep 1
                    target.backdoor("bdSetExtendParameter", "0")
                    target.backdoor("bdTouchTableRow", rowNum.to_s)
                    selectedFileName = target.backdoor("bdGetTextField", "audioFileName")
                end
                rowNum += 1;
            end

			if(selectedFileName != fileName)
				raise "the input file setting is wrong!"
			end
		elsif @host_ios_or_android == 1
            p "select audio file is", fileName
			if(videoOrAudio.eql?("video"))
				#select the first video file
				target.backdoor("bdEnableExternalVideoTrack", nil)
   				target.backdoor("bdSelectRawVideoFile", @inputFileName)
   				target.backdoor("bdApplyExternalVideoTrack", nil)
			else
				target.backdoor("bdSelectAudioTab", nil)
    			target.backdoor("bdSelectRawAudioFile", fileName)
    			target.backdoor("bdApplyAudioRawFile", nil)
				
			end
		else
			raise "unknown device type"
		end

	else
		if @client_ios_or_android == 0
			@outputFileName = fileName
			if(videoOrAudio.eql?("video"))
				target.backdoor("bdSetExtendParameter", fileName)
				target.backdoor("bdSetTextField", "outputVideoFileName")
			else
				target.backdoor("bdSetExtendParameter", fileName)
				target.backdoor("bdSetTextField", "outputAudioFileName")
			end
		elsif @client_ios_or_android == 1
			if(videoOrAudio.eql?("video"))
				target.backdoor("bdApplyVideoOutputFile", nil)
			else
				target.backdoor("bdApplyAudioOutputFile", nil)
			end
		else
			raise "unknown device type"
		end
	end
    if @client_ios_or_android == 0
        sleep 1
        target.backdoor("bdTouchBarButtonItem", "doneBarBtn")
        sleep 1
    end
end

Then (/^set up the connection$/) do
	if @host_ios_or_android == 0
    	@host.backdoor("bdTouchButton", "btStartServer")
    else
    	@ipaddr = @host.backdoor("bdConnectAsHost", nil)
    	p "ip address is:",@ipaddr
    end
    
    if @client_ios_or_android == 0
    	@client.backdoor("bdTouchButton", "btConnect")
    	sleep 3
        #close the alert successful window
        @host.backdoor("bdTouchAlertView", "0")
        @host.backdoor("bdTouchAlertView", "0")
    	@client.backdoor("bdTouchAlertView", "0")
    	@client.backdoor("bdTouchAlertView", "0")
    else
    	@client.backdoor("bdConnectAsClient", @ipaddr)
    end
end

Then (/^check the output (video|audio) file of the (host|client) with (.*)$/) do |videoOrAudio, hostOrClient, filename|
    #pending
    if(hostOrClient.eql?("host"))
        target = @host
    else
        target = @client
    end
    
    if @client_ios_or_android == 0
		#waiting for the file out is completed
		title = target.backdoor("bdGetAlertViewTitle", nil)
		while title != "Disconnect"
			title = target.backdoor("bdGetAlertViewTitle", nil)
		end
        target.backdoor("bdTouchAlertView", "0")
        target.backdoor("bdPerformSegue", "unwindSegueStopPlay")
        #check the output file
		target.backdoor("bdSetExtendParameter", @inputFileName)
		ret = target.backdoor("bdCheckQulityOfAudioFile", filename)
	    p "audio quality level:", ret
        fRet = @ret.to_f
        if(fRet < 3.5)
            raise "Did not pass the audio test"
        end
	else
		#sleep 10
        p "check audio file output", videoOrAudio, filename
		if(videoOrAudio.eql?("audio"))
			@ret = @client.backdoor("bdCheckAudioOutputFile", filename)
            p "bdCheckAudioOutputFile", @ret
            fRet = @ret.to_f
			while @ret.eql?("-1")
				sleep 10
				ret = @devices[1].backdoor("bdCheckAudioOutputFile", filename)
			end
			if(fRet < 3.5)
				raise "Did not pass the audio test"
			end
		else
		end
	end
end

Then (/^I cleanup resources$/) do
    @device[0].exitapp
    @device[1].exitapp
end
Then (/^testing TA$/) do
    @devices[0].backdoor("bdTouchTabBarItem", "1")
    @devices[0].backdoor("bdTouchButton", "btStartServer")
    sleep 1
    result = @devices[0].backdoor("bdTouchAlertView", "0")
    
    if(result.eql?("successfulx"))
        raise "zz_error"
    end
    #@devices[0].backdoor("bdPerformSegue", "PreMeetingSetting")
    #sleep 1
    #@devices[0].backdoor("bdTouchBarButtonItem", "doneBarBtn")

end
