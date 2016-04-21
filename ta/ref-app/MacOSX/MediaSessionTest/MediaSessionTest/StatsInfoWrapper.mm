#include "StatsInfoWrapper.h"


StatsInfoWrapper::StatsInfoWrapper()
{
    m_statsInfo.SetSink(this);
    m_statsInfo.Start();
}

StatsInfoWrapper::~StatsInfoWrapper()
{
    m_statsInfo.SetSink(NULL);
    m_statsInfo.Stop();
}


void StatsInfoWrapper::OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats)
{
    NSString *info = [NSString stringWithUTF8String:strInfo];
    [m_object performSelector:m_selector withObject:info];
}


void StatsInfoWrapper::setupCallback(id object, SEL selector)
{
    m_object = object;
    m_selector = selector;
}
