When(/^I start screenshare as (.*) on device (\d+) with the default setting and:$/) do |screenshare_role,which_device, table|
	p "which_device=#{which_device}"
	
    
    default_setting = {
        'LOOPBACK' => "true",
        'CALLIOPE' => "false",
        'LINUS_ADDR' => ENV["LINUS_SERVER"],
        'SHARE' => "sharer",
        'AUDIO' => "true",
        'VIDEO' => "false",
        'SOURCE' => "",
        'SHARE_SOURCE_FILE' => "",
        'MULTISTREAM' => "false",
        'ENABLEQOS' => "true"
    }
    if(table)
        setting = default_setting.merge(table.rows_hash)
    else
        setting = default_setting
    end
    
    option = {};
    option["audio"] = setting['AUDIO'].to_bool
    option["video"] = setting['VIDEO'].to_bool
    option["share"] = setting['SHARE'];


    idx = which_device.to_i - 1

    is_file = false
    is_timestamp = false
    is_loopback = true
    is_p2p = false
    is_mute = false
    is_multilayer = false
    is_loopfile = false
    enableQos = setting['ENABLEQOS'].to_bool

    
    is_calliope = setting['CALLIOPE'].to_bool
    is_multistream = setting['MULTISTREAM'].to_bool
    
    rs = 2
    class_name = @devices[0].class.name
    if(is_file)
        rs = set_capture_file(call, class_name, 0)
    end
    
    linus_url_str = ENV['LINUS_SERVER']
    linus_url_str = "" if(ENV['LINUS_SERVER'] == nil)
    my_json = {:loopback => is_loopback,
        :linus => linus_url_str,
        :filemode => is_file,
        :p2p => is_p2p,
        :mute => is_mute,
        :option => option,
        :multilayer => is_multilayer,
        :timestamp => is_timestamp,
        :isLoopFile => is_loopfile,
        :resolution => rs,
        :enableQos => enableQos,
        :sharesource => setting['SOURCE'],
        :sharesource_file => setting['SHARE_SOURCE_FILE']}
    
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
end


Then(/^I wait for screenshare ended on device (\d+)$/) do |which_device|
	p "which_device=#{which_device}"	
	idx = which_device.to_i - 1
	
	is_end = "";
	
	wait_time = 15
	while is_end != "end" &&  wait_time>0 do
		backdoorGetSharingStatus_param = {:action => "ShareSharingRuningStatus"}
		ret_json_string = @devices[idx].backdoor("backdoorGetSharingStatus", backdoorGetSharingStatus_param.to_json)		
		ret_json = JSON.parse(ret_json_string)		
		is_end = ret_json["ShareSharingRuningStatus"]
		sleep 1
		wait_time = wait_time-1
	end 
	
	if(wait_time==0)
		raise "I wait for screenshare ended >> time out!!!"
	end
	
end


