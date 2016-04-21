
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SvcClientEngineMock.h"
#include "WseVideoSourceChannel.h"
#include "WseEngineImp.h"
#include "WseVideoSample.h"
#include "WseVideoEncoderFactory.h"

class CWseVideoSourceChannelTest : public testing::Test
{
public:
	CWseVideoSourceChannelTest()
	{
		pEngine = new CWseEngineImp();
		pSrcChannel = new CWseVideoSourceChannel(0,pEngine);
        EXPECT_CALL(TransportSink, SendData(_,_,_,_,_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pSink, OnNotifySourceBitrateInfo(Ge(0), Ge(0))).WillRepeatedly(Return(0));
        EXPECT_CALL(pSink, OnSourceParameterChanged(_,_,_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pSink, OnCaptureParameterChanged(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pSink, OnKeyFrameRequested(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pDataSink, OnOutputData(_)).WillRepeatedly(Return(0));
	}

	virtual ~CWseVideoSourceChannelTest()
	{
		delete pSrcChannel;
		delete pEngine;
	}

	virtual void SetUp()
	{
        
	}

	virtual void TearDown()
	{

	}

public:
	CWseVideoSourceChannel *pSrcChannel;
	CWseEngineImp *pEngine;
    MockIWseTransportSink TransportSink;
    MockIWseVideoSourceChannelNotifySink pSink;
    MockIWseDataSink pDataSink;
    MockIRTPChannel s_cRTPChannel;
};

TEST_F(CWseVideoSourceChannelTest, SetConfigParams)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));
	
	EXPECT_EQ(WSE_S_OK,pSrcChannel->SetRtpTransportSink(&TransportSink));

	WseSourceConfigParams parm,parm2;
	memset(&parm, 0, sizeof(WseSourceConfigParams));
	memset(&parm2,0, sizeof(WseSourceConfigParams));

	parm.output_num = 1;
	parm.in_format.video_type = WseYUY2;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetConfigParams(&parm));
	EXPECT_EQ(WSE_S_OK, pSrcChannel->GetConfigParams(&parm2));

	EXPECT_EQ(0, memcmp(&parm, &parm2, sizeof(WseSourceConfigParams)));
}

TEST_F(CWseVideoSourceChannelTest, SetEncodeParam)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	CWseEncodeParam param;
	memset(&param, 0, sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 176;
	param.encode_height[0] = 144;
	pSrcChannel->SetEncodeParam(&param);

	EXPECT_EQ(1, pSrcChannel->m_EncodeParam.mode_num);
	EXPECT_EQ(176, pSrcChannel->m_EncodeParam.encode_width[0]);
	EXPECT_EQ(144, pSrcChannel->m_EncodeParam.encode_height[0]);

	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetEncodeParam(&param));
}

TEST_F(CWseVideoSourceChannelTest, InputFrame)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));

	EXPECT_EQ(WSE_S_OK,pSrcChannel->SetRtpTransportSink(&TransportSink));

	WseSourceConfigParams parm;
	memset(&parm, 0, sizeof(WseSourceConfigParams));

	parm.output_num = 1;
	parm.in_format.video_type = WseYUY2;
	parm.in_format.width = 176;
	parm.in_format.height = 144;
	parm.output_settings[0].width = 176;
	parm.output_settings[0].height = 144;
	parm.output_settings[0].bitrate = 64000;
	parm.output_settings[0].type = WseAVCStream;
	parm.output_settings[0].frame_rate = 5;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetConfigParams(&parm));

    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
    IWseVideoSample *pSample = NULL;
    pSampleAllocator->GetSample(parm.in_format, &pSample);
    pSample->SetVideoFormat(&parm.in_format);

	unsigned char data[176*144*2] = {0};
	pSrcChannel->m_bStartSend = true;
	pSrcChannel->m_pEncoder->SetDataSink(pSrcChannel);

	pSrcChannel->SetSourceChannelSink(&pSink);
    
    //subscribe first before InputMediaFrame;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x001e;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 200000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    unsigned long dwValue=0;
    unsigned long ret = pSrcChannel->SetParams(WSE_SUBSCRIBE_MULTI_STREAM, dwValue, &sSubscribedStr, sizeof(sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, ret);
    
    EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(pSample));
	EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(pSample));

	//parm.output_settings[0].type = WseRTPStream;
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->SetConfigParams(&parm));
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(parm.in_format, data, 176*144*2));
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(parm.in_format, data, 176*144*2));

	WseVideoFormat format;
	memcpy(&format, &(parm.in_format), sizeof(WseVideoFormat));
	unsigned char* outframe;
	unsigned int outLen;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->EncodeFrame(format, data, 176*144*2, &outframe, outLen));
	format.width = 320;
	format.height = 240;
    format.video_type = WseYUY2;
	unsigned char data2[320*240*2] = {0};
	EXPECT_EQ(WSE_S_OK, pSrcChannel->EncodeFrame(format, data2, 320*240*2, &outframe, outLen));

	CWseEncoderOutput output;
	memset(&output, 0, sizeof(CWseEncoderOutput));
	pSrcChannel->m_bStartSend = false;
	pSrcChannel->GetCodecType();

    pSampleAllocator->Release();
}

