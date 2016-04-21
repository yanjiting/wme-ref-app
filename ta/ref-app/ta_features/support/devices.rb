module WX2Calabash

attr_accessor :deviceman 

class AdroidDevice
    include Cucumber::RbSupport::RbWorld
    include Calabash::Android::Operations
    attr_reader :device, :deviceID
    
    def initialize(did, adb_port, test_port)
        @local_port = adb_port
        @deviceID = did
        @log_pid = 0
        p "Initialize android device: #{did}"
        app_path = ENV["APP_PATH"]
        test_app_path = ENV["TEST_APP_PATH"]
        @device = Device.new(self, did, adb_port, app_path, test_app_path, test_port)
    end
    
    def launch()
        ENV['DEVICE_ENDPOINT'] = "http://127.0.0.1:#{@local_port}"
        startCaptureLog
        @device.start_test_server_in_background
    end
    
    def ipaddr()
        return backdoor("getIPAddress", nil)
    end
    
    def backdoor(sel, arg)
        ENV['DEVICE_ENDPOINT'] = "http://127.0.0.1:#{@local_port}"
        
        result = @device.perform_action("backdoor", sel, arg)
        if !result["success"]
            @device.screenshot({:prefix => nil, :name => nil, :label => nil})
                raise result["message"]
        end
        # for android results are returned in bonusInformation
        return result["bonusInformation"].first
    end

    def fetchFile(srcFile, dstFile)
        `#{@device.adb_command} -s #{@deviceID} pull #{srcFile} #{dstFile}`

        return $?.success?
    end
    
    def pushFile(srcFile, dstFile)
        `#{@device.adb_command} -s #{@deviceID} push #{srcFile} #{dstFile}`
        
        return $?.success?
    end
    
    def deletePath(targetPath)
        `#{@device.adb_command} -s #{@deviceID} shell rm -r #{targetPath}`
    end
    
    def rotateDevice
        rotate(:left)
    end
    
    def startCaptureLog
        `#{@device.adb_command} logcat -c`
        devman = WX2Calabash.deviceman
        logfilename = "android_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.log"
        trace_path = File.join(ENV["TEST_TRACE_PATH"], logfilename)
        @trace_file = File.open(trace_path, 'w')
        STDOUT.sync = true
        @logcat_th = Thread.new do
            logcat_process = IO.popen("#{@device.adb_command} logcat -v time")
            Thread.current[:pid] = logcat_process.pid
            logcat_process.each do |line|
                line.chomp!
                @trace_file.puts(line) unless(line == "")
            end
        end
    end
    
    def stopCaptureLog
        if(@logcat_th)
            Process.kill(:SIGKILL, @logcat_th[:pid])
            @logcat_th.join()
            @logcat_th = nil
        end
        @trace_file.close() if @trace_file
        @trace_file = nil
    end
    
    def exitapp()
        stopCaptureLog
        getMiniDump
        ENV['DEVICE_ENDPOINT'] = "http://127.0.0.1:#{@local_port}"
        device.shutdown_test_server
    end
    
    def set_runtime(runtime)
        @__cucumber_runtime = runtime
    end

    def getMiniDump
        p "starting to get mini dump file..."
        dstPath = File.join(ENV["TEST_TRACE_PATH"], "breakpad_dump")
        srcFile = "/sdcard/breakpad_dump"
        fetchFile(srcFile,dstPath)
    end

    def start_capture_packet(captureMethod="default")
        devman = WX2Calabash.deviceman
        pcapfilename = "android_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.pcap"
        @pcap_path = File.join(ENV["TEST_TRACE_PATH"], pcapfilename)
        pcapfix = "android_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.fix.pcap"
        @pcap_fix_path = File.join(ENV["TEST_TRACE_PATH"], pcapfix)
        @pcap_path_device= File.join("/sdcard", pcapfilename)
        @tcpdump_th = Thread.new do
            p "android, start_capture_packet to #{@pcap_path_device}"
            tcpdump_process = IO.popen("adb -s #{@deviceID} shell \"su -c 'tcpdump -i wlan0 -w #{@pcap_path_device}'\"")
            Thread.current[:pid] = tcpdump_process.pid
        end
    end

    def stop_capture_packet(captureMethod = "default")
        p "android, stop_capture_packet, tcpdump_th[:pid]:" + "#{@tcpdump_th[:pid]}"
        if(@tcpdump_th)
            Process.kill(:SIGKILL, @tcpdump_th[:pid])
            @tcpdump_th.join()
            p "android, stop_capture_packet #{$?.to_i}"
            @tcpdump_th = nil
            fetchFile(@pcap_path_device, @pcap_path)
            system("./scripts/pcaptool/pcapfix #{@pcap_path} -o #{@pcap_fix_path}")
            p "fix pcap file: #{@pcap_fix_path}"
            system("rm #{@pcap_path}")
        else
            return nil
        end
    end

