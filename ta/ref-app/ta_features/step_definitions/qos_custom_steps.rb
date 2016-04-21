Then(/^I clear network simulator rules$/) do
	raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
	clear_network_simulator()
end

Then(/^I set Video initial bandwidth as (\d+)kbps for device (\d+)$/) do |initBandWidth, idxstr|
    uInitBandWidth=initBandWidth.to_i*1024
    my_json = {:caller => true, :param => {:video => {:uInitBandWidth => uInitBandWidth}}}
    idx = idxstr.to_i - 1
    @devices[idx].backdoor("backdoorSetVideoParam", my_json.to_json)

end

And(/^Skip if video bit rate is too low$/) do
    for device in @devices
        stat = get_stats(device)
        bitRate = stat["videoOutNet"]["uBitRate"]/1024

        pending ("Device:#{device.deviceID} video bit rate is too low, just #{bitRate}kbps, skip this scenario.") if bitRate < 300

    end
end

Then(/^Video evaluated bandwidth should (\W+) (\d+)kbps$/) do |op, expectedBW|
    expectedBW = expectedBW.to_i
    for device in @devices
        stat = get_stats(device)
        actualBW = stat["videoOutNet"]["uEvaluatedBW"]/1024

        case op
            when "<="
                raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW > expectedBW
            when "<"
                raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW >= expectedBW
            when ">="
                raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW < expectedBW
            when ">"
                raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW <= expectedBW
            when "=="
                raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW != expectedBW
        end
    end
end

Then(/^Video resolution should (\W+) (\d+)p$/) do |op, resolution|
    reso = resolution.to_i
    for device in @devices
        stat = get_stats(device)
        localReso = stat["videoLocal"]["uWidth"]
        if stat["videoLocal"]["uHeight"] < localReso
            localReso = stat["videoLocal"]["uHeight"]
        end

        case op
            when "<="
                raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso > reso
            when "<"
                raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso >= reso
            when ">="
                raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso < reso
            when ">"
                raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso <= reso
            when "=="
                raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso != reso
        end
    end
end

Then(/^Video frame max buffer time should (\W+) (\d+)ms$/) do |op, bufferTime|
    expBuffTime = bufferTime.to_i
    for device in @devices
        stat = get_stats(device)
        localBuffTime = stat["videoOutNet"]["uMaxBufferedMs"]

        case op
            when "<="
                raise "Device:#{device.deviceID} video frame max buffer time expect #{op} #{expBuffTime}ms, actual #{localBuffTime}ms" if localBuffTime > expBuffTime
            when "<"
                raise "Device:#{device.deviceID} video frame max buffer time expect #{op} #{expBuffTime}ms, actual #{localBuffTime}ms" if localBuffTime >= expBuffTime
            when ">="
                raise "Device:#{device.deviceID} video frame max buffer time expect #{op} #{expBuffTime}ms, actual #{localBuffTime}ms" if localBuffTime < expBuffTime
            when ">"
                raise "Device:#{device.deviceID} video frame max buffer time expect #{op} #{expBuffTime}ms, actual #{localBuffTime}ms" if localBuffTime <= expBuffTime
            when "=="
                raise "Device:#{device.deviceID} video frame max buffer time expect #{op} #{expBuffTime}ms, actual #{localBuffTime}ms" if localBuffTime != expBuffTime
        end
    end
end

Then(/^Video smoothSend drop frame count should (\W+) (\d+)$/) do |op, count|
    expCnt = count.to_i
    for device in @devices
        stat = get_stats(device)
        dropCnt = stat["videoOutNet"]["uAdaptedFrames"]

        case op
            when "<="
                raise "Device:#{device.deviceID} smoothSend drop frame count expect #{op} #{expCnt}, actual #{dropCnt}" if dropCnt > expCnt
            when "<"
                raise "Device:#{device.deviceID} smoothSend drop frame count expect #{op} #{expCnt}, actual #{dropCnt}" if dropCnt >= expCnt
            when ">="
                raise "Device:#{device.deviceID} smoothSend drop frame count expect #{op} #{expCnt}, actual #{dropCnt}" if dropCnt < expCnt
            when ">"
                raise "Device:#{device.deviceID} smoothSend drop frame count expect #{op} #{expCnt}, actual #{dropCnt}" if dropCnt <= expCnt
            when "=="
                raise "Device:#{device.deviceID} smoothSend drop frame count expect #{op} #{expCnt}, actual #{dropCnt}" if dropCnt != expCnt
        end
    end
end

Then(/^Video resolution should (\W+) (\d+)p for device (\d+)$/) do |op, resolution, idxstr|
    reso = resolution.to_i
    idx = idxstr.to_i - 1
    device = @devices[idx]

	stat = get_stats(device)
	localReso = stat["videoLocal"]["uWidth"]
	if stat["videoLocal"]["uHeight"] < localReso
		localReso = stat["videoLocal"]["uHeight"]
	end

	case op
		when "<="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso > reso
		when "<"
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso >= reso
		when ">="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso < reso
		when ">"
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso <= reso
		when "=="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso != reso
	end
end

Then(/^Video evaluated bandwidth should (\W+) (\d+)kbps for device (\d+)$/) do |op, expectedBW, idxstr|
    expectedBW = expectedBW.to_i
    idx = idxstr.to_i - 1
    device = @devices[idx]
        
    stat = get_stats(device)
    actualBW = stat["videoOutNet"]["uEvaluatedBW"]/1024

    case op
        when "<="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW > expectedBW
        when "<"
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW >= expectedBW
        when ">="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW < expectedBW
        when ">"
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW <= expectedBW
        when "=="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW != expectedBW
    end
end

