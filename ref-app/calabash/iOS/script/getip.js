
var target = UIATarget.localTarget();
var val = null;
while (true) {
	try {
		val = target.frontMostApp().preferencesValueForKey("local_ip_for_test_mode");
	} catch (e) {
		target.delay(0.5);
		continue;
	}
	
	if (!val) {
		target.delay(0.2);
		continue;
	}
	else{
		UIALogger.logPass(val);
		break;
	}
}