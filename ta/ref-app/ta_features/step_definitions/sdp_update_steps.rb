And(/^I (hold|unhold) the call for device (\d+)$/) do |op, idxstr|
    i = idxstr.to_i - 1
    my_json = {:op => op}
    @devices[i].backdoor("backdoorUpdateSdp", my_json.to_json)
end

And (/^I get the offer from device (\d+) to device (\d+)$/) do |idx1, idx2|
    idxsrc = idx1.to_i - 1
    idxdst = idx2.to_i - 1
    offer = @devices[idxsrc].backdoor("backdoorGetLocalSdp", nil)
    @devices[idxdst].backdoor("backdoorSetRemoteOffer", offer)
end
