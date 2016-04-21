require File.join(File.dirname(__FILE__), '.', 'analyze_wqos_log')

if AnalyzeWqosLog("logs/qos.log", 2000000)
    print `ls -ltr logs/*.png`
    `rm -f logs/*.png logs/wme_analyze.txt`
else
    puts "Analyze Wqos Logs Failed"
end
 
