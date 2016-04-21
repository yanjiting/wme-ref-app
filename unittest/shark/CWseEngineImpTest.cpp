
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseEngineImp.h"
#include "WseVideoSample.h"
#include "SvcClientEngineMock.h"
#include "codec_app_def.h"

namespace shark {
	IWseVideoSampleAllocator *g_pWseVideoSampleAllocator = new CWseVideoSampleAllocator(64);
}
class CWseEngineImpTest : public testing::Test
{
public:
	CWseEngineImpTest()
	{
		pEngine = new CWseEngineImp();
	}

	virtual ~CWseEngineImpTest()
	{
		delete pEngine;
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

public:
	CWseEngineImp *pEngine;
};

TEST_F(CWseEngineImpTest, Initialize)
{
	EXPECT_EQ(WSE_S_OK,pEngine->Initialize());

	int major, minor;
	pEngine->GetVersion(major, minor);
	EXPECT_EQ(3, major);
	EXPECT_EQ(0, minor);

	MockIWseEngineStatusSink psink;
	pEngine->SetEngineStatusSink(&psink);
	EXPECT_EQ(&psink, pEngine->m_pStatusSink);

	MockIWseTransportSink pRtcpSink;
	pEngine->SetRtcpTransportSink(&pRtcpSink);
	EXPECT_EQ(&pRtcpSink, pEngine->m_pRTCPTransportSink);
}

TEST_F(CWseEngineImpTest, ListenChannel)
{
	IWseVideoListenChannel* pListenChannel;
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoListenChannel(0
														,Wse_LCTM_Sync
														,WseI420
														,videoCodecH264AVC
														,WSE_REALTIME_APP
														,g_pWseVideoSampleAllocator
														,&pListenChannel));
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoListenChannel(0
		,Wse_LCTM_Sync
		,WseI420
		,videoCodecH264AVC
		,WSE_REALTIME_APP
		,g_pWseVideoSampleAllocator
		,&pListenChannel));
	EXPECT_EQ(WSE_S_OK, pEngine->DestroyVideoListenChannel(0));
	EXPECT_EQ(WSE_S_OK, pEngine->DestroyVideoListenChannel(1));
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoListenChannel(1
		,Wse_LCTM_Sync
		,WseI420
		,videoCodecH264AVC
		,WSE_REALTIME_APP
		,g_pWseVideoSampleAllocator
		,&pListenChannel));
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoListenChannel(2
		,Wse_LCTM_Sync
		,WseI420
		,videoCodecH264AVC
		,WSE_REALTIME_APP
		,g_pWseVideoSampleAllocator
		,&pListenChannel));
}

TEST_F(CWseEngineImpTest, SourceChannel)
{
	IWseVideoSourceChannel* pSrcChannel;
	MockIWseTransportSink pSink;
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoSourceChannel(0
														,WSE_REALTIME_APP
                                                        ,SourceType_Meeting_Camera
														,&pSink
														,&pSrcChannel));

	WseSourceConfigParams param;
	memset(&param, 0, sizeof(WseSourceConfigParams));
	param.output_num = 1;
	param.output_settings[0].width = 176;
	param.output_settings[0].height = 144;
	pSrcChannel->SetConfigParams(&param);
	
	EXPECT_EQ(WSE_S_OK, pEngine->KeyFrameLostIndication(0));


	SLTRRecoverRequest request;
	memset(&request, 0, sizeof(SLTRRecoverRequest));
	EXPECT_EQ(WSE_S_OK, pEngine->RequestIndication(0, LTR_RECOVER, &request));

	SLTRMarkingFeedback feedback;
	memset(&feedback, 0, sizeof(SLTRMarkingFeedback));
	EXPECT_EQ(WSE_S_OK, pEngine->FeedbackIndication(0,LTR_MARKING, &feedback));


	WsePerformanceProfileType profile;
	profile = Performance_Profile_Normal;
	EXPECT_EQ(WSE_S_OK, pEngine->SetStaticPerformanceProfile(profile));
//	EXPECT_EQ(Performance_Profile_Normal, pSrcChannel->m_pEncodeController->m_nStaticPerformanceProfile);

	profile = Performance_Profile_Low;
	EXPECT_EQ(WSE_S_OK, pEngine->SetStaticPerformanceProfile(profile));
