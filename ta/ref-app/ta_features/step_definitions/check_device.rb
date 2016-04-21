def check_device(count, os)
    check_skip_lab(@scenario_tags)
    is_windows = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
    if(os.eql?("pc"))
        if(is_windows)
            os = "windows"
        else
            os = "mac"
        end
    end
	
    n = count.to_i
    if(os.eql?("android"))
        if(WX2Calabash.deviceman.count_android < n)
            pending("You MUST attach at least #{count} android devices for this scenario.")
        end
        WX2Calabash.deviceman.count_android.times do |i|
            break if n <= 0
            device_temp = WX2Calabash.deviceman.get_android_device(i)
            next if @devices.include? device_temp
            @devices.push(device_temp)
            device_temp.launch
            n = n - 1
        end
        if(n > 0)
            raise "You still have #{n} android devices not lauched."
        end
    elsif(os.eql?("ios"))
        if(WX2Calabash.deviceman.count_ios < n)
            pending("You MUST attach at least #{count} ios devices for this scenario.")
        end
        WX2Calabash.deviceman.count_ios.times do |i|
            break if n <= 0
            device_temp = WX2Calabash.deviceman.get_ios_device(i)
            next if @devices.include? device_temp
            @devices.push(device_temp)
            device_temp.launch
            n = n - 1
        end
        if(n > 0)
            raise "You still have #{n} ios devices not lauched."
        end
    elsif(os.eql?("mac"))
        pending("You cannot run mac case in windows") if is_windows
        n.times do |i|
            device_temp = WX2Calabash.deviceman.get_mac_device(@devices.size)
            next if @devices.include? device_temp
            @devices.push(device_temp)
            device_temp.launch
        end
    elsif(os.eql?("windows"))
        pending("you cannot run windows case in macosx") unless is_windows
        n.times do |i|
            device_temp = WX2Calabash.deviceman.get_win_device(@devices.size)
            next if @devices.include? device_temp
            @devices.push(device_temp)
            device_temp.launch
        end
	elsif(os.eql?("winphone"))
        if(WX2Calabash.deviceman.count_winphone < n)
            pending("You MUST attach at least #{count} windows phone devices for this scenario.")
        end
        WX2Calabash.deviceman.count_winphone.times do |i|
            break if n <= 0
            device_temp = WX2Calabash.deviceman.get_winphone_device(i)
            next if @devices.include? device_temp
            @devices.push(device_temp)
            device_temp.launch
            n = n - 1
        end
        if(n > 0)
            raise "You still have #{n} windows phone devices not lauched."
        end
    else
        raise "The os is not supported yet."
    end
end


Given(/^I have (\d+) (.*) (?:devices|device)$/) do |count,os|
    @sdpOffer = []
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @devices = []
    if(os.eql?("any") || os.eql?("mobile"))
        devicecount = count.to_i
        devicecount -= 1 if os.eql?("any")
        acount = WX2Calabash.deviceman.count_android
        icount = WX2Calabash.deviceman.count_ios
		wcount = WX2Calabash.deviceman.count_winphone
        if (acount + icount + wcount) < devicecount
            pending("we don't have enough devices for this case.")
        end
        count1 = devicecount > acount ? acount : devicecount
        devicecount = devicecount - count1
        count2 = 0
        if(devicecount > 0)
            count2 = devicecount > icount ? icount : devicecount
            devicecount = devicecount - count2
        end    
		count3 = 0
        if(devicecount > 0)
            count3 = devicecount > wcount ? wcount : devicecount
            devicecount = devicecount - count3
        end  
        check_device(count1, "android") if count1 > 0
        check_device(count2, "ios") if count2 > 0
        check_device(count3, "winphone") if count3 > 0
        
        check_device(1, "pc") if os.eql?("any")
    else
        check_device(count, os)
    end
end

When(/^I joint further (\d+) (.*) (?:device|devices)$/) do |count, os|
    get_mobile_device(count, os)
end

