#pragma once

#include "WmeInterface.h"
#include "wp8helper.h"

using namespace wme;

Platform::String^ stdStrToPlatStr(const std::string& from)
{
    const size_t cSize = strlen(from.c_str()) + 1;
    wchar_t* wsdp = new wchar_t[cSize];
    mbstowcs(wsdp, from.c_str(), cSize);
    Platform::String^ strRet = ref new Platform::String(wsdp);
    delete[] wsdp;
    return strRet;
}

std::string platStrToStdStr(Platform::String^ from)
{
    std::wstring strRet = from->Data();
    return std::string(strRet.begin(), strRet.end());
}

const int nDEBUG = 1, nINFO = 2, nWARNING = 3, nERR = 4, nFATAL = 5;
class WMETraceSinkEnd : public IWmeTraceSink
{
public:
    WMETraceSinkEnd() 
        : _iTraceLevel(nINFO)
    {
    }
    virtual WMERESULT OnTrace(uint32_t trace_level, const char *szInfo)
    {
#ifdef _M_ARM

        if (!szInfo || _stricmp(szInfo, "") == 0)
            return WME_S_FALSE;


        if (trace_level <= _iTraceLevel)
        {
            Platform::String^ msg = stdStrToPlatStr(szInfo);
            switch (trace_level)
            {
            case nINFO:
                LogSink::G2LogSink::Instance->Info(msg);
                break;
            case nWARNING:
                LogSink::G2LogSink::Instance->Warn(msg);
                break;
            case nDEBUG:
                LogSink::G2LogSink::Instance->Debug(msg);
                break;
            case nERR:
                LogSink::G2LogSink::Instance->Error(msg);
                break;
            case nFATAL:
                LogSink::G2LogSink::Instance->Fatal(msg);
                break;

            default:
                break;
            }

        }
        return WME_S_OK;
#else
        throw ref new NotImplementedException("API only supported for ARM");
#endif
    }
    virtual ~WMETraceSinkEnd() {
        LogSink::G2LogSink::Instance->StartLog(false);
    }
    void SetTraceLevel(unsigned int traceLevel)
    {
        _iTraceLevel = traceLevel;
    }

private:
    unsigned int _iTraceLevel;
};