TEST_F(CWseVideoSourceChannelTest, StartStopSend)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));
	
	CWseEncodeParam param;
	memset(&param, 0, sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 176;
	param.encode_height[0] = 144;
	pSrcChannel->SetEncodeParam(&param);

	pSrcChannel->StartSend();
	EXPECT_EQ(true, pSrcChannel->m_bStartSend);
	
	pSrcChannel->StopSend();
	EXPECT_EQ(false, pSrcChannel->m_bStartSend);
}

TEST_F(CWseVideoSourceChannelTest, ForceKeyFrame)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	EXPECT_EQ(WSE_S_OK,pSrcChannel->SetRtpTransportSink(&TransportSink));

	WseSourceConfigParams param;
	memset(&param, 0, sizeof(WseSourceConfigParams));
	param.output_num = 1;
	param.output_settings[0].width = 176;
	param.output_settings[0].height = 144;
	pSrcChannel->SetConfigParams(&param);
	EXPECT_EQ(WSE_S_OK, pSrcChannel->ForceKeyFrame(false));

	EXPECT_EQ(WSE_S_OK, pSrcChannel->HandleKeyFrameLost());
}

TEST_F(CWseVideoSourceChannelTest, UpdateUpLinkStat)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	TUpLinkNetStat stat;
	memset(&stat,0,sizeof(TUpLinkNetStat));
	stat.dwEvaluateBandwidth = 150000;
	stat.nCongestStatus = 257;
	int FECLevel=-1;
	pSrcChannel->UpdateUpLinkStat(stat, FECLevel);


	stat.dwEvaluateBandwidth = 200000;
	stat.nCongestStatus = 257;
	pSrcChannel->UpdateUpLinkStat(stat, FECLevel);


	stat.dwEvaluateBandwidth = 200000;
	stat.nCongestStatus = 253;
	pSrcChannel->UpdateUpLinkStat(stat, FECLevel);


	TDownLinkNetStat dStat;
	memset(&dStat, 0, sizeof(TDownLinkNetStat));
	EXPECT_EQ(0, pSrcChannel->UpdateDownLinkStat(dStat));
}

TEST_F(CWseVideoSourceChannelTest, SetGetSourceID)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	pSrcChannel->SetSourceID(123456);
	EXPECT_EQ(123456, pSrcChannel->GetSourceID());
}

