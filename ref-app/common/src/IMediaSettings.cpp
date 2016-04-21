#include "IMediaSettings.h"

VideoSettings::VideoSettings(IWmeMediaTrack * &pTrack) : m_pTrackRef(pTrack) 
{
    m_bWindowLess = false;
    m_pRender = NULL;
	m_eRenderMode = WmeRenderModeFill;
#ifdef WIN32
    m_pDC = NULL;
#endif
}

VideoSettings::~VideoSettings() 
{
    m_pTrackRef = NULL; 
    m_pRender = NULL; 
#ifdef WIN32
    m_pDC = NULL;
#endif
}

long VideoSettings::SetDevice(IWmeMediaDevice* pCamera) 
{ 
    return WME_S_OK; 
}

long VideoSettings::SetScreenSource(IWmeScreenSource* pIWmeScreenSource)
{
    return WME_S_OK;
}

long VideoSettings::GetDevice(IWmeMediaDevice** ppCamera)
{
	return WME_S_OK;
}

long VideoSettings::SetCaptureCapability(const WmeVideoRawFormat & format)
{
    return WME_S_OK;
}

long VideoSettings::SetVideoQuality(WmeVideoQualityType quality)
{
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    ret = m_pTrackRef->SetOption(WmeTrackOption_VideoQuality, &quality, sizeof(quality));
    return ret;
}

long VideoSettings::SetStaticPerformance(WmePerformanceProfileType perf)
{
	returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    ret = m_pTrackRef->SetOption(WmeTrackOption_StaticPerformanceProfile, &perf, sizeof(perf));
    return ret;
}

long VideoSettings::SetRenderer(void *render, bool windowless) 
{
    returnv_if_fail(render != NULL, WME_E_INVALIDARG);

    m_bWindowLess = windowless;
    m_pRender = render;
#ifdef WIN32
    m_pDC = GetDC((HWND)m_pRender);
#endif

    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);

    if (m_bWindowLess) {
        m_pTrackRef->AddObserver(WMEIID_IWmeVideoRenderObserver, (IWmeVideoRenderObserver*)this);
        bool windowless = true;
        m_pTrackRef->SetOption(WmeTrackOption_VideoWindowLessRender, &windowless, sizeof(bool));
    }else{
        ((IWmeVideoTrack *)m_pTrackRef)->AddRenderWindow(m_pRender, NULL);
    }

    return WME_S_OK;
}

long VideoSettings::RemoveRenderer() 
{
    returnv_if_fail(m_pRender != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    if (m_pTrackRef) {
        if (m_bWindowLess) {
            m_pTrackRef->RemoveObserver(WMEIID_IWmeVideoRenderObserver, (IWmeVideoRenderObserver*)this);
            bool windowless = false;
            m_pTrackRef->SetOption(WmeTrackOption_VideoWindowLessRender, &windowless, sizeof(bool));
        }else {
            ((IWmeVideoTrack *)m_pTrackRef)->RemoveRenderWindow(m_pRender);
        }
        ret = WME_S_OK;
    }

#ifdef WIN32
    if(m_pRender && m_pDC) {
        ReleaseDC((HWND)m_pRender, (HDC)m_pDC);
    }
    m_pDC = NULL;
#endif
    m_pRender = NULL;

    return ret;
}

long VideoSettings::SetRenderAspectRatioSameWithSource(bool keepsame) 
{
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
    long ret = m_pTrackRef->SetOption(WmeTrackOption_AdaptiveAspectRatio, &keepsame, sizeof(keepsame));
    return ret;
}

long VideoSettings::SetRenderMode(WmeTrackRenderScalingModeType eMode)
{
	m_eRenderMode = eMode;
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
    long ret = m_pTrackRef->SetOption(WmeTrackOption_RenderScalingMode, &m_eRenderMode, sizeof(m_eRenderMode));
    return ret;
}

long VideoSettings::OnRenderWindowPositionChanged(void *render)
{
    returnv_if_fail(render != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);

#ifdef WIN32
    if(m_bWindowLess) 
    {
        CM_ERROR_TRACE_THIS("VideoSettings::OnRenderWindowPositionChanged, m_bWindowLess is true");
        return WME_E_FAIL;
    }

    if(render == m_pRender)
    {
        RECT theRect;
        GetClientRect((HWND)render, &theRect);
        CM_INFO_TRACE_THIS("VideoSettings::OnRenderWindowPositionChanged, window size changes to " <<theRect.right - theRect.left << "x" <<theRect.bottom - theRect.top << ", ");
        m_pTrackRef->SendEvent(WmeTrackEvent_RenderingPositionChanged, m_pRender, sizeof(m_pRender));
        return WME_S_OK;
    }
#endif

    return WME_E_FAIL;
}

long VideoSettings::OnRenderingDisplayChanged()
{
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);

#ifdef WIN32
    if(m_bWindowLess)
    {
        CM_ERROR_TRACE_THIS("VideoSettings::OnRenderingDisplayChanged, m_bWindowLess is true");
        return WME_E_FAIL;
    }

    m_pTrackRef->SendEvent(WmeTrackEvent_RenderingDisplayChanged, NULL, 0);
#endif

    return WME_S_OK;
}

