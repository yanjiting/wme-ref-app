lastPCProcessMemory = 0
lastMobileProcessMemory = 0
lastIOSProcessMemory = 0
lastAndroidProcessMemory = 0

Then(/^I get memory usage on the (pc|mobile|ios|android) device$/) do |os|
    device = get_device(os, 1)
    stat = device.backdoor("backdoorGetMemory", nil)
    p "stat is #{stat}"
    stat_json = JSON.parse(stat)
    memUsage = stat_json["fMemroyUsage"]
    memUsed = stat_json["uMemoryUsed"]
    memTotal = stat_json["uMemoryTotal"]
    proUsed = stat_json["uProcessMemroyUsed"]
    p  "#{os} memory info: memUsage is #{memUsage}, memUsed is #{memUsed}Kbps, memTotal is #{memTotal}Kbps, proUsed is #{proUsed}Kbps"
    if os == "pc"
        lastPCProcessMemory = proUsed
        p "lastPCProcessMemory=#{lastPCProcessMemory}"
    elsif os == "mobile"
        lastMobileProcessMemory = proUsed
        p "lastMobileProcessMemory=#{lastMobileProcessMemory}"
    elsif os == "ios"
        lastIOSProcessMemory = proUsed
        p "lastIOSProcessMemory=#{lastIOSProcessMemory}"
    elsif os == "android"
        lastAndroidProcessMemory = proUsed
        p "lastAndroidProcessMemory=#{lastAndroidProcessMemory}"
    end
end

Then(/^I get memory usage again and memory increasing should be < (.*) Mbps on the (pc|mobile|ios|android) device$/) do |increasedRegion, os|
    device = get_device(os, 1)
    mIncreasedRegion = increasedRegion.to_i * 1024#convert Mbps to Kbps
    p "mIncreasedRegion = #{mIncreasedRegion}"
    stat = device.backdoor("backdoorGetMemory", nil)
    p "stat is #{stat}"
    stat_json = JSON.parse(stat)
    memUsage = stat_json["fMemroyUsage"]
    memUsed = stat_json["uMemoryUsed"]
    memTotal = stat_json["uMemoryTotal"]
    proUsed = stat_json["uProcessMemroyUsed"]
    p  "#{os} memory info: memUsage is #{memUsage}, memUsed is #{memUsed}Kbps, memTotal is #{memTotal}Kbps, proUsed is #{proUsed}Kbps"
    actualIncreased = 0
    if os == "pc"
        actualIncreased = proUsed - lastPCProcessMemory
        lastPCProcessMemory = proUsed
        p "lastPCProcessMemory=#{lastPCProcessMemory}"
    elsif os == "mobile"
        actualIncreased = proUsed - lastMobileProcessMemory
        lastMobileProcessMemory = proUsed
        p "lastMobileProcessMemory=#{lastMobileProcessMemory}"
    elsif os == "ios"
        actualIncreased = proUsed - lastIOSProcessMemory
        lastIOSProcessMemory = proUsed
        p "lastIOSProcessMemory=#{lastIOSProcessMemory}"
    elsif os == "android"
        actualIncreased = proUsed - lastAndroidProcessMemory
        lastAndroidProcessMemory = proUsed
        p "lastAndroidProcessMemory=#{lastAndroidProcessMemory}"
    end
    p "actualIncreased = #{actualIncreased}"
    if actualIncreased > mIncreasedRegion
        raise "Device:#{device.deviceID} process memory used expect < #{mIncreasedRegion}Kbps, actual #{actualIncreased}Kbps"
    end
end