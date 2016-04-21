
def create_calmosvalue_offer(offerType, idx, params = nil)
    p "offerType=#{offerType}"
    is_p2p = offerType.include? "p2p"
    is_srtp = offerType.include? "srtp"
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
    
    #is_mute = true
    
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
               :multilayer => is_multilayer,
               :resolution => rs, 
               :overrideip => overrideip, 
               :overrideports => overrideports, 
               :filemode => is_file, 
               :isLoopFile => is_loopfile,
               #:mute => is_mute,
               :enableQos => enableQos}

    if params != nil
        @devices[0].backdoor("backdoorCalMos", my_json.merge(params).to_json)
    else
        @devices[0].backdoor("backdoorCalMos", my_json.to_json)
    end
    sleep 1
    @sdpOffer[idx] = @devices[idx].backdoor("backdoorGetLocalSdp", nil)
    Kernel.puts "SDP on device[#{idx}] is: #{@sdpOffer[idx]}"
end

def push_sample_file(device, options)
	class_name = device.class.name
    if(class_name == "WX2Calabash::IOSDevice")
       if (options == "MOS")
        	#device.pushFile(expand_local_path("../../sample/fem_wb4mos.wav"), "/Documents/fem_wb.wav")
        	#device.pushFile(expand_local_path("../../sample/settings.json"), "/Documents/settings.json")
            device.pushFile(expand_local_path("../../sample/WebExMediaEngine/"), "/Documents")
       end
	end
end

When(/^sq create (.*)offer to calculate mos value on device (\d+)$/) do |offerType, idxstr|
    push_sample_file(@devices[0],"MOS")
    sleep 2
    create_calmosvalue_offer(offerType, idxstr.to_i - 1)
end


