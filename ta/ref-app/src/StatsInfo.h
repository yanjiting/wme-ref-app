#ifndef __STATSINFO_H__
#define __STATSINFO_H__

#include "CmTimerWrapperID.h"
#include "MediaConnection.h"
#include <string>
#include <sstream>
#include <iostream>

class StatsInfoSink
{
public:
    virtual void OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats) = 0;
};

class StatsInfo : public CCmTimerWrapperIDSink
{
public:
    StatsInfo();
    ~StatsInfo();
    void SetSink(StatsInfoSink *sink);
    void Start();
    void Stop();
protected:
    virtual void OnTimer(CCmTimerWrapperID* aId);
    std::string GetConnectionType(wme::WmeTransportType type);
    std::string formatStatisticsAudio(wme::WmeAudioConnectionStatistics stStat);
    std::string formatStatisticsVideo(wme::WmeVideoConnectionStatistics stStat);
    std::string formatStatisticsScreen(wme::WmeScreenConnectionStatistics stStat);
private:
    CCmTimerWrapperID m_Timer;
    StatsInfoSink *m_sink;
	std::string m_strStatistic;
};


#endif //__STATSINFO_H__