long VideoSettings::GetStatistics(WmeVideoStatistics &stat)
{
	returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
	return ((IWmeVideoTrack *)m_pTrackRef)->GetStatistics(stat);
}

/// for IWmeVideoRenderObserver
WMERESULT VideoSettings::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeVideoRenderObserver
WMERESULT VideoSettings::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeVideoRenderObserver
WMERESULT VideoSettings::OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID)
{
#ifdef WIN32
    returnv_if_fail(pNotifier != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
    returnv_if_fail(m_pRender != NULL, WME_E_FAIL);
    returnv_if_fail(m_pDC != NULL, WME_E_FAIL);

    IWmeMediaTrack *pTrack = dynamic_cast<IWmeMediaTrack *>(pNotifier);
    if (m_pTrackRef != pTrack) {
        CM_ERROR_TRACE_THIS("VideoSettings::OnRequestedDC, m_pTrackRef != pTrack");
        return WME_E_FAIL;
    }

    WmeWindowLessRenderInfo theRenderInfo;
    memset(&theRenderInfo, 0, sizeof(WmeWindowLessRenderInfo));
    theRenderInfo.hDC = m_pDC;

    RECT theRect;
    GetClientRect((HWND)m_pRender, &theRect);
    //int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    theRenderInfo.stRect.origin.x = theRect.left;
    theRenderInfo.stRect.size.width = theRect.right - theRect.left;
    theRenderInfo.stRect.size.height = theRect.bottom - theRect.top;
    theRenderInfo.stRect.origin.y = theRect.top;
    theRenderInfo.bReDraw = false;

    return pTrack->SendEvent(WmeTrackEvent_RenderWithDC, &theRenderInfo, sizeof(WmeWindowLessRenderInfo));
#else
    return WME_S_OK;
#endif
}


//////////////////////////////////////////////
AudioSettings::AudioSettings(IWmeMediaTrack * &pTrack): m_pTrackRef(pTrack) 
{

}

AudioSettings::~AudioSettings()
{
	m_pTrackRef = NULL;
}

long AudioSettings::EnableEC(bool bEnable)
{
	if(!m_pTrackRef)
	{
		return WME_E_FAIL;
	}

	m_pTrackRef->SetOption(WmeTrackOption_EnableEC, (void *)(&bEnable), sizeof(bool));

	return WME_S_OK;
}

long AudioSettings::EnableVAD(bool bEnable)
{
	if(!m_pTrackRef)
	{
		return WME_E_FAIL;
	}

	m_pTrackRef->SetOption(WmeTrackOption_EnableVAD, (void *)(&bEnable), sizeof(bool));

	return WME_S_OK;
}

long AudioSettings::EnableNS(bool bEnable)
{
	if(!m_pTrackRef)
	{
		return WME_E_FAIL;
	}

	m_pTrackRef->SetOption(WmeTrackOption_EnableNS, (void *)(&bEnable), sizeof(bool));

	return WME_S_OK;
}

long AudioSettings::EnableAGC(bool bEnable)
{
	if(!m_pTrackRef)
	{
		return WME_E_FAIL;
	}

	m_pTrackRef->SetOption(WmeTrackOption_EnableAAGC, (void *)(&bEnable), sizeof(bool));

	return WME_S_OK;
}

long AudioSettings::EnableAutoDropData(bool bEnable)
{
	if(!m_pTrackRef)
	{
		return WME_E_FAIL;
	}

	m_pTrackRef->SetOption(WmeTrackOption_EnableDropSeconds, (void *)(&bEnable), sizeof(bool));

	return WME_S_OK;
}

long AudioSettings::GetStatistics(WmeAudioStatistics &stat)
{
	returnv_if_fail(m_pTrackRef != NULL, WME_E_FAIL);
	return ((IWmeAudioTrack *)m_pTrackRef)->GetStatistics(stat);
}