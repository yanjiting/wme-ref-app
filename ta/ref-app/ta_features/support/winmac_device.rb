require 'socket'
require 'timeout'

module WX2Calabash
    class WinMacDevice
        attr_reader :server_port, :sys_type, :app_type, :ipaddr, :isdebugmode
        attr_reader :deviceID
        
        def initialize(port, sys, type, debugmode)
            @server_port = port
            @sys_type = sys
			@app_type = type
			@ipaddr = my_first_public_ipv4.ip_address unless my_first_public_ipv4.nil?
			@isdebugmode = debugmode
			@process_id = 0
            @logname_mac = "wme_ta_mac"
            p "initialize WinMacdevice"
        end
        
		def my_first_public_ipv4
			Socket.ip_address_list.detect{|intf| intf.ipv4? and !intf.ipv4_loopback? and !intf.ipv4_multicast?}
		end
		
        def deleteTempWbtFiles()
            Kernel.puts "deleteTempWbtFiles: #{ENV["TEMP"]}/*.wbt"
            log_files = Dir["#{ENV["TEMP"].gsub("\\","/")}/*.wbt"]
            log_files.each() do |file|
                p file
                begin
                    File.delete(file)
                rescue
                end
            end    
        end
        
        def copyWbtFiles()
            Kernel.puts "copyWbtFiles, #{ENV["TEMP"]}\\*.wbt"
            log_files = Dir["#{ENV["TEMP"].gsub("\\","/")}/*.wbt"]
            log_files.each() do |file|
                p file
                begin
                    FileUtils.cp(file, ENV["TEST_TRACE_PATH"])
                rescue
                    p "copy error"
                end
            end    
        end
        
        def is_port_open?(ip, port)
            begin
                Timeout::timeout(1) do
                    begin
                        s = TCPSocket.new(ip, port)
                        s.close
                        return true
                        rescue Errno::ECONNREFUSED, Errno::EHOSTUNREACH
                        return false
                    end
                end
                rescue Timeout::Error
            end
            
            return false
        end
        

        def launch()
            deleteTempWbtFiles if @sys_type == :windows
            
            if(!@isdebugmode)
                while is_port_open?("127.0.0.1", @server_port) do
                    p "port #{@server_port} is in use"
                    port = @server_port.to_i + 1
                    @server_port = port.to_s
                end
            end
            
			if(@app_type == "as_dummy")
				start_test_server_in_background unless(@isdebugmode)
			else
				start_test_server_in_background unless(@isdebugmode)
			end

            puts "wait for listening on port #{@server_port} ..."
            $found = 0
            until $found == 1 do
                $found = 1 if is_port_open?("127.0.0.1", @server_port)
            end
        end

        def backdoor(sel, arg)
            return perform_action(sel, arg)
        end

        def perform_action(action, arguments)

          #p "Action: #{action} - Params: #{arguments.join(', ')}"
          params = {"selector" => action, "arg" => arguments}
          Timeout.timeout(300) do
            begin
              result = http("backdoor", params, {:read_timeout => 350})
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

        def http(path, data = {}, options = {})
          begin
            #p "data is #{data}"
            configure_http(@http, options)
            make_http_request(
                :method => :post,
                :body => data.to_json,
                :uri => url_for(path),
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

        def url_for(method)
          url = URI.parse("http://127.0.0.1:#{@server_port}")
          path = url.path
          if path.end_with? "/"
            path = "#{path}#{method}"
          else
            path = "#{path}/#{method}"
          end
          url.path = path
          url
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

        def start_test_server_in_background(options={})
            raise "Will not start test server because of previous failures." if ::Cucumber.wants_to_quit
		  
            application_path_setting = "" 
            log_param = ""
            if(@app_type == "as_dummy")
                if(@sys_type == :windows)
                    application_path_setting = WINDOWS_AS_DUMMY_PATH
                else
                    application_path_setting =  MAC_AS_DUMMY_APP_PATH
                end    
            else     
                if(@sys_type == :windows)
                    application_path_setting = WINDOWS_EXE_PATH
                else
                    application_path_setting = MAC_APP_PATH
                    @logname_mac = "wme_ta_#{@server_port}"
                    log_param = "--logname #{@logname_mac}"
                end
            end 
      
            application_path_file = ""
            if(application_path_setting.kind_of?(Array))
                application_path_setting.each do |item|
                    if(File.exist?(item))
                        application_path_file = item
                        break
                    end
                end
            else
                application_path_file = application_path_setting
            end     
        
            if(File.exist?(application_path_file))
                p "application path file :#{application_path_file}"
                cmd = "#{application_path_file} --ta #{@server_port} #{log_param}"
            else
                raise "application path file :#{application_path_file} is not exist" 
            end
		  
            @process_id = spawn(cmd)
            @deviceID = @process_id
            Process.detach(@process_id)
            p "pid is: #{@process_id}"
        end

        def app_killed?
          begin
            if(@sys_type == :windows)
                Process::kill 9, @process_id
            else
                Process::kill 9, @process_id
                Timeout::timeout(3) {
                    Process::waitpid(@process_id)
               }
            end
            p "app_killed? #{@process_id}"
            true
          rescue Errno::ESRCH
            p "app_killed? yes"
            false
          end  
        end

        def fetchFile(srcFile, dstFile)
            FileUtils.copy_file(srcFile, dstFile)
            #`cp #{srcFile} #{dstFile}`
        end

        def shutdown_test_server
          begin
            p "begin to shutdown test server, pid is: #{@process_id}"
            return if @process_id == 0
            begin
                if(@sys_type == :windows)
                    Process::kill 9, @process_id
                else
                    Process::kill 15, @process_id
                    Timeout::timeout(5) { 
                        Process::waitpid(@process_id)                
                    }
                end
            rescue
            end            
            Timeout::timeout(3) do
              sleep 0.3 while app_killed?
            end
          rescue Timeout::Error
            p "Could not kill app. Waited to 3 seconds."
          rescue EOFError
            p "Could not kill app. App is most likely not running anymore."
          end
          @process_id = 0
        end

        def exitapp()
            p "enter int exitapp on mac"
            getlog()
			if(@app_type == "as_dummy")
				shutdown_test_server unless(@isdebugmode)
			else
				shutdown_test_server unless(@isdebugmode)
			end
        end

        def getlog()
            p "starting to get log..."
            if(@sys_type == :windows)
                copyWbtFiles()
                deleteTempWbtFiles()
            else
                output = []
                devman = WX2Calabash.deviceman
                trace_path = File.join(ENV["TEST_TRACE_PATH"], "mac_#{devman.scenario_name}_#{devman.feature_name}_#{@server_port}_#{@deviceID}.log")
                #cmd = "./ios/script/iFileTransfer -o download -id #{@deviceID} -app #{IOS_APP_NAME} -from #{IOS_LOG_PATH} -to #{trace_path}"
                mac_log_path = File.join(ENV["HOME"], "/Library/Logs/wme_ta_mac/#{@logname_mac}.log")   #MAC_LOG_PATH
                cmd = "cp #{mac_log_path} #{trace_path}"
                p cmd
                IO.popen(cmd).each do |line|
                    p line.chomp
                    output << line.chomp
                end
            end    
        end

        def start_capture_packet(captureMethod="default")
            devman = WX2Calabash.deviceman
            if(@sys_type == :windows)
                p "start_capture_packet not implemented on windows"
            else
                pcapfilename = "mac_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.pcap"
                @pcap_path = File.join(ENV["TEST_TRACE_PATH"], pcapfilename)
                pcap_fix = "mac_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.fix.pcap"
                @pcap_fix_path = File.join(ENV["TEST_TRACE_PATH"], pcap_fix)
                @tcpdump_th = Thread.new do
                    p "mac device begin new thread, deviceID:" + "#{@deviceID}"
                    p "start_capture_packet: #{@pcap_path}"
                    network_inf = ENV['NETWORK_INF']
                    if(network_inf.to_s.empty?)
                        network_inf = 'en0'
                    end
                    p "network_inf: #{network_inf}"
                    tcpdump_process = IO.popen("tcpdump -i #{network_inf} -w #{@pcap_path}")
                    Thread.current[:pid] = tcpdump_process.pid
                end
            end
        end

        def stop_capture_packet(captureMethod = "default")
            if(@sys_type == :windows)
                p "stop_capture_packet not implemented on windows"
            else
                p "mac tcpdump_th[:pid]:" + "#{@tcpdump_th[:pid]}"
                if(@tcpdump_th)
                    Process.kill(:SIGKILL, @tcpdump_th[:pid])
                    @tcpdump_th.join()
                    @tcpdump_th = nil
                    system("./scripts/pcaptool/pcapfix #{@pcap_path} -o #{@pcap_fix_path}")
                    p "fix pcap file: #{@pcap_fix_path}"
                    system("rm #{@pcap_path}")
                else
                    return nil
                end
            end
        end

      
      def pushFile(srcFile, dstFile)
        FileUtils.mkdir_p(File.dirname(dstFile))
        FileUtils.cp(srcFile, dstFile)
      end
      
      def fetchFile(srcFile, dstFile)
        FileUtils.mkdir_p(File.dirname(dstFile))
        FileUtils.cp_r(srcFile, dstFile)
      end
      
      def deletePath(targetPath)
          p "delete #{targetPath}"
          FileUtils.rm_rf(targetPath)
      end
    end
end
