#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeDevice.h"
#include "IWbxAeBase.h"
#include "WmeAudioDeviceEnumerator.h"

#if defined (WIN32)

#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;

class CWmeAudioDeviceTest : public testing::Test
{
public:
	CWmeAudioDeviceTest()
	{
		m_pEnuCap = NULL;
		m_pEnuPla=NULL;
		m_pAudioDeCon=NULL;
//		m_pAudioDefault=NULL;
		WmeCreateMediaEngine(&m_pWmeEngine);
// 		CreateAudioEngineInst(m_pDolphinEng);
// 		if(m_pDolphinEng)
// 		{
// 		    m_pDolphinEng->Init();		
// 			m_pDolphinEng->GetDeviceEnumeratorInst(m_pDevice);
// 			m_pAudioDefault=new CWmeAudioSysDefaultDevice(m_pDevice);
// 		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeAudio,WmeDeviceIn,&m_pEnuCap);//  CreateLocalAudioTrack(&m_pTrack);
			m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeAudio,WmeDeviceOut,&m_pEnuPla);	
			m_pWmeEngine->CreateAudioDeviceController(&m_pAudioDeCon);			
		}
	}
	virtual ~CWmeAudioDeviceTest()
	{
		if (m_pEnuCap)
		{
			m_pEnuCap->Release();
			m_pEnuCap = NULL;
		}
		if (m_pEnuPla)
		{
			m_pEnuPla->Release();
			m_pEnuPla = NULL;
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
	IWmeMediaDeviceEnumerator* m_pEnuCap;
	IWmeMediaDeviceEnumerator* m_pEnuPla;	
	IWmeAudioDeviceController* m_pAudioDeCon;
	///////
// 	IWmeSysDefaultDevice*      m_pAudioDefault;
// 	IWbxAudioEngine*           m_pDolphinEng;
// 	IWBXDeviceEnumerator    *m_pDevice;
};
//////////////////default device
// TEST_F(CWmeAudioDeviceTest, QueryInterfaceDefalltDevice)
// {
// 	EXPECT_NE(NULL, (long)m_pAudioDefault);
// 	void* pInterface = NULL;
// 	EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeSysDefaultDevice, &pInterface));
// 	EXPECT_NE(NULL, (long)pInterface);
// 
// }
// TEST_F(CWmeAudioDeviceTest, GetDefaultDeice)
// {
// 	EXPECT_NE(NULL, (long)m_pAudioDefault);
// 	EXPECT_NE(NULL, (long)m_pEnuCap);
// 	EXPECT_NE(NULL, (long)m_pEnuPla);
// 	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
// 	IWmeMediaDevice *ppDevice=NULL;
// 	int32_t iNumber=0;
// 	WMERESULT res=WME_S_FALSE;
//     res=m_pAudioDefault->GetDefaultDeice(NULL,WmeDeviceIn);
// 	EXPECT_EQ(WME_E_INVALIDARG,res);
// 	res=m_pEnuCap->GetDeviceNumber(iNumber);
// 	EXPECT_EQ(WME_S_OK,res);
// 	if (iNumber>0)
// 	{
// 		res=m_pAudioDefault->GetDefaultDeice(&ppDevice,WmeDeviceIn);
// 		EXPECT_EQ(WME_S_OK,res);
// 		EXPECT_NE(NULL,(int)ppDevice);
// 		(ppDevice)->Release();
// 		ppDevice=NULL;
// 
// 	}
// 	
// 	res=m_pEnuPla->GetDeviceNumber(iNumber);
// 	EXPECT_EQ(WME_S_OK,res);
// 	if (iNumber>0)
// 	{
// 		res=m_pAudioDefault->GetDefaultDeice(&ppDevice,WmeDeviceIn);
// 		EXPECT_EQ(WME_S_OK,res);
// 		EXPECT_NE(NULL,(int)ppDevice);
// 		(ppDevice)->Release();
// 		ppDevice=NULL;
// 
// 	}
// 
// }

#if defined(WIN32) || defined(MACOS)

TEST_F(CWmeAudioDeviceTest, EngineGetDefaultDeice)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pEnuCap);
	EXPECT_NE(NULL, (long)m_pEnuPla);
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	IWmeMediaDevice *ppDevice=NULL;
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	res=m_pWmeEngine->GetSystemDefaultDevice(WmeMediaTypeAudio, 
		WmeDeviceIn, 
		NULL);
	
	EXPECT_EQ(WME_E_INVALIDARG,res);
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pWmeEngine->GetSystemDefaultDevice(WmeMediaTypeAudio, 
			WmeDeviceIn, 
			&ppDevice);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL,(long)ppDevice);
		(ppDevice)->Release();
		ppDevice=NULL;

	}

	res=m_pEnuPla->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pWmeEngine->GetSystemDefaultDevice(WmeMediaTypeAudio, 
			WmeDeviceOut, 
			&ppDevice);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL,(long)ppDevice);
		(ppDevice)->Release();
		ppDevice=NULL;

	}

}

