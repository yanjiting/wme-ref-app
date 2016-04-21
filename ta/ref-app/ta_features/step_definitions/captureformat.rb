
def push_capture_file(device, options)
	class_name = device.class.name
	audio_src = ""
	video_src = ""
	if (!options.nil?)
		if (!options[:audiosouce_file].nil? && !options[:audiosouce_file].empty?)
			audio_src = options[:audiosouce_file]
		elsif (!options[:files].nil? && !options[:files].empty?)
			if (!options[:files][:audio].nil? && !options[:files][:audio].empty?)
				if (!options[:files][:audio]["source"].nil? && !options[:files][:audio]["source"].empty?)
					audio_src = options[:files][:audio]["source"]
				end
			end
		end
		if !(options[:videosouce_file].nil? || options[:videosouce_file].empty?)
			video_src = options[:videosouce_file]
		elsif (!options[:files].nil? && !options[:files].empty?)
			if (!options[:files][:video].nil? && !options[:files][:video].empty?)
				if (!options[:files][:video]["source"].nil? && !options[:files][:video]["source"].empty?)
					video_src = options[:files][:video]["source"]
				end
			end
		end
	end
    if(class_name == "WX2Calabash::AdroidDevice")
        device.pushFile(expand_local_path("../../sample/#{audio_src}"), "/sdcard/wmetest/#{audio_src}")
        device.pushFile(expand_local_path("../../sample/#{video_src}"), "/sdcard/wmetest/#{video_src}")
    elsif(class_name == "WX2Calabash::IOSDevice")
        #tempWmePath = expand_local_path("../../Temp/wmetest")
        #FileUtils.mkdir_p(tempWmePath)
        #FileUtils.copy(expand_local_path("../../sample/#{audio_src}"),tempWmePath)
        #FileUtils.copy(expand_local_path("../../sample/#{video_src}"),tempWmePath)
        #device.pushFile((tempWmePath), "/Documents/")
        #FileUtils.rmdir(tempWmePath);
        device.pushFile(expand_local_path("../../sample/#{audio_src}"), "/Documents/wmetest@#{audio_src}")
        device.pushFile(expand_local_path("../../sample/#{video_src}"), "/Documents/wmetest@#{video_src}")
	end
end

def set_capture_file(call, class_name, idx)
    rs = convVideoLevel(call)
    
    if(class_name == "WX2Calabash::AdroidDevice")
        @devices[idx].pushFile(expand_local_path("../../sample/audio_1_8000_16.pcm"), "/sdcard/wmetest/audio_1_8000_16.pcm")
        if (0 == rs)
            @devices[idx].pushFile(expand_local_path("../../sample/video_160_90_6_i420.yuv"), "/sdcard/wmetest/video_160_90_6_i420.yuv")
        elsif (1 == rs)
            @devices[idx].pushFile(expand_local_path("../../sample/video_320_180_12_i420.yuv"), "/sdcard/wmetest/video_320_180_12_i420.yuv")
        elsif (2 == rs)
            @devices[idx].pushFile(expand_local_path("../../sample/video_640_360_24_i420.yuv"), "/sdcard/wmetest/video_640_360_24_i420.yuv")
        else
            @devices[idx].pushFile(expand_local_path("../../sample/video_1280_720_30_i420.yuv"), "/sdcard/wmetest/video_1280_720_30_i420.yuv")
        end
    end

    return rs
end

def GetDeviceRootWmeTest(device)
    retobj = nil
    class_name = device.class.name
    if(class_name == "WX2Calabash::AdroidDevice")
        retobj = ["/sdcard/wmetest/dst/", "android"]
    elsif (class_name == "WX2Calabash::IOSDevice")
        retobj = ["/Documents/", "ios"]
    elsif (class_name == "WX2Calabash::WinMacDevice")
        if(device.sys_type == :windows)
            retobj = ["./Temp/", "win"]
        else
            retobj = ["./Temp/", "macosx"]
        end
    else
        raise "not supported device type"
    end
    return retobj
end

def GetDeviceRoot(device)
    retobj = nil
    class_name = device.class.name
    if(class_name == "WX2Calabash::AdroidDevice")
        retobj = ["/sdcard/", "android"]
    elsif (class_name == "WX2Calabash::IOSDevice")
        retobj = ["/Documents/", "ios"]
    elsif (class_name == "WX2Calabash::WinMacDevice")
        if(device.sys_type == :windows)
            retobj = ["./", "win"]
        else
            retobj = ["/tmp/", "macosx"]
        end
    else
        raise "not supported device type"
    end
    return retobj
end