Then(/^I check screenshare  (.*) result on device (\d+)$/) do |screenshare_role,which_device|
	p "which_device=#{which_device}"
	p "share_end=#{screenshare_role}"	
	
	idx = which_device.to_i - 1
	
	screenshare_result_role =  "both";
	if(screenshare_role.include?"_sharer_")
		screenshare_result_role = "sharer"
	elsif(screenshare_role.include?"_viewer_")
		screenshare_result_role = "viewer"
	end
		
	backdoorGetSharingResult_param = {:action => "ShareScreenLastResult",:what_role=>screenshare_result_role}
    ret_json_string = @devices[idx].backdoor("backdoorGetSharingResult", backdoorGetSharingResult_param.to_json)		
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)
	
	assert(ret_json["common"]["screenConn"]["uRTCPReceived"] != 0, "Screen Share Result >> both >>  Connect >> uRTCPReceived is 0")
    assert(ret_json["common"]["screenConn"]["uRTCPSent"] != 0, "Screen Share Result >> both >> Connect >> uRTCPSent is 0")
	
	if(screenshare_result_role=="both" || screenshare_result_role=="sharer")
		assert(ret_json["common"]["screenConn"]["uRTPSent"] != 0, "Screen Share Result >>  Sharer >> Connect >> uRTPSent is 0")
		assert(ret_json["sharer"]["capturer"]["average_captured_time"] != 0, "Screen Share Result >> Sharer >> Capturer >> average_captured_time is 0")
		assert(ret_json["sharer"]["capturer"]["total_captured_frames"] != 0, "Screen Share Result >>  Sharer >>Capturer >> total_captured_frames is 0")
		assert(ret_json["sharer"]["video"]["fFrameRate"] != 0, "Screen Share Result >> Sharer >> video >> fFrameRate is 0")
        assert(ret_json["sharer"]["video"]["uEncodeFrameCount"] > 5, "Screen Share Result >> Sharer >> video >> uEncodeFrameCount less than 5")
	end

	if(screenshare_result_role=="both" || screenshare_result_role=="viewer")	
		assert(ret_json["common"]["screenConn"]["uRTPReceived"] != 0, "Screen Share Result >> viewer >>  Connect >> uRTPReceived is 0")
        frameCount = ret_json["viewer"]["video"]["uRenderFrameCount"];
        p "Screen Share Result >> viewer >> video >> uRenderFrameCount is #{frameCount}"
		assert(ret_json["viewer"]["video"]["uRenderFrameCount"] > 5, "Screen Share Result >> viewer >> video >> uRenderFrameCount less than 5")
#		assert(ret_json["viewer"]["video"]["fFrameRate"] != 0, "Screen Share Result >> viewer >> video >> fFrameRate is 0")
		
	end 
	
end

Then(/^I check screenshare  (.*) network statistic on device (\d+)$/) do |screenshare_role,which_device|
    p "which_device=#{which_device}"
    p "share_end=#{screenshare_role}"
    
    idx = which_device.to_i - 1
    
    screenshare_result_role =  "both";
    if(screenshare_role.include?"_sharer_")
        screenshare_result_role = "sharer"
        elsif(screenshare_role.include?"_viewer_")
        screenshare_result_role = "viewer"
    end
    
    backdoorGetSharingResult_param = {:action => "ShareScreenLastResult",:what_role=>screenshare_result_role}
    ret_json_string = @devices[idx].backdoor("backdoorGetSharingResult", backdoorGetSharingResult_param.to_json)
    ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)
    
    if(screenshare_result_role=="both" || screenshare_result_role=="sharer")
        assert(ret_json["common"]["screenOutNet"]["uLostPackets"] == 0, "Screen Share Result >>  Sharer >> Connect >> uRTPSent is 0")
    end
    
    if(screenshare_result_role=="both" || screenshare_result_role=="viewer")
        assert(ret_json["common"]["screenInNet"]["uLostPackets"] == 0, "Screen Share Result >> viewer >>  Connect >> uRTPReceived is 0")
    end

end

Then (/^I create (\d+) row (\d+) column qrcode view with x=(\d+) y=(\d+) w=(\d+) h=(\d+) on asDummyApp (\d+)$/) do |row,column,x,y,w,h,which_app|
	idx = which_app.to_i - 1
	p "create qrcode view which_app=#{which_app},row=#{row},column=#{column},x=#{x} y=#{y} w=#{w} h=#{h} on app index={#idx}"
	
	contents = []
	for nRow in 1..row.to_i
		for nCol in 1..column.to_i
			contents.push("QRCode_#{nRow}_#{nCol}")
		end
	end
	
	backdoorShowQRCodeView_param = {:action => "backdoorShowQRCodeView",:view_x=>x.to_i,:view_y=>y.to_i,:view_w=>w.to_i,:view_h=>h.to_i,:qrcode_grid_row=>row.to_i,:qrcode_grid_col=>column.to_i,:qrcode_contents=>contents}
    ret_json_string = @asDummyApps[idx].backdoor("backdoorShowQRCodeView", backdoorShowQRCodeView_param.to_json)	
	p "backdoorShowQRCodeView ret=#{ret_json_string}"	
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)
	
