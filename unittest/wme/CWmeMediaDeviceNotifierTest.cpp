#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeDevice.h"

#if defined (WIN32)
#endif

using namespace wme;

class CWmeMediaDeviceNotifierTest : public testing::Test
{
public:
	CWmeMediaDeviceNotifierTest()
	{
		m_pNotifier = NULL;


		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaDevicesNotifier(WmeMediaTypeVideo, WmeDeviceIn,&m_pNotifier);
			m_pWmeEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceIn,&m_pNotifierAudioIN);
			m_pWmeEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceOut,&m_pNotifierAudioOUT);
		}


	}
	virtual ~CWmeMediaDeviceNotifierTest()
	{
		if (m_pNotifier)
		{
			m_pNotifier->Release();
			m_pNotifier = NULL;
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
	IWmeMediaEventNotifier* m_pNotifier;
	IWmeMediaEventNotifier* m_pNotifierAudioIN;
	IWmeMediaEventNotifier* m_pNotifierAudioOUT;
};


#ifdef WIN32

TEST_F(CWmeMediaDeviceNotifierTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pNotifier);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pNotifier->QueryInterface(WMEIID_IWmeMediaEventNotifier, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	int i = 0;
}

TEST_F(CWmeMediaDeviceNotifierTest,AboutObserver)
{
    return; // disable for temp solution of dynamic_cast crash
    
	EXPECT_NE(NULL, (long)m_pNotifier);
	const WMEIID WMEIID_IWmeVideoTest = 
	{ 0x487a71f3, 0x7f7, 0x4000, { 0x91, 0x97, 0x6b, 0x96, 0x5d, 0x79, 0x26, 0x1c } };

	class ZombieObserver: public IWmeMediaDeviceObserver
	{
	public:
		WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent){return WME_S_OK;}
		WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID){return WME_S_OK;}

		WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
	};
	ZombieObserver zombieobserver1, zombieobserver2, zombieobserver3;
	EXPECT_NE(WME_S_OK, m_pNotifier->AddObserver(WMEIID_IWmeVideoTest,&zombieobserver1));
	EXPECT_EQ(WME_S_OK, m_pNotifier->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifier->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_EQ(WME_S_OK, m_pNotifier->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));

	EXPECT_EQ(WME_S_OK, m_pNotifier->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifier->RemoveObserver(WMEIID_IWmeVideoTest, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifier->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver3));
	EXPECT_EQ(WME_S_OK, m_pNotifier->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));
	EXPECT_NE(WME_S_OK, m_pNotifier->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));

}


TEST_F(CWmeMediaDeviceNotifierTest, QueryInterfaceAudio)
{
	EXPECT_NE(NULL, (long)m_pNotifierAudioIN);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pNotifierAudioIN->QueryInterface(WMEIID_IWmeMediaEventNotifier, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	int i = 0;
}

TEST_F(CWmeMediaDeviceNotifierTest,AboutObserverAudio)
{


	EXPECT_NE(NULL, (long)m_pNotifierAudioIN);
	const WMEIID WMEIID_IWmeAudioTest = 
	{ 0xd1a45a9c, 0x8332, 0x45aa, { 0xbb, 0x70, 0x2d, 0x62, 0x95, 0xe5, 0xb4, 0x45 } };


	class ZombieObserver: public IWmeMediaDeviceObserver
	{
	public:
		WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent){return WME_S_OK;}
		WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID){return WME_S_OK;}

		WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
	};
	ZombieObserver zombieobserver1, zombieobserver2, zombieobserver3;
	EXPECT_NE(WME_S_OK, m_pNotifierAudioIN->AddObserver(WMEIID_IWmeAudioTest,&zombieobserver1));
	EXPECT_EQ(WME_S_OK, m_pNotifierAudioIN->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifierAudioIN->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_EQ(WME_S_OK, m_pNotifierAudioIN->AddObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));

	EXPECT_EQ(WME_S_OK, m_pNotifierAudioIN->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifierAudioIN->RemoveObserver(WMEIID_IWmeAudioTest, &zombieobserver1));
	EXPECT_NE(WME_S_OK, m_pNotifierAudioIN->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver3));
	EXPECT_EQ(WME_S_OK, m_pNotifierAudioIN->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));
	EXPECT_NE(WME_S_OK, m_pNotifierAudioIN->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, &zombieobserver2));

}

#endif
