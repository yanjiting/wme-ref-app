#include "VideoSender.h"
#include "FileCaptureEngineImp.h"

#ifdef ANDROID
#include "wmenative-jni.h"
#endif //ANDROID

#define USE_VIDEO_FILE
VideoSender::VideoSender() : VideoSettings(m_pTrack)
{
    m_pCamera = NULL;
    memset(&m_rawFormat, 0, sizeof(m_rawFormat));
	m_pMediaFileCap = NULL;
	//m_pFileCapEngine = NULL;
	m_bUseVideoFile = FALSE;
}

VideoSender::~VideoSender()
{
    SAFE_RELEASE(m_pCamera);
	// if(m_pFileCapEngine)
	// {
	// 	m_pFileCapEngine->Uninit();
	// 	delete m_pFileCapEngine;
	// 	m_pFileCapEngine = NULL;
	// }
	if (m_pMediaFileCap)
	{
		m_pMediaFileCap->RemoveObserver(WMEIID_IWmeMediaFileCapturerObserver, this);
		m_pMediaFileCap->Release();
		m_pMediaFileCap = NULL;
	}
}

long VideoSender::SetCapability(WmeMediaBaseCapability *pMC)
{
    returnv_if_fail(pMC != NULL, WME_E_INVALIDARG);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    WmeVideoEncodeConfiguraion param;
    memset(&param, 0, sizeof(param));

    WmeVideoMediaCapability *pVMC = (WmeVideoMediaCapability *)pMC;

    param.iMaxFrameRate = pVMC->frame_layer[pVMC->frame_layer_number-1]/100.0; 
    param.iMaxWidth = pVMC->width;
    param.iMaxHeight = pVMC->height;

	IWmeMediaCodec* pCodec = NULL;
	m_pTrack->GetCodec(&pCodec);
	returnv_if_fail(pCodec != NULL, WME_E_POINTER);
	WmeCodecType codec_type = WmeCodecType_Unknown;
	pCodec->GetCodecType(codec_type);
	pCodec->Release();
	pCodec = NULL;
	if (codec_type == WmeCodecType_AVC)
	{
		param.iMaxSpacialLayer = 1;
	}
	else if (codec_type == WmeCodecType_SVC || codec_type == WmeCodecType_HEVC)
	{
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
	}

    m_pTrack->SetOption(WmeTrackOption_VideoEncodeConfiguraion, &param, sizeof(param));

#if 0
    if (m_rawFormat.iWidth < param.iMaxWidth || 
            m_rawFormat.iHeight < param.iMaxHeight || 
            m_rawFormat.fFrameRate < param.iMaxFrameRate) {
        WmeVideoRawFormat rawFormat;
        rawFormat.eRawType = WmeI420;
        rawFormat.iWidth = param.iMaxWidth;
        rawFormat.iHeight = param.iMaxHeight;
        rawFormat.fFrameRate = param.iMaxFrameRate;
        rawFormat.uTimestamp = 0;
        VideoSender::SetCaptureCapability(rawFormat);
    }
#endif

    return WME_S_OK;
}

long VideoSender::GetCapability(WmeMediaBaseCapability *pMC)
{
    returnv_if_fail(pMC != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    
    if (pMC->eMediaType != WmeMediaTypeVideo) {
        CM_ERROR_TRACE_THIS("VideoSender::GetCapability, invalid media type, should be WmeMediaTypeVideo!");
        return WME_E_FAIL;
    }
    
    WmeVideoEncodeConfiguraion param;
    memset(&param, 0, sizeof(param));
    m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &param, sizeof(param));
    
    WmeVideoMediaCapability * pVMC = (WmeVideoMediaCapability *)pMC;

    pVMC->width = param.iMaxWidth;
    pVMC->height = param.iMaxHeight;
    pVMC->frame_layer_number = 1;
    pVMC->frame_layer[pVMC->frame_layer_number-1] = param.iMaxFrameRate * 100;
    
    return WME_S_OK;
}

