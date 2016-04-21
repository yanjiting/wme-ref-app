#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeDevice.h"
#include "WmeError.h"

using namespace wme;

class CWmeVideoCaptureDeviceTest : public testing::Test
{
public:
	CWmeVideoCaptureDeviceTest() : m_pWmeEngine(NULL), m_pDeviceEnum(NULL)
	{
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeVideo, WmeDeviceIn, &m_pDeviceEnum);
		}


	}
	virtual ~CWmeVideoCaptureDeviceTest()
	{

		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
		}

		if (m_pDeviceEnum)
		{
			m_pDeviceEnum->Release();
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeMediaDeviceEnumerator* m_pDeviceEnum;
};


TEST_F(CWmeVideoCaptureDeviceTest, GetDeviceNumber)
{
	EXPECT_NE(NULL, (long)m_pDeviceEnum);
	int32_t num = 0;
	EXPECT_EQ(WME_S_OK, m_pDeviceEnum->GetDeviceNumber(num));
}

TEST_F(CWmeVideoCaptureDeviceTest, GetDefaultDevice)
{
	EXPECT_NE(NULL, (long)m_pDeviceEnum);
	int32_t num = 0;
	EXPECT_EQ(WME_S_OK, m_pDeviceEnum->GetDeviceNumber(num));

	if (num > 0)
	{
		int default_idx = 0;
		IWmeMediaDevice*	pDevice = NULL;
		EXPECT_EQ(WME_S_OK, m_pDeviceEnum->GetDevice(default_idx, &pDevice));
		EXPECT_NE(NULL, (long)pDevice);
		pDevice->Release();
	}
}

TEST_F(CWmeVideoCaptureDeviceTest, LoopDevices)
{
	EXPECT_NE(NULL, (long)m_pDeviceEnum);
	int32_t num = 0;
	EXPECT_EQ(WME_S_OK, m_pDeviceEnum->GetDeviceNumber(num));

	int device_idx = 0;
	while (num > 0 && device_idx < num)
	{
		//WmeRefPointer<IWmeMediaDevice> rfDevice;
		IWmeMediaDevice*	pDevice = NULL;
		EXPECT_EQ(WME_S_OK, m_pDeviceEnum->GetDevice(device_idx, &pDevice));
		if (pDevice)
		{
			WmeMediaType mediatype = WmeMediaTypeVideo;
			EXPECT_EQ(WME_S_OK, pDevice->GetMediaType(mediatype));

			WmeDeviceInOutType inouttype = WmeDeviceIn;
			EXPECT_EQ(WME_S_OK, pDevice->GetInOutType(inouttype));

            char szUniString[1024] = "hello";
            int szLen = 1024;
			EXPECT_EQ(WME_S_OK, pDevice->GetUniqueName(szUniString, szLen));

			EXPECT_EQ(WME_S_OK, pDevice->IsSameDevice(szUniString, szLen));

			char szFriendName[1024] = "hello";
            szLen = 1024;
			EXPECT_EQ(WME_S_OK, pDevice->GetFriendlyName(szFriendName, szLen));

			WmeCameraCapability camera_capa;
			WmeDeviceCapability device_capa;
			device_capa.eType = WmeDeviceCapabilityVideo;
			device_capa.iSize = sizeof(WmeCameraCapability);
			device_capa.pCapalibity = &camera_capa;
			int capa_idx = 0;
			while (WME_S_OK == pDevice->GetCapabilities(capa_idx, &device_capa))
			{
				//TODO: print them

				capa_idx ++;
			}

			pDevice->Release();
		}

		device_idx++;
	}
}