end

Then (/^I check screenshare content is (\d+) row (\d+) column qrcode on device (\d+)$/) do |row,column,which_device|
	idx = which_device.to_i - 1

	backdoorCollectReceivedQRCodeContents_param = {:action => "backdoorCollectReceivedQRCodeContents"}
    ret_json_string = @devices[idx].backdoor("backdoorCollectReceivedQRCodeContents", backdoorCollectReceivedQRCodeContents_param.to_json)	
	p "backdoorCollectReceivedQRCodeContents ret=#{ret_json_string}"	
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)	
	
	nRow = 1
	nCol = 1
	i = 0
	nCount = ret_json["qrcode_contents"].count
	 while i < nCount do
        assert(ret_json["qrcode_contents"][i]=="QRCode_#{nRow}_#{nCol}","QRCode_#{nRow}_#{nCol}!=#{ret_json["qrcode_contents"][i]}")
        
		i += 1
		nCol += 1
		if(nCol > column.to_i)
			nCol = 1
			nRow += 1
		end
		
		break if nRow>row.to_i
		
    end
	
end

Then (/^I set qrcode scan context with row=(\d+) column=(\d+) x=(\d+) y=(\d+) w=(\d+) h=(\d+) on device (\d+)$/) do |row,column,x,y,w,h,which_device|
	idx = which_device.to_i - 1	
	
	backdoorSetQRCodeContext_param = {:action => "backdoorSetQRCodeContext",:view_x=>x.to_i,:view_y=>y.to_i,:view_w=>w.to_i,:view_h=>h.to_i,:qrcode_grid_row=>row.to_i,:qrcode_grid_col=>column.to_i}
    ret_json_string = @devices[idx].backdoor("backdoorSetQRCodeContext", backdoorSetQRCodeContext_param.to_json)	
	p "backdoorSetQRCodeContext ret=#{ret_json_string}"	
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)	
end

Then(/^I keep the screenshare for (\d+) seconds$/) do |time_out|
    p "waiting for #{time_out} seconds..."
    sleep time_out.to_i
end

Then (/^I pause sharing on device (\d+)$/) do |which_device|
    idx = which_device.to_i - 1
    backdoorMuteUnMute_param = {:type => "sharing", :mute => true, :speaker => true}
    @devices[idx].backdoor("backdoorMuteUnMute", backdoorMuteUnMute_param.to_json)
end

Then (/^I resume sharing on device (\d+)$/) do |which_device|
    idx = which_device.to_i - 1
    backdoorMuteUnMute_param = {:type => "sharing", :mute => false, :speaker => true}
    @devices[idx].backdoor("backdoorMuteUnMute", backdoorMuteUnMute_param.to_json)
end

