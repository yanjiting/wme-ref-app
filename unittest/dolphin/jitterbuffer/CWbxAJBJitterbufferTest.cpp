/*****************************************************************************************
*Generate by AUT Tool on 2011-01-06
*class:CWbxAJBPolicyTest
********************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock/wbxmock.h"

#define private public
#define protected public
#include "WbxAeJitterPolicy.h"
#include "WbxAeJitterBuffer.h"
//#include "rtpfram.h"
using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAJBJitterbufferTest : public testing::Test
{
public:
	CWbxAJBJitterbufferTest()
	{
	}

	virtual ~CWbxAJBJitterbufferTest()
	{
	}

	virtual void SetUp()
	{
		m_pAudioJitterBuffer= new CAudioJitterBuffer();
	}

	virtual void TearDown()
	{
		delete m_pAudioJitterBuffer;
		m_pAudioJitterBuffer = NULL;
	}

protected:
	CAudioJitterBuffer * m_pAudioJitterBuffer;
};

TEST_F(CWbxAJBJitterbufferTest, InitialJitterBuffer)
{
	WbxAEPlayloadInfoPara para;
	para.codecName = "iLBC";
	para.ulPacketSize = 160;
	para.ulPayloadType = 97;
	para.ulSampleRate = 8000;
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InitialJitterBuffer(&para, 1));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->InitialJitterBuffer(NULL, 0 ));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->InitialJitterBuffer(&para, 0 ));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->InitialJitterBuffer(NULL, 1 ));
	para.codecName = "test_too'";
	para.ulPacketSize = 160;
	para.ulPayloadType = 100;
	para.ulSampleRate = 8000;
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InitialJitterBuffer(&para, 1));

}
TEST_F(CWbxAJBJitterbufferTest, SetExternalJitterPolicy)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetExternalJitterPolicy(NULL, true ));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetExternalJitterPolicy(NULL, false ));

	IWbxJitterPolicy *pPolicy = NULL;
	CreateIWbxJitterPolicy(&pPolicy);
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetExternalJitterPolicy(pPolicy, true ));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetExternalJitterPolicy(pPolicy, false ));

	DestoryIWbxJitterPolicy(pPolicy);

}

	/*Input one packet to buffer */
TEST_F(CWbxAJBJitterbufferTest,  InputPacket)
{
	unsigned  char buffer[60];
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 100; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_sequence_number((unsigned short)104);
	pTempFrame->set_timestamp(104 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

	pTempFrame->set_sequence_number((unsigned short)103);
	pTempFrame->set_timestamp(103 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

	pTempFrame->set_sequence_number((unsigned short)106);
	pTempFrame->set_timestamp(106 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

	pTempFrame->set_sequence_number((unsigned short)102);
	pTempFrame->set_timestamp(102 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	pTempFrame->set_sequence_number((unsigned short)105);
	pTempFrame->set_timestamp(105 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));


	for (unsigned long uli = 65535; uli < 65538; uli ++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_sequence_number((unsigned short)65534);
	pTempFrame->set_timestamp(65534 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));


	pTempFrame->set_sequence_number((unsigned short)65539);
	pTempFrame->set_timestamp(65539 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

	pTempFrame->set_sequence_number((unsigned short)65533);
	pTempFrame->set_timestamp(65533 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

	pTempFrame->set_sequence_number((unsigned short)65540);
	pTempFrame->set_timestamp(65540 * 160 );
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));


	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_ILBC);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_G711_ULAW);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 1; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_G711_ALAW);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	pTempFrame->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_OPUS);
	pTempFrame->set_sync_source(102100);
	IWbxJitterPolicy* pExternalPolicy = NULL;
	CreateIWbxJitterPolicy(&pExternalPolicy);
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, false);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, true);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}
	m_pAudioJitterBuffer->SetExternalJitterPolicy(NULL, true);


	pTempFrame->set_payload_type(200);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size(), false ));
	
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, true);

	pTempFrame->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_G711_ALAW);
	pTempFrame->set_sync_source(102102);
	m_pAudioJitterBuffer->m_pInternalPolicy = NULL;
	for ( unsigned long  uli =0; uli < 2; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}

	DestoryIWbxJitterPolicy(pExternalPolicy);

	if (pTempFrame)
	{
		delete pTempFrame;
	}


}
	/**Get a packet from itter buffer table.*/
