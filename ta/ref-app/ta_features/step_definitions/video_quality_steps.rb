def GetRenderFiles(class_name, resolution, file = nil)
    video_path_base = ""
    video_src_base = ""

    if (resolution & 0x20 == 0x20)
        video_src_base = format("%s",file)
        video_path_base = format("%s",file)
    elsif(resolution & 0x0f == 0)
        video_path_base = format("video_%d_%d_6_i420.yuv", 160, 90)
        name_tail = (resolution & 0x10) == 0x10 ? "" : format("_%dx%d",160,90)
        video_src_base = format("video_%d_%d_6_i420%s.yuv", 160, 90, name_tail)
    elsif(resolution & 0x0f == 1)
        video_path_base = format("video_%d_%d_12_i420.yuv", 320, 180)
        name_tail = (resolution & 0x10) == 0x10 ? "" : format("_%dx%d",320,180)
        video_src_base = format("video_%d_%d_12_i420%s.yuv", 320, 180, name_tail)
    elsif(resolution & 0x0f == 2)
        video_path_base = format("video_%d_%d_24_i420.yuv", 640, 360)
        name_tail = (resolution & 0x10) == 0x10 ? "" : format("_%dx%d",640,360)
        video_src_base = format("video_%d_%d_24_i420%s.yuv", 640, 360, name_tail)
    elsif(resolution & 0x0f == 3)
        video_path_base = format("video_%d_%d_30_i420.yuv", 1280, 720)
        name_tail = (resolution & 0x10) == 0x10 ? "" : format("_%dx%d",1280,720)
        video_src_base = format("video_%d_%d_30_i420%s.yuv", 1280, 720, name_tail)
    else
        raise "not supported resolution"
    end

    if(class_name == "WX2Calabash::AdroidDevice")
        audio_path = File.join(ENV["TEST_TRACE_PATH"], "android_audio_1_8000_16.pcm")
        video_path = File.join(ENV["TEST_TRACE_PATH"], format("android_%s",video_path_base))
        audio_src = "/sdcard/wmetest/dst/audio_1_8000_16.pcm"
        video_src = format("/sdcard/wmetest/dst/%s",video_src_base)
    elsif (class_name == "WX2Calabash::IOSDevice")
        audio_path = File.join(ENV["TEST_TRACE_PATH"], "ios_audio_1_8000_16.pcm")
        video_path = File.join(ENV["TEST_TRACE_PATH"], format("ios_%s",video_path_base))
        audio_src = "/Documents/audio_1_8000_16.pcm"
        video_src = format("/Documents/%s",video_src_base)
    elsif (class_name == "WX2Calabash::WinMacDevice")
        if(@devices[0].sys_type == :windows)
            audio_path = File.join(ENV["TEST_TRACE_PATH"], "win_audio_1_8000_16.pcm")
            video_path = File.join(ENV["TEST_TRACE_PATH"], format("win_%s",video_path_base))
            audio_src = File.join(WINDOWS_TEMP_PATH, "audio_1_8000_16.pcm")
            video_src = File.join(WINDOWS_TEMP_PATH, video_src_base)
        else
            audio_path = File.join(ENV["TEST_TRACE_PATH"], "mac_audio_1_8000_16.pcm")
            video_path = File.join(ENV["TEST_TRACE_PATH"], format("mac_%s",video_path_base))
            audio_src = File.join(WINDOWS_TEMP_PATH, "audio_1_8000_16.pcm")
            video_src = File.join(WINDOWS_TEMP_PATH, video_src_base)
        end
    else
        raise "not supported device type"
    end

	p video_src
	p video_path
    return [audio_path,video_path,audio_src,video_src]
end

And(/^I got all render files$/) do
    class_name = @devices[0].class.name
    render_files_device = ""
    dest_dir = ""
    if(class_name == "WX2Calabash::AdroidDevice")
        render_files_device = "/sdcard/wmetest/dst/"
    elsif (class_name == "WX2Calabash::IOSDevice")
        render_files_device = "/Documents/"
    elsif (class_name == "WX2Calabash::WinMacDevice")
        if(@devices[0].sys_type == :windows)
            render_files_device = File.join(WINDOWS_TEMP_PATH, "/")
        else
            render_files_device = File.join(WINDOWS_TEMP_PATH, "/")
        end
    else
        raise "not supported device type"
    end

    dest_dir = File.join(ENV["TEST_TRACE_PATH"], "#{@devices[0].deviceID}")

    p "copy from #{render_files_device} to #{dest_dir}"
    @devices[0].fetchFile(render_files_device, dest_dir)
    @devices[0].deletePath(render_files_device)
end

