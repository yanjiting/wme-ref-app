#include "DeskShareSender.h"

DeskShareSender::DeskShareSender() : VideoSettings(m_pTrack)
{
    m_pScreenSource = NULL;
    memset(&m_rawFormat, 0, sizeof(m_rawFormat));
}

DeskShareSender::~DeskShareSender()
{
    SAFE_RELEASE(m_pScreenSource);
}

long DeskShareSender::SetCapability(WmeMediaBaseCapability *pMC)
{
    returnv_if_fail(pMC != NULL, WME_E_INVALIDARG);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    WmeVideoEncodeConfiguraion param;
    memset(&param, 0, sizeof(param));

    WmeVideoMediaCapability *pVMC = (WmeVideoMediaCapability *)pMC;

    param.iMaxFrameRate = pVMC->frame_layer[pVMC->frame_layer_number-1]/100.0; 
    param.iMaxWidth = pVMC->width;
    param.iMaxHeight = pVMC->height;

    switch (pVMC->height)
    {
    case MAX_VIDEO_HEIGHT:
        param.iMaxSpacialLayer = MAX_VIDEO_SPACIAL_LAYER;
        break;
    case MAX_VIDEO_HEIGHT/2:
        param.iMaxSpacialLayer = MAX_VIDEO_SPACIAL_LAYER - 1;
        break;
    case MAX_VIDEO_HEIGHT/4:
        param.iMaxSpacialLayer = MAX_VIDEO_SPACIAL_LAYER - 2;
        break;
    default:
        param.iMaxSpacialLayer = 1;
        break;
    }

           param.iMaxFrameRate = 25;
           param.iMaxWidth = 1680;
           param.iMaxHeight = 1050;
           param.iMaxSpacialLayer = 1;


    m_pTrack->SetOption(WmeTrackOption_VideoEncodeConfiguraion, &param, sizeof(param));

    if (m_rawFormat.iWidth < param.iMaxWidth || 
            m_rawFormat.iHeight < param.iMaxHeight || 
            m_rawFormat.fFrameRate < param.iMaxFrameRate) {
        WmeVideoRawFormat rawFormat;
        rawFormat.eRawType = WmeI420;
        rawFormat.iWidth = param.iMaxWidth;
        rawFormat.iHeight = param.iMaxHeight;
        rawFormat.fFrameRate = param.iMaxFrameRate;
        rawFormat.uTimestamp = 0;
        DeskShareSender::SetCaptureCapability(rawFormat);
    }

    return WME_S_OK;
}

long DeskShareSender::CreateTrack()
{
    CM_INFO_TRACE_THIS("DeskShareSender::CreateTrack, begin!");
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_INFO_TRACE_THIS("DeskShareSender::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;
    
	ret = m_pEngine->CreateLocalScreenShareTrack((IWmeLocalScreenShareTrack **) &m_pTrack);
    if (ret != WME_S_OK || !m_pTrack) {
        CM_ERROR_TRACE_THIS("DeskShareSender::CreateTrack, fail to CreateLocalScreenShareTrack.");
        return WME_E_FAIL;
    }
    uint32_t label = kLocalScreenShareTrack_Label;
    m_pTrack->SetTrackLabel(label);

    return WME_S_OK;
}

long DeskShareSender::SetDefaultSettings()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;

    /// default settings for video encoder capability
    WmeVideoMediaCapability vmc;  ///???: cannot memset of this structure
    vmc.width = 640; //320; //whsu
    vmc.height = 360; //180;
    vmc.frame_layer_number = 1;
    vmc.frame_layer[vmc.frame_layer_number-1] = 30 * 100;
    ret = DeskShareSender::SetCapability((WmeMediaBaseCapability *)&vmc);

    /// default settings for video capture device
    WmeVideoRawFormat rawFormat;
    rawFormat.eRawType = WmeI420;
    rawFormat.iWidth = 640; //320; //whsu
    rawFormat.iHeight = 360; //180;
    rawFormat.fFrameRate = 30;
    rawFormat.uTimestamp = 0;
    ret = DeskShareSender::SetCaptureCapability(rawFormat);

    /// default settings for uplink bandwidth
    WmeUpLinkNetStat uplink;
    memset(&uplink, 0, sizeof(uplink));
    uplink.NetStat.delay = 0;
    uplink.NetStat.jitter = 0;
    uplink.NetStat.loss_ratio = 0;
    uplink.NetStat.recv_rate = 0;
    uplink.nCongestStatus = 1;
    uplink.dwEvaluateBandwidth = 640000; /// bandwidth for 360p
    ret = m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, &uplink, sizeof(uplink));

    WmeVideoEncodeLayerType single = WmeVideoSingleLayer;
    ret = m_pTrack->SetOption(WmeTrackOption_VideoEncodeLayerType, &single, sizeof(WmeVideoEncodeLayerType));

    WmePerformanceProfileType perf = WmePerformanceProfileNormal;	/// default 360p
    ret = m_pTrack->SetOption(WmeTrackOption_StaticPerformanceProfile, &perf, sizeof(perf));

    return ret;
}

