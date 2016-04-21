#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"

class CWmeInterfaceTest : public testing::Test,
						  public wme::IWmeTraceSink
{
public:
	CWmeInterfaceTest()
	{

	}

	virtual ~CWmeInterfaceTest()
	{

	}

	virtual void SetUp() {}
	virtual void TearDown() {}

	virtual WMERESULT OnTrace(uint32_t trace_level, const char* szInfo) { return WME_E_NOTIMPL; }
};

TEST_F(CWmeInterfaceTest, WmeGetVersion)
{
	wme::WmeVersion version = {0};
	EXPECT_EQ(WME_S_OK, wme::WmeGetVersion(version));
}

TEST_F(CWmeInterfaceTest, WmeInit)
{
	// The first init operator will success
	EXPECT_EQ(WME_S_OK, wme::WmeInit(0));
	
	// The second init operator will fail or success?
	EXPECT_EQ(WME_S_OK, wme::WmeInit(0));

	// Clear WME environment variables for sencond init
	EXPECT_EQ(WME_S_OK, wme::WmeUninit());

	// Clear WME environment variables for first init
	EXPECT_EQ(WME_S_OK, wme::WmeUninit());
}
/*
TEST_F(CWmeInterfaceTest, WmeUninit)
{
	// The uninit operator will fail or success before init operator ?
	EXPECT_EQ(WME_S_OK, wme::WmeUninit());

	// Init WME environment variables
	EXPECT_EQ(WME_S_OK, wme::WmeInit(0));
	
	// The uninit operator will success after init operator
	EXPECT_EQ(WME_S_OK, wme::WmeUninit());
}
*/
TEST_F(CWmeInterfaceTest, WmeCreateMediaEngine)
{
	// Init WME environment variables
	EXPECT_EQ(WME_S_OK, wme::WmeInit(0));

	// Test WmeCreateMediaEngine
	wme::IWmeMediaEngine* pMediaEngine = NULL;
	EXPECT_EQ(WME_S_OK, wme::WmeCreateMediaEngine(&pMediaEngine));
	EXPECT_EQ(true, pMediaEngine != NULL);
	if(NULL != pMediaEngine)
	{
		pMediaEngine->Release();
		pMediaEngine = NULL;
	}

	// Clear WME environment variables
	EXPECT_EQ(WME_S_OK, wme::WmeUninit());
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_TraceSink)
{
	// Test WmeOption_TraceSink
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceSink, this, 0));
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceSink, NULL, 0));
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_TraceMaxLevel)
{
	// Test WmeOption_TraceMaxLevel
	wme::WmeTraceLevel eLevel =  wme::WME_TRACE_LEVEL_NOTRACE;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));
	eLevel = wme::WME_TRACE_LEVEL_ERROR;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));
	eLevel = wme::WME_TRACE_LEVEL_WARNING;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));
	eLevel = wme::WME_TRACE_LEVEL_INFO;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));
	eLevel = wme::WME_TRACE_LEVEL_DEBUG;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));
	eLevel = wme::WME_TRACE_LEVEL_ALL;
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, sizeof(eLevel)));

	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, NULL, sizeof(eLevel)));
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_TraceMaxLevel, &eLevel, 0));
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_WorkPath)
{
	// Test WmeOption_WorkPath
	// This interface can't support in Windows platform
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_WorkPath, NULL, 0));
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_DataDumpFlag)
{
	// Test WmeOption_DataDumpFlag
	bool bDataDumpFlag = true;
#if defined(ENABLE_DATADUMP)
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpFlag, &bDataDumpFlag, sizeof(bDataDumpFlag)));
#endif
	bDataDumpFlag = false;
#if defined(ENABLE_DATADUMP)
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpFlag, &bDataDumpFlag, sizeof(bDataDumpFlag)));
#endif

	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpFlag, NULL, sizeof(bDataDumpFlag)));
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpFlag, &bDataDumpFlag, 0));
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_DataDumpPath)
{
	// Test WmeOption_DataDumpPath
	char sDataDumpPath[] = "C:\\dump";
	uint32_t uDataDumpPathLen = strlen(sDataDumpPath);
#if defined(ENABLE_DATADUMP)
	EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpPath, (unsigned char*)sDataDumpPath, uDataDumpPathLen));
#endif
	
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpPath, NULL, uDataDumpPathLen));
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_DataDumpPath, (unsigned char*)sDataDumpPath, 0));
}

TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_QoSMaxLossRatio)
{
	// Test WmeOption_QoSMaxLossRatio
    float fMaxLossRatio = 0.08;
    EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMaxLossRatio, &fMaxLossRatio, sizeof(fMaxLossRatio)));
	EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMaxLossRatio, NULL, sizeof(fMaxLossRatio)));
    EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMaxLossRatio, &fMaxLossRatio, sizeof(uint16_t)));
    EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMaxLossRatio, &fMaxLossRatio, sizeof(double)));
}
              
TEST_F(CWmeInterfaceTest, WmeSetMediaEngineOption_QoSMinBandwidth)
{
    // Test WmeOption_QoSMinBandwidth
    uint32_t uMinBandwidth = 16*1024;
    EXPECT_EQ(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMinBandwidth, &uMinBandwidth, sizeof(uMinBandwidth)));
    EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMinBandwidth, NULL, sizeof(uMinBandwidth)));
    EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMinBandwidth, &uMinBandwidth, sizeof(uint16_t)));
    EXPECT_NE(WME_S_OK, wme::WmeSetMediaEngineOption(wme::WmeOption_QoSMinBandwidth, &uMinBandwidth, sizeof(double)));
}