Then(/^Video should stop for the (pc|mobile|ios|android|winphone) device$/) do |os|
    device = get_device(os, 1)
    raise "Cannot find the #{os} device" if device == nil
    stat = get_stats(device)
    prev_bytes = stat["videoOutNet"]["uBytes"]
    sleep 2
    now_bytes = stat["videoOutNet"]["uBytes"]
    if prev_bytes != now_bytes
        raise "Local video expected to be muted when extremly bad network"
    end
end

Then(/^Video resolution should (\W+) (\d+)p for the (pc|mobile|ios|android|winphone) device$/) do |op, resolution, os|
    reso = resolution.to_i
    device = get_device(os, 1)
    raise "Cannot find the #{os} device" if device == nil

	stat = get_stats(device)
	localReso = stat["videoLocal"]["uWidth"]
	if stat["videoLocal"]["uHeight"] < localReso
		localReso = stat["videoLocal"]["uHeight"]
	end

	case op
		when "<="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso > reso
		when "<"
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso >= reso
		when ">="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso < reso
		when ">"
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso <= reso
		when "=="
			raise "Device:#{device.deviceID} local video resolution expect #{op} #{resolution}p, actual #{localReso}p" if localReso != reso
	end
end

Then(/^Video evaluated bandwidth should (\W+) (\d+)kbps for the (pc|mobile|ios|android|winphone) device$/) do |op, expectedBW, os|
    expectedBW = expectedBW.to_i
    device = get_device(os, 1)
    raise "Cannot find the #{os} device" if device == nil
       
    stat = get_stats(device)
    actualBW = stat["videoOutNet"]["uEvaluatedBW"]/1024

    case op
        when "<="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW > expectedBW
        when "<"
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW >= expectedBW
        when ">="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW < expectedBW
        when ">"
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW <= expectedBW
        when "=="
            raise "Device:#{device.deviceID} video evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW != expectedBW
    end
end

And(/^I setup(.*)network simulator with bw=(\d+)Kbps, qdelay=(\d+)ms and delay=(\d+)ms for the (pc|mobile|ios|android|winphone) device$/) do |direction, bw, qdelay, delay, os|
	raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    # device = get_device(os, 1)
    # raise "Cannot find the #{os} device" if device == nil
 
    # logfile=File.join(ENV["TEST_TRACE_PATH"], "#{WX2Calabash.deviceman.scenario_name}_#{WX2Calabash.deviceman.feature_name}_#{device.deviceID}_"+direction.delete(' ')+'_'+'simulator.log')
    # Note: network link conditioner only can limit network status in pc now, so device index is 0
    setup_network_simulator(0, direction, bw, qdelay, delay, nil, nil)
end

And(/^I setup(.*)network simulator with bw=(\d+)Kbps and qdelay=(\d+)ms for the (pc|mobile|ios|android|winphone) device$/) do |direction, bw, qdelay, os|
	raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    # device = get_device(os, 1)
    # raise "Cannot find the #{os} device" if device == nil
 
    # Note: network link conditioner only can limit network status in pc now, so device index is 0
    setup_network_simulator(0, direction, bw, qdelay, nil, nil)
end

And(/^I setup(.*)network simulator with delay=(\d+)ms for the (pc|mobile|ios|android|winphone) device$/) do |direction, delay, os|
	raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    # device = get_device(os, 1)
    # raise "Cannot find the #{os} device" if device == nil
 
    # Note: network link conditioner only can limit network status in pc now, so device index is 0
    setup_network_simulator(0, direction, nil, nil, delay, nil)
end

And(/^I setup(.*)network simulator with loss rate=(\d+)% for the (pc|mobile|ios|android|winphone) device$/) do |direction, lossrate, os|
	raise "Your OS doesn't support this case." if (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

    # device = get_device(os, 1)
    # raise "Cannot find the #{os} device" if device == nil
 
    lossrate=lossrate.to_f/100
    # Note: network link conditioner only can limit network status in pc now, so device index is 0
    setup_network_simulator(0, direction, nil, nil, nil, lossrate)
end

And(/^I set mari rate adaptation policy as (.*)$/) do |policy|

    rate_adaptation_policy = "{\"media-mari-rate-adaptation\": \"#{policy}\"}"
    p "set mari rate adaptation policy as #{rate_adaptation_policy}"
    @devices.each do |device|
        device.backdoor("backdoorSetFeatureToggles", rate_adaptation_policy)
    end
end

And(/^I set packet loss raw data recording policy as allowing maximum (\d+) bytes to be recorded$/) do |bytes|

    if bytes != "0"
    	policy = "{\"media-packet-loss-recording-audio-session\": true, \"media-packet-loss-recording-video-session\": true}"
    else
    	policy = "{\"media-packet-loss-recording-audio-session\": false, \"media-packet-loss-recording-video-session\": false}"
    end

    p "set packet loss raw data recording policy as #{policy}"
    @devices.each do |device|
        device.backdoor("backdoorSetFeatureToggles", policy)
    end
end


Then(/^I set the estimated bandwidth to (\d+) kbps mannually for device (\d+)$/) do |bitrate_kbps, device_idx|
    idx = device_idx.to_i - 1
    @devices[idx].backdoor("backdoorSetManualBandwidth", bitrate_kbps);
end

Then(/^I (\w+) wifi monitor$/) do |action|
    if action == 'start'
        pid = spawn("./scripts/wifi/WifiMonitor en0")
        Process.detach(pid)
    else
        system("killall WifiMonitor")
        trace_path = File.join(ENV["TEST_TRACE_PATH"], "wifi_monitor.txt")
        FileUtils.cp("/tmp/wifi_monitor.txt", trace_path)
    end
end
