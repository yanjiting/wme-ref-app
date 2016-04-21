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
    
    def startCaptureLog
        `#{@device.adb_command} logcat -c`
        devman = WX2Calabash.deviceman
        logfilename = "log_#{devman.scenario_name}_#{devman.feature_name}_#{@deviceID}.logcat"
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
    end
    
    def exitapp()
        stopCaptureLog
        device.shutdown_test_server
    end
    
    def set_runtime(runtime)
        @__cucumber_runtime = runtime
    end
end

class IOSDevice
    include Cucumber::RbSupport::RbWorld
    include Calabash::Cucumber::Core
    include Calabash::Cucumber::Operations
    
    attr_reader :deviceID
    attr_reader :ipaddr
    def initialize(udid)
        @deviceID = udid
        @ipaddr = "0.0.0.0"
    end
    
    def ensure_instruments_not_running!
      pids_str = `ps x -o pid,command | grep -v grep | grep "instruments" | awk '{printf "%s,", $1}'`.strip
      instruments_pids = pids_str.split(',').map { |pid| pid.to_i }
      instruments_pids.each do |pid|
        p "Found instruments #{pid}. Killing..."
        `kill -9 #{pid} && wait #{pid} &> /dev/null`
      end
    end
   
    def get_ip(app_path)
        p "starting to get IP address..."
        ensure_instruments_not_running!
        template_path = "/Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate"
        trace_path = File.join(TRACE_PATH, "getip.trace")
        output = `instruments -w #{@deviceID} -D #{trace_path}  -t #{template_path} #{app_path} -e UIASCRIPT #{GET_IP_SCRIPT_PATH}`
        ip_regex = "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
        output.split("\n").each do |line|
            line.scan(/Pass:[\s]?(#{ip_regex})[\s]?$/).each do |x|
                @ipaddr = x[0]
                p "found IP is:" + @ipaddr
            end
        end
        @ipaddr
    end
    
    def set_runtime(runtime)
        @__cucumber_runtime = runtime
    end
    
    def install(app_path)
        p "starting to install APP..."
        output = []
        IO.popen("./ios/script/ios-deploy  --id #{@deviceID}  --bundle #{app_path}").each do |line|
            p line.chomp
            output << line.chomp
        end
    end
    
    def backdoor(sel, arg)
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37265"
        ENV['DEVICE_TARGET'] = @deviceID
        
        return super(sel, arg)
    end
    
    def launch()
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37265"
        ENV['DEVICE_TARGET'] = @deviceID
        
        @launcher = Calabash::Cucumber::Launcher.new
        unless @launcher.calabash_no_launch?
            @launcher.relaunch
            @launcher.calabash_notify(self)
        end
    end
    
    def exitapp()
        getlog()
        
        ENV['DEVICE_ENDPOINT'] = "http://#{@ipaddr}:37265"
        ENV['DEVICE_TARGET'] = @deviceID
        
        unless @launcher.calabash_no_stop?
            calabash_exit
            if @launcher.active?
                @launcher.stop
            end
        end
    end
    
    def getlog()
        p "starting to get log..."
        output = []
        trace_path = File.join(ENV["TEST_TRACE_PATH"], "ios_#{@deviceID}.log")
        cmd = "./ios/script/iFileTransfer -o download -id #{@deviceID} -app #{IOS_APP_NAME} -from #{IOS_LOG_PATH} -to #{trace_path}"
        p cmd
        IO.popen(cmd).each do |line|
            p line.chomp
            output << line.chomp
        end
    end
end

class DeviceManager
    attr_reader :scenario_name, :feature_name
    def initialize
        @AppleDevices = []
        @AndroidDevices = []
        @adb_port = 37210
        @test_port = 7102
        ENV["APP_PATH"] = expand_local_path(USER_APK_PATH)
        ENV["TEST_APP_PATH"] = resigned_test_server_path(ENV["APP_PATH"])
        ENV["MAIN_ACTIVITY"] = main_activity(ENV["APP_PATH"])
        FileUtils.mkdir_p(TRACE_PATH)
        ENV["TEST_TRACE_PATH"] = File.join(TRACE_PATH, DateTime.now.strftime("%Y_%m_%d_%H_%M_%S"))
        FileUtils.mkdir_p(ENV["TEST_TRACE_PATH"])

        puts "Your APK is:" + ENV["APP_PATH"]
        puts "Test APK is:" + ENV["TEST_APP_PATH"]
        puts "Main activity is:" + ENV["MAIN_ACTIVITY"]
    end
    
    def load_android_devices
        puts "Start to load android devices"
        @AndroidDevices = []
        devices = `#{Env.adb_path} devices`
        devices.split("\n").each do |line|
            cols = line.split("\t")
            if(cols.length == 2 && cols[1] == "device")
                @AndroidDevices.push(AdroidDevice.new(cols[0], @adb_port, @test_port)) 
                @adb_port += 1
                @test_port += 1
            end    
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
    
    def get_android_device(index)
        return @AndroidDevices[index] if index < count_android()
        return nil
    end
    
    def get_ios_device(index)
        return @AppleDevices[index] if index < count_ios()
        return nil
    end
    
    def ios_getip
        @AppleDevices.each do |device|
            device.get_ip(APP_BUNDLE_PATH)
        end
    end
    
    def install_to_all
        @AndroidDevices.each do |device|
            device.device.reinstall_test_server
            device.device.reinstall_apps
        end
        
        @AppleDevices.each do |device|
            device.install(APP_BUNDLE_PATH)
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
