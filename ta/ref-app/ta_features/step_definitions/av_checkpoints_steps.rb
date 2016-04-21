
def check_media_received(is_overrideip, params = nil)
    idx = 0
    stats_check = []
    @devices.each do |device|
        Kernel.puts(device.deviceID)
        stats = get_stats(device)
        stats_check.push(stats)
    end
    stats_check.each do |stats|
        Kernel.puts("check_media_received, #{idx}")
        check_stats_all(stats, params)
        if(is_overrideip)
            if(@fakeAnswer)
                overrideip = @fakeAnswer[0]["ip"]
            else
                other = (idx == 0) ? 1 : 0
                overrideip = @devices[other].ipaddr
            end
            Kernel.puts(":::::::::::#{overrideip}")
            assert(stats["videoConn"]["remoteIp"] == overrideip, "video override ip is not correct")
            assert(stats["audioConn"]["remoteIp"] == overrideip, "audio override ip is not correct")
        end
        idx = idx + 1
    end
end

Then(/^I should have decoded some media in (?:devices|device) with:$/) do |table|
    params = {}
    table.raw.each do |row|
        type = row[0]
        key = row[1]
        value = row[2]
        params[type] = {} unless(params[type])
        params[type][key] = value
    end
    check_media_received(false, params)
end

Then(/^I should have decoded some media in (?:devices|device) with (.*) (.*) (.*)$/) do |name, cmpFunc, refValue|
    params = {}
    params[name] = {}
    params[name]["cmpFunc"] = cmpFunc
    params[name]["refValue"] = refValue
    check_media_received(false, params)
end

Then(/^Device (\d+) should have decoded some media with (.*) (.*) (.*)$/) do |deviceIdx, name, cmpFunc, refValue|
    i = deviceIdx.to_i - 1
    stats = get_stats(@devices[i])
    params = {}
    params[name] = {}
    params[name]["cmpFunc"] = cmpFunc
    params[name]["refValue"] = refValue
    check_decoded_video(stats, params)
end

Then(/^Device (\d+) should have encoded with (.*) (.*) (.*)$/) do |deviceIdx, name, cmpFunc, refValue|
    i = deviceIdx.to_i - 1
    stats = get_stats(@devices[i])
    params = {}
    params[name] = {}
    params[name]["cmpFunc"] = cmpFunc
    params[name]["refValue"] = refValue
    check_stats_all(stats, params)
end

Then(/^I should have received and decoded some media in (?:devices|device)(.*)$/) do |condition|
    is_overrideip = condition.include? "overrideip"
    check_media_received(is_overrideip)
end


def check_stats_encoded(stats, type)
    p "check_stats_encoded"
    if(type == "audio")
        assert(stats["audioLocal"]["uBitRate"] >= 1000, "audio local bitrate smaller than 1Kbps")
        assert(stats["audioLocal"]["uBitRate"] != 0, "audio local bitrate zero")
    end
    if(type == "video")
        assert(stats["videoLocal"]["fBitRate"] != 0, "video local bitrate zero")
        assert(stats["videoLocal"]["uEncodeFrameCount"] != 0, "video local encoded frame count zero")
    end
    if(type == "media")
        assert(stats["audioLocal"]["uBitRate"] != 0, "audio local bitrate zero")
        assert(stats["videoLocal"]["fBitRate"] != 0, "video local bitrate zero")
        assert(stats["videoLocal"]["uEncodeFrameCount"] != 0, "video local encoded frame count zero")
    end
end