TEST_F(CWseVideoSourceChannelTest, SetGetParams)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
	CWseEncodeParam param;
	memset(&param, 0, sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 176;
	param.encode_height[0] = 144;
	pSrcChannel->SetEncodeParam(&param);
	pSrcChannel->SetSourceChannelSink(&pSink);

	WseExtendedParam extParam;
	unsigned long dwValue;
	
	extParam = WSE_P2P_PARAM;
	dwValue = 0;
	long lret = pSrcChannel->SetParams(extParam, dwValue, NULL, 0);
	EXPECT_EQ(WSE_S_OK, lret);
	EXPECT_EQ(false, pSrcChannel->m_cRtpPacker.m_bP2PmodeFlag);

	extParam = WSE_VIDEO_MEETING_QUALITY;
	dwValue = Meeting_Video_Quality_HD_720P;
	lret = pSrcChannel->SetParams(extParam, dwValue, NULL, 0);
	EXPECT_EQ(WSE_S_OK, lret);
	
	extParam = WSE_PERIAD_INTRA_CODING;
	dwValue = 1;
	unsigned int idrInterval = 640; // fix 64bit error
	lret = pSrcChannel->SetParams(extParam, dwValue, &idrInterval, sizeof(idrInterval));
//	EXPECT_EQ(true, pSrcChannel->m_pEncodeController->m_CWseEncodeParam.enable_periodic_intra);//m_pParamGenerator not modified?
//	EXPECT_EQ(640, pSrcChannel->m_pEncodeController->m_CWseEncodeParam.idr_interval);
    EXPECT_EQ(WSE_S_OK, lret);
	
	extParam = WSE_SOURCE_INFO;
	WseVideoSourceInfoType infoType;
	infoType.nType = SourceType_Video_Sharing;
	infoType.nWidth = 320;
	infoType.nHeight = 240;
	infoType.nFrameRate = 15;
	lret = pSrcChannel->SetParams(extParam, dwValue, &infoType, 0);
	//EXPECT_EQ(SourceType_Video_Sharing, pSrcChannel->m_pEncodeController->m_eSourceType);
    EXPECT_EQ(WSE_S_OK, lret);
    EXPECT_EQ(infoType.nFrameRate,pSrcChannel->m_fInitCaptureFrameRate);
	infoType.nType = SourceType_Desktop_Sharing;
	infoType.nWidth = 1920;
	infoType.nHeight = 1080;
	infoType.nFrameRate = 10;
	lret = pSrcChannel->SetParams(extParam, dwValue, &infoType, 0);
	//EXPECT_EQ(SourceType_Desktop_Sharing, pSrcChannel->m_pEncodeController->m_eSourceType);
    EXPECT_EQ(WSE_S_OK, lret);
	EXPECT_EQ(infoType.nFrameRate,pSrcChannel->m_fInitCaptureFrameRate);
//	EXPECT_EQ(5000, pSrcChannel->m_pSendControl->m_iMaxDelayBase);

}

TEST_F(CWseVideoSourceChannelTest, Component)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

    CWseEncodeParam param;
    memset(&param, 0, sizeof(CWseEncodeParam));
    param.mode_num = 1;
    param.encode_width[0] = 176;
    param.encode_height[0] = 144;
    MockIWseEncoderSink pSink;
    IWseEncoder *enc = WseVideoEncoderFactory::instance()->CreateVideoEncoder(&param, WseSoftwareVideoCodec, &pSink);
	void *pEncoder;

	pSrcChannel->RegisterComponent(WSE_ENCODER, enc, NULL);
	pSrcChannel->GetComponent(WSE_ENCODER, &pEncoder);
	EXPECT_EQ(enc, pEncoder);

	MockIWseEncryptSink encrypt;
	unsigned long bEncryptFlag = 1;
	pSrcChannel->SetComponentStatus(WSE_CRYPTION, bEncryptFlag);
	EXPECT_EQ(true, pSrcChannel->m_bEncryptEnabled);
	pSrcChannel->RegisterComponent(WSE_CRYPTION, &encrypt, NULL);
	EXPECT_EQ(&encrypt, pSrcChannel->m_pEncryptSink);
	EXPECT_EQ(true, pSrcChannel->m_bEncryptEnabled);

	pSrcChannel->UnRegisterComponent(WSE_ENCODER, enc);
}

TEST_F(CWseVideoSourceChannelTest, UpdateFilterInfo)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	long srcID;
	CWseFilterInfo info;
	memset(&info, 0, sizeof(CWseFilterInfo));

	srcID = ServerFilterID;
	info.timestamp = 111111;
	info.BaseLostFlag = 0;
	info.inputBytes = 1024;
	info.outputBytes = 1024;
	info.estimatedBannwidth = 2000000/8;
	info.dataLossRate = 0;
//    pSrcChannel->m_pEncodeController->m_bSingleLayerFlag = true;
    unsigned long dwValue = 1;
    pSrcChannel->SetParams(WSE_P2P_PARAM, dwValue, NULL, 0);
    EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateFilterInfo(srcID, &info));
