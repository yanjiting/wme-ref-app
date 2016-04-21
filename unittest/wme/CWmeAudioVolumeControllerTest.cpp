#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeDevice.h"
#include "IWbxAeBase.h"
#include "WmeAudioVolumeController.h"

#if defined (WIN32)

#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;
class CTestEventVolumeObserver:public IWmeAudioVolumeObserver
{
public:
	CTestEventVolumeObserver();
	~CTestEventVolumeObserver();
public:
	virtual void OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &changeEvent);
	virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
	virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);

};
CTestEventVolumeObserver::CTestEventVolumeObserver()
{
}
CTestEventVolumeObserver::~CTestEventVolumeObserver()
{
}
void CTestEventVolumeObserver::OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &changeEvent)
{
}
WMERESULT CTestEventVolumeObserver::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
	return 0;
}
WMERESULT CTestEventVolumeObserver::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
	return 0;
}


class CWmeAudioVolumeControllerTest : public testing::Test
{
public:
	CWmeAudioVolumeControllerTest()
	{
		m_pWmeEngine = NULL;
		m_pMicVolumeController = NULL;
		m_pSpeakerVolumeController = NULL;
		m_pVolumeEventObserver = NULL;
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateAudioVolumeController(&m_pMicVolumeController,WmeDeviceIn);
			m_pWmeEngine->CreateAudioVolumeController(&m_pSpeakerVolumeController,WmeDeviceOut);			
		}
		m_pVolumeEventObserver = new CTestEventVolumeObserver();
	}
	virtual ~CWmeAudioVolumeControllerTest()
	{
		if (m_pMicVolumeController)
		{
			m_pMicVolumeController->Release();
			m_pMicVolumeController = NULL;
		}
		if (m_pSpeakerVolumeController)
		{
			m_pSpeakerVolumeController->Release();
			m_pSpeakerVolumeController = NULL;
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
		}
		if(NULL != m_pVolumeEventObserver)
		{
			delete m_pVolumeEventObserver;
			m_pVolumeEventObserver = NULL;
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeAudioVolumeController* m_pMicVolumeController;
	IWmeAudioVolumeController* m_pSpeakerVolumeController;
	CTestEventVolumeObserver*	m_pVolumeEventObserver;
};

#if defined (WIN32)

TEST_F(CWmeAudioVolumeControllerTest, GetDevice)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);
	IWmeMediaDevice *pdeviceMic,*pdeviceSpeaker;
	m_pMicVolumeController->GetDevice(&pdeviceMic);
	EXPECT_NE(NULL, (long)pdeviceMic);
	m_pSpeakerVolumeController->GetDevice(&pdeviceSpeaker);
	EXPECT_NE(NULL, (long)pdeviceSpeaker);
	pdeviceMic->Release();
	pdeviceSpeaker->Release();
}

///////////////
TEST_F(CWmeAudioVolumeControllerTest, SetVolume)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);
	for(unsigned int i=0;i<65535;i+=10000)
	{
		EXPECT_EQ(0,m_pMicVolumeController->SetVolume(i));
		EXPECT_EQ(0,m_pSpeakerVolumeController->SetVolume(i));
		EXPECT_EQ(0,m_pMicVolumeController->SetVolume(i,WME_VOL_SYSTEM));
		EXPECT_EQ(0,m_pSpeakerVolumeController->SetVolume(i,WME_VOL_SYSTEM));
		EXPECT_EQ(0, m_pMicVolumeController->SetVolume(i, WME_VOL_DIGITAL));
		EXPECT_EQ(0, m_pSpeakerVolumeController->SetVolume(i, WME_VOL_DIGITAL));
	}	
}
///////////////
TEST_F(CWmeAudioVolumeControllerTest, GetVolume)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);
	EXPECT_LE(0,m_pMicVolumeController->GetVolume());		
}

TEST_F(CWmeAudioVolumeControllerTest, Mute)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);
	
	EXPECT_EQ(0,m_pMicVolumeController->Mute());
	EXPECT_EQ(0,m_pSpeakerVolumeController->Mute());
	EXPECT_EQ(0,m_pMicVolumeController->Mute(WME_VOL_SYSTEM));
	EXPECT_EQ(0,m_pSpeakerVolumeController->Mute(WME_VOL_SYSTEM));
}

TEST_F(CWmeAudioVolumeControllerTest, UnMute)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);

	EXPECT_EQ(0,m_pMicVolumeController->UnMute());
	EXPECT_EQ(0,m_pSpeakerVolumeController->UnMute());
	EXPECT_EQ(0,m_pMicVolumeController->UnMute(WME_VOL_SYSTEM));
	EXPECT_EQ(0,m_pSpeakerVolumeController->UnMute(WME_VOL_SYSTEM));
}
TEST_F(CWmeAudioVolumeControllerTest, IsMute)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);

	bool bMute = m_pMicVolumeController->IsMute();
	
	bMute = m_pMicVolumeController->IsMute(WME_VOL_SYSTEM);
	
	bMute = m_pSpeakerVolumeController->IsMute();
	
	bMute = m_pSpeakerVolumeController->IsMute(WME_VOL_SYSTEM);
}
TEST_F(CWmeAudioVolumeControllerTest, AddObserver)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);

	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_FAIL,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,NULL));

	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_FAIL,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,NULL));
	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(NULL));
}

TEST_F(CWmeAudioVolumeControllerTest, RemoveObserver)
{
	EXPECT_NE(NULL, (long)m_pWmeEngine);
	EXPECT_NE(NULL, (long)m_pMicVolumeController);
	EXPECT_NE(NULL, (long)m_pSpeakerVolumeController);

	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(NULL));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));

	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,NULL));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_FAIL,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));

	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->AddObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_POINTER,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,NULL));
	EXPECT_EQ(WME_S_OK,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(m_pVolumeEventObserver));
	EXPECT_EQ(WME_E_FAIL,((CWmeAudioVolumeController*)m_pMicVolumeController)->RemoveObserver(WMEIID_IWmeAudioVolumeObserver,m_pVolumeEventObserver));
}

#endif

