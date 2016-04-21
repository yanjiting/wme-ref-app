#ifndef __STATSINFOWRAPPER_H__
#define __STATSINFOWRAPPER_H__

#import "StatsInfo.h"

class StatsInfoWrapper : public StatsInfoSink
{
public:
    StatsInfoWrapper();
    ~StatsInfoWrapper();
	virtual void OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats);
    
    void setupCallback(id object, SEL selector);

private:
    StatsInfo m_statsInfo;
    id m_object;
    SEL m_selector;
};


#endif //__STATSINFOWRAPPER_H__