//	EXPECT_EQ(2000000, pSrcChannel->m_pEncodeController->m_nDownlinkBandwidth);
//	EXPECT_EQ(111111, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].timestamp);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].inputBytes);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].outputBytes);
	info.timestamp = 222222;
	info.BaseLostFlag = 1;
	info.inputBytes = 256;
	info.outputBytes = 256;
	info.estimatedBannwidth = 1000000/8;
	info.dataLossRate = 10;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateFilterInfo(srcID, &info));
//	EXPECT_EQ(1000000, pSrcChannel->m_pEncodeController->m_nDownlinkBandwidth);
//	EXPECT_EQ(111111, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].timestamp);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].inputBytes);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[1].outputBytes);
//	EXPECT_EQ(0, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[2].timestamp);
//	EXPECT_EQ(0, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[2].inputBytes);
//	EXPECT_EQ(0, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cDownlinkFilterStatistcs.m_cFilterInfo[2].outputBytes);

	srcID = UpLinkFilterID;
	info.timestamp = 123456;
	info.BaseLostFlag = 0;
	info.inputBytes = 1024;
	info.outputBytes = 1024;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateFilterInfo(srcID, &info));
//	EXPECT_EQ(123456, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[1].timestamp);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[1].inputBytes);
//	EXPECT_EQ(1024, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[1].outputBytes);
	info.timestamp = 200000;
	info.BaseLostFlag = 0;
	info.inputBytes = 512;
	info.outputBytes = 512;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateFilterInfo(srcID, &info));
//	EXPECT_EQ(200000, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[2].timestamp);
//	EXPECT_EQ(512, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[2].inputBytes);
//	EXPECT_EQ(512, pSrcChannel->m_pEncodeController->m_pCWsePassRateCalc->m_cUplinkFilterStatistcs.m_cFilterInfo[2].outputBytes);

//	srcID = 2;
//	EXPECT_EQ(WSE_S_FALSE, pSrcChannel->UpdateFilterInfo(srcID, &info));
}

TEST_F(CWseVideoSourceChannelTest, OnEncodeParamChanged)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));

	SEncParamExt param;
	memset(&param, 0, sizeof(SEncParamExt));
	param.iSpatialLayerNum = 1;
	param.sSpatialLayers[0].fFrameRate = 10;

	pSrcChannel->OnEncodeParamChanged(param.sSpatialLayers[0].iVideoWidth, param.sSpatialLayers[0].iVideoHeight, param.sSpatialLayers[0].fFrameRate);
	EXPECT_EQ(10, pSrcChannel->m_dwEncodeMaxFps);
}

TEST_F(CWseVideoSourceChannelTest, OnInputFrameRateChanged)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));

	float fps = 10;

	pSrcChannel->OnInputFrameRateChanged(fps);
	EXPECT_EQ(10, pSrcChannel->m_dwFps);
}

TEST_F(CWseVideoSourceChannelTest, Fec)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));

	EXPECT_EQ(WSE_S_OK,pSrcChannel->SetRtpTransportSink(&TransportSink));

	pSrcChannel->SetSourceChannelSink(&pSink);
}


TEST_F(CWseVideoSourceChannelTest, SetPerformanceStatus)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	WsePerformanceDetectionAlert status;
	//status = Warning_alert_Init;
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	status = Warning_alert_Green_Jmp;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	//status = Warning_alert_Green;
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	//status = Warning_alert_Yellow;
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	status = Warning_alert_Red;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	status = Warning_alert_Black;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));
}

TEST_F(CWseVideoSourceChannelTest, OnTimer)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
	pSrcChannel->OnTimer(NULL);

	pSrcChannel->SetSourceChannelSink(&pSink);

	CWseTimer timer;
	for (int i=0; i<10; i++)
	{
#ifdef WIN32
		Sleep(i*100);
#else
		usleep(i*100000);
#endif
		pSrcChannel->OnTimer(&timer);
	}

	CWseEncodeParam param;
	memset(&param, 0, sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 176;
	param.encode_height[0] = 144;
	pSrcChannel->SetEncodeParam(&param);
	pSrcChannel->StartSend();
	EXPECT_EQ(true, pSrcChannel->m_bStartSend);

	pSrcChannel->OnTimer(&timer);
}

TEST_F(CWseVideoSourceChannelTest, NotifyRate)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
}