Then (/^I check screenshare (.*) (\d+) seconds at (.*) side on device (\d+)$/) do |feature_type, time_out, screenshare_role, which_device|
    idx = which_device.to_i - 1
	
	screenshare_result_role =  "both";
	if(screenshare_role.include?"_sharer_")
		screenshare_result_role = "sharer"
    elsif(screenshare_role.include?"_viewer_")
		screenshare_result_role = "viewer"
	end
    
	backdoorGetSharingResult_param = {:action => "ShareScreenLastResult",:what_role=>screenshare_result_role}
    ret_json_string = @devices[idx].backdoor("backdoorGetSharingResult", backdoorGetSharingResult_param.to_json)
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)
    
    if(screenshare_result_role=="both" || screenshare_result_role=="sharer")
        RTP_sent_num = ret_json["common"]["screenConn"]["uRTPSent"]
    end
    
    if(screenshare_result_role=="both" || screenshare_result_role=="viewer")
        RTP_received_num = ret_json["common"]["screenConn"]["uRTPReceived"]
    end
    
    p "waiting for #{time_out} seconds..."
    sleep time_out.to_i
    
    ret_json_string = @devices[idx].backdoor("backdoorGetSharingResult", backdoorGetSharingResult_param.to_json)
	ret_json = JSON.parse(ret_json_string)
    Kernel.puts JSON.pretty_generate(ret_json)
    
	if(screenshare_result_role=="both" || screenshare_result_role=="sharer")
        if(feature_type.include?"_pause_")
            assert(ret_json["common"]["screenConn"]["uRTPSent"] == RTP_sent_num, "Screenshare Pause Result >>  Sharer >> Connect >> uRTPSent changed from #{RTP_sent_num} to #{ret_json["common"]["screenConn"]["uRTPSent"]}")
        elsif (feature_type.include?"_resume_")
            assert(ret_json["common"]["screenConn"]["uRTPSent"] > RTP_sent_num, "Screenshare Resume Result >>  Sharer >> Connect >> uRTPSent changed from #{RTP_sent_num} to #{ret_json["common"]["screenConn"]["uRTPSent"]}")
        end
	end
    
    if(screenshare_result_role=="both" || screenshare_result_role=="viewer")
        if(feature_type.include?"_pause_")
            assert(ret_json["common"]["screenConn"]["uRTPReceived"] == RTP_received_num, "Screenshare Pause Result >>  Viewer >> Connect >> uRTPReceived changed from #{RTP_received_num} to #{ret_json["common"]["screenConn"]["uRTPReceived"]}")
        elsif(feature_type.include?"_resume_")
            assert(ret_json["common"]["screenConn"]["uRTPReceived"] > RTP_received_num, "Screenshare Resume Result >>  Viewer >> Connect >> uRTPReceived changed from #{RTP_received_num} to #{ret_json["common"]["screenConn"]["uRTPReceived"]}")
        end
	end
end

def convVideoLevel(call)
    rs = 2
    if (call.include? "sld")
        rs = 0
    elsif (call.include? "ld")
        rs = 1
    elsif (call.include? "sd")
        rs = 2
    elsif (call.include? "hd")
        rs = 3
    end
	return rs
end

Then (/^I (subscribe|unsubscribe|subscribe_another) (.*) video on device (\d+)$/) do |subscribe, level, which_device|
    resolution = convVideoLevel(level)
    is_new = subscribe.include? "another"
    is_unsubscribe = subscribe.include? "unsubscribe"
    backdoorSubscribe_param = {:resolution => resolution, :newTrack => is_new, :unsubscribe => is_unsubscribe}
    idx = which_device.to_i - 1
    @devices[idx].backdoor("backdoorSubscribe", backdoorSubscribe_param.to_json)
end

Then (/^I (.*) floor control on device (\d+)$/) do |action, which_device|
    is_requestfloor = true
    if(action.include?"giveup")
      is_requestfloor = false
    end
    
    backdoorRequestFloor_param = {:bRequestFloor => is_requestfloor}
    idx = which_device.to_i - 1
    @devices[idx].backdoor("backdoorRequestFloor", backdoorRequestFloor_param.to_json)
end

Then (/^I change screen capture fps to (\d+) on device (\d+)$/) do |fps, which_device|    
    backdoorScreenChangeCaptureFps_param = {:screenCaptureFps => fps}
    idx = which_device.to_i - 1
    @devices[idx].backdoor("backdoorScreenChangeCaptureFps", backdoorScreenChangeCaptureFps_param.to_json)
end

Then (/^I change screen capture file to (.*) on device (\d+)$/) do |file_capture_filename, which_device|    
    backdoorSetScreenFileCaptureFilePath_param = {:sharesource_file => file_capture_filename}
    idx = which_device.to_i - 1
    @devices[idx].backdoor("backdoorSetScreenFileCaptureFilePath", backdoorSetScreenFileCaptureFilePath_param.to_json)
