#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "CmThreadManager.h"
#include "sendwindow.h"
#include "sendcontrol.h"
#include "testutil.h"

using namespace wrtp;
#define FRAME_UNIT_LEN (1 * 1024 * 8) // 8kbps, 1kBps
//CASE DEFINE
#define BANDWIDTH_RATE (10 * FRAME_UNIT_LEN) // 80kbps, 8kBps
#define EXPECT_SENDING_WINDOW_SIZE 1000


class CWindowSendControlSinkMock : public ISendingControlSink
{
public:
    virtual int32_t OnPacketSmoothed(uint32_t msid)
    {
        return 0;
    }

    virtual int32_t OnT0FrameDropped(uint32_t ssrc)
    {
        return 0;
    }

    //UpdateStat
    virtual void OnPacketFiltered(uint32_t msid, int32_t dwTimeStamp, FilterResult sType, int32_t nLen, uint8_t ucPriority, FilterVideoDataInfo *infoData, BOOL bFECFlag)
    {

    }

    //virtual void OnDataDropped(int32_t eFrameIdc, int32_t iPriority , uint32_t dwTimeStamp) = 0;

    virtual void OnDataDropped(const FilterVideoDataInfo &filterInfo)
    {

    }
    virtual int32_t OnOutputData(CCmMessageBlock &mb, const CRTPPacketMetaInfo& pktInfo, const CMediaMetaInfo* mediaInfo)
    {
        m_pSendControl->NotifyPacketOutput(pktInfo.GetSSRC(), 0, mb.GetTopLevelLength() + 12);
        return 0;
    }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {

    }

    virtual int32_t  SendRTPPacket(const CRTPPacketAuto &packet, const CRTPPacketMetaInfo &metaInfo)
    {
        return 0;
    }
public:
    FilterResult m_filterType;
    ISendingControl *m_pSendControl;
};

class CSendingWindowTest : public ::testing::Test
{
protected:
    CSendingWindowTest()
    {
    }
    virtual void SetUp()
    {
        CCmMessageBlock mb(1000);
        char payloadData[] = "This is payload test data, it will repeat until payload length reach to 1000  abcdefghigklm123456789";
        expect_payload = (uint8_t *)mb.GetTopLevelReadPtr();
        expect_payloadLength = 1000;
        uint32_t written = 0;
        for (int i = 0; i < 10; ++i) {
            mb.Write(payloadData, 100, (DWORD *)&written);
        }
        expect_payload[999] = '\0';
        expect_refPayload = mb.DuplicateChained();
        
        m_sequence = 0;
        m_mediaInfo.SetPayloadType(0);
        m_mediaInfo.SetRTPTimestamp(234);
        m_pktInfo.SetSSRC(345);
        m_pktInfo.SetPacketLength(expect_refPayload->GetTopLevelLength()+12);

        m_pSendWindow.reset(new CSendWindow());
        m_pSendControl.reset(new CSendController(&sink, RTP_SESSION_WEBEX_AS));
        sink.m_pSendControl = (ISendingControl *)m_pSendControl.get();

    }
    virtual void TearDown()
    {
        m_pSendWindow.reset();
        m_pSendControl.reset();
        
        if (expect_refPayload) {
            expect_refPayload->DestroyChained();
            expect_refPayload = nullptr;
        }
    }

    CWindowSendControlSinkMock sink;
    std::unique_ptr<CSendWindow> m_pSendWindow;
    std::unique_ptr<CSendController> m_pSendControl;
    uint8_t *expect_payload;
    CCmMessageBlock *expect_refPayload;
    uint32_t expect_payloadLength;
    uint16_t m_sequence;
    CRTPPacketMetaInfo m_pktInfo;
    CMediaMetaInfoVideo m_mediaInfo;
};

TEST_F(CSendingWindowTest, create_sendwindow_successful)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_pSendWindow != nullptr);
    EXPECT_EQ(WRTP_MAX_SENDING_WINDOW_SIZE,m_pSendWindow->GetSendWindowSize());
}

TEST_F(CSendingWindowTest, set_sendwindow_size_successful)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_pSendWindow != nullptr);
    m_pSendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    EXPECT_EQ(EXPECT_SENDING_WINDOW_SIZE,m_pSendWindow->GetSendWindowSize());
}

TEST_F(CSendingWindowTest, pass_window_with_room_successful)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_pSendWindow != nullptr);
    EXPECT_TRUE(m_pSendWindow->CanPacketPass());
}

TEST_F(CSendingWindowTest, pass_window_without_room_fail)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_pSendWindow != nullptr);
    m_pSendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    EXPECT_TRUE(m_pSendWindow->CanPacketPass());
    m_pSendWindow->PacketPassWindow(m_pktInfo.GetSSRC(), m_sequence, m_pktInfo.GetPacketLength());
    EXPECT_FALSE(m_pSendWindow->CanPacketPass());
}

TEST_F(CSendingWindowTest, update_window_size)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_pSendWindow != nullptr);
    m_pSendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    EXPECT_TRUE(m_pSendWindow->CanPacketPass());
    m_pSendWindow->PacketPassWindow(m_pktInfo.GetSSRC(), m_sequence, m_pktInfo.GetPacketLength());
    EXPECT_FALSE(m_pSendWindow->CanPacketPass());
    m_pSendWindow->OnSendWindowUpdate(345,0);
    EXPECT_TRUE(m_pSendWindow->CanPacketPass());
}

TEST_F(CSendingWindowTest, output_data_from_send_controller_without_window_success)
{
    CScopedTracer test_info;
    int32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE);
    int32_t sendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(sendret,0);
}

TEST_F(CSendingWindowTest, output_data_from_send_controller_with_enough_window_success)
{
    CScopedTracer test_info;
    int32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    CSendWindowSharedPtr sendWindow = CCmMakeShared<CSendWindow>();
    sendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    m_pSendControl->SetSendWindow(sendWindow);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE);
    int32_t sendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(sendret,0);
}

TEST_F(CSendingWindowTest, output_data_from_send_controller_with_unenough_window_fail)
{
    CScopedTracer test_info;
    int32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    CSendWindowSharedPtr sendWindow = CCmMakeShared<CSendWindow>();
    sendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    m_pSendControl->SetSendWindow(sendWindow);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE);
    int32_t firstsendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(firstsendret,0);
    int32_t secondsendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(secondsendret,-1);
}

TEST_F(CSendingWindowTest, output_data_from_send_controller_with_uwindow_update)
{
    CScopedTracer test_info;
    int32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    CSendWindowSharedPtr sendWindow = CCmMakeShared<CSendWindow>();
    sendWindow->SetSendWindowSize(EXPECT_SENDING_WINDOW_SIZE);
    m_pSendControl->SetSendWindow(sendWindow);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE);
    int32_t firstsendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(firstsendret,0);
    sendWindow->OnSendWindowUpdate(345,0);
    int32_t secondsendret = m_pSendControl->OnOutputData(*expect_refPayload, m_pktInfo, &m_mediaInfo);
    EXPECT_EQ(secondsendret,0);
}
