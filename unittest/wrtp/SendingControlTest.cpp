#include "smoothsend.h"
#include "timer.h"
#include "wrtppacket.h"
#include "mmframemgr.h"
#include "rtpsessionconfig.h"
#include "testutil.h"

using namespace wrtp;

class CFrameSendingControlSinkMock : public IFrameSmoothSendSink
{
public:
    CFrameSendingControlSinkMock()
        : m_sendingController(nullptr)
        , m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
        m_smoothSendingScheduler = CCmMakeShared<CSmoothSendingScheduler>();
        m_smoothSendingScheduler->Start("smoothsend", false, TT_RTP_SMOOTH_SEND);
        m_sessionContext->GetOutboundConfig()->SetSmoothSendingScheduler(m_smoothSendingScheduler);
    }

    virtual ~CFrameSendingControlSinkMock()
    {
        delete m_sendingController;
        m_sendingController = nullptr;
        if (m_smoothSendingScheduler) {
            m_smoothSendingScheduler->Stop();
            m_smoothSendingScheduler.reset();
        }
    }

    // member function from IMessageBlockSendingControlSink
    virtual int32_t OnOutputData(CCmMessageBlock &mb, const CRTPPacketMetaInfo& pktInfo, const CMediaMetaInfo* mediaInfo)
    {
        int len1 = strlen(m_message);
        int len2 = mb.GetTopLevelLength();
        EXPECT_EQ(len1, len2);
        if (len1 != len2) {
            return 0;
        }
        EXPECT_EQ(0, memcmp(m_message, mb.GetTopLevelReadPtr(), len1));
        return 0;
    }
    int32_t OnPacketSmoothed(uint32_t msid)
    {
        return 0;
    }
    int32_t OnT0FrameDropped(uint32_t ssrc)
    {
        return 0;
    }
    void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }
    virtual bool IsSendWindowHasRoom()
    {
        return true;
    }
    void InitSendingController(WRTPSessionType sessionType)
    {
        m_sendingController = CreateFrameSmoothSend(sessionType, this);
        switch (sessionType) {
            case RTP_SESSION_WEBEX_VOIP: {
                EXPECT_TRUE(m_sendingController == nullptr);
            }
            break;

            case RTP_SESSION_WEBEX_VIDEO: {
                EXPECT_TRUE(m_sendingController != nullptr);
                CFrameSmoothSend *smoothSending = (CFrameSmoothSend *)(m_sendingController);
                EXPECT_TRUE(smoothSending != nullptr);
            }
            break;

            default: {
                EXPECT_TRUE(m_sendingController == nullptr);
            }
            break;
        }
    }

    void SendMessage()
    {
        uint32_t dataLen = strlen(m_message);

        uint32_t channelId = 1;
        uint32_t ssrc =  123;
        uint8_t payloadType = 89;
        uint32_t clockRate = 90000;
        uint32_t timeoffsetExtId = 2;
        uint32_t captureTimestamp = 100;
        uint32_t sampleTimestamp = 123;
        uint8_t priority = 0;

        auto frame = ConstructFrameUnit(channelId, ssrc, payloadType, clockRate, timeoffsetExtId, captureTimestamp, sampleTimestamp, priority,m_sessionContext);
        ASSERT_TRUE(!!frame);
        auto &nal = frame->CreateNAL();
        ASSERT_TRUE(!!nal);
        auto &fragments = nal->CreateFragment(dataLen, 1, m_sessionContext->GetFrameManager());
        auto &fu = fragments[0];
        ASSERT_TRUE(!!fu);
        int32_t ret = fu->CopyDataIn(reinterpret_cast<const uint8_t *>(m_message), dataLen);
        ASSERT_TRUE(0 == ret);
        fu->SetMarker(1);

        if (m_sendingController) {
            uint32_t bandwidth = 512*1024; // 512 kbytes/sec
            m_sendingController->UpdateBandwidth(bandwidth);
            ret = m_sendingController->OutputData(std::move(frame));
            EXPECT_TRUE(0 == ret);
        }
    }

private:
    static const char *m_message;
    IFrameSmoothSend *m_sendingController;
    RTPSessionContextSharedPtr m_sessionContext;
    CSmoothSendingSchedulerSharedPtr m_smoothSendingScheduler;
};

const char *CFrameSendingControlSinkMock::m_message = "a message to test sending controller";

class CSendingControlTest : public ::testing::Test
{
public:

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {
    }

};

TEST_F(CSendingControlTest, Test_Direct_Sending_Dataflow)
{
    CScopedTracer test_info;
    CFrameSendingControlSinkMock sendingControlSink;
    sendingControlSink.InitSendingController(RTP_SESSION_WEBEX_VOIP);
    sendingControlSink.SendMessage();
}


TEST_F(CSendingControlTest, Test_Smooth_Sending_Dataflow)
{
    CScopedTracer test_info;
    CFrameSendingControlSinkMock sendingControlSink;
    sendingControlSink.InitSendingController(RTP_SESSION_WEBEX_VIDEO);
    sendingControlSink.SendMessage();
}

//#define ENABLE_SMOOTH_SENDING_UT_LOG

class CFrameSendingControlSinkSmoothMock : public IFrameSmoothSendSink
{
public:
    CFrameSendingControlSinkSmoothMock()
        : m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
        m_initBandwidth = 20 * 1024; // 20k bytes/sec

