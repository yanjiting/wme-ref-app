require 'logger'

module Dummynet
  SYSTEMCMD="sudo ipfw "
  
  def Dummynet.add(proto='ip', srcaddr='any', srcport=nil, dstaddr='any', dstport=nil)
    p "Dummynet.add, proto=#{proto}, srcaddr=#{srcaddr}, srcport=#{srcport}, dstaddr=#{dstaddr}, dstport=#{dstport}"
    pipeindex=rand(9999)
    
    command=SYSTEMCMD + "add pipe #{pipeindex} #{proto} "
    if srcport != nil
        command=command + "from #{srcaddr} #{srcport} "
	else
	    command=command + "from #{srcaddr} "   
    end
    if dstport != nil
        command=command + "to #{dstaddr} #{dstport}"
	else
	    command=command + "to #{dstaddr}"   
    end 
      
    result, output=exec(command)
    if !result
      puts "Dummynet.add, exec #{command} failed"
      return nil, nil
    end
    
    ruleindex=output[0, 5]  
    
    return ruleindex, pipeindex
  end

  def Dummynet.config(pipeindex, bandwidth=nil, qdelay=nil, delay=nil, lossrate=nil, logfile=nil)
    p "Dummynet.config, pipeindex=#{pipeindex}, bandwidth=#{bandwidth}Kbit/s, qdelay=#{qdelay}ms, delay=#{delay}ms, lossrate=#{lossrate}"
    
    if bandwidth != nil && qdelay != nil && delay != nil && logfile != nil
        file = File.open(logfile, File::WRONLY | File::APPEND | File::CREAT)
        logger = Logger.new(file)
        lossrate = 0.000000 if lossrate == nil
        logger.info("[change] rate = %d bps, qdelay = %d ms, delay = %d ms, burst = 0, loss = %s%%" % \
              [bandwidth.to_i*1000, qdelay.to_i, delay.to_i, lossrate])
        file.close
    end

    command=SYSTEMCMD + "pipe #{pipeindex} config "
    if bandwidth != nil
        command=command + "bw #{bandwidth}Kbit/s "
    end
    if bandwidth != nil && qdelay != nil
        queue=(bandwidth.to_i/8)*qdelay.to_i/1000
        command=command + "queue #{queue}Kbytes "
    end
    if delay != nil
        command=command + "delay #{delay}ms "
    end
    if lossrate != nil
        command=command + "plr #{lossrate}"
    end
    
    if !exec(command)[0]
      p "Dummynet.config, exec #{command} failed"
      return false      
    end   
    
    return true
  end
  
  def Dummynet.del(ruleindex)
    p "Dummynet.del, ruleindex=#{ruleindex}"
    command=SYSTEMCMD+"-q delete #{ruleindex}"
    if !exec(command)[0]
      p "Dummynet.del, exec #{command} failed"
      return false      
    end
    return true        
  end
  
  def Dummynet.flush()
    p "Dummynet.flush"
    command=SYSTEMCMD+"-q flush"
    return exec(command)[0]
  end
  
  def Dummynet.exec(command)
    #p "Dummynet.exec #{command}"
    output=`#{command}`
    return [$?.success?, output]
  end
  
end