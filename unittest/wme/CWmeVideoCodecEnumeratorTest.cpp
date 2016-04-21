#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"

#if defined (WIN32)
#include <Windows.h>
#endif

using namespace wme;

class CWmeVideoCodecEnumeratorTest : public testing::Test
{
public:
	CWmeVideoCodecEnumeratorTest()
	{
		m_pMediaCodecEnumerator = NULL;


		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaCodecEnumerator(WmeMediaTypeVideo, &m_pMediaCodecEnumerator);
		}


	}
	virtual ~CWmeVideoCodecEnumeratorTest()
	{
		if (m_pMediaCodecEnumerator)
		{
			m_pMediaCodecEnumerator->Release();
			m_pMediaCodecEnumerator = NULL;
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
			m_pWmeEngine = NULL;
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeMediaCodecEnumerator* m_pMediaCodecEnumerator;
};

TEST_F(CWmeVideoCodecEnumeratorTest, InterfaceTestPlusQueryInterfacesTest)
{
	IWmeMediaCodecEnumerator* pMediaCodecEnumerator = NULL;
	int32_t uiCodecNum;
	EXPECT_EQ(WME_S_OK, m_pMediaCodecEnumerator->GetNumber(uiCodecNum));
	EXPECT_EQ(3, uiCodecNum);
	IWmeMediaCodec* pMediaCodec = NULL;
	char CodecName[128];
	int32_t NamLen = 128;
	EXPECT_EQ(WME_S_OK, m_pMediaCodecEnumerator->GetCodec(0,&pMediaCodec));
	EXPECT_EQ(WME_S_OK, m_pMediaCodecEnumerator->GetCodec(1,&pMediaCodec));
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetCodecName(CodecName, NamLen));
	EXPECT_EQ(WME_S_OK, m_pMediaCodecEnumerator->GetCodec(0,&pMediaCodec));
	EXPECT_NE(NULL, (long)pMediaCodec);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, pMediaCodec->QueryInterface(WMEIID_IWmeMediaCodec, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	NamLen = 128;
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetCodecName(CodecName, NamLen));
	EXPECT_EQ(NamLen, strlen("H264"));
	CodecName[NamLen] = '\0';
	EXPECT_STREQ(CodecName,"H264");
	NamLen = 0;
	EXPECT_NE(WME_S_OK, pMediaCodec->GetCodecName(CodecName, NamLen));
	WmeCodecType eCodecType;
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetCodecType(eCodecType));
	EXPECT_EQ((int)WmeCodecType_AVC, (int)eCodecType);
	WmeMediaType eMediaType;
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetMediaType(eMediaType));
	EXPECT_EQ((int)WmeMediaTypeVideo, (int)eMediaType);
	uint32_t uiPayloadType;
	uint32_t uiClockRate;
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetPayloadType(uiPayloadType));
	EXPECT_EQ(97,uiPayloadType);
	EXPECT_EQ(WME_S_OK, pMediaCodec->GetClockRate(uiClockRate));
	EXPECT_EQ(90000, uiClockRate);

}