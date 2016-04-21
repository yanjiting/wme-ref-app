
def start_loopback_call(call, params = nil, deviceId = nil)
    is_file = call.include? "file"
    is_timestamp = call.include? "timestamp"
    is_loopback = call.include? "loopback"
    is_p2p = call.include? "p2p"
    is_mute = call.include? "mute"
    is_multilayer = call.include? "multilayer"
    is_loopfile = false
    is_limitnal = call.include? "limitNal"
    is_train = call.include? "train"
    is_dtls_srtp = call.include? "dtls_srtp"
    is_svs = call.include? "svs"
    idx = 0
    if(deviceId)
      idx = deviceId
    end
    
    if call.include? "loopFile"
        is_file = true
        is_loopfile = true
    end
    if call.include? "disableQos"
        enableQos = false
    else
        enableQos = true     
    end
    rs = 2
    class_name = @devices[idx].class.name
    if(is_file)
        push_capture_file(@devices[0], params)
        rs = set_capture_file(call, class_name, 0)
    end
        
    linus_url_str = ENV['LINUS_SERVER']
    linus_url_str = "" if(ENV['LINUS_SERVER'] == nil) 
    my_json = {:loopback => is_loopback,
               :linus => linus_url_str,
               :filemode => is_file,
               :p2p => is_p2p,
               :svs => is_svs,
               :train => is_train,
               :mute => is_mute,
               :multilayer => is_multilayer,
               :timestamp => is_timestamp,
               :isLoopFile => is_loopfile,
               :limitNalSize => is_limitnal,
               :resolution => rs,
               :enableQos => enableQos,
               :dtls_srtp => is_dtls_srtp}
    
    if(params)
                # params should override those in the description
                @devices[idx].backdoor("backdoorStartCall", params.merge(my_json).to_json)
    else
        @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    end	
end

def default_call_options()
    linus_url_str = ENV['LINUS_SERVER']
    linus_url_str = "" if(ENV['LINUS_SERVER'] == nil) 
    options = {:loopback => false,
               :linus => linus_url_str,
               :files => {
                    :audio => {
                        "source" => "",
                        "render" => "",
                        "loop" => true
                    },
                    :video => {
                        "source" => "",
                        "render" => "",
                        "loop" => true
                    }
               },
               :filemode => false,
               :p2p => false,
               :svs => false,
               :mute => false,
               :multilayer => true,
               :timestamp => false,
               :limitNalSize => false,
               :resolution => 2,
               :enableQos => true,
               :overrideip => "", 
               :overrideports => {},
               :option => {"audio" => true, "video" => true, "share" => ""},
               :params => {}
               }
end

def default_call_options_test()
    linus_url_str = ENV['LINUS_SERVER']
    linus_url_str = "" if(ENV['LINUS_SERVER'] == nil) 
    options = {:loopback => false,
               :linus => linus_url_str,
            #	:files => {
            #		:audio => {
            #			"source" => "",
            #			"render" => "",
            #			"loop" => true
            #		},
            #		:video => {
            #			"source" => "",
            #			"render" => "",
            #			"loop" => true
            #		}
            #  },
               :filemode => false,
               :p2p => false,
               :mute => false,
               :multilayer => true,
               :timestamp => false,
               :limitNalSize => false,
               :resolution => 2,
               :enableQos => true,
                     :overrideip => "", 
               :overrideports => {},
                     :option => {"audio" => true, "video" => true, "share" => "", "share_mline_fake" => false},
                     :params => {},
            #	:isLoopFile => false,
            #	:ice => false,
            #	:mux => true,
            #	:srtp => false,
                     :sharesource => "",
                     :sharesource_file => "",
                     :videosouce_file => "",
                     :audiosouce_file => "" 
               }
end