///////////////
TEST_F(CWmeAudioDeviceTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pEnuCap);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	
}
TEST_F(CWmeAudioDeviceTest, QueryInterface2)
{
	EXPECT_NE(NULL, (long)m_pAudioDeCon);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pAudioDeCon->QueryInterface(WMEIID_IWmeAudioDeviceController, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	
}

TEST_F(CWmeAudioDeviceTest, GetDeviceNumber)
{
	EXPECT_NE(NULL, (long)m_pEnuCap);
	EXPECT_NE(NULL, (long)m_pEnuPla);
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	
	res=m_pEnuPla->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	

}
TEST_F(CWmeAudioDeviceTest, GetDevice)
{
	EXPECT_NE(NULL, (long)m_pEnuCap);
	EXPECT_NE(NULL, (long)m_pEnuPla);
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	IWmeMediaDevice* dev=NULL;	
	//EXPECT_EQ(WME_S_OK, m_pEnuCap->QueryInterface(WMEIID_IWmeMediaDeviceEnumerator, &pInterface));
	res=m_pEnuCap->GetDevice(1,NULL);
    EXPECT_EQ(WME_E_INVALIDARG,res);
	res=m_pEnuCap->GetDevice(100,&dev);
	EXPECT_EQ(WME_E_INVALIDARG,res);


   //get device
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pEnuCap->GetDevice(0,&dev);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev);// ());
		dev->Release();
		dev=NULL;
	}
	res=m_pEnuPla->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pEnuPla->GetDevice(0,&dev);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev);// ());
		dev->Release();
		dev=NULL;
	}	
	
}

TEST_F(CWmeAudioDeviceTest, GetCaptureDevice )
{
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;
	IWmeMediaDevice* dev=NULL;	
	EXPECT_NE(NULL, (long)m_pAudioDeCon);	
	//get device
	EXPECT_NE(NULL, (long)m_pEnuCap);
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioDeCon->GetCaptureDevice(&dev);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev);// ());

		dev->Release();
		dev=NULL;
	}

	EXPECT_NE(NULL, (long)m_pEnuPla);
	res=m_pEnuPla->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioDeCon->GetPlayDevice(&dev);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev);// ());
		dev->Release();
		dev=NULL;
	}	
	
	res=m_pAudioDeCon->GetCaptureDevice(NULL);
	EXPECT_EQ(WME_E_INVALIDARG,res);

	res=m_pAudioDeCon->GetPlayDevice(NULL);
	EXPECT_EQ(WME_E_INVALIDARG,res);
}

