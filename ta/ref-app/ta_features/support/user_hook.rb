include WX2Calabash

AfterConfiguration do |config|
    WX2Calabash.deviceman = DeviceManager.new
    WX2Calabash.deviceman.load_android_devices unless ENV["SKIPANDROID"]
    WX2Calabash.deviceman.resign_android_apk unless ENV["SKIPANDROID"]
    WX2Calabash.deviceman.load_ios_devices unless ENV["SKIPIOS"]
    WX2Calabash.deviceman.load_winphone_devices unless ENV["SKIPWP8"]
    WX2Calabash.deviceman.install_to_all unless ENV["NOINSTALL"]
    WX2Calabash.deviceman.ios_getip
    Calabash::Android::VERSION = "0.4.22.pre4"

	if(ENV["NETWORKEMULATE"])
		flush_network_simulator() unless (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
	end
end

def check_skip_lab(tags)
	is_windows = (RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/) != nil
	tags.each do |tag|
		if(tag.include?("lab") && (!ENV["NETWORKEMULATE"] || is_windows))
			pending("You are not ready to run network emulates, skip this scenario.")
			break
		end
	end
end

Before do |scenario|
	@scenario_tags = scenario.source_tag_names
    sname_key = scenario.source_tag_names[0]
    case scenario
        when Cucumber::Ast::OutlineTable::ExampleRow
            fname_key = scenario.scenario_outline.feature.name.split().first
            sname_key += scenario.name.split("|").map(&:strip).join("_")
        when Cucumber::Ast::Scenario
            fname_key = scenario.feature.name.split().first
    end
    WX2Calabash.deviceman.set_location(fname_key, sname_key)
    Kernel.puts "Start to execute scenario <#{sname_key}> in feature <#{fname_key}>"
    Kernel.puts "Attached #{WX2Calabash.deviceman.count_android} android devices ,#{WX2Calabash.deviceman.count_winphone} winphone devices,and #{WX2Calabash.deviceman.count_ios} iOS devices."
end

After do |scenario|
    #hook here to clean up all resources even when failure
    cleanup_resources
end