def make_options_params(table)
    # options = default_call_options
    options = default_call_options_test
    table.raw.each do |row|
        key = row[0]
    value = row[1]
    
        begin
            value = JSON.parse(value)
            value = value.inject({}){|memo,(k,v)| memo[k.to_sym] = v; memo}
        rescue JSON::ParserError
            next
        end

      if(key == "global")
            options.merge!(value)
        elsif(key == "files")	
            options[:files] = {} unless(options[:files])
            options[:files].merge!(value)
        elsif(key == "feature")
            options[:option] = {} unless(options[:option])
            options[:option].merge!(value)
        elsif(key == "video" || key == "audio" || "global_param")
            options[:params][key] = {} unless(options[:params][key])
            options[:params][key].merge!(value)
        end
    end
    options
end

def create_offer_simple(idx, options)
    push_capture_file(@devices[idx], options)
    @devices[idx].backdoor("backdoorStartCall", options.to_json)
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

def make_params(table)
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
    return {:params => params}
end

When(/^I start (.*) call with:$/) do |call, table|
    start_loopback_call(call, make_options_params(table))
end

When(/^I start (.*) call$/) do |call|
    start_loopback_call(call)
end

When(/^I start (.*) call on device (\d+)$/) do |call, idxstr|
  start_loopback_call(call,nil,idxstr.to_i - 1)
end

When(/^I start (.*) call on device (\d+) with:$/) do |call, idxstr, table|
  start_loopback_call(call,make_options_params(table),idxstr.to_i - 1)
end

Then(/^I start loopback call (\d+) times$/) do |num_str|
   num = num_str.to_i
   num.times do |x|
       start_loopback_call("loopback")
       sleep 10
       @devices.each do |device|
           device.backdoor("backdoorStopCall", nil)
       end
       sleep 5
    end
end

def capture_type(capture)
    ct = 0;
    ncapture = capture.to_i
    if( ncapture > 0 && ncapture <= 24 )
        ct = ncapture
        elsif( capture == "Capture" )
        ct = 1
        elsif (capture == "Playback" )
        ct = 2
        elsif (capture == "SA_Post_Before_AEC_In" )
        ct = 3
        elsif (capture == "SA_Post_Before_AEC_Out" )
        ct = 4
        elsif (capture == "SA_Pre_Before_AEC_In" )
        ct = 5
        elsif (capture == "SA_Pre_Before_AEC_Out" )
        ct = 6
        elsif (capture == "SA_Post_After_AEC_In" )
        ct = 7
        elsif (capture == "SA_Post_After_AEC_Out" )
        ct = 8
        elsif (capture == "SA_Pre_After_AEC_In" )
        ct = 9
        elsif (capture == "SA_Pre_After_AEC_Out" )
        ct = 10
        elsif (capture == "Encoder" )
        ct = 11
        elsif (capture == "Decoder" )
        ct = 12
        elsif (capture == "Mix_In" )
        ct = 13
        elsif (capture == "Mix_Out" )
        ct = 14
        elsif (capture == "AEC_NearIn" )
        ct = 15
        elsif (capture == "AEC_FarIn" )
        ct = 16
        elsif (capture == "AEC_Out" )
        ct = 17
        elsif (capture == "AAGC_In" )
        ct = 18
        elsif (capture == "AAGC_Out" )
        ct = 19
        elsif (capture == "DAGC_In" )
        ct = 20
        elsif (capture == "DAGC_Out" )
        ct = 21
        elsif (capture == "NR_In" )
        ct = 22
        elsif (capture == "NR_Out" )
        ct = 23
        elsif (capture == "All" )
        ct = 24
    end
    return ct
end