def dtmf_remove_duplicate(val)
	val_array = val.split(//)
	last = ""
	ret_array = []
	val_array.each do |ch|
		if(last != ch)
			ret_array.push(ch)
			last = ch
		end
	end
	ret_array.join("")
end

def detect_dtmf(path, dstpath, verify_code)
    isWin = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
    if(isWin)
        os = "win32"
    else
        os = "mac"
    end

	sox_path = File.absolute_path(File.join(File.dirname(__FILE__), '..', '..', '..', 'vendor', 'sox-14.4.1', os, "sox"))
    system("chmod +x \"#{sox_path}\"") unless isWin
	cmdline = "#{sox_path} -t raw -e signed-integer -b 16 -c 1 -r 16000 -L \"#{path}\" -traw -e signed-integer -b 16 -c 1 -r 8000 -L \"#{dstpath}\""
	Kernel.puts(cmdline)
	system(cmdline)
	dtmf_path = File.absolute_path(File.join(File.dirname(__FILE__), '..', '..', '..', 'vendor', 'dtmf', "dtmf"))
	dtmf_detected = `#{dtmf_path} -f "#{dstpath}"`
	dtmf_detected = dtmf_remove_duplicate(dtmf_detected)
	Kernel.puts("###########DTMF detected as: #{dtmf_detected}, expected=#{verify_code}")
	vcodes = verify_code.split(",")
	bSuccess = false
	vcodes.each do |vcode|
		vcode.strip!
		if(vcode.eql?(dtmf_detected))
			bSuccess = true
			break
		end	
	end
	assert(bSuccess, "dtmf verification failed")
end

Then(/^I check dtmf on all devices$/) do | table |
	stopcall_for_filecapture(@devices)
    dtmf_dir = File.join(ENV["TEST_TRACE_PATH"], "dtmf")
    FileUtils.mkdir_p(dtmf_dir)
	dtmf_raw_files = []
	verify_results = []
	table.raw.each do |row| 
		verify_results.push(row[0])
	end
    @devices.each do |device|
        device_base_dir, target_prefix = GetDeviceRootWmeTest(device)
        src = File.join(device_base_dir, "audio_1_8000_16.pcm")
		dstfilename = "#{target_prefix}_audio_#{device.deviceID}.raw"
        dst = File.join(dtmf_dir, dstfilename)
		dtmf_raw_files.push(dstfilename)
        Kernel.puts("src = #{src} , dst = #{dst}")
        device.fetchFile(src, dst)
    end
	i = 0
	dtmf_raw_files.each do |rawfile|
		detect_dtmf(File.join(dtmf_dir, rawfile), File.join(dtmf_dir, "8k_#{rawfile}"), verify_results[i])
		i = i + 1
	end
end

Then(/^I check dtmf file (\S*) is (\S*)$/) do |filename, results|
	stopcall_for_filecapture(@devices)
    dtmf_dir = File.join(ENV["TEST_TRACE_PATH"], "dtmf")
    FileUtils.mkdir_p(dtmf_dir)

    device_base_dir, target_prefix = GetDeviceRootWmeTest(@devices[0])
    src = File.join(device_base_dir, filename)
    dstfilename = "#{target_prefix}_audio_#{filename}.raw"
    dst = File.join(dtmf_dir, dstfilename)
    Kernel.puts("src = #{src} , dst = #{dst}")
    @devices[0].fetchFile(src, dst)

    detect_dtmf(dst, File.join(dtmf_dir, "8k_#{filename}"), results)
end

Then(/^I got files on (.*) (?:device|devices)\s*(\d*):$/) do |isall, idxstr, table|
    raise("I got files without AST tables") unless(table)
    if(isall.downcase == "all")
        verify_devices = @devices 
    else
        idx = isall.to_i - 1
		verify_devices = []
        verify_devices.push(@devices[idx])
    end 
    
    files_dir = File.join(ENV["TEST_TRACE_PATH"], "files")
    FileUtils.mkdir_p(files_dir)
    verify_devices.each do |device|
        device_base_dir, target_prefix = GetDeviceRoot(device)
        table.raw.each do |row|
            src = File.join(device_base_dir, row[0])
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            Kernel.puts("src = #{src} , dst = #{dst}")
            device.fetchFile(src, dst)
        end
        table.raw.each do |row|
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            raise "I didn't get file:" unless(File.file?(dst))
        end 
    end
end

def stopcall_for_filecapture(chk_devices)
	chk_devices.each do |device|
		device.backdoor("backdoorStopCall", nil) #// comment first, because filecapture release will be blocked in android.
		device.exitapp() if(device.class.name == "WX2Calabash::WinMacDevice" && device.sys_type == :windows)
	end	
end

def check_file_ended(chk_devices, timeout = 8, type = "all", stopcall = true)
    i = 0
    p "before loop capture"
	while (i < timeout)
		all_ended = true
		chk_devices.each do |device|
			if(device.backdoor("backdoorIsFileCaptureEnded", type) != "ended")
				p "waiting for file capture end... (#{device.deviceID})"
				all_ended = false
				break
			else
				p "file capture is ended in device: #{device.deviceID}"
			end
		end
		if(all_ended)
			p "file cpature is ended"
			break
		else
			sleep 1 
		end
		i = i + 1
	end
    
	stopcall_for_filecapture(chk_devices) if stopcall
end

Then(/^I wait for file capture ended$/) do
	check_file_ended([@devices[0]])
end

Then(/^I wait for file capture ended in all devices$/) do
	check_file_ended(@devices, 16, "audio", false)
end
    
Then(/^I got compare echo files on (.*) (?:device|devices)\s*(\d*):$/) do |isall, idxstr, table|
    raise("I got files without AST tables") unless(table)
    if(isall.downcase == "all")
        verify_devices = @devices
        else
        idx = isall.to_i - 1
        verify_devices = []
        verify_devices.push(@devices[idx])
    end
    
    dst = nil
    src = nil
    files_dir = File.join(ENV["TEST_TRACE_PATH"], "files")
    FileUtils.mkdir_p(files_dir)
    verify_devices.each do |device|
        device_base_dir, target_prefix = GetDeviceRoot(device)
        table.raw.each do |row|
            src = File.join(device_base_dir, row[0])
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            Kernel.puts("src = #{src} , dst = #{dst}")
            #p "111 src=#{src}, 222 dst=#{dst}"
            device.fetchFile(src, dst)
        end
        table.raw.each do |row|
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            raise "I didn't get file:" unless(File.file?(dst))
        end
    end
    
    tool_dir =  File.join(File.dirname(__FILE__), '../../sample/mactool ')
    reffiles_dir = File.join(File.dirname(__FILE__), '../../sample/fem_wb.wav ')
    outputfiles_dir = dst
    
    #p "reffiles_dir=#{reffiles_dir}, tool_dir = #{tool_dir},outputfiles_dir = #{outputfiles_dir}"
    str = tool_dir + reffiles_dir + outputfiles_dir
    #p "str = #{str}"
    #system(str)
    a = `#{str}`
    #p a
    start = a.index("##&&BEGIN")
    finish = a.index("##&&END")
    #p start
    #p finish
    val = a[start+9..finish-1]
    #p val
    tt = val.to_f
    p tt
    if ((tt<=>1) == 1)
        #p "Result:Find Echo!!!"
        raise("Compare Result:Find echo!")
    else
        p "Result:NO Echo"
    end
    
end

Then(/^I got calculate mos value on (.*) (?:device|devices)\s*(\d*):$/) do |isall, idxstr, table|
    raise("I got files without AST tables") unless(table)
    if(isall.downcase == "all")
        verify_devices = @devices
        else
        idx = isall.to_i - 1
        verify_devices = []
        verify_devices.push(@devices[idx])
    end
    
    dst = nil
    src = nil
    files_dir = File.join(ENV["TEST_TRACE_PATH"], "files")
    FileUtils.mkdir_p(files_dir)
    verify_devices.each do |device|
        device_base_dir, target_prefix = GetDeviceRoot(device)
        table.raw.each do |row|
            src = File.join(device_base_dir, row[0])
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            Kernel.puts("src = #{src} , dst = #{dst}")
            #p "111 src=#{src}, 222 dst=#{dst}"
            device.fetchFile(src, dst)
        end
        table.raw.each do |row|
            dst = File.join(files_dir, "#{target_prefix}_#{row[0]}")
            raise "I didn't get file:" unless(File.file?(dst))
        end
    end
    
    tool_dir =  File.join(File.dirname(__FILE__), '../../sample/mactool ')
    reffiles_dir = File.join(File.dirname(__FILE__), '../../sample/fem_wb4mos.wav ')
    outputfiles_dir = "/tmp/Playback-short16-Chn1-16000.wav"
    
    #p "reffiles_dir=#{reffiles_dir}, tool_dir = #{tool_dir},outputfiles_dir = #{outputfiles_dir}"
    str = tool_dir + reffiles_dir + outputfiles_dir
    #p "str = #{str}"
    #system(str)
    a = `#{str}`
    #p a
    start = a.index("##&&BEGIN")
    finish = a.index("##&&END")
    #p start
    #p finish
    val = a[start+9..finish-1]
    #p val
    tt = val.to_f
    p tt
    if (tt < 3)
        #p "Low MOS!!!"
        raise("Low MOS!")
    else
        p "High MOS!"
    end
    
end