TEST_F(CWseVideoSourceChannelTest, SetRTPChannel)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	MockIRTPChannel cRTPChannel;
    EXPECT_CALL(cRTPChannel, IncreaseReference()).WillOnce(Return(0));
    EXPECT_CALL(cRTPChannel, DecreaseReference()).WillOnce(Return(0));
    EXPECT_CALL(cRTPChannel, SetPacketizationOperator(_)).WillRepeatedly(Return());
    EXPECT_CALL(s_cRTPChannel, IncreaseReference()).WillRepeatedly(Return(0));
    EXPECT_CALL(s_cRTPChannel, DecreaseReference()).WillRepeatedly(Return(0));
    EXPECT_CALL(s_cRTPChannel, SetPacketizationOperator(_)).WillRepeatedly(Return());
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetRTPChannel((wrtp::IRTPChannel*)&cRTPChannel));
//	EXPECT_EQ(WSE_E_INVALIDARG, pSrcChannel->SetRTPChannel(NULL));
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetRTPChannel(NULL));
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetRTPChannel((wrtp::IRTPChannel*)&s_cRTPChannel));
}

TEST_F(CWseVideoSourceChannelTest, SendMediaData)
{
	EXPECT_EQ(WSE_S_OK,pSrcChannel->Initialize(WSE_REALTIME_APP));

	EXPECT_CALL(s_cRTPChannel, IncreaseReference()).WillRepeatedly(Return(0));
    EXPECT_CALL(s_cRTPChannel, DecreaseReference()).WillRepeatedly(Return(0));
    EXPECT_CALL(s_cRTPChannel, SetPacketizationOperator(_)).WillRepeatedly(Return());
    EXPECT_CALL(s_cRTPChannel, SendMediaData(NotNull())).WillRepeatedly(Return(0));
    EXPECT_EQ(WSE_S_OK, pSrcChannel->SetRTPChannel((wrtp::IRTPChannel*)&s_cRTPChannel));

	WseSourceConfigParams parm;
	memset(&parm, 0, sizeof(WseSourceConfigParams));

	parm.output_num = 1;
	parm.in_format.video_type = WseYUY2;
	parm.in_format.width = 176;
	parm.in_format.height = 144;
	parm.output_settings[0].width = 176;
	parm.output_settings[0].height = 144;
	parm.output_settings[0].bitrate = 64000;
	parm.output_settings[0].type = WseSVCStream;
	parm.output_settings[0].frame_rate = 5;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetConfigParams(&parm));

    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
    IWseVideoSample *pSample = NULL;
    pSampleAllocator->GetSample(parm.in_format, &pSample);
    pSample->SetVideoFormat(&parm.in_format);
    
	unsigned char data[176*144*2] = {0};
	pSrcChannel->m_bStartSend = true;

	pSrcChannel->SetSourceChannelSink(&pSink);
    
    
    //subscribe first before InputMediaFrame;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x001e;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 200000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    unsigned long dwValue=0;
    unsigned long ret = pSrcChannel->SetParams(WSE_SUBSCRIBE_MULTI_STREAM, dwValue, &sSubscribedStr, sizeof(sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, ret);

	EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(pSample));
	EXPECT_EQ(WSE_S_OK, pSrcChannel->InputMediaFrame(pSample));

	WseVideoFormat format;
	memcpy(&format, &(parm.in_format), sizeof(WseVideoFormat));
	unsigned char* outframe;
	unsigned int outLen;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->EncodeFrame(format, data, 176*144*2, &outframe, outLen));
	format.width = 320;
	format.height = 240;
	unsigned char data2[320*240*2] = {0};
	EXPECT_EQ(WSE_S_OK, pSrcChannel->EncodeFrame(format, data2, 320*240*2, &outframe, outLen));
    
    pSampleAllocator->Release();
}