def dump_path(device, create_if_not_exist)
    class_name = device.class.name
    if( class_name == "WX2Calabash::AdroidDevice" )
        local_path = File.join(Dir.tmpdir, "audio_android")
        device_path = "/sdcard/audiodump"
        if( create_if_not_exist != nil and create_if_not_exist )
            FileUtils.remove_dir(local_path, true);
            FileUtils.mkdir_p(local_path)
            device.pushFile(local_path, device_path)
        end
    elsif( class_name == "WX2Calabash::IOSDevice" )
        local_path = File.join(Dir.tmpdir, "audiodump")
        device_path = "/Documents/audiodump"
        if( create_if_not_exist != nil and create_if_not_exist )
            FileUtils.remove_dir(local_path, true);
            FileUtils.mkdir_p(local_path)
            device.pushFile(local_path, "/Documents")
        end
            
    elsif( class_name == "WX2Calabash::WinMacDevice" )
        device_path = File.join(Dir.tmpdir, "audio_dump")
        if( create_if_not_exist != nil and create_if_not_exist )
            FileUtils.mkdir_p(device_path)
        end
    end
    return device_path;
end

def prepare_play_audio(device, file, capture)
    class_name = device.class.name
    if(class_name == "WX2Calabash::AdroidDevice")
        setting_path = "/sdcard/WebExMediaEngine/"
        elsif (class_name == "WX2Calabash::IOSDevice")
        setting_path = "/WebExMediaEngine"
        elsif (class_name == "WX2Calabash::WinMacDevice")
        if(@devices[0].sys_type == :windows)
            setting_path = ".\\WebExMediaEngine\\"
            else
            setting_path = "/tmp/WebExMediaEngine/"
        end
        else
        raise "not supported device type"
    end
    
    dest_sound_file = File.join(setting_path, "#{file}.wav")
    path = File.join(Dir.tmpdir, "WebExMediaEngine")
    FileUtils.mkdir_p(path)
    tempfile = File.join(path, "settings.json")
    default_engine_settings = JSON.parse(File.read("settings.json"));
    debug_settings = {:FeedSource =>
            {
                :Enable => 1,
                :Format => 0,
                :PlayoutType => 0,
                :PlayoutFileName => "#{dest_sound_file}"}
        }
    ct = 0;
    if( capture != nil && capture.strip.length > 0 )
        if( capture.start_with?(" and capture ") )
            capture = capture[13,64].strip
            ct = capture_type(capture)
        end
        p "capture = #{capture}, capture_type = #{ct}"
        if( ct != 0 )
            capture_setting =
                {:DumpFile =>
                    {
                        :Format => 0,
                        :Modules => [ct]}
                }
                debug_settings = capture_setting.merge(debug_settings)
        else
            p "Warning: invalid capture string #{capture}"
        end
    end
    debug_settings = {:DebugSetting=>debug_settings}
    play_settings = Hash.new
    default_engine_settings["AudioEngineSettings"].each{|key, value|
        value = value.merge!(debug_settings)
        value = JSON.parse(value.to_json);
        play_settings[key] = value
    }
    play_settings = {:AudioEngineSettings => play_settings}
    File.open(tempfile, 'w') { |fo| fo.puts play_settings.to_json }
    if( class_name == "WX2Calabash::WinMacDevice" )
        FileUtils.mkdir_p(setting_path)
        FileUtils.copy(tempfile, File.join(setting_path, "settings.json"))
        FileUtils.copy(expand_local_path("../../sample/#{file}.wav"), dest_sound_file)
    else
        device.pushFile(path, "/Documents")
        if( class_name == "WX2Calabash::IOSDevice" )
            device.pushFile(expand_local_path("../../sample/#{file}.wav"), "/Documents/#{dest_sound_file}")
        else
            device.pushFile(expand_local_path("../../sample/#{file}.wav"), dest_sound_file)
        end
    end
end