long VideoSender::CreateTrack()
{
    CM_INFO_TRACE_THIS("VideoSender::CreateTrack, begin!");
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_INFO_TRACE_THIS("VideoSender::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_E_FAIL;
	
	if(m_bUseVideoFile)
	{
		if (m_pMediaFileCap == NULL)
		{
			return ret;
		}


		ret = m_pEngine->CreateLocalVideoExternalTrack((IWmeLocalVideoExternalTrack **)&m_pTrack);
		if (ret != WME_S_OK || !m_pTrack) {
			CM_ERROR_TRACE_THIS("VideoSender::CreateTrack, fail to CreateLocalVideoTrack.");
			return WME_E_FAIL;
		}
		uint32_t label = kVideoLocalTrack_Label;
		m_pTrack->SetTrackLabel(label);


		// if(!m_pFileCapEngine)
		// {
		// 	m_pFileCapEngine = new CFileCaptureEngineImp();
		// }

		// if(!m_pFileCapEngine)
		// {
		// 	CM_ERROR_TRACE_THIS("VideoSender::CreateTrack, fail to create file capture engine.");
		// 	return WME_E_FAIL;
		// }

		IWmeExternalInputter *pInputter = NULL;
		((IWmeLocalVideoExternalTrack *)m_pTrack)->GetExternalInputter(&pInputter);
		// m_pFileCapEngine->SetExternalInputter(pInputter);
		// m_pFileCapEngine->Init();
		// m_pFileCapEngine->Start();

		m_pMediaFileCap->SetExternalInputter(WmeMediaTypeVideo, pInputter);
        pInputter->Release();
        
		//m_pMediaFileCap->Start();

	}
	else
	{
		ret = m_pEngine->CreateLocalVideoTrack((IWmeLocalVideoTrack **)&m_pTrack);
		if (ret != WME_S_OK || !m_pTrack) {
			CM_ERROR_TRACE_THIS("VideoSender::CreateTrack, fail to CreateLocalVideoTrack.");
			return WME_E_FAIL;
		}
		uint32_t label = kVideoLocalTrack_Label;
		m_pTrack->SetTrackLabel(label);

		if(m_pCamera)
		{
			ret = ((IWmeLocalVideoTrack *)m_pTrack)->SetCaptureDevice(m_pCamera);
		}
		if(m_rawFormat.iWidth > 0)
		{
			ret = ((IWmeLocalVideoTrack *)m_pTrack)->SetCaptureFormat(&m_rawFormat);
		}
	}

	m_pTrack->AddObserver(WMEIID_IWmeMediaCaptureObserver, this);
	m_pTrack->AddObserver(WMEIID_IWmeLocalVideoTrackObserver, this);


    return WME_S_OK;
}

long VideoSender::SetTrackCodec(IWmeMediaCodec *pCodec)
{
	long ret = WME_S_OK;
	if ( m_pTrack )
	{
		ret = m_pTrack->SetCodec(pCodec);
		if ( WME_S_OK != ret )
		{
			CM_ERROR_TRACE_THIS("VideoSender::SetTrackCodec, failed.");
			return WME_E_FAIL;
		}
	}	

	return WME_S_OK;
}

long VideoSender::SetDefaultSettings()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;

    /// default settings for video encoder capability
    WmeVideoMediaCapability vmc;  ///???: cannot memset of this structure
    vmc.width = 320;
    vmc.height = 180;
    vmc.frame_layer_number = 1;
    vmc.frame_layer[vmc.frame_layer_number-1] = 30 * 100;
    ret = VideoSender::SetCapability((WmeMediaBaseCapability *)&vmc);

    /// default settings for video capture device
    WmeVideoRawFormat rawFormat;
    rawFormat.eRawType = WmeI420;
    rawFormat.iWidth = 320;
    rawFormat.iHeight = 180;
    rawFormat.fFrameRate = 30;
    rawFormat.uTimestamp = 0;
    ret = VideoSender::SetCaptureCapability(rawFormat);

    /// default settings for uplink bandwidth
    WmeUpLinkNetStat uplink;
    memset(&uplink, 0, sizeof(uplink));
    uplink.NetStat.delay = 0;
    uplink.NetStat.jitter = 0;
    uplink.NetStat.loss_ratio = 0;
    uplink.NetStat.recv_rate = 0;
    uplink.nCongestStatus = 1;
#ifdef WIN32
	uplink.dwEvaluateBandwidth = 3000000; /// bandwidth for 720p
#else
    uplink.dwEvaluateBandwidth = 640000; /// bandwidth for 360p
#endif
    ret = m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, &uplink, sizeof(uplink));

    WmeVideoEncodeLayerType single = WmeVideoSingleLayer;
    ret = m_pTrack->SetOption(WmeTrackOption_VideoEncodeLayerType, &single, sizeof(WmeVideoEncodeLayerType));

    WmePerformanceProfileType perf = WmePerformanceProfileNormal;	/// default 360p
    ret = m_pTrack->SetOption(WmeTrackOption_StaticPerformanceProfile, &perf, sizeof(perf));

    return ret;
}