TEST_F(CWseVideoSourceChannelTest, UpdateSendingFilterInfo)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	//for encoder instance
	CWseEncodeParam param;
	memset(&param, 0, sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 176;
	param.encode_height[0] = 144;
	pSrcChannel->SetEncodeParam(&param);

	WseFilterType eFilterType = WSE_PACKET_DROP;
	unsigned long uTotalLen = 512;

	wrtp::FilterVideoDataInfo local_info;
	local_info.frame_idc = FRAMEIDC_IDR;	//Test value
	local_info.timestamp = 123456789;		//Random value
	local_info.DID = 0;						//Test value
	local_info.priority = 0;				//Test value

	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateSendingFilterInfo(eFilterType, uTotalLen, (unsigned char*)&local_info, sizeof(local_info)));

	EXPECT_NE(WSE_S_OK, pSrcChannel->UpdateSendingFilterInfo(eFilterType, uTotalLen, (unsigned char*)&local_info, 0/*bad value*/));
	EXPECT_NE(WSE_S_OK, pSrcChannel->UpdateSendingFilterInfo(eFilterType, uTotalLen, NULL, sizeof(local_info)));

	eFilterType = WSE_PACKET_NOUSE;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateSendingFilterInfo(eFilterType, uTotalLen, (unsigned char*)&local_info, sizeof(local_info)));

	WseVideoSourceInfoType sourceInfo;
	sourceInfo.nType = SourceType_Desktop_Sharing;
	sourceInfo.nWidth = 1440;
	sourceInfo.nHeight = 900;
	sourceInfo.nFrameRate = 10;
	pSrcChannel->SetSourceInfo(&sourceInfo);

	eFilterType = WSE_PACKET_DROP;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->UpdateSendingFilterInfo(eFilterType, uTotalLen, (unsigned char*)&local_info, sizeof(local_info)));
}

TEST_F(CWseVideoSourceChannelTest, InputCompressedData)
{
    EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
    
    IWseEncodedFrameInputProtocal  * pCompressedFrameInputProtocal = nullptr;
    
    EXPECT_EQ(WSE_S_OK,pSrcChannel->QueryInterface(WSEIID_IWseEncodedFrameInputProtocal, (void**)&pCompressedFrameInputProtocal));
    
    EXPECT_NE(nullptr, pCompressedFrameInputProtocal);
    
    //pCompressedFrameInputProtocal->Release();
    //pCompressedFrameInputProtocal = nullptr;
}

TEST_F(CWseVideoSourceChannelTest, CaptureCameraCapability)
{
	EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

	WseCameraCapability capability[5];
	capability[0].type = WseYUY2;
	capability[0].width = 160;
	capability[0].height = 90;
	capability[0].MinFPS = 5;
	capability[0].MaxFPS = 30;
	capability[1].type = WseYUY2;
	capability[1].width = 640;
	capability[1].height = 360;
	capability[1].MinFPS = 5;
	capability[1].MaxFPS = 30;
	capability[2].type = WseRGB24;
	capability[2].width = 176;
	capability[2].height = 144;
	capability[2].MinFPS = 5;
	capability[2].MaxFPS = 15;
	capability[3].type = WseRGB24;
	capability[3].width = 640;
	capability[3].height = 480;
	capability[3].MinFPS = 5;
	capability[3].MaxFPS = 15;
	capability[4].type = WseUnknown;
	capability[4].width = 1280;
	capability[4].height = 720;
	capability[4].MinFPS = 5;
	capability[4].MaxFPS = 5;

	EXPECT_EQ(WSE_S_OK, pSrcChannel->SetCaptureCameraCapability(5,capability));

  unsigned long dwValue = Meeting_Video_Quality_SD;
  pSrcChannel->SetParams(WSE_VIDEO_MEETING_QUALITY, dwValue, NULL, 0);

	WseCameraCapability cap;
	EXPECT_EQ(WSE_S_OK, pSrcChannel->GetCaptureCapability(5,capability, &cap));
	EXPECT_EQ(640, cap.width);
	EXPECT_EQ(360, cap.height);
	//TODO: input and output parameters match
	//EXPECT_EQ(WSE_S_OK, pEncodeController->GetCaptureCapability(5,capability, Camera_Performance_180P_ONLY, &cap));
	//EXPECT_EQ(160, cap.width);
	//EXPECT_EQ(90, cap.height);
}