        WRTPSessionType sessionType = RTP_SESSION_WEBEX_VIDEO;
        m_sendingController = CreateFrameSmoothSend(sessionType, this, m_initBandwidth);

        m_accumulateDataLength = 0;
        m_outputCount = 0;
    }

    ~CFrameSendingControlSinkSmoothMock()
    {
        DestroyFrameSmoothSend(&m_sendingController);
        EXPECT_TRUE(m_sendingController == nullptr);
    }

    // member function from IFrameSmoothSendSink
    virtual int32_t OnOutputData(CCmMessageBlock &mb, const CRTPPacketMetaInfo& pktInfo, const CMediaMetaInfo* mediaInfo)
    {
        m_accumulateDataLength += pktInfo.GetPacketLength();
        uint32_t elapseMS = DEFAULT_SMOOTH_SENDING_INTERVAL_US / 1000 * (++m_outputCount) ;
        if (elapseMS >= 1000) {
            uint32_t realBandwidth = m_accumulateDataLength * 1000 / elapseMS;
            uint32_t bandwidthHigh = static_cast<uint32_t>(m_initBandwidth * 1.2);
            uint32_t bandwidthLow = static_cast<uint32_t>(m_initBandwidth * 0.8);

            if (realBandwidth > bandwidthHigh || realBandwidth < bandwidthLow) {
                WRTP_INFOTRACE_THIS("SendingControl: m_outputCount = " << m_outputCount << ", elapsedMS = " << elapseMS);
            }

            EXPECT_LE(realBandwidth, bandwidthHigh);
            EXPECT_GE(realBandwidth, bandwidthLow);
#ifdef ENABLE_SMOOTH_SENDING_UT_LOG
            printf("realBandwidth = %d, interval = %d\n", realBandwidth, elapseMS);
#endif // ~ENABLE_SMOOTH_SENDING_UT_LOG

            m_accumulateDataLength = 0;
            m_outputCount = 0;
        }
        return 0;
    }
    int32_t OnPacketSmoothed(uint32_t msid)
    {
        return 0;
    }
    int32_t OnT0FrameDropped(uint32_t ssrc)
    {
        return 0;
    }
    virtual bool IsSendWindowHasRoom()
    {
        return true;
    }
    void SendData()
    {
#define DATA_LEN 1024
        char *data = new char[DATA_LEN];
        char ch = 'a';
        for (int ii = 0; ii < DATA_LEN; ++ii) {
            data[ii] = ch;

            ++ch;
            if (ch > 'z') {
                ch = 'a';
            }
        }

        uint32_t channelId = 1;
        uint32_t ssrc =  123;
        uint8_t payloadType = 89;
        uint32_t clockRate = 90000;
        uint32_t timeoffsetExtId = 2;
        uint32_t captureTimestamp = 100;
        uint32_t sampleTimestamp = 123;
        uint8_t priority = 0;

        for (int ii = 0; ii < 1024; ++ii) {
            auto frame = ConstructFrameUnit(channelId, ssrc, payloadType, clockRate, timeoffsetExtId, captureTimestamp, sampleTimestamp, priority,m_sessionContext);
            ASSERT_TRUE(!!frame);
            auto &nal = frame->CreateNAL();
            ASSERT_TRUE(!!nal);
            auto &fragments = nal->CreateFragment(DATA_LEN, 1, m_sessionContext->GetFrameManager());
            auto &fu = fragments[0];
            ASSERT_TRUE(!!fu);
            int32_t ret = fu->CopyDataIn(reinterpret_cast<const uint8_t *>(data), DATA_LEN);
            ASSERT_TRUE(0 == ret);
            fu->SetMarker(1);

            m_sendingController->OutputData(std::move(frame));
#if (defined ENABLE_SMOOTH_SENDING_UT_LOG)
            ::SleepMs(10);
#endif // ~ENABLE_SMOOTH_SENDING_UT_LOG
        }

#if (defined ENABLE_SMOOTH_SENDING_UT_LOG)
        ::SleepMs(10 * 1000);
#endif // ~ENABLE_SMOOTH_SENDING_UT_LOG
        delete[] data;
    }

    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }

private:
    IFrameSmoothSend *m_sendingController;
    uint32_t m_initBandwidth;
    uint32_t m_accumulateDataLength;
    low_ticker m_ticker;
    RTPSessionContextSharedPtr m_sessionContext;
    uint32_t m_outputCount;
};

class CSmoothSendingControlTest : public ::testing::Test
{
public:

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {
    }

};

class CMockTickerForTokenBucket : public IFakeTicker
{
public:
    virtual CClockTime Now()
    {
        uint64_t tick = 0;
        tick = ticker::now();
        return CClockTime(tick);
    }

    MOCK_METHOD0(TimeElaspsed, uint64_t());
    MOCK_METHOD0(TimeElaspsedInMilliSec, uint32_t());
};

TEST_F(CSmoothSendingControlTest, Test_Smooth_Sending_Bandwidth)
{
    CScopedTracer test_info;
    NiceMock<CMockTickerForTokenBucket> mockTicker;
    CRTPTicker::SetRTPFakeTicker(&mockTicker);
    ON_CALL(mockTicker, TimeElaspsedInMilliSec()).WillByDefault(Return(20));

    CFrameSendingControlSinkSmoothMock smoothMock;
    smoothMock.SendData();
    //::SleepMs(1000);
    WRTP_TEST_TRIGGER_ON_TIMER(10, 100);

    CRTPTicker::SetRTPFakeTicker(nullptr);
}