long VideoSender::DeleteTrack()
{
	StopTrack();
    SetTransport(NULL);

    if( m_pTrack )
    {
	m_pTrack->RemoveObserver(WMEIID_IWmeMediaCaptureObserver, this);
	m_pTrack->RemoveObserver(WMEIID_IWmeLocalVideoTrackObserver, this);
    }
    return MediaClient::DeleteTrack();
}

long VideoSender::SetTransport(IWmeMediaTransport *sink)
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

long VideoSender::SetDevice(IWmeMediaDevice* pCamera)
{
    SAFE_RELEASE(m_pCamera);
    m_pCamera = pCamera;
    if(m_pCamera) {
        m_pCamera->AddRef();
    }else {
        return WME_S_OK;
    }

    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;

	if(!m_bUseVideoFile)
	{
		IWmeLocalVideoTrack *pTrack = (IWmeLocalVideoTrack *)m_pTrack;
		if (m_bStarted) {
			ret = pTrack->Stop();
			ret = pTrack->SetCaptureDevice(m_pCamera);

			if(m_rawFormat.eRawType != WmeVideoUnknown)
				ret = pTrack->SetCaptureFormat(&m_rawFormat);
			ret = pTrack->Start();
		}else {
			ret = pTrack->SetCaptureDevice(m_pCamera);

			if(m_rawFormat.eRawType != WmeVideoUnknown)
				ret = pTrack->SetCaptureFormat(&m_rawFormat);
		}
	}
    
    return ret;
}

long VideoSender::GetDevice(IWmeMediaDevice** ppCamera)
{
	if(!ppCamera)
		return WME_E_FAIL;

	returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

	long ret = WME_E_FAIL;

	if(!m_bUseVideoFile)
	{
		IWmeLocalVideoTrack *pTrack = (IWmeLocalVideoTrack *)m_pTrack;
		ret = pTrack->GetCaptureDevice(ppCamera);
	}

	return ret;
}

long VideoSender::StartTrack()
{
	long ret = WME_S_OK;
	ret = MediaClient::StartTrack();
	if (ret != WME_S_OK)
	{
		CM_ERROR_TRACE_THIS("VideoSender::StartTrack, call MediaClient::StartTrack fail");
		return ret;
	}

	if (m_bUseVideoFile)
	{
		if (m_pMediaFileCap)
		{
			ret = m_pMediaFileCap->Start();
		}
	}
	return ret;
}

long VideoSender::StopTrack()
{
	if (m_bUseVideoFile)
	{
		if (m_pMediaFileCap)
		{
			m_pMediaFileCap->Stop();
		}
	}
	return MediaClient::StopTrack();
}

long VideoSender::SetCaptureCapability(const WmeVideoRawFormat & format)
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    m_rawFormat = format;
    long ret = WME_S_OK;

	if(!m_bUseVideoFile){
		if (!m_bStarted) {
			ret = ((IWmeLocalVideoTrack *)m_pTrack)->SetCaptureFormat(&m_rawFormat);
		}
	}
    return ret;
}

