Then(/^I register (.*) fec payloadType (\d+) and clockRate (\d+) on device (\d+)$/) do |sessionType, payloadType, clockRate,idxstr|
    mSessionType = -1
    if sessionType.include? "audio"
        mSessionType = 0
    elsif sessionType.include? "video"
        mSessionType = 1
    elsif sessionType.include? "share"
        mSessionType = 2
    end
    mPayloadType = payloadType.to_i
    mClockRate = clockRate.to_i
    p "I register fec sessionType #{mSessionType} payloadType #{mPayloadType} and clockRate #{mClockRate} on device #{idxstr.to_i}"
    my_json = {:rsfec => {:uSessionType => mSessionType, :uPayloadType => mPayloadType, :uClockRate => mClockRate, :bEnableFec => true}}
    set_fec = my_json.to_json
    p set_fec
    @devices[idxstr.to_i-1].backdoor("backdoorSetFecParam", set_fec)
end

Then(/^I set (.*) fec dynamic fec level scheme on device (\d+)$/) do |targetScheme, idxstr|
    mSessionType = 1
    p "I set #{targetScheme} fec dynamic fec level scheme on device #{idxstr.to_i}"
    my_json = {:rsfec => {:uSessionType => mSessionType, :dynamicFecScheme => targetScheme}}
    set_fec = my_json.to_json
    p set_fec
    @devices[idxstr.to_i-1].backdoor("backdoorSetFecParam", set_fec)
end

Then(/^I disable (.*) fec on device (\d+)$/) do |sessionType, idxstr|
    mSessionType = -1
    if sessionType.include? "audio"
        mSessionType = 0
    elsif sessionType.include? "video"
        mSessionType = 1
    elsif sessionType.include? "share"
        mSessionType = 2
    end 
    my_json = {:rsfec => {:uSessionType => mSessionType, :bEnableFec => false}}
    set_fec = my_json.to_json
    p set_fec
    @devices[idxstr.to_i-1].backdoor("backdoorSetFecParam", set_fec)
end

Then(/^I set (.*) session (.*) fec order on device (\d+)$/) do |sessionType,fecOrder,idxstr|
     mSessionType = -1
    if sessionType.include? "audio"
        mSessionType = 0
    elsif sessionType.include? "video"
        mSessionType = 1
    elsif sessionType.include? "share"
        mSessionType = 2
    end

    mFecOrder = "ORDER_SRTP_FEC"
    if fecOrder.include? "ORDER_FEC_SRTP"
        mFecOrder = "ORDER_FEC_SRTP"
    end
    p "I set #{mFecOrder} fec order on device #{idxstr.to_i}"

    my_json = {:rsfec => {:uSessionType => mSessionType,:fecFmtInfo => {:srtpFecOrder => mFecOrder}}}
    set_fec = my_json.to_json
    p set_fec
    @devices[idxstr.to_i-1].backdoor("backdoorSetFecParam", set_fec)
end

Then(/^(.*) recovered packet count should be (\W+) (\d+)$/) do |sessionType, op, expect|
    p "#{sessionType} recovered packet count should be #{op} #{expect.to_i}"
    expectRecoveredPacket = expect.to_i
    for device in @devices
        stat = get_stats(device)
        actualRecoveredPacket = 0
        isFecEnabled = false
        if sessionType.include? "audio"
            isFecEnabled = stat["audioInNet"]["bFecEnabled"]
            actualRecoveredPacket = stat["audioInNet"]["uFECRecoveredPackets"]
        elsif sessionType.include? "video"
            isFecEnabled = stat["videoInNet"]["bFecEnabled"]
            actualRecoveredPacket = stat["videoInNet"]["uFECRecoveredPackets"]
        elsif sessionType.include? "share"
            isFecEnabled = stat["shareInNet"]["bFecEnabled"]
            actualRecoveredPacket = stat["shareInNet"]["uFECRecoveredPackets"]
        end

        if isFecEnabled == true
            case op
                when ">="
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket < expectRecoveredPacket
                when "<="
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket > expectRecoveredPacket
                when "=="
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket != expectRecoveredPacket
                when ">"
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket <= expectRecoveredPacket
                when "<"
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket >= expectRecoveredPacket
                when "!="
                    raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket == expectRecoveredPacket
            end
        else
            p "fec negotiation failed"
        end
    end
end

Then(/^(.*) recovered packet count should be (\W+) (\d+) for device (\d+)$/) do |sessionType, op, expect, idxstr|
    p "#{sessionType} recovered packet count should be #{op} #{expect.to_i} for device #{idxstr}"
    expectRecoveredPacket = expect.to_i
    idx = idxstr.to_i - 1
    device = @devices[idx]

    stat = get_stats(device)
    actualRecoveredPacket = 0
    isFecEnabled = false
    if sessionType.include? "audio"
        isFecEnabled = stat["audioInNet"]["bFecEnabled"]
        actualRecoveredPacket = stat["audioInNet"]["uFECRecoveredPackets"]
    elsif sessionType.include? "video"
        isFecEnabled = stat["videoInNet"]["bFecEnabled"]
        actualRecoveredPacket = stat["videoInNet"]["uFECRecoveredPackets"]
    elsif sessionType.include? "share"
        isFecEnabled = stat["shareInNet"]["bFecEnabled"]
       actualRecoveredPacket = stat["shareInNet"]["uFECRecoveredPackets"]
    end

    if isFecEnabled == true
        case op
            when ">="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket < expectRecoveredPacket
            when "<="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket > expectRecoveredPacket
            when "=="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket != expectRecoveredPacket
            when ">"
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket <= expectRecoveredPacket
            when "<"
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket >= expectRecoveredPacket
            when "!="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket == expectRecoveredPacket
        end
    else
        p "fec negotiation failed"
    end
end

Then(/^(.*) recovered packet count should be (\W+) (\d+) for the (pc|mobile|ios|android|winphone) device$/) do |sessionType, op, expect, os|
    p "#{sessionType} recovered packet count should be #{op} #{expect.to_i} for the #{os} device"
    expectRecoveredPacket = expect.to_i
    device = get_device(os, 1)
    raise "Cannot find the #{os} device" if device == nil
    
    stat = get_stats(device)
    actualRecoveredPacket = 0
    isFecEnabled = false
    if sessionType.include? "audio"
        isFecEnabled = stat["audioInNet"]["bFecEnabled"]
        actualRecoveredPacket = stat["audioInNet"]["uFECRecoveredPackets"]
    elsif sessionType.include? "video"
        isFecEnabled = stat["videoInNet"]["bFecEnabled"]
        actualRecoveredPacket = stat["videoInNet"]["uFECRecoveredPackets"]
    elsif sessionType.include? "share"
        isFecEnabled = stat["shareInNet"]["bFecEnabled"]
        actualRecoveredPacket = stat["shareInNet"]["uFECRecoveredPackets"]
    end

    if isFecEnabled == true
        case op
            when ">="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket < expectRecoveredPacket
            when "<="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket > expectRecoveredPacket
            when "=="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket != expectRecoveredPacket
            when ">"
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket <= expectRecoveredPacket
            when "<"
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket >= expectRecoveredPacket
            when "!="
                raise "Device:#{device.deviceID} fec recovered packet expect #{op} #{expectRecoveredPacket}, actual #{actualRecoveredPacket}" if  actualRecoveredPacket == expectRecoveredPacket
        end
    else
        p "fec negotiation failed"
    end
end