end

Given (/^I screen share file capture resource (.*)$/) do |filesource|
  filepath = expand_local_path("../../sample/#{filesource}")
  p "file capture resource #{filepath}"
  unless(File.exist?(filepath))
    filepathArchive = "#{filepath}.zip"
    ZipFile.new(filepathArchive).extract(filesource,filepath)
  end
end

Then (/^I print info trace log (.*) on device (\d+)$/) do |outputInfo, which_device|
  p "Now is>>#{outputInfo}"
    
    backdoorOutputTrace_param = {:trace_info => outputInfo}
    idx = which_device.to_i - 1
    @devices[idx].backdoor("backdoorOutputTrace", backdoorOutputTrace_param.to_json)
end

def getPerformanceRuningTimeFactor()
    running_time_factor = 10
    running_time_factor_config = ENV["PERFORMANCE_TIME_FACTOR"]
    if(running_time_factor_config == nil || running_time_factor_config == "")
      running_time_factor = 10
    else
      running_time_factor = running_time_factor_config.to_i
    end
  return [running_time_factor,1].max
end

Given (/^I run all performance test case on mobile devices$/) do
 
  p2p_each_step_running_time = 9*getPerformanceRuningTimeFactor()
 #mobile_device_count =  WX2Calabash.deviceman.count_android + WX2Calabash.deviceman.count_ios;
 test_steps=[
#    ["screen_ppt_2880_1800_RGBA.raw","3","#{p2p_each_step_running_time}","video_640_360_24_i420.yuv","audio_1_8000_16.pcm"],
#    ["screen_video_2880_1800_RGBA.raw","3","#{p2p_each_step_running_time}","video_640_360_24_i420.yuv","audio_1_8000_16.pcm"],
#    ["screen_video_2880_1800_RGBA.raw","5","#{p2p_each_step_running_time}","video_640_360_24_i420.yuv","audio_1_8000_16.pcm"],
    ["screen_ppt_2880_1800_RGBA.raw","5","#{p2p_each_step_running_time}","video_640_360_24_i420.yuv","audio_1_8000_16.pcm"]
  ]
  steps %{
        Given I have 1 mac and all mobile
      }
 mobile_device_count=@devices.length-1
 p "TA>>Performance>>total_devices=#{mobile_device_count}"

 for i in 1..mobile_device_count.to_i
    test_steps.each do |row|
      SceenCaptureFile = row[0]
      capture_fps  = row[1]
      running_time  = row[2]
      VideoCaptureFile = row[3]
      AudioCaptureFile = row[4]
      cur_mobile_device_index = i+1
      p "TA>>Performance>>testing on mobile device index=#{i}, id=#{@devices[i].deviceID}"
       steps %{
          Given I screen share file capture resource #{SceenCaptureFile}
          Then I keep the screenshare for 5 seconds
                When I create disableQos offer on device 1 with the default setting and:
                            | ICE     | true  |
                            | SHARE   | sharer   |
                            | SOURCE    | screen: |
                            | SHARE_SOURCE_FILE | #{SceenCaptureFile} |
                            | AUDIO   | true    |
                            | VIDEO   | true   |
                            | VIDEO_SOURCE_FILE | #{VideoCaptureFile} |
                            | AUDIO_SOURCE_FILE | #{AudioCaptureFile} |
                            And I set offer to device #{cur_mobile_device_index} with the default setting and:
                            | ICE     | true  |
                            | SHARE   | viewer  |
                            | AUDIO   | true    |
                            | VIDEO   | true    |
              Then I should receive answer from device #{cur_mobile_device_index} and set to device 1
              Then I change screen capture fps to #{capture_fps} on device 1
              Then I print info trace log TA>>Performance>>Step>>Audio+Send_V+Rcv_V+Screen  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I keep the screenshare for #{running_time} seconds
              Then I print info trace log TA>>Performance>>Step>>Audio+Send_V+Screen  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I stop local video track on device 1
              Then I stop remote video track on device #{cur_mobile_device_index}
              Then I keep the screenshare for #{running_time} seconds
              Then I print info trace log TA>>Performance>>Step>>Audio+Send_V+Rcv_V  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I stop local sharing track on device 1
              Then I stop remote sharing track on device #{cur_mobile_device_index}
              Then I start local video track on device 1
              Then I start remote video track on device #{cur_mobile_device_index}
              Then I keep the screenshare for #{running_time} seconds  
              Then I print info trace log TA>>Performance>>Step>>Audio+Rcv_V  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I stop remote video track on device 1    
              Then I stop local video track on device #{cur_mobile_device_index}
              Then I keep the screenshare for #{running_time} seconds
              Then I print info trace log TA>>Performance>>Step>>Audio+Screen  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I start local sharing track on device 1
              Then I start remote sharing track on device #{cur_mobile_device_index}
              Then I stop local video track on device 1
              Then I stop remote video track on device #{cur_mobile_device_index}  
              Then I keep the screenshare for #{running_time} seconds
              Then I print info trace log TA>>Performance>>Step>>Screen  with sceen_source=#{SceenCaptureFile} fps=#{capture_fps} ,audio_source=#{AudioCaptureFile},video_source=#{VideoCaptureFile} on device #{cur_mobile_device_index}
              Then I stop local audio track on device #{cur_mobile_device_index}
              Then I stop remote audio track on device #{cur_mobile_device_index}
              Then I keep the screenshare for #{running_time} seconds
              Then I stop the call on device 1
              Then I stop the call on device #{cur_mobile_device_index}
              Then I keep the screenshare for 5 seconds
      } 
    end