TEST_F(CWmeAudioDeviceTest, SetCaptureDevice )
{
	int32_t iNumber=0;
	WMERESULT res=WME_S_FALSE;

	IWmeMediaDevice* dev2=NULL;
	EXPECT_NE(NULL, (long)m_pAudioDeCon);
	//set null
	res=m_pAudioDeCon->SetCaptureDevice(NULL);
	EXPECT_EQ(WME_E_INVALIDARG,res);
	res=m_pAudioDeCon->SetPlayDevice(NULL);
	EXPECT_EQ(WME_E_INVALIDARG,res);

	EXPECT_NE(NULL, (long)m_pEnuCap);
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pEnuCap->GetDevice(0,&dev2);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev2);// ());

		res=m_pAudioDeCon->SetCaptureDevice(dev2);
		EXPECT_EQ(WME_S_OK,res);
		dev2->Release();
		dev2=NULL;

	}


	EXPECT_NE(NULL, (long)m_pEnuPla);
	res=m_pEnuPla->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pEnuPla->GetDevice(0,&dev2);
		EXPECT_EQ(WME_S_OK,res);
		EXPECT_NE(NULL, (long)dev2);// ());

		res=m_pAudioDeCon->SetPlayDevice(dev2);
		EXPECT_EQ(WME_S_OK,res);
		dev2->Release();
		dev2=NULL;
	}	

}