long DeskShareSender::DeleteTrack()
{
    SetTransport(NULL);

    return MediaClient::DeleteTrack();
}

long DeskShareSender::SetTransport(IWmeMediaTransport *sink)
{
    returnv_if_fail(m_pSession != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    WMEIID ulObserverID = WMEIID_IWmeMediaMessageTransmitterObserver;
    if (sink != NULL) {
        ret = m_pSession->AddObserver(ulObserverID, (IWmeMediaMessageTransmitterObserver*)this);
    }else{
        ret = m_pSession->RemoveObserver(ulObserverID, (IWmeMediaMessageTransmitterObserver *)this);
    }

    return MediaSender::SetTransport(sink);
}

long DeskShareSender::SetScreenSource(IWmeScreenSource* pIWmeScreenSource)
{
    IWmeScreenSource *oldSource = m_pScreenSource;
    
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    
    m_pScreenSource = pIWmeScreenSource;
    m_pScreenSource->AddRef();

    long ret = WME_E_FAIL;
    IWmeLocalScreenShareTrack *pTrack = (IWmeLocalScreenShareTrack *)m_pTrack;
    if (m_bStarted) {
//        ret = pTrack->Stop();
        if( oldSource ) {
            pTrack->RemoveScreenSource(oldSource);
            ret = pTrack->AddScreenSource(m_pScreenSource);
        }
        else {
            ret = pTrack->AddScreenSource(m_pScreenSource);
            //ret = pTrack->SetCaptureFormat(&m_rawFormat);
            ret = pTrack->Start();
        }
    }else {
        ret = pTrack->AddScreenSource(m_pScreenSource);
        //ret = pTrack->SetCaptureFormat(&m_rawFormat);
    }
    
    SAFE_RELEASE(oldSource);
    if(oldSource) {
        oldSource->AddRef();
    }
    return ret;
}


long DeskShareSender::SetCaptureCapability(const WmeVideoRawFormat & format)
{
//    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
//    m_rawFormat = format;
//    ((IWmeLocalVideoTrack *)m_pTrack)->SetCaptureFormat(&m_rawFormat);
    return WME_S_OK;
}


// for IWmeMediaMessageTransmitterObserver
WMERESULT DeskShareSender::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

// for IWmeMediaMessageTransmitterObserver
WMERESULT DeskShareSender::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

// for IWmeMediaMessageTransmitterObserver
WMERESULT DeskShareSender::OnPictureLossIndication(IWmeMediaEventNotifier *pNotifier, uint32_t uSourceID, uint32_t uStreamId)
{
    returnv_if_fail(m_pTrack != NULL, WME_S_FALSE);
    return m_pTrack->SendEvent(wme::WmeTrackEvent_ForceKeyFrame, NULL, 0);
}