TEST_F(CWbxAJBJitterbufferTest, GetOutputPacket)
{


	unsigned  char buffer[60];
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	WBXAE_AJB_GET_FRAME_TYPE type = WBXAE_AJB_GET_FRAME_TYPE_ZERO; 
	WBXAE_AJB_SPEED_CONTROL speedControl=WBXAE_AJB_SPEED_CONTROL_NORMAL;
	m_pAudioJitterBuffer->GetOutputPacket(type,speedControl);

	for ( unsigned long  uli =0; uli < 100; uli++)
	{
		if ((uli+ 1) % 10 == 0)
		{
		}
		else
		{
			pTempFrame->set_sequence_number((unsigned short)uli);
			pTempFrame->set_timestamp(uli * 160 );
			if (uli == 55)
			{
				EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(), 10  ));
			}
			else
			{
				EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));

			}

		}
		m_pAudioJitterBuffer->GetOutputPacket(type,speedControl);

#ifdef  WIN32
		Sleep(20);
#elif defined LINUX
		usleep(20000);
#endif
	}
	
	for ( int i =0; i < 50; i ++)
	{
#ifdef  WIN32
		Sleep(20);
#elif defined LINUX
		usleep(20000);
#endif
		m_pAudioJitterBuffer->GetOutputPacket(type,speedControl);

	}



	if (pTempFrame)
	{
		delete pTempFrame;
	}
}

	// Get nStartedSequnce, nStartedSequnce+1, ..., nStartedSequnce+ulStructNum-1 packets, if it's not exist, it need set dateLen to zero;
TEST_F(CWbxAJBJitterbufferTest, GetPacketDataCopy)
{
	unsigned char buffer[60] = {0};
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 15; uli++)
	{
		if (7 == uli)
		{
		}
		else
		{
			pTempFrame->set_sequence_number((unsigned short)uli);
			pTempFrame->set_timestamp(uli * 160 );
			EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
		}
	}

	WbxAEFecRecoveStruct pFecRecover[2];
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(8, &pFecRecover[0], 2));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, &pFecRecover[0], 2));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, NULL, 0));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, &pFecRecover[0], 0));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, 0, 2));

	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->Empty());

	for ( unsigned long  uli =0; uli < 10; uli++)
	{
		if (uli == 7)
		{
		}
		else
		{
			EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(buffer,60, false ));
		}
	}
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(8, &pFecRecover[0], 2));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, &pFecRecover[0], 2));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, NULL, 0));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, &pFecRecover[0], 0));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetPacketDataCopy(7, 0, 2));
	int k = 0;
}
TEST_F(CWbxAJBJitterbufferTest, RemoveFirstPacket)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->RemoveFirstPacket());
	unsigned char buffer[60];
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 15; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->RemoveFirstPacket());

}
	// adjust current play time, it can be negative or positive
TEST_F(CWbxAJBJitterbufferTest, AdjustPlaytime)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->AdjustPlaytime(200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->AdjustPlaytime(-200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->AdjustPlaytime(0));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->AdjustPlaytime(-201));
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->AdjustPlaytime(201));
}
	/*Set min delay in ms*/
TEST_F(CWbxAJBJitterbufferTest, SetDelay)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(-1, 0));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(100, 200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(-200, 200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(-200, -200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(20000, 0));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(0, 200000));
	m_pAudioJitterBuffer->m_pInternalPolicy = NULL;
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(100, 200));
	m_pAudioJitterBuffer->SetExternalJitterPolicy(NULL,false);
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(100, 200));

	IWbxJitterPolicy* pExternalPolicy = NULL;
	CreateIWbxJitterPolicy(&pExternalPolicy);
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, false);
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(100, 200));
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, true);
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->SetDelay(100, 200));

}
TEST_F(CWbxAJBJitterbufferTest, Empty)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->Empty());
	unsigned char buffer[60];
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 15; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
	}
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->Empty());

}

TEST_F(CWbxAJBJitterbufferTest, Size)
{
	m_pAudioJitterBuffer->Size();	
}
TEST_F(CWbxAJBJitterbufferTest, GetStatus)
{
	WBXAE_AJB_BUFFER_STATUS bufferStatus = WBXAE_AJB_BUFFER_STATUS_BUFFERING_EMPTY;

	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatus(bufferStatus));
	unsigned char buffer[60];
	RTP_Frame* pTempFrame = new  RTP_Frame(buffer, 60);
	pTempFrame->set_payload_type(101);
	pTempFrame->set_sync_source(102100);
	for ( unsigned long  uli =0; uli < 15; uli++)
	{
		pTempFrame->set_sequence_number((unsigned short)uli);
		pTempFrame->set_timestamp(uli * 160 );
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->InputPacket(pTempFrame->get_buffer(),pTempFrame->get_header_size() +pTempFrame->get_payload_size()  ));
		EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatus(bufferStatus));