end

end

Given (/^I run all loopback performance test case on mobile devices$/) do
 
  loopback_running_time = 120*getPerformanceRuningTimeFactor()
  steps %{
        Given I have all mobile device
      }
 mobile_device_count=@devices.length
 p "TA>>Performance>>total_devices=#{mobile_device_count}"
 
for i in 0..mobile_device_count.to_i-1
      cur_mobile_device_index = i+1
      p "TA>>Performance>>loopback testing on mobile device index=#{i}, id=#{@devices[i].deviceID}"
       steps %{
          Then I prepare video feed source on device #{cur_mobile_device_index}
          Then I prepare audio feed source on device #{cur_mobile_device_index}
          Then I print info trace log TA>>Performance>>mobile loopback>>software video codec on device #{cur_mobile_device_index}
          When I start loopback call on device #{cur_mobile_device_index} with:
            |global|{"enableQos": false,"enableAVCSimulcast": false,"loopback": true}|
            |video|{"bHWAcceleration": false}|
            |global_param|{"enablePerformanceStatDump": "All"}|
            |feature|{"calliope": false}|
          Then I keep the call for #{loopback_running_time} seconds
          Then I stop the call on device #{cur_mobile_device_index}
          Then I clean video feed source on device #{cur_mobile_device_index}
          Then I clean audio feed source on device #{cur_mobile_device_index}
      } 
end

end



Given (/^I run all loopback hardware performance test case on mobile devices$/) do
 
  loopback_running_time = 60
  steps %{
        Given I have all mobile device
      }
 mobile_device_count=@devices.length
 p "TA>>Performance>>total_devices=#{mobile_device_count}"
 
for i in 0..mobile_device_count.to_i-1
      cur_mobile_device_index = i+1
      p "TA>>Performance>>loopback testing on mobile device index=#{i}, id=#{@devices[i].deviceID}"
       steps %{
          Then I prepare video feed source on device #{cur_mobile_device_index}
          Then I prepare audio feed source on device #{cur_mobile_device_index}
          Then I print info trace log TA>>Performance>>mobile loopback>>hardware video codec on device #{cur_mobile_device_index}
          When I start loopback call on device #{cur_mobile_device_index} with:
            |global|{"enableQos": false,"enableAVCSimulcast": false,"loopback": true}|
            |video|{"bHWAcceleration": true}|
            |global_param|{"enablePerformanceStatDump": "All"}|
            |feature|{"calliope": false}|
          Then I keep the call for #{loopback_running_time} seconds
          Then I stop the call on device #{cur_mobile_device_index}
          Then I clean video feed source on device #{cur_mobile_device_index}
          Then I clean audio feed source on device #{cur_mobile_device_index}
      } 
