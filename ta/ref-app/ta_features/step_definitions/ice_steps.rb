#include Test::Unit::Assertions

def get_option_from_setting(setting)
    option = {};
    option["audio"] = setting['AUDIO'].to_bool
    option["video"] = setting['VIDEO'].to_bool
    option["share"] = setting['SHARE']
    option["share_mline_fake"] = setting['SHARE_MLINE_FAKE'].to_bool
	
	return option
end

When(/^I create (.*)offer on device (\d+) with the default setting and:$/) do |offerType, idxstr, table|
    default_setting = {
        'ICE' => "false",
        'RTCPMUX' => "true",
        'SRTP' => "false",
        'AUDIO' => "true",
        'VIDEO' => "true",
        'SHARE' => "",
        'SHARE_MLINE_FAKE' => "false",
        'SOURCE' => "",
        'SHARE_SOURCE_FILE' => "",
        'VIDEO_SOURCE_FILE' => "",
        'AUDIO_SOURCE_FILE' => "",
        'VIDEO_STREAMS' => "1"
    }
    if(table)
        setting = default_setting.merge(table.rows_hash)
    else
        setting = default_setting
    end
    isIce = setting['ICE'].to_bool
    isMux = setting['RTCPMUX'].to_bool
    isSrtp = setting['SRTP'].to_bool
    
	option = get_option_from_setting(setting)
    idx = idxstr.to_i - 1
    isP2p = offerType.include? "p2p"
    isMultilayer = offerType.include? "multilayer"

    if offerType.include? "disableQos"
        isEnableQos = false
    else
        isEnableQos = true
    end

    is_file = offerType.include? "file"
    is_loopfile = false
    if offerType.include? "loopFile"
        is_file = true
        is_loopfile = true
    end

    performanceTraceDump = "Unknown"
    if  offerType.include? "DumpPerformanceTrace"
        performanceTraceDump = "All"
    end

    rs = 2
    class_name = @devices[idx].class.name
    if(is_file)
        rs = set_capture_file(offerType, class_name, idx)
    end

    my_json = {:loopback => false, :filemode => is_file, :isLoopFile => is_loopfile, :p2p => isP2p, :multilayer => isMultilayer, 
        :ice => isIce, :mux => isMux, :srtp => isSrtp, :enablePerformanceStatDump => performanceTraceDump, :option => option, :sharesource => setting['SOURCE'], 
        :sharesource_file => setting['SHARE_SOURCE_FILE'], 
        :videosouce_file => setting['VIDEO_SOURCE_FILE'], 
        :audiosouce_file => setting['AUDIO_SOURCE_FILE'], 
        :enableQos => isEnableQos,
        :videoStreams => setting['VIDEO_STREAMS'].to_i,
        :resolution => rs}
    
    p my_json
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

And(/^I set (.*)offer to device (\d+) with the default setting and:$/) do |offerType, idxstr, table|
    default_setting = {
        'ICE' => "false",
        'RTCPMUX' => "true",
        'SRTP' => "false",
        'AUDIO' => "true",
        'VIDEO' => "true",
        'SHARE' => "",
        'SHARE_MLINE_FAKE' => "false",
        'SOURCE' => "",
        'SHARE_SOURCE_FILE' => "",
        'VIDEO_SOURCE_FILE' => "",
        'AUDIO_SOURCE_FILE' => "",
        'SDP_IDX' => "1"
    }
    if(table)
        setting = default_setting.merge(table.rows_hash)
    else
        setting = default_setting
    end
    isIce = setting['ICE'].to_bool
    isMux = setting['RTCPMUX'].to_bool
    isSrtp = setting['SRTP'].to_bool
    sdpIdx = setting['SDP_IDX'].to_i - 1
    option = get_option_from_setting(setting)
    
    idx = idxstr.to_i - 1
    
    is_file = offerType.include? "file"
    is_loopfile = false
    if offerType.include? "loopFile"
        is_file = true
        is_loopfile = true
    end

    performanceTraceDump = "Unknown"
    if  offerType.include? "DumpPerformanceTrace"
        performanceTraceDump = "All"
    end

    rs = 2
    class_name = @devices[idx].class.name
    if(is_file)
        rs = set_capture_file(offerType, class_name, idx)
    end

    my_json = {:loopback => false, :filemode => is_file, :isLoopFile => is_loopfile, :p2p => true, :multilayer => false, 
        :ice => isIce, :mux => isMux, :srtp => isSrtp,
        :enablePerformanceStatDump => performanceTraceDump, 
        :sdp => @sdpOffer[sdpIdx], :option => option, :sharesource => setting['SOURCE'], 
        :sharesource_file => setting['SHARE_SOURCE_FILE'], 
        :videosouce_file => setting['VIDEO_SOURCE_FILE'], 
        :audiosouce_file => setting['AUDIO_SOURCE_FILE'],
        :resolution => rs}
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

Then(/^I should receive answer from device (\d+) and set to device (\d+)$/) do |idx1, idx2|
    idxdst = idx2.to_i - 1
    idxsrc = idx1.to_i - 1
    @devices[idxdst].backdoor("backdoorSetRemoteSdp", @sdpOffer[idxsrc])
end

def check_stuntrace_ended(device, timeout = 200)
	
	i = 0
	p "Stuntrace starts."
	bSuccess = false;

	#sleep 10

	while (i < timeout)

		ret = device.backdoor("backdoorGetStunTraceResult", "")
		if(ret != "failed")
    		jsonRet = JSON.parse(ret)
			bSuccess = true
			p "StunTrace has returned with:"
			p jsonRet
			p "The destination is " + jsonRet["dest"]
			p "The result is " + jsonRet["ret"]
			#sz = jsonRet["trace"].size
			#assert(sz > 0, "[FAILED] - The trace string is empty")
			#p "The last node is " + jsonRet["trace"][sz-1]["type"] + " : " + jsonRet["trace"][sz-1]["ip"]
			break
		else
			p "Waiting for stuntrace to return..."
		end

		sleep 1
		i = i + 1
	end

	assert(bSuccess, "[FAILED] - No stuntrace results received.")
end

Then(/^I wait out the stuntrace on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
	p "Test stuntrace on device"
	check_stuntrace_ended(@devices[i])
end

def check_traceserver_ended(device, timeout = 200)
	
	i = 0
	p "TraceServer TA starts."
	bSuccess = false;

	while (i < timeout)

		ret = device.backdoor("backdoorGetTraceServerResult", "")
		if(ret != "failed")
    		jsonRet = JSON.parse(ret)
			bSuccess = true
			p "TraceServer has returned with:"
			p jsonRet
			break
		else
			p "Waiting for trace server to return..."
		end

		sleep 1
		i = i + 1
	end

	assert(bSuccess, "[FAILED] - No traceserver results received.")
end

Then(/^I wait out the traceserver on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
	p "Test traceserver on device"
	check_traceserver_ended(@devices[i])
end