And(/^I got audio and (.*) video render files$/) do |rs|
    class_name = @devices[0].class.name
    if (rs == "sld")
        resolution = 0
    elsif (rs == "ld")
        resolution = 1
    elsif (rs == "sd")
        resolution = 2
    elsif (rs == "hd")
        resolution = 3
    else
        raise "wrong rs parameters"
    end
    files = GetRenderFiles(class_name, resolution)
    audio_path = files[0]
    video_path = files[1]
    audio_src = files[2]
    video_src = files[3]
    @devices[0].fetchFile(audio_src, audio_path)
    @devices[0].fetchFile(video_src, video_path)
    raise "audio render file is not found" unless(File.file?(audio_path))
    raise "video render file is not found" unless(File.file?(video_path))
    
    #check_sample_files(expand_local_path("../../sample/audio_1_8000_16.pcm"), audio_path)
    #check_video_files(expand_local_path("../../sample/video_160_90_6_i420.yuv"), video_path)
end

Then(/^I got good quality of (.*) video (\d+) with received psnr > (\d+), average psnr > (\d+) and loss frame rate < (\d+)%$/) do |rs, idx, psnr_r, psnr_a, loss|
    class_name = @devices[0].class.name
    if (rs == "sld")
        resolution = 0
        video_src_path = "../../sample/video_160_90_6_i420.yuv"
    elsif (rs == "ld")
        resolution = 1
        video_src_path = "../../sample/video_320_180_12_i420.yuv"
    elsif (rs == "sd")
        resolution = 2
        video_src_path = "../../sample/video_640_360_24_i420.yuv"
    elsif (rs == "hd")
        resolution = 3
        video_src_path = ""
    else
        raise "wrong rs parameters"
    end
    files = GetRenderFiles(class_name,resolution | 0x10)
    video_path = files[1]
    video_dst = files[3]
    @devices[0].fetchFile(video_dst, video_path)

    raise "video render file is not found" unless(File.file?(video_path))

    video_dst_path = format("%s/%s_%d.yuv",File.dirname(video_path),File.basename(video_path,".yuv"),idx)
    File.rename(video_path,video_dst_path)
    check_video_files_ts(expand_local_path(video_src_path),video_dst_path, resolution, psnr_r, psnr_a, loss)
end

Then (/^I got good quality of (\d+)Kbps video from device (\d+), file name is:$/) do |bitrate, idxstr, table|
	idx = idxstr.to_i - 1
	p bitrate
	class_name = @devices[idx].class.name
	file = ""
	table.raw.each do |row|
		file = row[0]
    end
	resolution = 0x20
	file_path = file
	files = GetRenderFiles(class_name, resolution, file_path)

	video_path = files[1]
    video_dst = files[3]
    @devices[0].fetchFile(video_dst, video_path)

    raise "video render file is not found" unless(File.file?(video_path))

	video_src_path = format("../../sample/%s",file)
    video_dst_path = format("%s/%s_%d_%d.yuv",File.dirname(video_path),File.basename(video_path,".yuv"),bitrate,idx)
    File.rename(video_path,video_dst_path)
    check_video_files_quality(expand_local_path(video_src_path),video_dst_path,bitrate)
end

def check_video_files(src, dst)
    input = File.open(src, 'r')
    output = File.open(dst, 'r')

    size = [input.size, output.size].min
	error = 0.0

	i = 0
	while i < size-1 do
		x = input.getbyte - output.getbyte
		x = x.to_f
		error +=((x*x)/size.to_f)
		#print "error = ", error, "\n";
		#print "i = ", i, "\n"
		i +=1
	end
	psnr = 10.0 * Math.log10((255.0*255.0)/error)
	p "PSNR is ", psnr
    assert_operator(psnr, :>=, 0, "PSNR is less than 30! The decoded data has a very low PSNR compared to the original data")
    input.close
    output.close
end

def check_video_files_ts(src, dst, resolution, psnr_r, psnr_a, loss)
    class_name = @devices[0].class.name
    width = 160
    height = 90
    if(resolution == 0)
        width = 160
        height = 90
    elsif(resolution == 1)
        width = 320
        height = 180
    elsif(resolution == 2)
        width = 640
        height = 360
    elsif(resolution == 3)
        width = 1280
        height = 720
    else
        raise "unsupported resolution"
    end
    p src
    p dst

    command = format("./scripts/vqtest/VQ_Testtool %s %s %d %d %d %d %d",dst,src,width,height,psnr_r,psnr_a,loss)
    p command
    if (system(command) == false)
        raise "low psnr"
    end
end

def check_video_files_quality(src, dst, bitrate)
	p src
	p dst
	
	cfg = "./scripts/vqtest/video.cfg"
	cfgfile = File.open(cfg,"w")
	cfgfile.puts("CaptureFile "+src)
	cfgfile.puts("SrcFile /tmp/Encode2RTP_layer0_dump.data")
	cfgfile.puts("SrcInfoFile /tmp/Encode2RTP_layer0_dump.info")
	cfgfile.puts("ReceiverFile "+dst)
	cfgfile.puts("Bitrate "+bitrate)
	cfgfile.close

	FileUtils.cp '../../vendor/openh264/libs/mac64/libopenh264.dylib', './scripts/vqtest/libopenh264.dylib'
	command = format("./scripts/vqtest/VideoQualityMetricTool %s",cfg)
	p command
	if (system(command)  == false)
		raise "error"
	end
end
