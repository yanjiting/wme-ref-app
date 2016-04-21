
var target = UIATarget.localTarget();
var val = null;
while (true) {
	try {
		val = target.frontMostApp().preferencesValueForKey("local_ip_for_test_mode");
        if(!val)
        {
            val = target.frontMostApp().mainWindow().scrollViews()[0].staticTexts()["IPLabel"].value();
        }
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