TEST_F(CWmeAudioDeviceTest, GetFormate )
{
	WmeAudioRawFormat format;
	WMERESULT res=WME_S_FALSE;

	EXPECT_NE(NULL, (long)m_pAudioDeCon);

	res=m_pAudioDeCon->GetCaptureFormat(format);
	EXPECT_EQ(WME_E_NOTIMPL,res);
	res=m_pAudioDeCon->GetPlayFormat(format);
	EXPECT_EQ(WME_E_NOTIMPL,res);
}
TEST_F(CWmeAudioDeviceTest, SetFormate )
{
	WmeAudioRawFormat format;
	WMERESULT res=WME_S_FALSE;

	EXPECT_NE(NULL, (long)m_pAudioDeCon);

	res=m_pAudioDeCon->SetCaptureFormat(&format);
	EXPECT_EQ(WME_E_NOTIMPL,res);
	res=m_pAudioDeCon->SetPlayFormat(&format);
	EXPECT_EQ(WME_E_NOTIMPL,res);
}

 TEST_F(CWmeAudioDeviceTest, QueryInterfaceDevice)
 {
 	EXPECT_NE(NULL, (long)m_pEnuCap);

	WMERESULT res=WME_S_FALSE;
	void* pInterface = NULL;
    IWmeMediaDevice* dev=NULL;

	int32_t iNumber=0;
	EXPECT_NE(NULL, (long)m_pEnuCap);
	res=m_pEnuCap->GetDeviceNumber(iNumber);
	EXPECT_EQ(WME_S_OK,res);
	if (iNumber>0)
	{
		res=m_pAudioDeCon->GetCaptureDevice(&dev);
		EXPECT_EQ(WME_S_OK, res);
		EXPECT_NE(NULL, (long)dev);
		EXPECT_EQ(WME_S_OK, dev->QueryInterface(WMEIID_IWmeMediaDevice, &pInterface));
		EXPECT_NE(NULL, (long)pInterface);
		dev->Release();
		dev=NULL;
	}	
}

 TEST_F(CWmeAudioDeviceTest,  GetMediaType)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 WmeMediaType eMediaType;
	 int32_t iNumber=0;
	  EXPECT_NE(NULL, (long)m_pAudioDeCon);

	  EXPECT_NE(NULL, (long)m_pEnuCap);
	  res=m_pEnuCap->GetDeviceNumber(iNumber);
	  EXPECT_EQ(WME_S_OK,res);
	  if (iNumber>0)
	  {
		  res=m_pAudioDeCon->GetCaptureDevice(&dev);
		  EXPECT_EQ(WME_S_OK, res);
		  EXPECT_NE(NULL, (long)dev);
		  res=dev->GetMediaType(eMediaType);
		  EXPECT_EQ(WME_S_OK, res);
		  EXPECT_EQ(WmeMediaTypeAudio, eMediaType);
		  dev->Release();
		  dev=NULL;
	  }
	
 }
 TEST_F(CWmeAudioDeviceTest,  GetInOutType)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 WmeDeviceInOutType eMediaType;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 EXPECT_NE(NULL, (long)m_pAudioDeCon);
		 res=m_pAudioDeCon->GetCaptureDevice(&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 res=dev->GetInOutType(eMediaType);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_EQ(WmeDeviceIn, eMediaType);
		 dev->Release();
		 dev=NULL;
	 }
 }
 TEST_F(CWmeAudioDeviceTest,  GetUniqueID)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 void* SID=NULL;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 EXPECT_NE(NULL, (long)m_pAudioDeCon);
		 res=m_pAudioDeCon->GetCaptureDevice(&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		

		 dev->Release();
		 dev=NULL;
	 }
 }
 TEST_F(CWmeAudioDeviceTest,  GetUniqueString)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 char* SID=new char[1024];
	 
	  int32_t iLen=0;
	  int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 res=m_pEnuCap->GetDevice(0,&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 res=dev->GetUniqueName(SID,iLen);
	
		 EXPECT_EQ(WME_E_INVALIDARG, res);
         iLen=1024;
		 res=dev->GetUniqueName(SID,iLen);
		 EXPECT_NE(0, iLen);
		 res=dev->GetUniqueName(NULL,iLen);
		 EXPECT_EQ(WME_E_INVALIDARG, res);
		 dev->Release();
		 dev=NULL;
	 }
	 delete SID;
 }
 TEST_F(CWmeAudioDeviceTest,  GetFriendlyString)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 char* SID[1024]={};
	 int32_t iLen=0;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 res=m_pEnuCap->GetDevice(0,&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 res=dev->GetFriendlyName((char*)SID,iLen);
		 EXPECT_EQ(WME_E_INVALIDARG, res);
         iLen=1024;
		 res=dev->GetFriendlyName((char*)SID,iLen);

		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(0, iLen);
		 res=dev->GetFriendlyName(NULL,iLen);
		 EXPECT_EQ(WME_E_INVALIDARG, res);
		 dev->Release();
		 dev=NULL;
	 }
 }

 TEST_F(CWmeAudioDeviceTest,  GetCapabilities)
 {
	

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 WmeDeviceCapability temp;
	 int32_t iLen=0;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 res=m_pEnuCap->GetDevice(0,&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 res=dev->GetCapabilities(iLen,&temp);
		 EXPECT_EQ(WME_E_NOTIMPL, res);
		
		 dev->Release();
		 dev=NULL;
	 }
 }
 TEST_F(CWmeAudioDeviceTest,  IsSameDevice)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 IWmeMediaDevice* dev2=NULL;
	 void* SID=NULL;
	 int32_t iLen=0;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 res=m_pEnuCap->GetDevice(0,&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 
		 res=dev->IsSameDevice(dev);
		 EXPECT_EQ(WME_S_OK, res);
		 res=dev->IsSameDevice(NULL);
		 EXPECT_EQ(WME_E_INVALIDARG, res);


		 EXPECT_NE(NULL, (long)m_pEnuPla);
		 res=m_pEnuPla->GetDeviceNumber(iNumber);
		 EXPECT_EQ(WME_S_OK,res);
		 if (iNumber>0)
		 {
			  res=m_pEnuPla->GetDevice(0,&dev2);
			  EXPECT_EQ(WME_S_OK, res);
			  EXPECT_NE(NULL, (long)dev2);
			  res=dev2->IsSameDevice(dev);
			  EXPECT_EQ(WME_S_FALSE, res);
			  dev2->Release();
			  dev2=NULL;			  
		 }
		 dev->Release();
		 dev=NULL;
	 }
 }
 TEST_F(CWmeAudioDeviceTest,  IsSameDevice2)
 {
	 EXPECT_NE(NULL, (long)m_pEnuCap);

	 WMERESULT res=WME_S_FALSE;
	 void* pInterface = NULL;
	 IWmeMediaDevice* dev=NULL;
	 IWmeMediaDevice* dev2=NULL;
	 char* SID=new char[1024];
	 memset(SID,0,1024);
	 int32_t iLen=1024;
	 int32_t iNumber=0;

	 EXPECT_NE(NULL, (long)m_pEnuCap);
	 res=m_pEnuCap->GetDeviceNumber(iNumber);
	 EXPECT_EQ(WME_S_OK,res);
	 if (iNumber>0)
	 {
		 res=m_pEnuCap->GetDevice(0,&dev);
		 EXPECT_EQ(WME_S_OK, res);
		 EXPECT_NE(NULL, (long)dev);
		 res=dev->GetUniqueName(SID,iLen);
		 EXPECT_EQ(WME_S_OK, res);
		 res=dev->IsSameDevice(SID,iLen);
		 EXPECT_EQ(WME_S_OK, res);
		 res=dev->IsSameDevice(NULL,iLen);
		 EXPECT_EQ(WME_E_INVALIDARG, res);


		 EXPECT_NE(NULL, (long)m_pEnuPla);
		 res=m_pEnuPla->GetDeviceNumber(iNumber);
		 EXPECT_EQ(WME_S_OK,res);
		 if (iNumber>0)
		 {
			 res=m_pEnuPla->GetDevice(0,&dev2);
			 EXPECT_EQ(WME_S_OK, res);
			 EXPECT_NE(NULL, (long)dev2);
			 res=dev2->IsSameDevice(SID,iLen);
			 EXPECT_EQ(WME_S_FALSE, res);
			 dev2->Release();
			 dev2=NULL;			  
		 }
		 dev->Release();
		 dev=NULL;
	 }
 }