Then /^I start play sound (.*) on device (\d+)( and capture .*|)/ do |wavfile, deviceIdx, capture|
    idx = deviceIdx.to_i - 1
    adevice = @devices[idx]
    prepare_play_audio(adevice, wavfile, capture)

    call_setting = {:loopback => true,
        :mute => false,
        :audioDumpPath => dump_path(adevice, true),
                :option => {"audio" => true, "video" => false, "share" => ""}
            }
    audio_only_options = default_call_options.merge(call_setting);
    p audio_only_options
    adevice.backdoor("backdoorStartCall", audio_only_options.to_json)
    class_name = adevice.class.name
    if( class_name != "WX2Calabash::WinMacDevice" )
        sleep 3
        adevice.backdoor("backdoorSetPlaybackDevice", "Speaker")
    end
    
end

Then /^I stop play sound on device (\d+)/ do |deviceIdx|

    idx = deviceIdx.to_i - 1
    device = @devices[idx]
    device.backdoor("backdoorStopCall", nil)
    
    class_name = device.class.name
    
    if(class_name == "WX2Calabash::AdroidDevice")
        setting_path = "/sdcard/WebExMediaEngine/"
    elsif (class_name == "WX2Calabash::IOSDevice")
        setting_path = "/Documents/WebExMediaEngine"
    elsif (class_name == "WX2Calabash::WinMacDevice")
        if(device.sys_type == :windows)
            setting_path = ".\\WebExMediaEngine\\"
            else
            setting_path = "/tmp/WebExMediaEngine/"
        end
    else
        raise "not supported device type"
    end

    if( class_name == "WX2Calabash::WinMacDevice" )
        FileUtils.remove_dir(setting_path, true)
    elsif( class_name == "WX2Calabash::AdroidDevice" )
        path = File.join(Dir.tmpdir, "WebExMediaEngine")
        FileUtils.mkdir_p(path)
        tempfile = File.join(path, "settings.json")
        File.open(tempfile, 'w') { |fo| fo.puts "" }
        device.pushFile(tempfile, File.join(setting_path, "settings.json"))
        device.deletePath(setting_path)
    elsif( class_name == "WX2Calabash::IOSDevice" )
            device.deletePath(setting_path)
    end
end

def copy_captured(file, deviceIdx)
    idx = deviceIdx.to_i - 1
    device = @devices[idx]
    class_name = device.class.name
    
    local_path = File.join(Dir.tmpdir, "CapturedSound")
    FileUtils.mkdir_p(local_path)
    device_path = dump_path(device, false)
    
    path_with_datetime = local_path + DateTime.now.strftime("%Y_%m_%d_%H_%M_%S")
    if( class_name == "WX2Calabash::WinMacDevice" )
        FileUtils.move(device_path, path_with_datetime);
    else
    FileUtils.mkdir_p(path_with_datetime);
        device.fetchFile(device_path, path_with_datetime);
    end

    p "Capture file is in folder: #{path_with_datetime}"
end

Then /^I copy captured file (.*) from device (\d+)/ do |file, deviceIdx|
    copy_captured(file, deviceIdx)
end

Then /^I copy captured file from device (\d+)/ do |deviceIdx|
    copy_captured(nil, deviceIdx)
end
        
def create_offer(offerType, idx, params = nil)
    p "offerType=#{offerType}"
    is_p2p = offerType.include? "p2p"
    is_srtp = offerType.include? "srtp"
    is_dtls_srtp = offerType.include? "dtls_srtp"
    is_multilayer = offerType.include? "multilayer"
    is_file = offerType.include? "file" 
    is_loopfile = false
    if offerType.include? "loopFile" 
        is_file = true
        is_loopfile = true
    end

    if offerType.include? "disableQos"
        enableQos = false
    else
        enableQos = true     
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

    overrideip = ""
    overrideports = {}
    if(offerType.include? "overrideip")
        other = (idx == 0) ? 1 : 0
        overrideip = @devices[other].ipaddr
        overrideports["audio"] = 36788
        overrideports["video"] = 36790
        overrideports["sharing"] = 36792
    end
    if(@fakeAnswer && @fakeAnswer[idx])
        overrideip = @fakeAnswer[idx]["ip"]
        overrideports["audio"] = @fakeAnswer[idx]["audio"]
        overrideports["video"] = @fakeAnswer[idx]["video"]
        overrideports["sharing"] = @fakeAnswer[idx]["sharing"] || 0
    end
    my_json = {:loopback => false, 
               :p2p => is_p2p, 
               :srtp => is_srtp, 
               :dtls_srtp => is_dtls_srtp,
               :multilayer => is_multilayer,
               :resolution => rs, 
               :overrideip => overrideip, 
               :overrideports => overrideports, 
               :filemode => is_file, 
               :isLoopFile => is_loopfile, 
               :enableQos => enableQos,
               :enablePerformanceStatDump => performanceTraceDump}

    if params != nil
        @devices[idx].backdoor("backdoorStartCall", my_json.merge(params).to_json)
    else
        @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    end
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

