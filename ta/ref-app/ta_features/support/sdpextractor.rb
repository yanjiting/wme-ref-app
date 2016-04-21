def get_ports(type, content, port)
	return nil if(!type && !content)
	return ["sharing", port] if(content == "slides")	
	return [type, port]
end

def extract_sdp_port(sdp)
	ports = {}
	type = nil
	content = nil
	port = 0
	sdp.lines.each do |line|
		line.scan(/m=(audio|video)\s(\d*)(.*)/).each do |x|
			type_port = get_ports(type, content, port)
			if(type_port != nil)
				ports[type_port[0]] = type_port[1] 
				type = nil
				content = nil
			end	
			type = x[0]
			port = x[1].to_i
		end
		if(matched = line.match(/a=content:(\S*)/))
			content = matched[1]
		end
		if(matched = line.match(/c=IN\sIP4\s(\S*)/))
			ports["ip"] = matched[1]
		end
	end
	type_port = get_ports(type, content, port)
	ports[type_port[0]] = type_port[1]
	return ports
end