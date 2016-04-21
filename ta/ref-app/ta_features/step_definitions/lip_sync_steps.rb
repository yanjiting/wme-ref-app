Then (/^I mute (.*) in (?:devices|device)$/) do |session|
	@devices.each do |device|
    	p "mute #{session} in #{device.deviceID}."
    	if(session.eql?("audio") || session.eql?("video"))
    		# mute local audio or video
    		my_json = {:type => session, :mute => true, :speaker => true}
    		device.backdoor("backdoorMuteUnMute", my_json.to_json)
    		# mute remote audio or video
    	#	my_json = {:type => session, :mute => true, :speaker => false}
    	#	device.backdoor("backdoorMuteUnMute", my_json.to_json)
    	else
    		raise "The session is not supported yet."
    	end
    end
end

Then (/^I unmute (.*) in (?:devices|device)$/) do |session|
	@devices.each do |device|
    	p "unmute #{session} in #{device.deviceID}."
    	if(session.eql?("audio") || session.eql?("video"))
    		# unmute local audio or video
    		my_json = {:type => session, :mute => false, :speaker => true}
    		device.backdoor("backdoorMuteUnMute", my_json.to_json)
    		# unmute remote audio or video
    	#	my_json = {:type => session, :mute => false, :speaker => false}
    	#	device.backdoor("backdoorMuteUnMute", my_json.to_json)
    	else
    		raise "The session is not supported yet."
    	end
    end
end

def IsMobileDevice(device)
    class_name = device.class.name
    if( class_name.eql?("WX2Calabash::AdroidDevice") )
        return true
    elsif ( class_name.eql?("WX2Calabash::IOSDevice") )
        return true
    else
        return false
    end
end
 

def ConfigNetworkSimulator(session, device, bw, qdelay, delay, loss)
    raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    ip = nil
    port = nil
    if( session.eql?("audio") )
        ip, port = get_remote_audio_ip_port(device)
    elsif ( session.eql?("video") )
        ip, port = get_remote_video_ip_port(device)
    elsif ( session.eql?("any") )
        ip, port = get_remote_video_ip_port(device)
        port = nil
    else 
        raise "Unknown Session #{session}"
    end

    p "AVSYNC:  config network: ip = #{ip}, port = #{port}"

    if( @network_rules == nil )
        @network_rules = Array.new
    end
    rule_idx, pipe_idx = Dummynet.add("ip", "any", nil, ip, port)
    Dummynet.config(pipe_idx, bw, qdelay, delay, loss)
    @network_rules.push([pipe_idx, rule_idx])
end 

def ClearNetworkSimulator()
    raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    if (@network_rules != nil)
        for idxs in @network_rules
            p "Remove network simulator rules #{idxs[0]} fail" if !Dummynet.del(idxs[1])
    	end
    end
end

Then (/^I clean network rules$/) do
    ClearNetworkSimulator()
end

Then (/^I delay (.*) with (\d+) milliseconds in (?:devices|device)$/) do |session, delay_time|
    @devices.each do |device|
	    p "delay #{session} with #{delay_time} milliseconds in #{device.deviceID}."
        if( IsMobileDevice(device) )
            p "find mobile device: #{device.deviceID}"                
            ConfigNetworkSimulator(session, device, nil, nil, delay_time, nil)
        end
    end
end

Then (/^I make a (.*) jitter with (\d+) seconds in (?:devices|device)$/) do |session, jitter_time|
    @devices.each do |device|
	    p "make a #{session} jitter with #{jitter_time} seconds in #{device.deviceID}."
        if( IsMobileDevice(device) )
            p "find mobile device: #{device.deviceID}"
            p "Jitter is not support by network simulator"
        end
    end
end

Then (/^I make a (.*) loss with (\d+) precent in (?:devices|device)$/) do |session, loss_rate|
    @devices.each do |device|
	    p "make a #{session} loss with #{loss_rate} precent in #{device.deviceID}."
        if( IsMobileDevice(device) )
            p "find mobile device: #{device.deviceID}"
            ConfigNetworkSimulator(session, device, nil, nil, nil, loss_rate.to_i/100.0)
        end
    end
end

Then (/^I limit (\d+) Kbps in (?:devices|device)$/) do |bandwidth|
    @devices.each do |device|
	    p "limit bandwidth with #{bandwidth} Kbps in #{device.deviceID}."
        if( IsMobileDevice(device) )
            p "find mobile device: #{device.deviceID}"
            ConfigNetworkSimulator("any", device, bandwidth, nil, nil, nil)
        end
    end
end

def check_sync_result(result, rate, device)
	bPass = false;
	if(result.eql?("sync") || result.eql?("delay") || result.eql?("ahead") || result.eql?("nosync"))
		my_json = {:result => result, :rate => rate}
		status = device.backdoor("backdoorCheckSyncStatus", my_json.to_json)
		p "return #{status} with #{result} over #{rate} precent in #{device.deviceID}."
		if(status.eql?"pass")
			bPass = true;
		else
			p "ERROR: #{result} rate is samller than #{rate} in #{device.deviceID}."
		end
    else
    	raise "The result is not supported yet."
    end
    return bPass;
end

Then (/^I should get the lip sync result with (.*) over (\d+) precent in (?:devices|device)$/) do |result, rate|
	bCheckResult = true;
	@devices.each do |device|
		p "get the lip sync result with #{result} over #{rate} precent in #{device.deviceID}."

        bPass = check_sync_result(result, rate.to_i, device)
        bCheckResult &= bPass

    #    device.backdoor("backdoorStopCall", nil)
	end
	assert(bCheckResult == true, "the gotten lip sync results are not pass in some devices.")
end