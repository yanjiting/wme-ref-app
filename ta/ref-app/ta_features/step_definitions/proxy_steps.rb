#include Test::Unit::Assertions

When(/^I do proxy setting from device (\d+) for (.*)$/) do |deviceIdx, proxyType|
	system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable /t REG_DWORD /d 1 /f'
	system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyOverride /t REG_SZ /d "" /f'
    if(proxyType == "BASIC")
		system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyServer /d "http=10.224.204.38:3128;https=10.224.204.38:3128" /f'
        p "supported proxy type BASIC"
    elsif (proxyType == "DIGEST")
		system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyServer /d "http=10.224.204.38:3128;https=10.224.204.38:3128" /f'
        p "supported proxy type DIGEST"
    elsif (proxyType == "NTLM")
		system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyServer /d "http=10.224.204.38:3128;https=10.224.204.38:3128" /f'
        p "supported proxy type NTLM"
    elsif (proxyType == "SOCKS")
		system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyServer /d "socks=10.224.204.38:1080" /f'
        p "supported proxy type SOCKS"
    else
        p "not supported proxy type"
    end

end

Then(/^I clean proxy setting from device (\d+)$/) do |deviceIdx|
	system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyEnable /t REG_DWORD /d 0 /f'
	system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyServer /d "" /f'
	system 'reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings" /v ProxyOverride /t REG_SZ /d 0 /f'
    p "clean supported proxy setting"
end

When(/^I do proxy setting from mac device (\d+) for (.*)$/) do |deviceIdx, proxyType|
    if(proxyType == "AUTO")
        system 'sudo networksetup -setautoproxystate Wi-Fi on'
        system 'sudo networksetup -setautoproxyurl Wi-Fi http://10.224.204.38/test.pac'
        p "supported proxy type AUTO"
    elsif (proxyType == "HTTP")
        system 'sudo networksetup -setwebproxystate Wi-Fi on'
        system 'sudo networksetup -setwebproxy Wi-Fi 10.224.204.38 3128'
        p "supported proxy type HTTP"
    elsif (proxyType == "SOCKS")
        system 'sudo networksetup -setsocksfirewallproxystate Wi-Fi on'
        system 'sudo networksetup -setsocksfirewallproxy Wi-Fi 10.224.204.38 1080'
        p "supported proxy type SOCKS"
    else
        p "not supported proxy type"
    end
    
end

Then(/^I clean proxy setting from mac device (\d+)$/) do |deviceIdx|
    system 'sudo networksetup -setwebproxystate Wi-Fi off'
    system 'sudo networksetup -setsocksfirewallproxystate Wi-Fi off'
    system 'sudo networksetup -setautoproxystate Wi-Fi off'
    p "clean supported proxy setting"
end