//	EXPECT_EQ(Performance_Profile_Low, pSrcChannel->m_pEncodeController->m_nStaticPerformanceProfile);

	
	WsePerformanceDetectionAlert status;
	//status = Warning_alert_Init;
	//EXPECT_EQ(WSE_S_OK, pSrcChannel->SetPerformanceStatus(status));

	status = Warning_alert_Green_Jmp;
	EXPECT_EQ(WSE_S_OK, pEngine->SetPerformanceStatus(status));

	//status = Warning_alert_Green;
	//EXPECT_EQ(WSE_S_OK, pEngine->SetPerformanceStatus(status));

	//status = Warning_alert_Yellow;
	//EXPECT_EQ(WSE_S_OK, pEngine->SetPerformanceStatus(status));

	status = Warning_alert_Red;
	EXPECT_EQ(WSE_S_OK, pEngine->SetPerformanceStatus(status));

	status = Warning_alert_Black;
	EXPECT_EQ(WSE_S_OK, pEngine->SetPerformanceStatus(status));


	TNetStat stat;
	memset(&stat,0,sizeof(TNetStat));
	EXPECT_EQ(WSE_S_OK, pEngine->UpdateNetStat(0,stat));
	EXPECT_EQ(WSE_NET_STATUS_UNLOADED, pEngine->m_nNetworkStatus);
	
	stat.loss_ratio = 150;
	EXPECT_EQ(WSE_S_OK, pEngine->UpdateNetStat(0,stat));
	EXPECT_EQ(WSE_NET_STATUS_LOADED, pEngine->m_nNetworkStatus);

	stat.loss_ratio = 201;
	EXPECT_EQ(WSE_S_OK, pEngine->UpdateNetStat(0,stat));
	EXPECT_EQ(WSE_NET_STATUS_CONGESTED, pEngine->m_nNetworkStatus);

	memset(&stat,0,sizeof(TNetStat));
	stat.delay = 200;
	EXPECT_EQ(WSE_S_OK, pEngine->UpdateNetStat(0,stat));
	EXPECT_EQ(WSE_NET_STATUS_LOADED, pEngine->m_nNetworkStatus);

	stat.delay = 501;
	EXPECT_EQ(WSE_S_OK, pEngine->UpdateNetStat(0,stat));
	EXPECT_EQ(WSE_NET_STATUS_CONGESTED, pEngine->m_nNetworkStatus);	


	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoSourceChannel(0
		,WSE_REALTIME_APP
        ,SourceType_Meeting_Camera
		,&pSink
		,&pSrcChannel));
	EXPECT_EQ(WSE_S_OK, pEngine->DestroyVideoSourceChannel(0));
	EXPECT_EQ(WSE_E_FAIL, pEngine->DestroyVideoSourceChannel(1));

	EXPECT_EQ(WSE_E_FAIL, pEngine->KeyFrameLostIndication(0));
	EXPECT_EQ(WSE_E_FAIL, pEngine->RequestIndication(0, LTR_RECOVER,&request));
	EXPECT_EQ(WSE_E_FAIL, pEngine->FeedbackIndication(0,LTR_MARKING,&feedback));
//	EXPECT_EQ(WSE_E_FAIL, pEngine->SetStaticPerformanceProfile(profile));
//	EXPECT_EQ(WSE_E_FAIL, pEngine->SetPerformanceStatus(status));
	EXPECT_EQ(WSE_S_OK, pEngine->CreateVideoSourceChannel(1
		,WSE_REALTIME_APP
        ,SourceType_Meeting_Camera
		,&pSink
		,&pSrcChannel));
}
/*
TEST_F(CWseEngineImpTest, Rtcpcontroller)
{
#ifdef ENABLE_RTCP_CONTROLLER
	unsigned char *pData = NULL;
	int iLen = 0;
	EXPECT_EQ(WSE_E_INVALIDARG, pEngine->InputRTCPPacket(pData, iLen, 0));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnKeyFrameLost(0));

	LTR_Recover_Request request;
	memset(&request, 0, sizeof(LTR_Recover_Request));
	LTR_Marking_Feedback feedback;
	memset(&feedback, 0, sizeof(LTR_Marking_Feedback));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnLTRrecoveryRequest(0, &request));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnLTRMarking(0, &feedback));

	pData = new unsigned char[256];
	memset(pData, 0, 256);
	iLen = 128;
	pData[0] = 2;
	pData[1] = RTCP_PT_SR;
	pData[2] = 0;
	pData[3] = 40;
	EXPECT_EQ(WSE_S_OK, pEngine->InputRTCPPacket(pData, iLen, 0));


	EXPECT_EQ(WSE_E_POINTER, pEngine->SendRtcpPacket(pData, iLen, 0));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnKeyFrameLost(0));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnLTRrecoveryRequest(0, &request));
	EXPECT_EQ(WSE_E_POINTER, pEngine->OnLTRMarking(0, &feedback));

	MockIWseTransportSink pRtcpSink;
	pEngine->SetRtcpTransportSink(&pRtcpSink);
	EXPECT_EQ(&pRtcpSink, pEngine->m_pRTCPTransportSink);
	EXPECT_EQ(WSE_S_OK, pEngine->SendRtcpPacket(pData, iLen, 0));
	EXPECT_EQ(WSE_S_OK, pEngine->OnKeyFrameLost(0));
	EXPECT_EQ(WSE_S_OK, pEngine->OnLTRrecoveryRequest(0, &request));
	EXPECT_EQ(WSE_S_OK, pEngine->OnLTRMarking(0, &feedback));
#endif
}
*/
TEST_F(CWseEngineImpTest, SendPacket)
{
#ifdef ENABLE_RTCP_CONTROLLER
	pEngine->EnableRTCP(false);
	EXPECT_EQ(false, pEngine->IsRTCPEnabled());

	EXPECT_EQ(WSE_S_OK, pEngine->SendSdesPacket(0,0));
	EXPECT_EQ(WSE_S_OK, pEngine->SendByePacket(0));
	
	pEngine->EnableRTCP(true);
	EXPECT_EQ(true, pEngine->IsRTCPEnabled());

	EXPECT_EQ(WSE_E_POINTER, pEngine->SendByePacket(0));

	MockIWseTransportSink pRtcpSink;
	pEngine->SetRtcpTransportSink(&pRtcpSink);
	EXPECT_EQ(WSE_S_OK, pEngine->SendSdesPacket(0,0));
	EXPECT_EQ(WSE_S_OK, pEngine->SendSdesPacket(5000,0));
	EXPECT_EQ(WSE_S_OK, pEngine->SendByePacket(0));
#endif
}