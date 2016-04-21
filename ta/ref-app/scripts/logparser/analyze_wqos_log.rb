
def AnalyzeWqosLog(logFile, bandwidthLimit, lossThreshold=0.15, qdelayThreshold=150, rttThreshold=300)
    logparser=File.join(File.dirname(__FILE__), 'logparser.py')
    if logFile.length <= 0
        p 'Please supply log file for AnalyzeWqosLog'
        return false
    end
    `python #{logparser} #{logFile} --qos-bw-limit #{bandwidthLimit} --qos-loss-threshold #{lossThreshold} --qos-qdelay-threshold #{qdelayThreshold} --qos-rtt-threshold #{rttThreshold} --qos --savepng --nogrid`

    return $?.success?
end
