require File.join(File.dirname(__FILE__), '../../scripts/logparser', 'analyze_wqos_log')
require File.join(File.dirname(__FILE__), '../../scripts/simulators', 'simulator')
#require File.join(File.dirname(__FILE__), '../../scripts/simulators', 'nlc')

def get_ta_app(package_config)
      app_package_setting = APP_BUNDLE_PATH_CONFIG
      app_package_path = ""
      if(app_package_setting.kind_of?(Array))
        app_package_setting.each do |item|
          if(File.exist?(item))
            app_package_path = item
            break
          end
        end
       else
         app_package_path = app_package_setting
       end 
       p "current ios bundle= #{app_package_path}"
       return app_package_path
end

USER_APK_PATH = "../../android/ClickCall/target/com-wx2-clickcallapp-1.0.0-SNAPSHOT.apk"
TUA_APK_PATH = "../../android/test-server/TestServer.apk"
TUA_MANIFEST_PATH = "../../android/test-server/AndroidManifest.xml"
RESIGN_TARGET_PATH = "../../android/test-server/target/"
TRACE_PATH = expand_local_path("../../trace/")
GET_IP_SCRIPT_PATH = expand_local_path("../../iOS/script/getip.js")
APP_BUNDLE_PATH_CONFIG =[expand_local_path("../../iOS/build/Debug-iphoneos/MediaSessionIntegrationTest.app"),expand_local_path("../../iOS/build/Release-iphoneos/MediaSessionIntegrationTest.app")]
APP_BUNDLE_PATH = get_ta_app(APP_BUNDLE_PATH_CONFIG)
MAC_APP_PATH = [expand_local_path("../../MacOSX/MediaSessionTest/bin/Debug/MediaSessionTATest.app/Contents/MacOS/MediaSessionTATest"),expand_local_path("../../MacOSX/MediaSessionTest/bin/Release/MediaSessionTATest.app/Contents/MacOS/MediaSessionTATest")]
WINDOWS_EXE_PATH = [expand_local_path("../../../../mediaengine/bin/Win32/Debug/ClickCall.exe"),expand_local_path("../../../../mediaengine/bin/Win32/Release/ClickCall.exe")]
MAC_AS_DUMMY_APP_PATH = [expand_local_path("../../ta_as_dummy_app/macosx/bin/Debug/DummyApp.app/Contents/MacOS/DummyApp"),expand_local_path("../../ta_as_dummy_app/macosx/bin/Release/DummyApp.app/Contents/MacOS/DummyApp")]
WINDOWS_AS_DUMMY_PATH = [expand_local_path("../../../../mediaengine/bin/Win32/Debug/DummyApp.exe"),expand_local_path("../../../../mediaengine/bin/Win32/Release/DummyApp.exe")]
WINDOWS_TEMP_PATH = expand_local_path("../../Temp")
MAC_LOG_PATH = "/Library/Logs/wme_ta_mac/wme_ta_mac.log"
IOS_APP_NAME = "com.cisco.MediaSessionIntegrationTest"
IOS_LOG_PATH = "/Library/Caches/Logs/wme_ta_ios.log"
WINPHONE_APP_PATH = "../../WP8/Refapp-wp/Refapp-wp/Bin/ARM/Release/Refapp_wp_TA_ARM.xap"
PARAM_AUDIO = "1 "
PARAM_VIDEO = "2 "
PARAM_SCREEN = "3 "
PARAM_QOS = "4 "
PARAM_FEC = "5 "