end

class IOSDevice
    include Cucumber::RbSupport::RbWorld
    include Calabash::Cucumber::Core
    include Calabash::Cucumber::Operations
    
    attr_reader :deviceID, :ipaddr
    def initialize(udid)
        p "Initialize ios device: #{udid}"
        @deviceID = udid
    end
    
    def ensure_instruments_not_running!
      pids_str = `ps x -o pid,command | grep -v grep | grep "instruments" | awk '{printf "%s,", $1}'`.strip
      instruments_pids = pids_str.split(',').map { |pid| pid.to_i }
      instruments_pids.each do |pid|
        p "Found instruments #{pid}. Killing..."
        `kill -9 #{pid} && wait #{pid} &> /dev/null`
      end
    end
    
    def get_template_path
        xcodepath = `xcode-select --print-path`
        xcodepath.strip!
        template_path = File.join(xcodepath, "../Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate")
        template_path = File.join(xcodepath, "../Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.xrplugin/Contents/Resources/Automation.tracetemplate") if(!File.exist?(template_path))
        return template_path
    end
   
    def get_ip(app_path)
        if(ENV["DEBUGIOS"])
            @ipaddr = ENV["DEBUGIOS"]
            return @ipaddr
        end
        p "Starting to get IP address..."
        ensure_instruments_not_running!
        template_path = get_template_path()
        trace_path = File.join(TRACE_PATH, "getip.trace")
        #FileUtils.mkdir_p(trace_path)
        
        ip_regex = "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
        getip_cmd = "instruments -w #{@deviceID} -D #{trace_path}  -t #{template_path} #{app_path} -e UIASCRIPT #{GET_IP_SCRIPT_PATH}"
        #Kernel.puts getip_cmd
        is_ip_found = false
        getip_process = IO.popen(getip_cmd)
        getip_process.each do |line|
            line.chomp!
            line.scan(/Pass:[\s]?(#{ip_regex})[\s]?$/).each do |x|
                @ipaddr = x[0]
                Kernel.puts "found IP is:" + @ipaddr
                is_ip_found = true
            end
            break if is_ip_found
        end

        @ipaddr
    end
    
    def set_runtime(runtime)
        @__cucumber_runtime = runtime
    end
    
    def install(app_path)
        return if(ENV["DEBUGIOS"])
        p "Starting to install iOS APP..."
        output = []
        IO.popen("./ios/script/ios-deploy  --id #{@deviceID}  --bundle #{app_path}").each do |line|
            p line.chomp
            output << line.chomp
        end
    end
    
    def backdoor(sel, arg)
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37264"
        ENV['DEVICE_TARGET'] = @deviceID
        
        super(sel, arg)
    end
    
    def launch()
        return if(ENV["DEBUGIOS"])
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37264"
        ENV['DEVICE_TARGET'] = @deviceID
        ENV['TRACE_TEMPLATE'] = get_template_path()
        
        @launcher = Calabash::Cucumber::Launcher.new
        unless @launcher.calabash_no_launch?
            @launcher.relaunch
            @launcher.calabash_notify(self)
        end
    end
    
    def exitapp()
        getlog()
        
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37264"
        ENV['DEVICE_TARGET'] = @deviceID
        
        unless ENV["DEBUGIOS"]
            unless @launcher.calabash_no_stop?
                calabash_exit
                if @launcher.active?
                    @launcher.stop
                end
            end
        end
    end
    
    def fetchFile(srcFile, dstFile)
        `./ios/script/iFileTransfer -o download -id #{@deviceID} -app #{IOS_APP_NAME} -from #{srcFile} -to #{dstFile}`
        return $?.success?
    end
    
    def pushFile(srcFile, dstFile)
        cmds = "./iOS/script/iFileTransfer -o copy -f true -id #{@deviceID} -app #{IOS_APP_NAME} -from #{srcFile} -to #{dstFile}"
        p "pushFile begin"
        p cmds
        p "pushFile end"
        system(cmds)
        return $?.success?
    end
    
    def deletePath(targetPath)
        cmds = "./iOS/script/iFileTransfer -o delete -id #{@deviceID} -app #{IOS_APP_NAME} -target #{targetPath}"
        p cmds
        system(cmds)
    end
    
    def rotateDevice(dir)
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37264";
        ENV['DEVICE_TARGET'] = @deviceID
        rotate(dir)
    end

    def rotateHomeButton(dir)
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37264";
        ENV['DEVICE_TARGET'] = @deviceID
        rotate_home_button_to(dir)
    end
    
    def getlog()
        p "starting to get log..."
        output = []
        devman = WX2Calabash.deviceman
        trace_path = File.join(ENV["TEST_TRACE_PATH"], "ios_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.log")
        cmd = "./ios/script/iFileTransfer -o download -id #{@deviceID} -app #{IOS_APP_NAME} -from #{IOS_LOG_PATH} -to #{trace_path}"
        p cmd
        IO.popen(cmd).each do |line|
            p line.chomp
            output << line.chomp
        end
    end

    def start_capture_packet(captureMethod = "rvi")
        # p "ios, in start_capture_packet(), deviceID:" + "#{@deviceID}"
        devman = WX2Calabash.deviceman
        @pcap_path = File.join(ENV["TEST_TRACE_PATH"], "ios_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.pcap")
        p "captureMethod:#{captureMethod}"

        if (captureMethod == "device") or (captureMethod == "default")
            @tcpdump_th = Thread.new do
                puts "ssh root@#{@ipaddr} 'tcpdump -w #{@pcap_path}'"
                tcpdump_process = IO.popen("ssh root@#{@ipaddr} 'tcpdump -w /#{@pcap_path}'")
                Thread.current[:pid] = tcpdump_process.pid
                p "ios, after open tcpdump thread, id:#{tcpdump_process.pid}"
            end
        elsif (captureMethod == "rvi")
            p "capture on host machine by remoteVirtualInterface, id:#{@deviceID}"
            @tcpdump_th = Thread.new do
                p "new thread, id:#{@deviceID}"
                # on tyler's mac machine, returned value of rvictl -l is not includes rvi0 interface infor,
                # so we use ifconfig -l and check new added interface, then get rvi port inforamtion
                old_rvi_port_array = `ifconfig -l | awk '{for (i = 1; i <= NF; i++) if ($i ~ /rvi/) print $i}'`.split("\n")
                result = `rvictl -s #{@deviceID}`
                p "rvictl result -> #{result}"

                new_rvi_port_array = `ifconfig -l | awk '{for (i = 1; i <= NF; i++) if ($i ~ /rvi/) print $i}'`.split("\n")
                add_rvi_port = new_rvi_port_array - old_rvi_port_array
                rvi_port = add_rvi_port[0]

                if (rvi_port == nil)
                    p "open rvictl failed on device #{@devicdID}"
                else
                    tcpdump_process = IO.popen("tcpdump -w #{@pcap_path} -i #{rvi_port}")
                    Thread.current[:pid] = tcpdump_process.pid
                end
            end
        else
            p "start_capture_packet, captureMethod:#{captureMethod} is unvalid"
        end
    end

    def stop_capture_packet(captureMethod = "default")
        if(@tcpdump_th)
            if (@tcpdump_th[:pid])
                Process.kill(:SIGKILL, @tcpdump_th[:pid])
            end
            @tcpdump_th.join()
            @tcpdump_th = nil

            if (captureMethod == "rvi")
                p "rvictl stop rvi on device:#{@deviceID}"
                `rvictl -x #{@deviceID}`
            else
                p "stop_capture_packet, captureMethod:#{captureMethod} is invalid"
            end
        else
            return nil
       end
    end

end

class DeviceManager
    attr_reader :scenario_name, :feature_name, :AppleDevices, :AndroidDevices, :WinPhoneDevices
    def initialize
        @AppleDevices = []
        @AndroidDevices = []
        @WinPhoneDevices = []
        @adb_port = 37210
        @win_mac_port = 43333
        @test_port = 7102

        #ENV["APP_PATH"] = expand_local_path(USER_APK_PATH)
        #ENV["TEST_APP_PATH"] = resigned_test_server_path(ENV["APP_PATH"])
        #ENV["MAIN_ACTIVITY"] = main_activity(ENV["APP_PATH"])
        FileUtils.mkdir_p(TRACE_PATH)
        FileUtils.rm_rf(WINDOWS_TEMP_PATH)
        FileUtils.mkdir_p(WINDOWS_TEMP_PATH)
        ENV["TEST_TRACE_PATH"] = File.join(TRACE_PATH, DateTime.now.strftime("%Y_%m_%d_%H_%M_%S"))
        FileUtils.mkdir_p(ENV["TEST_TRACE_PATH"])

        #puts "Your APK is:" + ENV["APP_PATH"]
        #puts "Test APK is:" + ENV["TEST_APP_PATH"]
        #puts "Main activity is:" + ENV["MAIN_ACTIVITY"]

    end
   
    def adb_list_devices
        lstDevices = []
        devices = `#{Env.adb_path} devices`
        devices.split("\n").each do |line|
            cols = line.split("\t")
            if(cols.length == 2 && cols[1] == "device")
                p "adb_list_devices, detected android device: #{cols[0]}" 
                lstDevices.push(cols[0])
            end    
        end
        return lstDevices
    end
    
    def load_android_devices

        ENV["APP_PATH"] = expand_local_path(USER_APK_PATH)
        ENV["TEST_APP_PATH"] = resigned_test_server_path(ENV["APP_PATH"])
        ENV["MAIN_ACTIVITY"] = main_activity(ENV["APP_PATH"])
        expectedAndroidNum = ENV["EXPECTED_ANDROID_NUM"].to_i

        puts "Your APK is:" + ENV["APP_PATH"]
        puts "Test APK is:" + ENV["TEST_APP_PATH"]
        puts "Main activity is:" + ENV["MAIN_ACTIVITY"]

        i = 0
        while i < 3 do
            lstDevices = adb_list_devices
            puts "list devices round #{i}, listed devices=#{lstDevices.length}, expected=#{expectedAndroidNum}"
            break if(expectedAndroidNum == 0 || lstDevices.length >= expectedAndroidNum)
            i = i + 1
        end

        @AndroidDevices = []
        lstDevices.each do |dev|
            @AndroidDevices.push(AdroidDevice.new(dev, @adb_port, @test_port)) 
            @adb_port += 1
            @test_port += 1 
        end
    end
    
    def load_ios_devices
        @AppleDevices = []
        deviceIds=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
        deviceIds.split("\n").each do |line|
            p "Detected ios device: #{line}"
            @AppleDevices.push(IOSDevice.new(line)) 
        end
    end

    def load_winphone_devices
        p "load_winphone_devices"
        @WinPhoneDevices = []
        is_windows = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
        @WinPhoneDevices.push(WinPhone.new) if(is_windows)
    end


    def get_debug_port(app_type,device_index)
      #
      debug_port_config = ""
      debug_port = 0
      if(app_type == "as_dummy")
        debug_port_config = ENV["DEBUGMODE_AS_DUMMY"]
        p "============ #{ENV["DEBUGMODE_AS_DUMMY"]}"
      else
        debug_port_config = ENV["DEBUGMODE"]
        p "============ #{ENV["DEBUGMODE"]}"
      end
      
      if(debug_port_config == nil || debug_port_config == "")
        debug_port = 0
      else
        debug_ports = debug_port_config.split(':')
        if(debug_port==nil)
          debug_ports = 0;
        else
          debug_port = debug_ports[device_index]
          if(debug_port==nil)
              debug_port = 0
           else
              debug_port = debug_port.to_i
           end
        end
      end
      
      #
      device_port = debug_port
      if(device_port==0)
        @win_mac_port = @win_mac_port + 1
        device_port = @win_mac_port          
      end
        
      return debug_port!=0,device_port
    end
    
    def get_mac_device(device_index)
        debug_mode,device_port = get_debug_port("default",device_index)
        p "============new device  debug_mode=#{debug_mode},device_port=#{device_port},device_index=#{device_index}"
        return WinMacDevice.new("#{device_port}", :macosx, "default",debug_mode)
    end
    
    def get_win_device(device_index)
        debug_mode,device_port = get_debug_port("default",device_index)
        p "============new device  debug_mode=#{debug_mode},device_port=#{device_port},device_index=#{device_index}"
        return WinMacDevice.new("#{device_port}", :windows, "default",debug_mode)
    end
      
      def get_mac_as_dummy_device(device_index)
        debug_mode,device_port = get_debug_port("as_dummy",device_index)
        p "============new device  debug_mode=#{debug_mode},device_port=#{device_port},device_index=#{device_index}"
        return WinMacDevice.new("#{device_port}", :macosx, "as_dummy",debug_mode)
    end
    
    def get_win_as_dummy_device(device_index)
        debug_mode,device_port = get_debug_port("as_dummy",device_index)
        p "============new device  debug_mode=#{debug_mode},device_port=#{device_port},device_index=#{device_index}"
        return WinMacDevice.new("#{device_port}", :windows, "as_dummy",debug_mode)
    end
    
    def resign_android_apk
        #if the app has been rebuilt, we might need to resign the test app
        resign_test_server_hook(ENV["APP_PATH"]) unless File.exist?(ENV["TEST_APP_PATH"])
    end
    
    def count_android
        @AndroidDevices.length
    end
   
    def count_ios
        @AppleDevices.length
    end
    
    def count_winphone
        @WinPhoneDevices.length
    end

    
    def get_android_device(index)
        return @AndroidDevices[index] if index < count_android()
        return nil
    end
    
    def get_ios_device(index)
        return @AppleDevices[index] if index < count_ios()
        return nil
    end
   
    def get_winphone_device(index)
        return @WinPhoneDevices[index] if index < count_winphone()
        return nil
    end
   
    def ios_getip
        @AppleDevices.each do |device|
            device.get_ip(APP_BUNDLE_PATH)
        end
    end
    
    def install_to_all
        p "install to all mobile devices"
        @AndroidDevices.each do |device|
            device.device.reinstall_test_server
            device.device.reinstall_apps
        end
        
        @AppleDevices.each do |device|
            device.install(APP_BUNDLE_PATH)
        end

        @WinPhoneDevices.each do |device|
            result = device.install(expand_local_path(WINPHONE_APP_PATH))
            if !result["success"]
                p "delete winphone device"
                @WinPhoneDevices.delete_at(0)
            end
        end

    end
    
    def set_runtime(runtime)
        @AndroidDevices.each do |device|
            device.set_runtime(runtime)
        end
        
        @AppleDevices.each do |device|
            device.set_runtime(runtime)
        end
    end
    
    def set_location(sname, feature)
        @scenario_name = sname
        @feature_name = feature
    end
end

end