TEST_F(CWmeAudioDeviceTest,  GetDeviceInfo)
{
    EXPECT_NE(NULL, (long)m_pEnuCap);
    EXPECT_NE(NULL, (long)m_pEnuPla);

    WMERESULT res=WME_S_FALSE;
    void* pInterface = NULL;
    IWmeMediaDevice* dev=NULL;
    IWmeMediaDevice* dev2=NULL;
    void* SID=NULL;
    int32_t iLen=0;
    int32_t iNumber=0;
    
    EXPECT_NE(NULL, (long)m_pEnuCap);
    res=m_pEnuCap->GetDeviceNumber(iNumber);
    EXPECT_EQ(WME_S_OK,res);
    if (iNumber>0)
    {
        res=m_pEnuCap->GetDevice(0,&dev);
        EXPECT_EQ(WME_S_OK, res);
        EXPECT_NE(NULL, (long)dev);
        char frinedlyName[256] = {0};
        int32_t len = 256;
        dev->GetFriendlyName(frinedlyName, len);
        EXPECT_NE(0, strlen(frinedlyName));
        
        len = 256;
        char interfaceID[256] = {0};
        dev->GetInterfaceID(interfaceID, len);
        
        len = 256;
        char shortName[256] = {0};
        dev->GetShortName(shortName, len);
#ifdef WIN32
        EXPECT_NE(0, strlen(shortName));
#endif
        len = 256;
        char uniqueName[256] = {0};
        dev->GetUniqueName(uniqueName, len);
        EXPECT_NE(0, strlen(uniqueName));
        
        
        dev->GetDeviceID(len);
        dev->Release();
        dev=NULL;
    }
    
    EXPECT_NE(NULL, (long)m_pEnuPla);
    res=m_pEnuPla->GetDeviceNumber(iNumber);
    EXPECT_EQ(WME_S_OK,res);
    if (iNumber>0)
    {
        res=m_pEnuPla->GetDevice(0,&dev);
        EXPECT_EQ(WME_S_OK, res);
        EXPECT_NE(NULL, (long)dev);
        char frinedlyName[256] = {0};
        int32_t len = 256;
        dev->GetFriendlyName(frinedlyName, len);
        EXPECT_NE(0, strlen(frinedlyName));
        
        len = 256;
        char interfaceID[256] = {0};
        dev->GetInterfaceID(interfaceID, len);
        
        len = 256;
        char shortName[256] = {0};
        dev->GetShortName(shortName, len);
#ifdef WIN32
        EXPECT_NE(0, strlen(shortName));
#endif
        len = 256;
        char uniqueName[256] = {0};
        dev->GetUniqueName(uniqueName, len);
        EXPECT_NE(0, strlen(uniqueName));
        
        
        dev->GetDeviceID(len);
        dev->Release();
        dev=NULL;
    }
}

#endif
