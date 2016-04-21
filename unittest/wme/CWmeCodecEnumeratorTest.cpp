#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeDevice.h"
#include "IWbxAeBase.h"
#include "WmeCodec.h"
#include "WmeAudioCodecEnumerator.h"

#if defined (WIN32)

#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;

class CWmeCodecEnumeratorTest : public testing::Test
{
public:
	CWmeCodecEnumeratorTest()
	{
		WmeCreateMediaEngine(&m_pWmeEngine);
		
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaCodecEnumerator(WmeMediaTypeAudio,&m_pAudioCodecEnu);
			
		}
	}
	virtual ~CWmeCodecEnumeratorTest()
	{
		if (m_pAudioCodecEnu)
		{
			m_pAudioCodecEnu->Release();
			m_pAudioCodecEnu = NULL;
		}
		
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeMediaCodecEnumerator* m_pAudioCodecEnu;
	
};
///AudioCodec interface
TEST_F(CWmeCodecEnumeratorTest, AudioCodecQueryInterface)
{
	EXPECT_NE(NULL, (long)m_pAudioCodecEnu);

	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	IWmeMediaCodec* codec=NULL;	
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	//get codec
	res=m_pAudioCodecEnu->GetNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioCodecEnu->GetCodec(0,&codec);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)codec);// ());
		void* pInterface = NULL;
		EXPECT_EQ(WME_S_OK, codec->QueryInterface(WMEIID_IWmeMediaCodec, &pInterface));
		EXPECT_NE(NULL, (long)pInterface);
		codec->Release();
		codec=NULL;
	}	

}
TEST_F(CWmeCodecEnumeratorTest, AudioCodecInterface)
{
	EXPECT_NE(NULL, (long)m_pAudioCodecEnu);

	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	IWmeMediaCodec* codec=NULL;	
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	//get codec
	res=m_pAudioCodecEnu->GetNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioCodecEnu->GetCodec(0,&codec);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)codec);// ());
		///////////////////////////////
		WmeMediaType mediatype;
		res=codec->GetMediaType(mediatype);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_EQ(WmeMediaTypeAudio,mediatype);
		//////////////
		char codecname[1024];
		int32_t namelen=0;
		res=codec->GetCodecName(NULL,namelen);
		EXPECT_EQ(WME_E_INVALIDARG,res);
		res=codec->GetCodecName(codecname,namelen);
		EXPECT_EQ(WME_E_INVALIDARG,res);
		namelen=1024;
		res=codec->GetCodecName(codecname,namelen);
		EXPECT_EQ(WME_S_OK,res);
		//////////////////
		WmeCodecType codectype;
        res=codec->GetCodecType(codectype);
		EXPECT_EQ(WME_S_OK,res);
		////////////////
		uint32_t tempint=0;
		res=codec->GetClockRate(tempint);
		EXPECT_EQ(WME_S_OK,res);
		//////////////////////////////////
		res=codec->GetPayloadType(tempint);
		EXPECT_EQ(WME_S_OK,res);

		uint32_t uCodeBandWidthAfter,uCodecBandWidthOri = 40000;
		res = codec->SetCodecBandwidth(uCodecBandWidthOri);
		EXPECT_EQ(WME_S_OK, res);
		res = codec->GetCodecBandwidth(uCodeBandWidthAfter);
		EXPECT_EQ(WME_S_OK, res);
		EXPECT_EQ(uCodecBandWidthOri, uCodeBandWidthAfter);
		codec->Release();
		codec=NULL;
	}	

}

///AudioCodecEnumerator interface
TEST_F(CWmeCodecEnumeratorTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pAudioCodecEnu);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pAudioCodecEnu->QueryInterface(WMEIID_IWmeMediaCodecEnumerator, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);

}

TEST_F(CWmeCodecEnumeratorTest, GetAudioCodecNumber)
{
	EXPECT_NE(NULL, (long)m_pAudioCodecEnu);
	
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	res=m_pAudioCodecEnu->GetNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	EXPECT_NE(0,iNumber);
}
TEST_F(CWmeCodecEnumeratorTest, GetAudioCodec)
{
	EXPECT_NE(NULL, (long)m_pAudioCodecEnu);
	
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	IWmeMediaCodec* codec=NULL;	
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	res=m_pAudioCodecEnu->GetCodec(1,NULL);
	EXPECT_EQ(WME_E_INVALIDARG,res);
	res=m_pAudioCodecEnu->GetCodec(1000,&codec);
	EXPECT_EQ(WME_E_INVALIDARG,res);

	//get codec
	res=m_pAudioCodecEnu->GetNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioCodecEnu->GetCodec(0,&codec);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)codec);// ());
		codec->Release();
		codec=NULL;
	}	
}

