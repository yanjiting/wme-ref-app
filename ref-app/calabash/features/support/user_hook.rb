
include WX2Calabash

AfterConfiguration do |config|
    WX2Calabash.deviceman = DeviceManager.new
    WX2Calabash.deviceman.load_android_devices unless ENV["SKIPANDROID"]
    WX2Calabash.deviceman.resign_android_apk unless ENV["SKIPANDROID"]
    WX2Calabash.deviceman.load_ios_devices unless ENV["SKIPIOS"]
    WX2Calabash.deviceman.install_to_all unless ENV["NOINSTALL"]
    WX2Calabash.deviceman.ios_getip
    Calabash::Android::VERSION = "0.4.22.pre4"
end

Before do |scenario|
    sname_key = scenario.name.split().first
    fname_key = scenario.feature.name.split().first
    p "Start to execute scenario #{sname_key} in freature: #{fname_key}."
    WX2Calabash.deviceman.set_location(fname_key, sname_key)
    p "Attached #{WX2Calabash.deviceman.count_android} android devices and #{WX2Calabash.deviceman.count_ios} iOS devices."
end

After do |scenario|
    #hook here to clean up all resources even when failure
    cleanup_resources
end