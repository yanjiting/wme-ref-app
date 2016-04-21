class WinPhone

    attr_reader :deviceID, :product_id, :trace_path
    def initialize
        p "Initialize winphone device"
        @deviceID = 0
        app_path = ENV["APP_PATH"]
        @product_id = "a0f06b8f-af36-4f55-83d1-57cd5c2d7144"
		@trace_path = ENV["TEST_TRACE_PATH"]
		@front_port = 6621
		@back_port = 6620
    end
    
    
    def get_phone8_0_tools_path
        phone8_0_tools_path = "\"C:/Program Files (x86)/Microsoft SDKs/Windows Phone/v8.0/Tools"
       return phone8_0_tools_path
    end    

	def get_phone8_1_tools_path
        phone8_1_tools_path = "\"C:/Program Files (x86)/Microsoft SDKs/Windows Phone/v8.1/Tools"
       return phone8_1_tools_path
    end  
    
    def install(app_path)
        p "Starting to install winphone APP..."
		p get_phone8_0_tools_path + "/XAP Deployment/XapDeployCmd.exe\" /installlaunch #{app_path} /targetdevice:de"
        
        output = []
			IO.popen(get_phone8_0_tools_path + "/XAP Deployment/XapDeployCmd.exe\" /installlaunch #{app_path} /targetdevice:de").each do |line|
            p line.chomp
            output << line.chomp
            
            if (line.chomp.include? "Done")
              return "success"
            end
        end
        return "fail"
    end

    def backdoor(sel, arg)
		if (sel == "backdoorGetLocalSdp")
			p "backdoorGetLocalSdp"
			sdps = Array.new(2);
	   		sdps[0] = perform_action_front(sel, arg)
			p sdps[0]
			sdps[1] = perform_action_back(sel, arg)
			p "backdoorGetLocalSdp 2"
			p sdps[1]
			p sdps
			return sdps
		elsif(sel == "backdoorSetRemoteSdp") 
			p "backdoorSetRemoteSdp"
			sdps = arg
			p arg
	   		perform_action_front(sel, arg[0])
			perform_action_back(sel, arg[1])

		#elsif (sel == "backdoorUpdateSdp")
		elsif (sel == "backdoorGetStatistics")
            json_video = perform_action_front(sel, arg)
            json_audio = perform_action_back(sel, arg)
            json = {"videoConn" => json_audio["videoConn"], "videoInNet" => json_audio["videoInNet"], "videoLocal" => json_audio["videoLocal"], "videoOutNet" => json_audio["videoOutNet"], "videoRemote" => json_audio["videoRemote"], "audioConn" => json_audio["audioConn"], "audioInNet" => json_audio["audioInNet"], "audioLocal" => json_audio["audioLocal"], "audioOutNet" => json_audio["audioOutNet"], "audioRemote" => json_audio["audioRemote"]}.to_json
            p json
            return json
		else
	   		perform_action_front(sel, arg)
			perform_action_back(sel, arg)

		end

    end

    def perform_action_front(action, arguments)

        #p "Action: #{action} - Params: #{arguments.join(', ')}"
        params = {"selector" => action, "arg" => arguments}
        Timeout.timeout(300) do
        begin
            result = http_front("backdoor", params, {:read_timeout => 350})
        rescue Exception => e
            p "Error communicating with test server: #{e}"
            raise e
        end
        #p "Result:'" + result.strip! + "'"
        raise "Empty result from TestServer" if result.chomp.empty?
        json = JSON.parse(result)
        if not json["outcome"].eql?("SUCCESS") then
            raise "Action '#{action}' unsuccessful: #{json["message"]}"
        end
        return json["result"]
        end
	rescue Timeout::Error
        raise Exception, "Step timed out"
    end

    def perform_action_back(action, arguments)

        #p "Action: #{action} - Params: #{arguments.join(', ')}"
        params = {"selector" => action, "arg" => arguments}
        Timeout.timeout(300) do
        begin
            result = http_back("backdoor", params, {:read_timeout => 350})
        rescue Exception => e
            p "Error communicating with test server: #{e}"
            raise e
        end
        #p "Result:'" + result.strip! + "'"
        raise "Empty result from TestServer" if result.chomp.empty?
        json = JSON.parse(result)
        if not json["outcome"].eql?("SUCCESS") then
            raise "Action '#{action}' unsuccessful: #{json["message"]}"
        end
        return json["result"]
        end
    rescue Timeout::Error
        raise Exception, "Step timed out"

    end


	def http_front(path, data = {}, options = {})
        begin
        #p "data is #{data}"
        configure_http(@http, options)


        make_http_request(
            :method => :post,
            :body => data.to_json,
            :uri => url_for_front(path),
            :header => {"Content-Type" => "application/json;charset=utf-8"})

	
        rescue HTTPClient::TimeoutError,
                HTTPClient::KeepAliveDisconnected,
                Errno::ECONNREFUSED, Errno::ECONNRESET, Errno::ECONNABORTED,
                Errno::ETIMEDOUT => e
        p "It looks like your app is no longer running. \nIt could be because of a crash or because your test script shut it down."
        raise e

        end
    end

	def http_back(path, data = {}, options = {})
        begin
        #p "data is #{data}"
        configure_http(@http, options)

 
		make_http_request(
            :method => :post,
            :body => data.to_json,
            :uri => url_for_back(path),
            :header => {"Content-Type" => "application/json;charset=utf-8"})

        rescue HTTPClient::TimeoutError,
                HTTPClient::KeepAliveDisconnected,
                Errno::ECONNREFUSED, Errno::ECONNRESET, Errno::ECONNABORTED,
                Errno::ETIMEDOUT => e
        p "It looks like your app is no longer running. \nIt could be because of a crash or because your test script shut it down."
        raise e

        end
    end

    def set_http(http)
        @http = http
    end

	def url_for_front(method)
        url = URI.parse("http://127.0.0.1:#{@front_port}")
        path = url.path
        if path.end_with? "/"
        path = "#{path}#{method}"
        else
        path = "#{path}/#{method}"
        end
        url.path = path
        url
    end

	def url_for_back(method)
        url = URI.parse("http://127.0.0.1:#{@back_port}")
        path = url.path
        if path.end_with? "/"
        path = "#{path}#{method}"
        else
        path = "#{path}/#{method}"
        end
        url.path = path
        url
    end


	def configure_http(http, options)
        return unless http
        http.connect_timeout = options[:open_timeout] || 15
        http.send_timeout = options[:send_timeout] || 15
        http.receive_timeout = options[:read_timeout] || 15
        if options.has_key?(:debug) && options[:debug]
        http.debug_dev= $stdout
        else
        if ENV['DEBUG_HTTP'] and (ENV['DEBUG_HTTP'] != '0')
            http.debug_dev = $stdout
        else
            http.debug_dev= nil
        end
        end
        http
    end

	def make_http_request(options)
        begin
        unless @http
            @http = init_request(options)
        end
        header = options[:header] || {}
        header["Content-Type"] = "application/json;charset=utf-8"
        options[:header] = header


        response = if options[:method] == :post
            @http.post(options[:uri], options)
        else
            @http.get(options[:uri], options)
        end
        raise Errno::ECONNREFUSED if response.status_code == 502
        response.body
        rescue Exception => e
        if @http
            @http.reset_all
            @http=nil
        end
        raise e
        end
    end


	def init_request(options)
        http = HTTPClient.new
        configure_http(http, options)
    end

	def uninstall()
        p "Starting to uninstall winphone APP..."
      
        output = []
        IO.popen(get_phone8_0_tools_path + "/XAP Deployment/XapDeployCmd.exe\" /uninstall #{@product_id} /targetdevice:de").each do |line|
            p line.chomp
            output << line.chomp
        end      
    end
    
    def launch()
		uninstall
		install(expand_local_path(WINPHONE_APP_PATH))

        p "Starting to launch winphone APP..."
		p get_phone8_0_tools_path + "/XAP Deployment/XapDeployCmd.exe\" /launch #{@product_id} /targetdevice:de"

        output = []
        IO.popen(get_phone8_0_tools_path + "/XAP Deployment/XapDeployCmd.exe\" /launch #{@product_id} /targetdevice:de").each do |line|
            p line.chomp
            output << line.chomp
        end      
    end
    
    def exitapp()
	    p "exit winphone"
	    perform_action_front("backdoorQuitWP8", "")
		sleep(1)

		p "exit winphone done"
        getLog()
        ENV['DEVICE_ENDPOINT'] = "http://127.0.0.1:#{@front_port}"
    end
    
    def fetchFile(srcFile, dstFile)
        cmds = get_phone8_1_tools_path + "/IsolatedStorageExplorerTool/isetool.exe\" ts de #{@product_id} #{dstFile}"
        p "fetchFile begin"
        p cmds
        p "fetchFile end"
        system(cmds)   
             
        return $?.success?
    end
    
    def pushFile(srcFile, dstFile)
        cmds = get_phone8_1_tools_path + "/IsolatedStorageExplorerTool/isetool.exe\" rs de #{@product_id} #{srcFile}"
        p "pushFile begin"
        p cmds
        p "pushFile end"
        system(cmds)
        
        return $?.success?
    end
    
    def deletePath(targetPath)
      p "deletePath not supported"
    end
    
    
    def getLog()
	  p "getLog"

      fetchFile("", "#{trace_path}")
    end
    
end