TEST_F(CWseVideoSourceChannelTest, SubscribeMultiStream)
{
    EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
    CWseEncodeParam param;
    memset(&param, 0, sizeof(CWseEncodeParam));
    param.mode_num = 1;
    param.encode_width[0] = 176;
    param.encode_height[0] = 144;
    pSrcChannel->SetEncodeParam(&param);
    pSrcChannel->SetSourceChannelSink(&pSink);
    
    WseExtendedParam extParam;
    unsigned long dwValue;
    
    extParam = WSE_SUBSCRIBE_MULTI_STREAM;
    dwValue = 0;
    long ret = pSrcChannel->SetParams(extParam, dwValue, NULL, 0);
    EXPECT_EQ(WSE_E_INVALIDARG, ret);
    
    ret = pSrcChannel->SetParams(extParam, dwValue, NULL, 1);
    EXPECT_EQ(WSE_E_INVALIDARG, ret);
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 4;
    
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x001e;
    sSubscribedStr.stStream[3].uProfileLevelID =  0x42E000 + 0x0020;
    
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxFPS = 24;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxFPS = 22;
    
    sSubscribedStr.stStream[0].uMaxBitRate = 200000;
    sSubscribedStr.stStream[1].uMaxBitRate = 400000;
    sSubscribedStr.stStream[2].uMaxBitRate = 700000;
    sSubscribedStr.stStream[3].uMaxBitRate = 1700000;

    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 1;
    sSubscribedStr.stStream[1].uVids[1] = 2;
    
    sSubscribedStr.stStream[2].uNumVids = 3;
    sSubscribedStr.stStream[2].uVids[0] = 3;
    sSubscribedStr.stStream[2].uVids[1] = 4;
    sSubscribedStr.stStream[2].uVids[2] = 5;
    
    sSubscribedStr.stStream[3].uNumVids = 4;
    sSubscribedStr.stStream[3].uVids[0] = 6;
    sSubscribedStr.stStream[3].uVids[1] = 7;
    sSubscribedStr.stStream[3].uVids[2] = 8;
    sSubscribedStr.stStream[3].uVids[3] = 9;
    
    ret = pSrcChannel->SetParams(extParam, dwValue, &sSubscribedStr, sizeof(sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, ret);
    
    WseVideoSubscribeMultiStream sGottenMulStr;
    pSrcChannel->GetParams(extParam, dwValue, &sGottenMulStr, sizeof(sGottenMulStr));
    
    EXPECT_EQ (sGottenMulStr.uNumStreams, 4);
    
    EXPECT_EQ (sGottenMulStr.stStream[0].uProfileLevelID,  0x42E000 + 0x000a);
    EXPECT_EQ (sGottenMulStr.stStream[1].uProfileLevelID,  0x42E000 + 0x000c);
    EXPECT_EQ (sGottenMulStr.stStream[2].uProfileLevelID,  0x42E000 + 0x001e);
    EXPECT_EQ (sGottenMulStr.stStream[3].uProfileLevelID,  0x42E000 + 0x0020);
    
    EXPECT_GE (sGottenMulStr.stStream[0].uMaxFPS, 22);
    EXPECT_GE (sGottenMulStr.stStream[1].uMaxFPS, 22);
    EXPECT_GE (sGottenMulStr.stStream[2].uMaxFPS, 22);
    EXPECT_GE (sGottenMulStr.stStream[3].uMaxFPS, 22);
    
    EXPECT_EQ (sSubscribedStr.stStream[0].uNumVids , 1);
    EXPECT_EQ (sSubscribedStr.stStream[0].uVids[0] , 0);
    
    EXPECT_EQ (sSubscribedStr.stStream[1].uNumVids , 2);
    EXPECT_EQ (sSubscribedStr.stStream[1].uVids[0] , 1);
    EXPECT_EQ (sSubscribedStr.stStream[1].uVids[1] , 2);
    
    EXPECT_EQ (sSubscribedStr.stStream[2].uNumVids , 3);
    EXPECT_EQ (sSubscribedStr.stStream[2].uVids[0] , 3);
    EXPECT_EQ (sSubscribedStr.stStream[2].uVids[1] , 4);
    EXPECT_EQ (sSubscribedStr.stStream[2].uVids[2] , 5);
    
    EXPECT_EQ (sSubscribedStr.stStream[3].uNumVids , 4);
    EXPECT_EQ (sSubscribedStr.stStream[3].uVids[0] , 6);
    EXPECT_EQ (sSubscribedStr.stStream[3].uVids[1] , 7);
    EXPECT_EQ (sSubscribedStr.stStream[3].uVids[2] , 8);
    EXPECT_EQ (sSubscribedStr.stStream[3].uVids[3] , 9);
  
}

TEST_F(CWseVideoSourceChannelTest, UpdateLinksStatus)
{
    EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));

}
