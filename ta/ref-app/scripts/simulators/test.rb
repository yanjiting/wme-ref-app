=begin
require File.join(File.dirname(__FILE__), 'dummynet')

Dummynet.flush()
ruleindex, pipeindex = Dummynet.add('udp', 'any', nil, '10.224.255.255', '9000')
if ruleindex != nil &&  pipeindex != nil
    if Dummynet.config(pipeindex, '1024', '10', '10', nil, 'action.log')
          
    end
    if Dummynet.config(pipeindex, nil, nil, nil, '0.05')
          
    end    
    Dummynet.del(ruleindex)
end
Dummynet.flush()
=end
require File.join(File.dirname(__FILE__), 'nlc')
NLC.start(1000, 50, 0.02, 0, 0, 0)
sleep 1
NLC.stop()