def check_stats_decoded(stats, type)
    p "check_stats_decoded"
    if(type == "audio")
        assert(stats["audioRemote"]["uBitRate"] >= 1000, "audio remote bitrate smaller than 1Kbps")    
    end
    if(type == "video")
        assert(stats["videoRemote"]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
    end
    if(type == "media")
        assert(stats["audioRemote"]["uBitRate"] != 0, "audio remote bitrate zero")
        assert(stats["videoRemote"]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
    end
end

def check_stats(stats, direction, type)
    p "check_stats"
    if(direction == "encoded")
         check_stats_encoded(stats, type)
    end
    if(direction == "decoded")
         check_stats_decoded(stats, type)
    end
    if(direction == "encoded and decoded")
         check_stats_encoded(stats, type)
         check_stats_decoded(stats, type)
    end
end

def get_stats(device)
    statistics = device.backdoor("backdoorGetStatistics", nil)
    Kernel.puts statistics
    stats = JSON.parse(statistics)
    puts "device #{device.deviceID}:"
    #Kernel.puts JSON.pretty_generate(stats)
    return stats
end


def compare_value(value1, value2, cmpFunc)
    if(cmpFunc.eql?("=") || cmpFunc.eql?("=="))
        assert(value1 == value2) 
    end
    if(cmpFunc.eql?(">"))
        assert(value1 > value2) 
    end
    if(cmpFunc.eql?("<"))
        assert(value1 < value2) 
    end
    if(cmpFunc.eql?(">="))
        assert(value1 >= value2) 
    end
    if(cmpFunc.eql?("<="))
        assert(value1 <= value2) 
    end
end

def check_stats_all(stats, params = nil)
    assert(stats["videoLocal"]["fAvgBitRate"] != 0, "video local average bitrate zero")
    assert(stats["videoLocal"]["uEncodeFrameCount"] != 0, "video local encoded frame count zero")
    assert(stats["videoRemote"]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
    unless (params && params["audio"] && params["audio"]["isSvs"])
        assert(stats["audioLocal"]["uBitRate"] != 0, "audio local bitrate zero")
    end    
    assert(stats["audioRemote"]["uBitRate"] != 0, "audio remote bitrate zero")
    if(stats["videoTracks"])
        Kernel.print "check_stats_all, has videoTracks"
        stats["videoTracks"].size.times do |i|
            assert(stats["videoTracks"][i]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
        end
    end
    if(params)
        if(params["video"])
            video_params = params["video"]
            if(video_params["pixels"])
                pixels = video_params["pixels"].to_i
                assert(stats["videoRemote"]["uWidth"].to_i * stats["videoRemote"]["uHeight"].to_i >= pixels) 
            end
            
            if(video_params["fps"])
                fps = video_params["fps"].to_i
                assert(stats["videoRemote"]["fFrameRate"].to_f >= fps);
            end
        end
         if(params["videoPixels"])
            video_params = params["videoPixels"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_i
            videoRemotePixels = stats["videoRemote"]["uWidth"].to_i * stats["videoRemote"]["uHeight"].to_i
            compare_value(videoRemotePixels, refValue, cmpFunc)
        end
        if(params["videoFPS"])
            video_params = params["videoFPS"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_f
            fps = stats["videoRemote"]["fFrameRate"].to_f 
            compare_value(fps, refValue, cmpFunc)
       end
        if (params["encodedLayers"])
            video_params = params["encodedLayers"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_i
            numLayers = stats["videoLocal"]["uStreamNum"].to_i
            compare_value(numLayers, refValue, cmpFunc)
            
        else
            
        end

    end
end
def check_decoded_video(stats, params = nil)
    assert(stats["videoRemote"]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
    if(stats["videoTracks"])
        Kernel.print "check_stats_all, has videoTracks"
        stats["videoTracks"].size.times do |i|
            assert(stats["videoTracks"][i]["uRenderFrameCount"] != 0, "video remote rendered frame count zero")
        end
    end
    if(params)
        if(params["video"])
            video_params = params["video"]
            if(video_params["pixels"])
                pixels = video_params["pixels"].to_i
                assert(stats["videoRemote"]["uWidth"].to_i * stats["videoRemote"]["uHeight"].to_i >= pixels) 
            end
            
            if(video_params["fps"])
                fps = video_params["fps"].to_i
                assert(stats["videoRemote"]["fFrameRate"].to_f >= fps);
            end
        end
         if(params["videoPixels"])
            video_params = params["videoPixels"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_i
            videoRemotePixels = stats["videoRemote"]["uWidth"].to_i * stats["videoRemote"]["uHeight"].to_i
            compare_value(videoRemotePixels, refValue, cmpFunc)
        end
        if(params["videoFPS"])
            video_params = params["videoFPS"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_f
            fps = stats["videoRemote"]["fFrameRate"].to_f 
            compare_value(fps, refValue, cmpFunc)
       end
        if (params["encodedLayers"])
            video_params = params["encodedLayers"]
            cmpFunc = video_params["cmpFunc"]
            refValue = video_params["refValue"].to_i
            numLayers = stats["videoLocal"]["uStreamNum"].to_i
            compare_value(numLayers, refValue, cmpFunc)
            
        else
            
        end

    end
end
def compare_stats(stats1, stats2)
    assert(stats2["videoInNet"]["uPackets"] > stats1["videoInNet"]["uPackets"], "video no data received")
    assert(stats2["audioInNet"]["uPackets"] > stats1["audioInNet"]["uPackets"], "audio no data received")
end

Then(/^device (\d+) should have (encoded|decoded|encoded and decoded) some (media|audio|video)$/) do |idxstr, direction, type| #hess
    i = idxstr.to_i - 1
    stats = get_stats(@devices[i])
    check_stats(stats, direction, type)
end

Then(/^I should get participants count is (\d+) in (?:devices|device)$/) do |count|
    otherParticipants = count.to_i - 1
    @devices.each do |device|
        Kernel.puts(device.deviceID)
        strCount = device.backdoor("backdoorGetCSICount", "")
        p strCount
        assert(otherParticipants == strCount.to_i, "CSI count is not correct")
    end
end

Then(/^I should have active speaker of device (\d+) come from other devices$/) do |idxstr|
    i = idxstr.to_i - 1
    strActiveCSI = @devices[i].backdoor("backdoorGetVideoCSI", "recv");
    p strActiveCSI
    bSameCSI = false
    @devices.each do |device|
        if(device != @devices[i])
            strLocalCSI = device.backdoor("backdoorGetVideoCSI", "send")
            p strLocalCSI
            bSameCSI = ((strActiveCSI.to_i & 0xFFFFFF00) == (strLocalCSI.to_i & 0xFFFFFF00))
            break if bSameCSI
        end
    end
    assert(bSameCSI, "the active speaker CSI is wrong!")
end

Then(/^I should continue to receive some media in device$/) do
    @stats_list.push(get_stats(@devices[0]))
    Kernel.puts(@devices[0].deviceID)
    check_stats_all(@stats_list[1])
    compare_stats(@stats_list[0], @stats_list[1])
end

Then(/^I check the CPU metrics$/) do
    @devices.each do |device|
        ret = device.backdoor("backdoorStopCall", nil)
        Kernel.puts "Session Metrics: #{ret}"
    end	
end

Then(/^I check assertion is zero$/) do
    @devices.each do |device|
        device.backdoor("backdoorStopCall", nil)
        ret = device.backdoor("backdoorGetAssertionsCount", nil)
        Kernel.puts "\n#{device.deviceID} - Assertions: #{ret}"
        assert(ret.to_i == 0, "There are unexpected assertions.")
    end	
end

Then(/^I check CSI change history$/) do
    @devices.each do |device|
        ret = device.backdoor("backdoorGetCSIChangeHistory", nil)
        Kernel.puts "CSI change history on device #{device.deviceID}: #{ret}"
        csiHistory = JSON.parse(ret)
        assert(csiHistory["audio"].has_key?("0"), "audio doesn't have csi changed")
        assert(csiHistory["video"].has_key?("0"), "video doesn't have csi changed on vid 0")
        assert(csiHistory["video"].has_key?("1"), "video doesn't have csi changed on vid 1")
        #assert(csiHistory["video"]["0"].length > 1, "video csi changed less than twice")
        assert(csiHistory["audio"]["0"].length > 1 || csiHistory["audio"]["0"][0].length > 1, "audio csi count is les than 2")
    end	
end

Then(/^I check the sdp has (.*)$/) do |max_fs|
    @devices.each do |device|
        local_sdp = device.backdoor("backdoorGetLocalSdp", nil)
        Kernel.puts local_sdp
        assert(local_sdp.match(max_fs) != nil, "local SDP failed to find: #{max_fs}")
    end
end

Then(/^I check the mari rate adaptation policy is (.*)$/) do |policy|
    @devices.each do |device|
        ret = device.backdoor("backdoorGetFeatureToggles", nil)
        Kernel.puts "policy is #{ret}"
        local_policy = JSON.parse(ret)
        assert(local_policy["featureToggles"].has_key?("media-mari-rate-adaptation"), "policy don't have key named media-mari-rate-adaptation")
        assert((local_policy["featureToggles"]["media-mari-rate-adaptation"] == "#{policy}"), "error mari rate adaptation policy,expect policy is #{policy}, actual policy is #{local_policy["featureToggles"]["mari-rate-adaptation"]}")
    end
end

Then(/^I check the packet loss raw data recording policy is allowing maximun (\d+) bytes to be recorded$/) do |bytes|
    @devices.each do |device|
        if bytes != "0"
        toggle = true
    else
        toggle = false
    end

        ret = device.backdoor("backdoorGetFeatureToggles", nil)
        Kernel.puts "feature toggles policy are #{ret}."	
        local_policy = JSON.parse(ret)

        key_audio = "media-packet-loss-recording-audio-session"
        key_video = "media-packet-loss-recording-video-session"

    if local_policy["featureToggles"].has_key?(key_audio)
        assert((local_policy["featureToggles"][key_audio] == toggle), "error expect and actual policy in rtp audio session don't match.")
            Kernel.puts "media audio session matches."
    end
    
    if local_policy["featureToggles"].has_key?(key_video)
        assert((local_policy["featureToggles"][key_video] == toggle), "error expect and actual policy in rtp video session don't match.")
            Kernel.puts "media video session matches."
    end
    end
end