long VideoSender::SetVideoFile(const char* pFileName, WmeVideoRawFormat *pFormat)
{
	CM_INFO_TRACE_THIS("VideoSender::SetVideoFile, file:"<<pFileName);
	// if(!m_pFileCapEngine)
	// {
	// 	m_pFileCapEngine = new CFileCaptureEngineImp();
	// }
	WMERESULT res = WME_S_OK;

	if (m_pMediaFileCap)
	{
		m_pMediaFileCap->Release();
		m_pMediaFileCap = NULL;
	}

	res = m_pEngine->CreateMediaFileCapturer(&m_pMediaFileCap);
	CM_INFO_TRACE_THIS("VideoSender::SetVideoFile, CreateMediaFileCapturer");
	if (res != WME_S_OK || m_pMediaFileCap == NULL)
	{
		return WME_E_FAIL;
	}

	m_pMediaFileCap->AddObserver(WMEIID_IWmeMediaFileCapturerObserver, this);

	//hardcode to disable loop
	bool loop = false;
	m_pMediaFileCap->SetConfig(WmeMediaFileConfig_GoLoop, &loop, sizeof(bool));

	m_pMediaFileCap->SetCaptureFile((char*)pFileName, strlen(pFileName));

	m_pMediaFileCap->SetConfig(WmeMediaFileConfig_VideoRawFormat, pFormat, sizeof(WmeVideoRawFormat));

	return WME_S_OK;
}

void VideoSender::SetVideoSource(int nSource)
{
	m_bUseVideoFile = FALSE;
	if(nSource == 1)
		m_bUseVideoFile = TRUE;
}
// for IWmeMediaMessageTransmitterObserver
WMERESULT VideoSender::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

// for IWmeMediaMessageTransmitterObserver
WMERESULT VideoSender::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

// for IWmeMediaMessageTransmitterObserver
WMERESULT VideoSender::OnPictureLossIndication(IWmeMediaEventNotifier *pNotifier, uint32_t uSourceID, uint32_t uStreamId)
{
    returnv_if_fail(m_pTrack != NULL, WME_S_FALSE);
    return m_pTrack->SendEvent(wme::WmeTrackEvent_ForceKeyFrame, NULL, 0);
}

WMERESULT VideoSender::OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult)
{
	CM_INFO_TRACE_THIS("VideoSender::OnStartedCapture, eStartedResult="<<eStartedResult);
	return WME_S_OK;
}

WMERESULT VideoSender::OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight)
{
	CM_INFO_TRACE_THIS("VideoSender::OnUpdateCaptureResolution, uWidth="<<uWidth<<",uHeight="<<uHeight);
#ifdef ANDROID
	javaSetVideoCaptureResolution_STAT(uWidth, uHeight);
#endif	//ANDROID
	return WME_S_OK;
}

WMERESULT VideoSender::OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS)
{
	CM_INFO_TRACE_THIS("VideoSender::OnUpdateCaptureFPS, uFPS="<<uFPS);
#ifdef ANDROID
	javaSetVideoCaptureFPS_STAT(uFPS);
#endif //ANDROID
	return WME_S_OK;
}

WMERESULT VideoSender::OnRequiredFrameRateUpdate(float fRequiredFrameRate)
{
	CM_INFO_TRACE_THIS("VideoSender::OnRequiredFrameRateUpdate, fRequiredFrameRate="<<fRequiredFrameRate);
	return WME_S_OK;
}

WMERESULT VideoSender::OnEncodedResolutionUpdate(uint32_t uWidth, uint32_t uHeight)
{
	CM_INFO_TRACE_THIS("VideoSender::OnEncodedResolutionUpdate, uWidth="<<uWidth<<", uHeight="<<uHeight);
#ifdef ANDROID
	//javaSetVideoEncodeResolution_STAT(uWidth, uHeight);
#endif	//ANDROID
	return WME_S_OK;
}

WMERESULT VideoSender::OnEncodedFrameRateUpdate(uint32_t uFPS)
{
	CM_INFO_TRACE_THIS("VideoSender::OnEncodedFrameRateUpdate, uFPS="<<uFPS);
#ifdef ANDROID
	//javaSetVideoEncodeFPS_STAT(uFPS);
#endif	//ANDROID
	return WME_S_OK;
}

WMERESULT VideoSender::OnEndOfStream(IWmeMediaFileCapturer* pCapturer)
{
	CM_INFO_TRACE_THIS("VideoSender::OnEndOfStream, pCapturer="<<pCapturer);

	if (m_pMediaFileCap != pCapturer)
	{
		CM_INFO_TRACE_THIS("VideoSender::OnEndOfStream, ignore unknown capturer");
		return WME_S_OK;
	}

	StopTrack();

	if (m_pSink)
		m_pSink->OnEndOfStream(DEMO_MEDIA_VIDEO);
	return WME_S_OK;
}