def get_mobile_device(count_str, os)
	count = count_str.to_i
	return check_device(count, os) unless os.eql?("mobile")
	if(WX2Calabash.deviceman.count_android + WX2Calabash.deviceman.count_ios + WX2Calabash.deviceman.count_winphone >= count)
		return check_device(count, "ios") if(WX2Calabash.deviceman.count_android == 0 && WX2Calabash.deviceman.count_winphone == 0)
		return check_device(count, "android") if(WX2Calabash.deviceman.count_ios == 0 && WX2Calabash.deviceman.count_winphone == 0)
		return check_device(count, "winphone") if(WX2Calabash.deviceman.count_android == 0 && WX2Calabash.deviceman.count_ios == 0)
		if(count == 1)
			r = Random.new
			rv = r.rand(0...100)
            is_windows = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
            if is_windows
                if(rv > 0 && rv < 50)
                    check_device(count, "android")
                else
                    check_device(count, "winphone")
                end
            else
                if(rv > 0 && rv < 50)
                    check_device(count, "android")
                else
                    check_device(count, "ios")
                end
            end
		else
			acount = WX2Calabash.deviceman.count_android
			icount = WX2Calabash.deviceman.count_ios
			wcount = WX2Calabash.deviceman.count_winphone
			count1 = count2 = count3 = 0
			count.times do |i|
			  count1 += 1 if(count1 < acount)
			  break if(count1 + count2 + count3 == count)
				count2 += 1 if(count2 < icount)
				break if(count1 + count2 + count3 == count)
				count3 += 1 if(count3 < wcount)
				break if(count1 + count2 + count3 == count)
			end
			check_device(count1, "android") if count1 > 0
			check_device(count2, "ios") if count2 > 0
			check_device(count3, "winphone") if count3 > 0

		end
	else
		pending("You MUST attach at least #{count} mobile devices for this scenario.")
	end
end

def get_all_device_count(os)
  acount = WX2Calabash.deviceman.count_android
  icount = WX2Calabash.deviceman.count_ios
  wcount = WX2Calabash.deviceman.count_winphone

  mobilecount = acount + icount + wcount
  pccount = 1
  newcount="1"
  
  if(os.eql?("android"))
    newcount="#{acount}"
  elsif(os.eql?("ios"))
    newcount="#{icount}" 
  elsif(os.eql?("winphone"))
	newcount="#{wcount}" 
  elsif(os.eql?("mobile"))
    newcount="#{mobilecount}" 
  else
    newcount="#{pccount}" 
  end
  
  return newcount
end

Given(/^I have (\d+|all) (.*) and (\d+|all) (.*)$/) do |count1, os1, count2, os2|
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @sdpOffer = []
    @devices = []
    
    count1=get_all_device_count(os1) if(count1.eql?("all"))
    count2=get_all_device_count(os2) if(count2.eql?("all"))
    
    get_mobile_device(count1, os1)
    get_mobile_device(count2, os2)
end

Given(/^I have all (.*) device$/) do |os1|
    WX2Calabash.deviceman.set_runtime(@__cucumber_runtime)
    @sdpOffer = []
    @devices = []
    
    count1=get_all_device_count(os1)
    get_mobile_device(count1, os1)
end

#get the seq-th device of os
def get_device(os, seq)
  count=0
  for device in @devices
    class_name = device.class.name
    if(class_name == "WX2Calabash::AdroidDevice")
      if(os.eql?("android") || os.eql?("mobile"))
        count = count + 1
        if (count == seq)
          return device
        end
      end
    elsif (class_name == "WX2Calabash::IOSDevice")
       if(os.eql?("ios") || os.eql?("mobile"))
        count = count + 1
        if (count == seq)
          p "class_name = #{class_name}"
          return device
        end
      end
    elsif (class_name == "WinPhone")
      if(os.eql?("WinPhone") || os.eql?("mobile"))
        count = count + 1
        if (count == seq)
          return device
        end
      end
    elsif (class_name == "WX2Calabash::WinMacDevice")
      if(os.eql?("pc"))
        count = count + 1
        if (count == seq)
          p "class_name = #{class_name}"
          return device
        end
      end
    end
  end
  return nil
end