end

end

Then(/^I set Share initial bandwidth as (\d+)kbps for device (\d+)$/) do |initBandWidth, idxstr|
    uInitBandWidth=initBandWidth.to_i*1024
    my_json = {:caller => true, :param => {:video => {:uInitBandWidth => uInitBandWidth}}}
    idx = idxstr.to_i - 1
    @devices[idx].backdoor("backdoorSetShareParam", my_json.to_json)
    
end



Then(/^Share evaluated bandwidth should (\W+) (\d+)kbps$/) do |op, expectedBW|
    expectedBW = expectedBW.to_i
    for device in @devices
        stat = get_stats(device)
        actualBW = stat["shareOutNet"]["uBitRate"]/1024
        
        case op
            when "<="
            raise "Device:#{device.deviceID} share evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW > expectedBW
            when "<"
            raise "Device:#{device.deviceID} share evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW >= expectedBW
            when ">="
            raise "Device:#{device.deviceID} share evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW < expectedBW
            when ">"
            raise "Device:#{device.deviceID} share evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW <= expectedBW
            when "=="
            raise "Device:#{device.deviceID} share evaluated bandwidth expect #{op} #{expectedBW}, actual #{actualBW}" if actualBW != expectedBW
        end
    end
end

def getaudiodir(device)
  doc_path = ""
  class_name = device.class.name
  if(class_name == "WX2Calabash::AdroidDevice")
    doc_path = "/sdcard/WebExMediaEngine/"
  elsif (class_name == "WX2Calabash::IOSDevice")
    doc_path = "/Documents/WebExMediaEngine"
    #iFileTransfer has bug ,don't help create dir, create it
    tmp_dir = File.join(Dir.tmpdir, "WebExMediaEngine")
    FileUtils::mkdir_p(tmp_dir)
    device.pushFile(tmp_dir,"/Documents/");
    p "tmp_dir=#{tmp_dir}"
    FileUtils.rm_rf(tmp_dir)
  elsif (class_name == "WX2Calabash::WinMacDevice")
    if(device.sys_type == :windows)
      doc_path = ".\\WebExMediaEngine\\"
    else
      doc_path = "/tmp/WebExMediaEngine/"
    end
  else
    doc_path = ""
    raise "not supported device type"
  end
  return doc_path
end

def getAudioConfigOS(device)
  os_config_name = ""
  class_name = device.class.name
  p "class_name=#{class_name}"
  if(class_name == "WX2Calabash::AdroidDevice")
    os_config_name = "Android"
  elsif (class_name == "WX2Calabash::IOSDevice")
    os_config_name = "IOS"
  elsif (class_name == "WX2Calabash::WinMacDevice")
    if(device.sys_type == :windows)
      os_config_name = "Windows"
    else
      os_config_name = "MACOS"
    end
  else
    os_config_name = ""
    raise "not supported device type"
  end
  return os_config_name
end