When(/^I create (.*)offer on device (\d+) with:$/) do |offerType, idxstr, table|
    create_offer(offerType, idxstr.to_i - 1, make_params(table))
end

When(/^I create (.*)offer on all devices with options and params:$/) do |offerType, table|
    cnt = @devices.length
    cnt.times do |i|
        params = make_options_params(table)
        if(params[:files] && params[:files][:audio])
            params[:files][:audio]["render"] = "auto_#{i}.pcm"  if(params[:files][:audio]["render"].eql?("auto"))
            params[:files][:audio]["source"] = "clientmix/auto#{i}_1_8000_16.pcm" if(params[:files][:audio]["source"].eql?("auto"))
            params[:files][:audio]["source"] = "clientmix/auto#{i*2}_1_8000_16.pcm" if(params[:files][:audio]["source"].eql?("autoeven"))
            params[:files][:audio]["render"] = "auto_#{i*2}.pcm" if(params[:files][:audio]["render"].eql?("autoeven"))
            params[:files][:audio]["render"] = "o_#{i}.pcm"  if(params[:files][:audio]["render"].eql?("overlap"))
            params[:files][:audio]["source"] = "clientmix/o#{i}_1_8000_16.pcm" if(params[:files][:audio]["source"].eql?("overlap"))
        end        
        
        create_offer(offerType, i, params)
    end
end

When(/^I create offer on (\d+)(?:st|nd|rd|th) device with options and params:$/) do |idxstr, table|
    create_offer_simple(idxstr.to_i - 1, make_options_params(table))
end

When(/^I create offer on (\d+)(?:st|nd|rd|th) device with default options and params$/) do |idxstr|
    create_offer_simple(idxstr.to_i - 1, default_call_options_test())
end

When(/^I create (.*)offer on device (\d+)$/) do |offerType, idxstr|
    create_offer(offerType, idxstr.to_i - 1)
end

When(/^I create (.*)offer on device (\d+) with (.*) = (.*)$/) do |offerType, idxstr, param, value|
    my_param = {}
    my_param[param] = value.to_i 
    create_offer(offerType, idxstr.to_i - 1, my_param)
end

When(/^I create (.*)offer on device (\d+) with BGRA=(true|false)$/) do |offerType, idxstr, bgraenable|
    idx = idxstr.to_i - 1
    isBgra = bgraenable.to_bool
    isP2p = offerType.include? "p2p"
    isMultilayer = offerType.include? "multilayer"
    
    my_json = {:loopback => false, :p2p => isP2p, :multilayer => isMultilayer, 
        :bgra => isBgra}
        
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

When(/^I start call without creating offer on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    my_json = {:loopback => false}
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    sleep 1
end

When(/^I start call on device (\d+)$/) do |idxstr|
    idx = idxstr.to_i - 1
    linus_url_str = ENV['LINUS_SERVER']
    linus_url_str = "" if(ENV['LINUS_SERVER'] == nil) 
    my_json = {:loopback => true,
               :linus => linus_url_str}
    @devices[idx].backdoor("backdoorStartCall", my_json.to_json)
    sleep 1
end