#ifdef  WIN32
		Sleep(20);
#elif defined LINUX
		usleep(20000);
#endif
		WBXAE_AJB_GET_FRAME_TYPE type = WBXAE_AJB_GET_FRAME_TYPE_ZERO; 
		WBXAE_AJB_SPEED_CONTROL speedcontrol=WBXAE_AJB_SPEED_CONTROL_NORMAL;
		CWbxAERTPSample* pSample = m_pAudioJitterBuffer->GetOutputPacket(type,speedcontrol);
		if (pSample)
		{
			int kk;
			kk = 0;
		}
	}


}
TEST_F(CWbxAJBJitterbufferTest, GetStatistics)
{
	WbxAEAJBStatistics tmpSta;
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatistics(tmpSta));
	IWbxJitterPolicy* pExternalPolicy = NULL;
	CreateIWbxJitterPolicy(&pExternalPolicy);
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, false);
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatistics(tmpSta));
	m_pAudioJitterBuffer->SetExternalJitterPolicy(pExternalPolicy, true);
	EXPECT_EQ(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatistics(tmpSta));
	m_pAudioJitterBuffer->SetExternalJitterPolicy(NULL, true);

	m_pAudioJitterBuffer->m_pInternalPolicy = NULL;
	EXPECT_NE(WBXAE_SUCCESS, m_pAudioJitterBuffer->GetStatistics(tmpSta));
}

TEST_F(CWbxAJBJitterbufferTest, CreateDestroy)
{
	IWbxAdaptiveJitterBuffer* pAdaptiveJitterBuffer;
	EXPECT_NE(WBXAE_SUCCESS, CreateIWbxAdaptiveJitterBuffer(NULL));
	EXPECT_NE(WBXAE_SUCCESS, DestroyIWbxAdaptiveJitterBuffer(NULL));
	EXPECT_EQ(WBXAE_SUCCESS, CreateIWbxAdaptiveJitterBuffer(&pAdaptiveJitterBuffer));
	EXPECT_EQ(WBXAE_SUCCESS, DestroyIWbxAdaptiveJitterBuffer(pAdaptiveJitterBuffer));
}

TEST_F(CWbxAJBJitterbufferTest, TestWbxRTPSample)
{
	unsigned char buff[100];
	CWbxAERTPSample* pSample1 = new CWbxAERTPSample(NULL, 0, 0 , false, 0, 0,0, 0, 0, TRUE);
	CWbxAERTPSample* pSample2 = new CWbxAERTPSample(NULL, 10, 0 , false, 0, 0,0, 0, 0, TRUE);
	CWbxAERTPSample* pSample3 = new CWbxAERTPSample(buff, 0, 0 , false, 0, 0,0, 0, 0, TRUE);
	CWbxAERTPSample* pSample4 = new CWbxAERTPSample(buff, 100, 0 , false, 0, 0,0, 0, 0, TRUE);

	CWbxAERTPSample* pSample5 = new CWbxAERTPSample(NULL, 0, 0 , true, 0, 0,0, 0, 0, TRUE);
	CWbxAERTPSample* pSample6 = new CWbxAERTPSample(NULL, 10, 0 , true, 0, 0,0, 0, 0, FALSE);
	CWbxAERTPSample* pSample7 = new CWbxAERTPSample(buff, 0, 0 , true, 0, 0,0, 0, 0, TRUE);
	CWbxAERTPSample* pSample8 = new CWbxAERTPSample(buff, 100, 0 , true, 0, 0,0, 0, 0, TRUE);
	pSample8->GetDataLen();
	pSample8->GetDataPtr();
	pSample8->GetRTPSequence();
	pSample8->GetReceivedTime();
	pSample8->GetSampleSequence();
	pSample8->GetRTPHeadFlag();

	pSample8->SetEstimatedPlaybackTime(200);
	EXPECT_EQ(200, 	pSample8->GetEstimatedPlaybackTime());
	pSample8->SetReceivedTime(100);
	EXPECT_EQ(100, 	pSample8->GetReceivedTime());
	pSample8->SetRTPSequence(1100);
	EXPECT_EQ(1100, 	pSample8->GetRTPSequence());
	pSample8->SetSampleSequence(20);
	EXPECT_EQ(20, 	pSample8->GetSampleSequence());
	pSample8->SetRTPHeadFlag(false);
	EXPECT_EQ(false, 	pSample8->GetRTPHeadFlag());

}
