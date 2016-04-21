require File.join(File.dirname(__FILE__), 'dummynet')
require File.join(File.dirname(__FILE__), 'nlc')

def setup_network_simulator(idx, direction, bw, qdelay, delay, lossrate,  logfile=nil)
   if(!ENV["USENLC"])
        ip = @devices[idx].backdoor("getIPAddress", nil)
        if ip[0, 4] != "192."
            raise "You MUST have connect device to the local wifi AP for this scenario."
        end
    end

    if direction.eql?(" uplink ") || direction.eql?(" ")
        if(!ENV["USENLC"])
            if @uplinkRuleIndex == nil && @uplinkPipeIndex == nil && @downlinkRuleIndex == nil && @downlinkPipeIndex == nil
                @uplinkRuleIndex = Array.new
                @uplinkPipeIndex = Array.new
                @downlinkRuleIndex = Array.new
                @downlinkPipeIndex = Array.new
            end
            if @uplinkRuleIndex[idx] == nil && @uplinkPipeIndex[idx] == nil
               uplinkRuleIndex, uplinkPipeIndex = Dummynet.add('ip', ip, nil, 'any', nil)
                raise "Setup uplink network simulator failed" if (uplinkRuleIndex == nil || uplinkPipeIndex == nil)
                @uplinkRuleIndex[idx] = uplinkRuleIndex
                @uplinkPipeIndex[idx] = uplinkPipeIndex
            end
             Dummynet.config(@uplinkPipeIndex[idx], bw, qdelay, delay, lossrate, logfile)
        else
            NLC.start(0, 0, 0, bw, delay, lossrate, logfile)
        end
    end

    if direction.eql?(" downlink ") || direction.eql?(" ")
        if(!ENV["USENLC"])
            if @downlinkRuleIndex[idx] == nil && @downlinkPipeIndex[idx] == nil
                downlinkRuleIndex, downlinkPipeIndex = Dummynet.add('ip', 'any', nil, ip, nil)
                raise "Setup downlink network simulator failed" if (downlinkRuleIndex == nil || downlinkPipeIndex == nil)
                @downlinkRuleIndex[idx] = downlinkRuleIndex
                @downlinkPipeIndex[idx] = downlinkPipeIndex
            end
            Dummynet.config(@downlinkPipeIndex[idx], bw, qdelay, delay, lossrate, logfile)
        else
            NLC.start(bw, delay, lossrate, 0, 0, 0, logfile)
        end
    end
end

def clear_network_simulator()
    if(!ENV["USENLC"])
        for ruleIndex in @uplinkRuleIndex
            p "Remove dummynet uplink rule failed, index="+@ruleIndex if !Dummynet.del(ruleIndex)
        end

        for ruleIndex in @downlinkRuleIndex
            p "Remove dummynet downlink rule failed, index="+@ruleIndex if !Dummynet.del(ruleIndex)
        end
    else
        NLC.stop()
    end
end

def flush_network_simulator()
    if(!ENV["USENLC"])
        Dummynet.flush()
    else
        NLC.stop()
    end
end

