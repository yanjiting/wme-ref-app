require 'logger'

module NLC
  cur_dir = File.dirname(__FILE__)
  SYSTEMCMD="osascript #{cur_dir}/emulator.applescript "

  def NLC.start(downlinkBandwidth=nil, downlinkDelay=nil, downlinkDrops=nil, uplinkBandwidth=nil, uplinkDelay=nil, uplinkDrops=nil, logfile=nil)
    downlinkDrops = downlinkDrops.to_f*100
    downlinkDrops = downlinkDrops.to_i
    uplinkDrops = uplinkDrops.to_f * 100
    uplinkDrops = uplinkDrops.to_i
    p "NLC.start: downlinkBandwidth=#{downlinkBandwidth}, downlinkDelay=#{downlinkDelay}, downlinkDrops=#{downlinkDrops}, uplinkBandwidth=#{uplinkBandwidth}, uplinkDelay=#{uplinkDelay}, uplinkDrops=#{uplinkDrops}"
    
    command=SYSTEMCMD + "start downlinkBandwidth=#{downlinkBandwidth} downlinkDelay=#{downlinkDelay} downlinkDrops=#{downlinkDrops} uplinkBandwidth=#{uplinkBandwidth} uplinkDelay=#{uplinkDelay} uplinkDrops=#{uplinkDrops}"
    
    result=system(command)
    if !result
      puts "NLC.start, exec #{command} failed"
      return nil
    end
    return result
  end

def NLC.change(downlinkBandwidth=nil, downlinkDelay=nil, downlinkDrops=nil, uplinkBandwidth=nil, uplinkDelay=nil, uplinkDrops=nil, logfile=nil)
    downlinkDrops = downlinkDrops.to_f*100
    downlinkDrops = downlinkDrops.to_i
    uplinkDrops = uplinkDrops.to_f * 100
    uplinkDrops = uplinkDrops.to_i
    p "NLC.change: downlinkBandwidth=#{downlinkBandwidth}, downlinkDelay=#{downlinkDelay}, downlinkDrops=#{downlinkDrops}, uplinkBandwidth=#{uplinkBandwidth}, uplinkDelay=#{uplinkDelay}, uplinkDrops=#{uplinkDrops}"
    
    command=SYSTEMCMD + "change downlinkBandwidth=#{downlinkBandwidth} downlinkDelay=#{downlinkDelay} downlinkDrops=#{downlinkDrops} uplinkBandwidth=#{uplinkBandwidth} uplinkDelay=#{uplinkDelay} uplinkDrops=#{uplinkDrops}"
    
    result=system(command)
    if !result
        puts "NLC.change, exec #{command} failed"
        return nil
    end
    return result
  end

  def NLC.stop()
    p "NLC.stop"
    command=SYSTEMCMD+"stop"
    result = system(command) 
    if !result    
      p "NLC.stop, exec #{command} failed"
      return false      
    end
    return true        
  end
  
end