Then(/^I (prepare|clean) (audio|video) feed source on device (\d+)$/) do |action, type, which_device|
  is_prepare = action.include? "prepare"
  is_video = type.include? "video"
  idx = which_device.to_i - 1
  p "is_prepare=#{is_prepare},is_video=#{is_video},device_idx=#{idx}"

    
  device_test_dir = getaudiodir(@devices[idx])  
  #feed files
  audio_feed_filename = "Audio_Capture_replace-short16-48000.wav"
  video_feed_filename = "Video_Capture_720p_IWseVideoSample.dump"
  audio_feed_pathfilename_src = File.join(Dir.tmpdir,audio_feed_filename)
  video_feed_pathfilename_src = File.join(Dir.tmpdir,video_feed_filename)
  audio_feed_pathfilename_device_dst = File.join(device_test_dir,audio_feed_filename)
  video_feed_pathfilename_device_dst = File.join(device_test_dir,video_feed_filename)
  audio_config_file_pathname = File.join(device_test_dir,"settings.json")
  
  p "audio_feed_pathfilename_src=#{audio_feed_pathfilename_src}"
  p "video_feed_pathfilename_src=#{video_feed_pathfilename_src}"
  p "audio_feed_pathfilename_device_dst=#{audio_feed_pathfilename_device_dst}"
  p "video_feed_pathfilename_device_dst=#{video_feed_pathfilename_device_dst}"
  
  unless(File.exist?(audio_feed_pathfilename_src))
    cmds = "curl -L -o #{audio_feed_pathfilename_src} https://raw.githubusercontent.com/vagouzhou/test_data/master/Audio_Capture_replace-short16-48000.wav"
    p cmds
    system(cmds)
  end
  
  unless(File.exist?(video_feed_pathfilename_src))
    cmds = "curl -L -o #{video_feed_pathfilename_src} https://raw.githubusercontent.com/vagouzhou/test_data/master/Video_Capture_720p_IWseVideoSample.dump"
    p cmds
    system(cmds)
  end
  
  if is_prepare
    if is_video
      @devices[idx].pushFile("#{video_feed_pathfilename_src}",video_feed_pathfilename_device_dst)
      backdoorConfig_param = {:video_config => {:video_capture_feed_file => "#{video_feed_pathfilename_device_dst}",:video_size => "720"}}
      @devices[idx].backdoor("backdoorConfig", backdoorConfig_param.to_json)
    else
      @devices[idx].pushFile("#{audio_feed_pathfilename_src}",audio_feed_pathfilename_device_dst)
=begin
      backdoorConfig_param = {:audio_config => {:audio_capture_feed_file => "#{audio_feed_pathfilename_device_dst}"}}
      @devices[idx].backdoor("backdoorConfig", backdoorConfig_param.to_json)
=end
      #audio config file 
      audio_config_file_pathanem_tmp = File.join(Dir.tmpdir, "tmp.json")
      p audio_config_file_pathanem_tmp;
      
      audio_capture_replace_file_name = "#{audio_feed_pathfilename_device_dst}"
      class_name = @devices[idx].class.name
      if (class_name == "WX2Calabash::IOSDevice")
        audio_capture_replace_file_name = "/WebExMediaEngine/#{audio_feed_filename}"
      end
      
      json_settings = {:AudioEngineSettings =>
        {:"#{getAudioConfigOS(@devices[idx])}"=>
          {:DebugSetting =>
            {:FeedSource =>
              {
                :Enable => 1,
                :Format => 0,
                :CaptureReplaceFileName => "#{audio_capture_replace_file_name}" ,
                :CaptureReplaceCircleEnable => 1,
              }
            }
          }
        }
      }
      File.open("#{audio_config_file_pathanem_tmp}","w") do |f|
          f.write(json_settings.to_json)
      end
      p "audio_config_file_pathanem_tmp=#{audio_config_file_pathanem_tmp},audio_config_file_pathname=#{audio_config_file_pathname}" 
      @devices[idx].pushFile(audio_config_file_pathanem_tmp,audio_config_file_pathname)
      FileUtils.rm_rf(audio_config_file_pathanem_tmp)
    end
  else
    if is_video
      @devices[idx].deletePath(video_feed_pathfilename_device_dst)
    else
      @devices[idx].deletePath(audio_feed_pathfilename_device_dst)
      @devices[idx].deletePath(audio_config_file_pathname)
    end
  end
end
