def checkFileSize
  if(@devices)
    @devices.each do |device|
        begin
          device.getlog()
        rescue
        end
      end
  end
  
  Dir.chdir ENV["TEST_TRACE_PATH"]
  p "Logs available:"
  p Dir.glob("*")
  logArray = Array.new

  Dir.glob("*") do |fileName| 
    logArray.push(fileName)
  end

  logArray
end

def exhaustCPUUsage(status, os, cores)
    if (status == "true")
        if (os == "win")
            app = expand_local_path("../../../../vendor/cpuGrabEx/CPUGrabEx.exe")
            cmd = "start #{app} /u 100 /s"
            pid = spawn (cmd)
        elsif (os == "mac")
            cmd = ""
            cores.to_i.times do
                cmd.concat("yes > /dev/null & ")
            end
        
            print "Execute this command = "
            p cmd
            pid = spawn (cmd)
            Process.detach pid
        end
    elsif (status == "false")
        if (os == "win")
            cmd = "taskkill /IM CPUGrabEx.exe"
            spawn(cmd)
            
        elsif (os == "mac")
            pid = spawn ("killall yes")
            Process.detach pid
        end
    end
end
        

def check_as_dummy(count, os)
    if(os.eql?("pc"))
        if((RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil)
            os = "windows"
        else
            os = "mac"
        end
    end
    
    if(os.eql?("mac"))
        count.to_i.times do |i|
            @asDummyApps.push(WX2Calabash.deviceman.get_mac_as_dummy_device(@asDummyApps.size))
            @asDummyApps[@asDummyApps.size-1].launch
        end
    elsif(os.eql?("windows"))
        count.to_i.times do |i|
            @asDummyApps.push(WX2Calabash.deviceman.get_win_as_dummy_device(@asDummyApps.size))
            @asDummyApps[@asDummyApps.size-1].launch
        end
    else
        raise "The os is not supported yet."
    end
end

Given(/^I have (\d+) (.*) screenshare dummy (?:apps|app)$/) do |count,os|
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @asDummyApps = []
    check_as_dummy(count, os)
end

Given(/^I create an override venue with (.*)$/) do |ext|
    raise "You cannot run override linus case without set linus address" if !ENV['LINUS_SERVER']
    @override_calliope_client = CalliopeClient.new(true)
    @override_venue_url = SecureRandom.uuid()
    Kernel.puts("The override venue URL is: #{@override_venue_url}")
    fakeOffer = []
    sdpTemplate = File.read(expand_local_path("../../sample/sdp.#{ext}.template"))
    fakeOffer.push(sdpTemplate.dup)
    fakeOffer.push(sdpTemplate.dup)
    
    port_begin = 23232
    fakeOffer.each do |offer|
        offer.gsub!(/<AUDIO_PORT>/, "#{port_begin}")
        offer.gsub!(/<VIDEO_PORT>/, "#{port_begin + 2}")
        offer.gsub!(/<IP_ADDR>/, "192.168.1.100")
        port_begin = port_begin + 4
    end
    @fakeAnswer = []
    fakeOffer.each do |offer|
        sdpAnswer = @override_calliope_client.createLinusConfluence(@override_venue_url, offer)
        sdpAnswer.merge!(extract_sdp_port(sdpAnswer[:sdpAnswer]))
        Kernel.puts(sdpAnswer)
        @fakeAnswer.push(sdpAnswer)
    end
end

def create_venue()
    if(@linus_server_url == nil)
        @calliope_client = CalliopeClient.new(false)
        @venue_url = @calliope_client.createVenue()
    else
        @calliope_client = CalliopeClient.new(true)
        @venue_url = SecureRandom.uuid()
    end
    @linusUrl = []
    @requestFloorUrl = []
    @releaseFloorUrl = []
    @sdpOffer = []
    @override_calliope_client = nil
    @override_venue_url = nil
    p "The venue URL is: #{@venue_url}"
end

Then(/^I recreate a venue$/) do
    if(@linusUrl)
        @linusUrl.each do |url|
            next if(url == nil)
            p "delete confluence url: " + url
            @calliope_client.deleteLinusConfluence(url)
        end
        @sdpOffer = nil
        @venue_url = nil
    end
    create_venue
end

Given(/^I create a default venue for mari data collection test call$/) do
    @linus_server_url = ENV['LINUS_SERVER']
    @calliope_client = CalliopeClient.new(true)
    @venue_url = "mari_data_collection_venue_uuid"
    @linusUrl = []
    @requestFloorUrl = []
    @releaseFloorUrl = []
    @sdpOffer = []
    @override_calliope_client = nil
    @override_venue_url = nil
    p "The venue URL is: #{@venue_url}"
end

Given(/^I create a venue(.*)$/) do |extra|
    @linus_server_url = ENV['LINUS_SERVER']
    @linus_server_url = nil if(@linus_server_url == "" || @linus_server_url == "null" || @linus_server_url == "nil")
    @linus_server_url = nil if(extra.include?("production"))
    
    create_venue()
end

def cleanup_resources
    p "entering cleanup_resources"
    
    if (@os != nil && @cores != nil)
        exhaustCPUUsage("false", @os, @cores) # Release CPU Usage software in case tests fails
    end
    
    if(@linusUrl)
        @linusUrl.each do |url|
            next if(url == nil)
            p "delete confluence url: " + url
            @calliope_client.deleteLinusConfluence(url)
        end
        @calliope_client.deleteVenue(@venue_url) unless(@linus_server_url)
        @sdpOffer = nil
        @venue_url = nil
    end
    @linusUrl = nil
    if(@override_venue_url && @override_calliope_client)
        @fakeAnswer.each do |sdp_answer|
            next if(sdp_answer == nil || sdp_answer[:confluenceUrl] == nil)
            p "delete override confluence url: " + sdp_answer[:confluenceUrl]
            @override_calliope_client.deleteLinusConfluence(sdp_answer[:confluenceUrl])
        end
    end

    if(@devices)
        @devices.each do |device|
            begin
                device.backdoor("backdoorStopCall", nil)
            rescue
            end
            begin
                p "Device:#{device.deviceID} Exit app"
                device.exitapp()
            rescue
            end
        end
    end
    @devices = nil
    
    if(@asDummyApps)
        @asDummyApps.each do |asDummyApp|
            asDummyApp.backdoor("backdoorStopDummyApp", nil)
            p "Dummy screenshare APP:#{asDummyApp.deviceID} Exit app"
            asDummyApp.exitapp()
        end
    end
    @asDummyApps = nil

    if(ENV["NETWORKEMULATE"])
 #       Dummynet.flush() unless (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil

        flush_network_simulator() unless (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
    end
end

When(/^I open camera$/) do
    @devices[0].backdoor("backdoorOpenCamera", nil)
end 

When(/^I close camera$/) do
    @devices[0].backdoor("backdoorCloseCamera", nil)
end 

Then (/^I get media session return error code (.*)$/) do |expectRetCode|
    retCode = @devices[0].backdoor("backdoorGetMSLastRetCode", nil)
    assert_equal(expectRetCode, retCode, "The media session return error code is not expected!")
end

Then (/^I create my SDP offer for my device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    p "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

Then(/^I stop device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @devices[idx].exitapp
end

def create_confluence(idx, override = false)
    raise "I didn't get the SDP from device #{idx}." if !@sdpOffer[idx]
    if(@linus_server_url == nil)
        sdpAnswer = @calliope_client.createConfluence(@venue_url, @sdpOffer[idx])
    else
        sdpAnswer = @calliope_client.createLinusConfluence(@venue_url, @sdpOffer[idx])
    end
    Kernel.puts(sdpAnswer)
    @linusUrl[idx] = sdpAnswer[:confluenceUrl]
    @requestFloorUrl[idx] = sdpAnswer[:requestFloorUrl]
    @releaseFloorUrl[idx] = sdpAnswer[:releaseFloorUrl]
    sdpRemote = sdpAnswer[:sdpAnswer]
    if(override)
        sdpRemote.gsub! '=900', '=920'
        Kernel.puts(sdpRemote)
    end
    @devices[idx].backdoor("backdoorSetRemoteSdp", sdpRemote)
end

def update_confluence(idx)
    offer = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts(offer)
    sdpAnswer = @calliope_client.updateLinusConfluence(@linusUrl[idx], offer)
    Kernel.puts(sdpAnswer)
    @devices[idx].backdoor("backdoorSetRemoteSdp", sdpAnswer)
end

Then(/^I should get SDP from device (\d+) and create confluence with mangling sdp$/) do |idxstr|    
    idx = idxstr.to_i - 1
    create_confluence(idx, true)
end

Then(/^I should get SDP for each device and create confluence$/) do 
    cnt = @devices.length
    cnt.times do |i|
        create_confluence(i, false)
    end
end    

Then(/^I should get SDP from device (\d+) and (create|update) confluence$/) do |idxstr, op|
    idx = idxstr.to_i - 1
    if(op.include? "create")
        create_confluence(idx)
    elsif(op.include? "update")
        update_confluence(idx)
    end
end

Then(/^I request floor on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @calliope_client.requestFloor(@requestFloorUrl[idx])
end

Then(/^I release floor on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @calliope_client.releaseFloor(@releaseFloorUrl[idx])
end

Then(/^I keep the call for (\d+) seconds$/) do |time_out|
    p "waiting for #{time_out} seconds..."
    sleep time_out.to_i
end

Then(/^I keep the call for (\d+) milliseconds$/) do |time_out|
    p "waiting for #{time_out} seconds..."
    sleep time_out.to_f/1000.0
end

Then(/^I keep the call forever$/) do
    p "waiting forever..."
    sleep
end

Then(/^I keep debug the call for (\d+) seconds$/) do |time_out|
    time_out.to_i.times do |i|
        statistics = @devices[0].backdoor("backdoorGetStatistics", nil)
        p statistics
        statistics = @devices[1].backdoor("backdoorGetStatistics", nil)
        p statistics
        sleep 1
    end 
end

Then(/^I keep checking voice level for (\d+) seconds$/) do |time_out|
    time_out.to_i.times do |i|
        @devices.each do |device|
            micLevel = device.backdoor("backdoorGetVoiceLevel", "microphone")
            spkLevel = device.backdoor("backdoorGetVoiceLevel", "speaker")
            p ("Voice Level(#{device.deviceID}): mic=#{micLevel}, spk=#{spkLevel}")
            sleep 1
        end
    end
end

def get_remote_audio_ip_port(device)
    stats = get_stats(device)
    return stats["audioConn"]["remoteIp"],  stats["audioConn"]["uRemotePort"]
end

def get_remote_video_ip_port(device)
    stats = get_stats(device)
    return stats["videoConn"]["remoteIp"],  stats["videoConn"]["uRemotePort"]
end

And(/^I (disable|enable) wifi$/) do |op|
    p "#{op} wifi..."
    @devices[0].backdoor("backdoorChangeWifi", op)
end

Then(/^I wait for wifi enable$/) do
    counter = 0
    while @devices[0].backdoor("backdoorIsWifiEnabled", nil) != "enabled" do
        p "waiting for wifi enabled"
        sleep 1
        counter = counter + 1
        break if(counter >= 120)
    end
    p "wifi is enabled"
    @stats_list = []
    @stats_list.push(get_stats(@devices[0]))
end

Then(/^I wait for wifi disable$/) do
    counter = 0
    while @devices[0].backdoor("backdoorIsWifiEnabled", nil) != "disabled" do
        p "waiting for wifi to disable"
        sleep 1
        counter = counter + 1
        break if(counter >= 120)
    end
    p "wifi is disabled"
    # @stats_list = []
    # @stats_list.push(get_stats(@devices[0]))
end

Then(/I stop the call on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorStopCall", nil)
end

Then (/^I rotate camera on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorRotateCamera", nil)
end

Then (/^I rotate camera on device (\d+) (\d+) times every (\d+) seconds$/) do |idxstr, times, duration|
    i = idxstr.to_i - 1
    count = times.to_i
    while (count != 0) do
        @devices[i].backdoor("backdoorRotateCamera", nil)
        sleep duration.to_i
        count = count.to_i - 1
        p count
    end
end

Then (/^I should received specific layers video less then (\d+)p$/) do |rs|
    Kernel.puts(@devices[0].deviceID)
    stats = get_stats(@devices[0])
    check_stats_all(stats)
    size = [stats["videoRemote"]["uHeight"], stats["videoRemote"]["uWidth"]].min
    p "videosize is "
    p size.to_i
    assert(size < rs.to_i, "video resolution is incorrect")
    @devices[0].backdoor("backdoorStopCall", nil)
end
Then (/^I should received specific layers video less than (\d+)p on device (\d+)$/) do |rs, idxstr|
    i = idxstr.to_i - 1
    Kernel.puts(@devices[i].deviceID)
    stats = get_stats(@devices[i])
    check_stats_all(stats)
    size = [stats["videoRemote"]["uHeight"], stats["videoRemote"]["uWidth"]].min
    p "videosize is "
    p size.to_i
    assert(size < rs.to_i, "video resolution is incorrect")
    @devices[i].backdoor("backdoorStopCall", nil)
end
And(/^I subscribe specific layers video with mbps (\d+) fs (\d+) fps (\d+) dpb (\d+)$/) do |mbps,fs,fps,dpb|
    my_json = {:scr => [{:mbps => mbps.to_i, :fs => fs.to_i, :fps => fps.to_i, :dpb => dpb.to_i}]}
    p my_json
    @devices[0].backdoor("backdoorRequestVideo", my_json.to_json)
end

def make_scrs(table)
    params = {}
    table.raw.each do |row|
        type = row[0]
        value = row[1]
        begin
            value = JSON.parse(value)
        rescue JSON::ParserError
        end
        params[type] = {} unless(params[type])
        params[type] = value
    end
    return params
end

And(/^I subscribe multi layers video with on device (\d+):$/) do |devIdx, table|
    my_json = make_scrs(table)
    p my_json
    i = devIdx.to_i - 1
    @devices[i].backdoor("backdoorRequestVideo", my_json.to_json)
end
Then(/^I set max video capability with params: (\d+) (\d+) (\d+) (\d+) (\d+) (\d+) on device (\d+)$/) do |uPLID, uMMBPS, uMFS, uMFPS, uMBR, uMNUS, idxstr|
    i = idxstr.to_i - 1;
    my_json = {:caller=>true,:param=>{:video=>{:maxEncCapability=>{:uProfileLevelID=>uPLID.to_i,:uMaxMBPS=>uMMBPS.to_i,:uMaxFS=>uMFS.to_i,:uMaxFPS=>uMFPS.to_i,:uMaxBitRate=>uMBR.to_i,:uMaxNalUnitSize=>uMNUS.to_i}}}}
    @devices[i].backdoor("backdoorSetVideoParam", my_json.to_json)
end

Then(/^I check max video capability setting result (\d+) (\d+) on device (\d+)$/) do|uMFS, uMBR, idxstr|
    i = idxstr.to_i - 1
    stat = get_stats(@devices[i])
    br   = stat["videoLocal"]["fBitRate"]
    width = stat["videoLocal"]["uWidth"]
    height = stat["videoLocal"]["uHeight"]
    fs = width * height
    fs = fs / 256
    umfs = uMFS.to_i
    umbr = uMBR.to_i
    ret1 = fs <=> umfs
    ret2 = br <=> umbr
    assert((ret1<=0)&&(ret2<=0), "max video capability setting failed!!!!!")
end

Then (/^I check if remote rendering surface in device (\d+) is using CropFill$/) do |idxstr|
    i = idxstr.to_i - 1
    currRenderMode = @devices[i].backdoor("backdoorGetScalingMode", nil)
    assert_equal("CropFill", currRenderMode, "It is not using CropFill!")
end

Then (/^I check if remote rendering surface in device (\d+) is using LetterBox$/) do |idxstr|
    i = idxstr.to_i - 1
    currRenderMode = @devices[i].backdoor("backdoorGetScalingMode", nil)
    assert_equal("LetterBox", currRenderMode, "It is not using LetterBox!")
end

Then (/^I set device (\d+) to default portrait orientation$/) do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorSetDefaultOrientationPort", nil)
end

Then (/^I set device (\d+) to default landscape orientation$/) do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorSetDefaultOrientationLand", nil)
end

Then (/^I set the correct render mode to device (\d+) while being in portrait mode$/)  do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorSetRenderModePort", nil)
end

Then (/^I set the correct render mode to device (\d+) while being in landscape mode$/)  do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorSetRenderModeLand", nil)
end

Then (/^device (\d+) should not be receiving audio data$/) do |idxstr|
    i = idxstr.to_i - 1
    #We changed to 25,000 that is 25Kbps, because TC AEC uses 48kbps.
    assert(get_stats(@devices[i])["audioRemote"]["uBitRate"] < 25000, "Still receiving audio while caller is muted")
end

Then (/^device (\d+) should not be receiving video data$/) do |idxstr|
    i = idxstr.to_i - 1
    assert(get_stats(@devices[i])["videoRemote"]["fBitRate"] == 0, "Still receiving video while caller has paused sending video")
end

Then (/^I mute local audio on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "audio", :mute => true, :speaker => true}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I pause to send video on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "video", :mute => true, :speaker => true}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I unmute local audio on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "audio", :mute => false, :speaker => true}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I unpause to send video on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "video", :mute => false, :speaker => true}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I mute remote audio on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "audio", :mute => true, :speaker => false}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I pause to receive video on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "video", :mute => true, :speaker => false}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I unmute remote audio on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "audio", :mute => false, :speaker => false}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then (/^I unpause to receive video on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    my_json = {:type => "video", :mute => false, :speaker => false}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end

Then(/^I (start|stop) (local|remote) (audio|video|sharing) track on device (\d+)$/) do |action, direction, media, idxstr|
    i = idxstr.to_i - 1
    
    my_json = {:type => media, :start => action.eql?("start"), :remote => direction.eql?("remote")}
    @devices[i].backdoor("backdoorStartStopTrack", my_json.to_json)
end

Then(/^I (mute|unmute) (local|remote) (audio|video) track on device (\d+)$/) do |action, direction, media, idxstr|
    i = idxstr.to_i - 1
    
    my_json = {:type => media, :mute => action.eql?("mute"), :speaker => direction.eql?("remote")}
    @devices[i].backdoor("backdoorMuteUnMute", my_json.to_json)
end
    
Then /^device (\d+) SDP type should be (.*)$/ do |idxstr, sdpType|
    i = idxstr.to_i - 1
    if (sdpType == "None")
        assert_equal("None", @devices[i].backdoor("backdoorCheckSDPType", nil), "It is not using None!")
    elsif (sdpType == "Offer")
        assert_equal("Offer", @devices[i].backdoor("backdoorCheckSDPType", nil), "It is not using Offer!")
    elsif (sdpType == "Answer")
        assert_equal("Answer", @devices[i].backdoor("backdoorCheckSDPType", nil), "It is not using Answer!")
    elsif (sdpType == "Pranswer")
        assert_equal("Pranswer", @devices[i].backdoor("backdoorCheckSDPType", nil), "It is not using Pranswer!")
    else
        assert_equal("Unknown", @devices[i].backdoor("backdoorCheckSDPType", nil), "It is Unknown!")
    end
end 

Then (/^device (\d+) must NOT be blocked$/) do |idxstr|
    i = idxstr.to_i - 1
    assert_equal("false", @devices[i].backdoor("backdoorIsMediaBlocked", nil), "Media flow is being blocked!")
end

Then (/^device (\d+) must be blocked$/) do |idxstr|
    i = idxstr.to_i - 1
    assert_equal("true", @devices[i].backdoor("backdoorIsMediaBlocked", nil), "Media is still flowing")
end

Then (/^device (\d+) must be connected$/) do |idxstr|
    i = idxstr.to_i - 1
    assert_equal("true", @devices[i].backdoor("backdoorIsConnected", nil), "Device is not connected!")
end

Then (/^I set MaxWidth as (\d+), MaxHeight as (\d+), MaxFrameRate as (\d+), in device (\d+)$/) do |maxWidth, maxHeight, maxFPS, idxstr|
    i = idxstr.to_i - 1
    
    maxWidth = maxWidth.to_i
    maxHeight = maxHeight.to_i
    maxFPS = maxFPS.to_f
    
    my_json = {:iMaxWidth => maxWidth, :iMaxHeight => maxHeight, :iMaxFrameRate => maxFPS}
    my_json = {:video => my_json}
    set_video = PARAM_VIDEO + my_json.to_json
    @devices[i].backdoor("backdoorSetParam", set_video)
end

Then(/^I check EC status on device (\d+) when (.*)$/) do|idxstr, value|
    i = idxstr.to_i - 1
    stat = get_stats(@devices[i])
    ecNum = stat["videoRemote"]["uEcNumCount"] 
    p "Current EC num is ", ecNum 
    if (value == "enable")  
        assert(ecNum>0, "enable but no EC happens!!!")
    elsif (value == "disable")
        assert(ecNum<=0, "disable but EC happens!!!")	
    end
end

#TODO, need to change to AST
Then (/^I set the following (.*) parameters for device (\d+) as: (.*) = (.*)$/) do |mediaType, idxstr, key, value|
    i = idxstr.to_i - 1
                                                                
    if (key == "uMaxBandwidth" || key == "uMaxPacketSize" || key == "iMaxWidth" || key == "iMaxHeight" || key == "ePacketizationMode")
        value = value.to_i
        my_json = {key => value}
    elsif (key == "iMaxFrameRate")
        value = value.to_f
        my_json = {key => value}
    elsif (key == "enableQos" || key == "enableAGC" || key == "enableEC" || key == "enableVAD" || key == "enableNS" || key == "enableDropSeconds")
        if (value == "false")
            my_json = {key => false}
        else 
            my_json = {key => true}
        end
    elsif (key == "eVideoQualityLevel")
        rs = convVideoLevel(value)
        my_json = {key => rs}
    elsif (key == "enableKeyDumpFile")
        my_json = {key => value.to_i}    
    elsif (key == "enableDecoderMosaic")
        if (value == "false")
            my_json = {key => false}
        else 
            my_json = {key => true}
        end
    else
        my_json = {key => value}
    end   
    if (mediaType == "Video")
        p "Setting Video parameter"
        my_json = {:video => my_json}
        p my_json
        set_video = PARAM_VIDEO + my_json.to_json
        @devices[i].backdoor("backdoorSetParam", set_video)
    elsif (mediaType == "Audio")
        p "Setting Audio parameter"
        my_json = {:audio => my_json}
        p my_json
        set_audio = PARAM_AUDIO + my_json.to_json
        @devices[i].backdoor("backdoorSetParam", set_audio)
    end
    if (key == "enableDropSeconds")
        p "check stats after setting"
        stats = get_stats(@devices[i])
        @audioOutNet_stats = []
        @audioOutNet_stats.push(stats["audioOutNet"])
    end
end

Then (/^I should receive the following value from device (\d+), (.*) parameter: (.*) == (.*)$/) do |idxstr, mediaType, key, value|
    i = idxstr.to_i - 1
    p mediaType
    if (mediaType == "Video")
        assert_equal(value, @devices[i].backdoor("backdoorGetVideoParameters", key), "Incorrect value for given video parameter!")
    elsif (mediaType == "Audio")
        assert_equal(value, @devices[i].backdoor("backdoorGetAudioParameters", key), "Incorrect value for given audio parameter!")
    end
end

Then (/^I should have sent and received (.*) video in devices$/) do |level|
    if (level == "sld")
        size = 90
    elsif (level == "ld")
        size = 180
    elsif (level == "sd")
        size = 360
    elsif (level == "hd")
        size = 720
    else
        size = 0
    end
    stats = get_stats(@devices[0])
    assert_equal(size, [stats["videoLocal"]["uWidth"], stats["videoLocal"]["uHeight"]].min, "video local resolution not equal to corresponding quality level")
    stats = get_stats(@devices[1])
    assert_equal(size, [stats["videoRemote"]["uWidth"], stats["videoRemote"]["uHeight"]].min, "video remote resolution not equal to corresponding quality level")
end

Then (/^I should sent video with MaxWidth == (\d+), MaxHeight == (\d+), MaxFrameRate == (\d+)$/) do |maxWidth, maxHeight, maxFPS|
    stats = get_stats(@devices[0])
    maxWidth = maxWidth.to_i
    maxHeight = maxHeight.to_i
    maxFPS = maxFPS.to_f
    localWidth = stats["videoLocal"]["uWidth"]
    localHeight = stats["videoLocal"]["uHeight"]
    if (localWidth < localHeight)
        localWidth, localHeight = localHeight, localWidth
    end
    assert(maxWidth >= localWidth, "video local width exceed the limitation")
    assert(maxHeight >= localHeight, "video local height exceed the limitation")
end

Then (/^I should sent out audio packets (.*) than (\d+) on device (\d+)$/) do |op, packetstr, idx|
    i = idx.to_i - 1
    stats = get_stats(@devices[i])
    audio_out_packets1 = @audioOutNet_stats[@audioOutNet_stats.length-1]["uPackets"].to_i
    audio_out_packets2 = stats["audioOutNet"]["uPackets"].to_i
    out_packets = audio_out_packets2 - audio_out_packets1
    p out_packets
    packets = packetstr.to_i
    if (op == "less")
        assert(out_packets < packets, "not drop enough packets")
    elsif (op == "more")
        assert(out_packets > packets, "drop enough packets")
    end
end

When(/^I enable SRTP feature on both devices$/) do 
    my_json = {:audio => {:enableSRTP => true}, :video => {:enableSRTP => true}}
    set_audio = PARAM_AUDIO + {:enableSRTP => true}.to_json
    p set_audio
    set_video = PARAM_VIDEO + {:enableSRTP => true}.to_json
    p set_video
    i = 0
    while i < 2 do
        p "enable SRTP on #{@devices[i].deviceID}"
        @devices[i].backdoor("backdoorSetParam", set_audio)
        @devices[i].backdoor("backdoorSetParam", set_video)
        i += 1
    end
end
    
Then (/^I set the audio playback on device (\d+) to (.*)$/) do |idxstr, playbackType|
    i = idxstr.to_i - 1
    assert_equal("done", @devices[i].backdoor("backdoorSetPlaybackDevice", playbackType), "Invalid audio playback type or not using PeerCall mode")
end

Then (/^I test IOS backdoor on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    @devices[i].backdoor("backdoorTestIOS", nil)
    p "Test IOS backdoor: Ruby"
end

Then (/^I change the display port on device (\d+) to (.*)$/) do |idxstr, orientation|
    i = idxstr.to_i - 1
    if (orientation == "portrait")
        @devices[i].backdoor("backdoorRotateWsePort", nil)
    elsif (orientation == "landscape")
        @devices[i].backdoor("backdoorRotateWseLand", nil)
    else
        exit 1
    end
end

Then (/^I check I have all media tracks running on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    assert_equal("pass", @devices[i].backdoor("backdoorCheckOnMediaReady", nil), "Missing media tracks after confluence!")
end

Then (/^I set the packet loss rate to (\d+)% for device (\d+)$/) do |lossrate, idxstr|
    lossrate = lossrate.to_f / 100
    i = idxstr.to_i - 1
    setup_network_simulator(i, 'uplink', nil, nil, nil, lossrate) 
    setup_network_simulator(i, 'downlink', nil, nil, nil, lossrate) 
end

Then (/^I flush all IPFW rules$/) do
#    Dummynet.flush()
    flush_network_simulator()
end
    
Then(/^The bit\-rate of video bitstream output from device (\d+) should be in \+\/\- (\d+)% of (\d+) Kbps$/) do |devIdx, percent, targetBR|
    stats = get_stats(@devices[0])
    bitratelocal = stats["videoLocal"]["fBitRate"] 
    p bitratelocal
    p targetBR.to_i*1024
    assert(bitratelocal.to_i<targetBR.to_i*1024*(percent.to_i+100)/100, "Bitrate break the ceiling of BW limitation!")
    assert(bitratelocal.to_i>targetBR.to_i*1024*(100-percent.to_i)/100, "Bitrate break the floor of BW limitation!")
end

Then (/^I set MaxWidth as (\d+), MaxHeight as (\d+), MaxFrameRate as (\d+), MaxSpatialLayer as (\d+) in device (\d+)$/) do |maxWidth, maxHeight, maxFPS, maxSptial, idxstr|
    i = idxstr.to_i - 1
    
    maxWidth = maxWidth.to_i
    maxHeight = maxHeight.to_i
    maxFPS = maxFPS.to_f
    maxSptial = maxSptial.to_i
    
    my_json = {:iMaxWidth => maxWidth, :iMaxHeight => maxHeight, :iMaxFrameRate => maxFPS, :iMaxSpacialLayer => maxSptial}
    my_json = {:video => my_json}

    set_video = PARAM_VIDEO + my_json.to_json
    @devices[i].backdoor("backdoorSetParam", set_video)
end

Then (/^I set audio codec (.*) on device (\d+)$/) do |type, idxstr|
    i = idxstr.to_i - 1
    my_json = {:caller=>true, :param=>{:audio=>{:audioCodec=>type}}}
    @devices[i].backdoor("backdoorSetAudioParam", my_json.to_json)
end

Then (/^I check audio codec (\d+) set result via device (\d+)$/) do |type_value, idxstr|
    i = idxstr.to_i - 1
    type_given = type_value.to_i
    p "I check audio codec set result via device.........."
    p "set codec value is"
    p type_given
    stats = get_stats(@devices[i])
    p stats["audioRemote"]
    remote_codec = stats["audioRemote"]["codecType"]
    p "remote_codec is"
    p remote_codec
    ret = type_given == remote_codec
    p "ret is "
    p ret
    assert(ret==true, "audio codec setting failed!!!!!")
end

And (/^I should get (audio|video|share) media status (available|unavailable) on device (\d+)$/) do |media, status, idxstr|
    i = idxstr.to_i - 1
    mediaStatus = @devices[i].backdoor("backdoorGetMediaStatus", media)
    assert(status==mediaStatus, "the media status was wrong!")
end

Then (/^I check current (.*) device (\d+) supports HW acceleration in advance$/) do |device, idxstr|
    i = idxstr.to_i - 1
    p "I check current device supports HW or not in advance:"
    p device
    bHWSupport = @devices[i].backdoor("backdoorCheckHWSupport", device)
    p "bHWSupport:"
    p bHWSupport
    if bHWSupport == "false"
        pending "Current device doesn't support HW Acceleration, Pending and it can be seen as passed!!!!"
    end
end

Then (/^I check the HW acceleration result from video statistics on device (\d+)$/) do |idxstr|
    i = idxstr.to_i - 1
    stats = get_stats(@devices[i])
    hw_acc = stats["videoLocal"]["bHWEnable"]
    assert(hw_acc==true, "HW Acceleration checking failed from local video statistics!!!!")
    hw_acc = stats["videoRemote"]["bHWEnable"]
    assert(hw_acc==true, "HW Acceleration checking failed from remote video statistics!!!!")
    p "Success!!!! Current device supports HW codec from video statistics" 
end
    
Then (/^I check if current device (\d+), supports HW Acceleration with the following device from WDM whitelist, (.*)$/) do |idxstr, name|
    i = idxstr.to_i - 1

    if (@devices[i].backdoor("backdoorHWCheckDevice", name)=="true" && @devices[i].backdoor("backdoorHWCheckAPILevel", nil)=="true")
        p "Device qualified for HW Codec, moving forward with test case" 
    else
        pending "Device does not qualify for HW Codec, ending test case"
    end 
end

Then (/^I check dynamic performance when the CPU usage is changed in (.*). Test will take about ~2 minutes$/) do |os|
    @os = nil
    @os = os
    @cores = nil
    winmacDevice = nil
    
    @devices.each do |currDevice|
        if (currDevice.class.name == "WX2Calabash::WinMacDevice")
            winmacDevice = currDevice
            p "Win/Mac device found"
        end
    end
    
    assert(winmacDevice != nil, "No Mac/Win devices found!")
    
    @cores = winmacDevice.backdoor("backdoorGetCPUCores", nil);
    print "CPU Core numbers = "
    p  @cores
    
    exhaustCPUUsage("true", os, @cores)
    
    sleep 60
    
    stats = get_stats(winmacDevice)
    videoLocalHeight = stats["videoLocal"]["uHeight"]
    videoLocalWidth = stats["videoLocal"]["uWidth"]
    videoRemoteHeight = stats["videoRemote"]["uHeight"]
    videoRemoteWidth = stats["videoRemote"]["uWidth"]
    local_res = videoLocalHeight * videoLocalWidth
    remote_res = videoRemoteHeight * videoRemoteWidth
    
    exhaustCPUUsage("false", os, @cores)
	
	if (local_res != 0)
		assert(local_res <= 320 * 180, "TX video was not <= 180p when CPU usage was high for a long time")
		p "TX video test passed with high CPU Usage"
	end
	if (remote_res != 0)
		assert(remote_res <= 320 * 180, "RX video was not <= 180p when CPU usage was high for a long time")
		p "RX video test passed with high CPU Usage"
	end
    
    sleep 60
    
    stats = get_stats(winmacDevice)
    videoLocalHeight = stats["videoLocal"]["uHeight"]
    videoLocalWidth = stats["videoLocal"]["uWidth"]
    videoRemoteHeight = stats["videoRemote"]["uHeight"]
    videoRemoteWidth = stats["videoRemote"]["uWidth"]
    local_res = videoLocalHeight * videoLocalWidth
    remote_res = videoRemoteHeight * videoRemoteWidth

    if (local_res != 0 && os != "win")
        assert(local_res >= 320 * 180, "TX video was not >= 180p when CPU usage was low")
    	p "TX video test passed with low CPU Usage"
    end
    if (remote_res != 0 && os != "win")
        assert(remote_res >= 320 * 180, "RX video was not >= 180p when CPU usage was low")
    	p "RX video test passed with low CPU Usage"
    end
end

When (/^I override static perf with (.*)$/) do |perf_filename|
    perf = File.read(expand_local_path("../../sample/#{perf_filename}"))
    @devices.each do |device|
        device.backdoor("backdoorOverridePerfJson", perf)
    end
end

Then (/^I check file size of each trace in a call lasting (\d+) seconds, should be under (\d+) MBs$/) do |seconds, capacity|
    sleep seconds.to_i

    HOME_PATH = Dir.pwd

    logArray = []
    logArray = checkFileSize()
    Dir.chdir ENV["TEST_TRACE_PATH"]
    capacity = capacity.to_i * 1000000 # 5MB
    
    if (logArray)
        logArray.each do |log|
          name = log
          size = File.size(log)
            p "File name: #{name}. Size: #{size} bytes"
            
            assert(size < capacity, "File named #{name}, is greater than #{capacity} byte(s), with a call lasting for #{seconds} seconds")
        end
    end
    
    Dir.chdir(HOME_PATH)
end

Then (/^I check file size of each trace in a call lasting (\d+) seconds$/) do |seconds|
    sleep seconds.to_i

    HOME_PATH = Dir.pwd

    logArray = []
    logArray = checkFileSize()
    Dir.chdir ENV["TEST_TRACE_PATH"]
    
    if (logArray)
        logArray.each do |log|
          name = log
          size = File.size(log)
            p "File name: #{name}. Size: #{size} bytes"
        end
    end
    
    Dir.chdir(HOME_PATH)
end

Then (/^I set the packet loss rate to (\d+)% for the (pc|mobile|ios|android|winphone) device$/) do |lossrate, os|
    lossrate = lossrate.to_f / 100
    setup_network_simulator(0, 'uplink', nil, nil, nil, lossrate) 
    setup_network_simulator(0, 'downlink', nil, nil, nil, lossrate) 
end

Then (/^I start tcpdump on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @devices[idx].start_capture_packet()
end

Then (/^I stop tcpdump on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    @devices[idx].stop_capture_packet()
end

Then (/^I start tcpdump on devices$/) do
    @devices.each do |device|
        device.start_capture_packet()
    end
end

Then (/^I stop tcpdump on devices$/) do
    @devices.each do |device|
        device.stop_capture_packet()
    end
end

Then (/^I change it back to (camera|svs) video$/) do |type|
    @devices.each do |device|
        device.backdoor("backdoorChangeLocalTrack", type)
    end
end    
