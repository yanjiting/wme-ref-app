#include "activebuffer.h"
#include "testutil.h"
#include "rtptime.h"
#include "rtpsessionconfig.h"

using namespace wrtp;


class CTokenBucketTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        WRTP_INFOTRACE("CTokenBucketTest:setup");

    }

    virtual void TearDown()
    {
        WRTP_INFOTRACE("CTokenBucketTest:teardown");
    }
};

class CMockTicker : public IFakeTicker
{
public:
    MOCK_METHOD0(Now, CClockTime());
    MOCK_METHOD0(TimeElaspsed, uint64_t());
    MOCK_METHOD0(TimeElaspsedInMilliSec, uint32_t());
};

#if 1
//annotated by Joyce
TEST_F(CTokenBucketTest, Test_Init_State)
{
    CScopedTracer test_info;
    NiceMock<CMockTicker> mockTicker;
    CRTPTicker::SetRTPFakeTicker(&mockTicker);
    ON_CALL(mockTicker, Now()).WillByDefault(Return(CClockTime(0, 0)));

    uint32_t bandwidth = 1000*1024;   // 1000 kbytes/sec
    uint32_t maxBurstTime = 200;
    CTokenBucket tokenBucket(bandwidth, maxBurstTime);

    int32_t burstSize = bandwidth * maxBurstTime / 1000;
    int32_t availableTokens = tokenBucket.QueryAvailableTokensInByte();
    //EXPECT_EQ(availableTokens, INIT_TOKENS_IN_BYTE);
    EXPECT_LE(availableTokens, burstSize);
    CRTPTicker::SetRTPFakeTicker(nullptr);
}

TEST_F(CTokenBucketTest, Test_Token_Consumption)
{
    CScopedTracer test_info;
    NiceMock<CMockTicker> mockTicker;
    CRTPTicker::SetRTPFakeTicker(&mockTicker);
    ON_CALL(mockTicker, Now()).WillByDefault(Return(CClockTime(0, 0)));

    uint32_t bandwidth = 1000*1024;   // 1000 kbytes/sec
    uint32_t maxBurstTime = 200;
    CTokenBucket tokenBucket(bandwidth, maxBurstTime);

    int32_t burstSize = bandwidth * maxBurstTime / 1000;
    int32_t availableTokens = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_LE(availableTokens, burstSize);

    int32_t usedTokens = burstSize / 2;
    int32_t ret = tokenBucket.ConsumeTokensInByte(usedTokens);
    EXPECT_TRUE(0 == ret);

    availableTokens = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_EQ(availableTokens, 0);

    usedTokens = burstSize;
    ret = tokenBucket.ConsumeTokensInByte(usedTokens);
    EXPECT_TRUE(0 == ret);

    availableTokens = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_EQ(availableTokens, 0);
    CRTPTicker::SetRTPFakeTicker(nullptr);
}

TEST_F(CTokenBucketTest, Test_Bandwidth_Change)
{
    CScopedTracer test_info;
    NiceMock<CMockTicker> mockTicker;
    CRTPTicker::SetRTPFakeTicker(&mockTicker);
    ON_CALL(mockTicker, Now()).WillByDefault(Return(CClockTime(0, 0)));

    uint32_t bandwidth0 = 2000*1024;       // 2000 kbytes/sec
    uint32_t maxBurstTime = 500;
    CTokenBucket tokenBucket(bandwidth0, maxBurstTime);

    int32_t burstSize0 = bandwidth0 * maxBurstTime / 1000;
    int32_t availableTokens0 = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_LE(availableTokens0, burstSize0);

    int32_t bandwidth1 = 3000*1024;    // 3000 kbytes/sec
    int32_t burstSize1 = bandwidth1 * maxBurstTime / 1000;
    ON_CALL(mockTicker, Now()).WillByDefault(Return(CClockTime(0, 100000)));
    tokenBucket.UpdateRate(bandwidth1);
    int32_t availableTokens1 = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_LE(availableTokens1, availableTokens0);
    EXPECT_LE(availableTokens1, burstSize1);

    uint32_t bandwidth2 = 1000*1024;  // 1000 kbytes/sec
    int32_t burstSize2 = bandwidth2 * maxBurstTime / 1000;
    tokenBucket.UpdateRate(bandwidth2);
    int32_t availableTokens2 = tokenBucket.QueryAvailableTokensInByte();
    EXPECT_LE(availableTokens2, burstSize2);
    CRTPTicker::SetRTPFakeTicker(nullptr);
}

TEST_F(CTokenBucketTest, Test_Bandwidth_Sharing)
{
    CScopedTracer test_info;
    NiceMock<CMockTicker> mockTicker;
    CRTPTicker::SetRTPFakeTicker(&mockTicker);
    ON_CALL(mockTicker, Now()).WillByDefault(Return(CClockTime(0, 0)));

    uint32_t bwTotal = 8000*1024;
    CCmComAutoPtr<CTokenBucket> pTokenBucketRoot = new CTokenBucket(bwTotal);

    uint32_t bw1 = 30*1024;
    CCmComAutoPtr<CTokenBucket> pTokenBucket1 = new CTokenBucket(bw1);
    pTokenBucket1->SetParent(pTokenBucketRoot.ParaIn());

    uint32_t bw2 = 50*1024;
    CCmComAutoPtr<CTokenBucket> pTokenBucket2 = new CTokenBucket(bw2);
    pTokenBucket2->SetParent(pTokenBucketRoot.ParaIn());

    int32_t token1 = pTokenBucket1->QueryAvailableTokensInByte();
    int32_t token2 = pTokenBucket2->QueryAvailableTokensInByte();
    int32_t tokenWithBorrow = pTokenBucket2->QueryAvailableTokensInByte(true);
    int32_t tokenTotal = pTokenBucketRoot->QueryAvailableTokensInByte();
    EXPECT_EQ(tokenTotal, pTokenBucketRoot->QueryAvailableTokensInByte(true));
    EXPECT_LE(token1, token2);
    //currently there are tokens avaible in bucket 2, so no borrow tokens
    EXPECT_EQ(token2, tokenWithBorrow);
    EXPECT_LE(tokenWithBorrow, tokenTotal);

    //consume token2 of bucket2 itself
    pTokenBucket2->ConsumeTokensInByte(token2);
    token2 = pTokenBucket2->QueryAvailableTokensInByte();
    tokenWithBorrow = pTokenBucket2->QueryAvailableTokensInByte(true);
    EXPECT_LE(token2, tokenWithBorrow);

    //comsume part of token1
    token1 = pTokenBucket1->QueryAvailableTokensInByte();
    pTokenBucket1->ConsumeTokensInByte(token1/2);

    token1 = pTokenBucket1->QueryAvailableTokensInByte();
    int32_t tokenWithBorrowNow = pTokenBucket2->QueryAvailableTokensInByte(true);
    EXPECT_LE(tokenWithBorrowNow, tokenWithBorrow);
    tokenWithBorrow = tokenWithBorrowNow;

    //consume borrowed tokens
    pTokenBucket2->ConsumeTokensInByte(tokenWithBorrow);
    int32_t token1AfterBorrow = pTokenBucket1->QueryAvailableTokensInByte();
    EXPECT_EQ(token1AfterBorrow, token1);
    token2 = pTokenBucket2->QueryAvailableTokensInByte();
    EXPECT_EQ(token2, 0);
    tokenTotal = pTokenBucketRoot->QueryAvailableTokensInByte();
    EXPECT_EQ(tokenTotal, 0);

    CRTPTicker::SetRTPFakeTicker(nullptr);
}
#endif

class MockFrameSendingBufferSink
    : public IFrameSendBufferSink
{
public:
    MockFrameSendingBufferSink()
        : m_forwardDataResult(0)
        , m_channelId(0)
        , m_bT0FrameDropped(false)
    {
    }

    ~MockFrameSendingBufferSink()
    {
    }
public:
    virtual int32_t ForwardData(CCmMessageBlock &mb, const CRTPPacketMetaInfo& pktInfo, const CMediaMetaInfo* mediaInfo)
    {
        return m_forwardDataResult;
    }
    virtual int32_t OnPacketSmoothed(uint32_t msid)
    {
        m_channelId = msid;
        return 0;
    }
    virtual int32_t OnT0FrameDropped(uint32_t ssrc)
    {
        m_bT0FrameDropped = true;
        return 0;
    }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }
public:
    int32_t m_forwardDataResult;
    uint32_t m_channelId;
    uint32_t m_bT0FrameDropped;
};

class CFrameSmoothSendingBufferASTest : public ::testing::Test
{
public:
    CFrameSmoothSendingBufferASTest()
        : m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
        , m_parentToken(new CTokenBucket(WRTP_SESSION_INITIAL_BANDWIDTH))
    {
        m_smoothSendingScheduler = CCmMakeShared<CSmoothSendingScheduler>();
        m_smoothSendingScheduler->Start("smoothsend", false, TT_RTP_SMOOTH_SEND);
        m_sessionContext->GetOutboundConfig()->SetSmoothSendingScheduler(m_smoothSendingScheduler);
    }
    ~CFrameSmoothSendingBufferASTest()
    {
        if (m_smoothSendingScheduler) {
            m_smoothSendingScheduler->Stop();
            m_smoothSendingScheduler.reset();
        }
    }
    virtual void SetUp()
    {
        WRTP_INFOTRACE("CFrameSmoothSendingBufferASTest:setup");
        m_channelId = 3;
        m_ssrc =  123;
        m_payloadType = 89;
        m_clockRate = 90000;
        m_timeoffsetExtId = 2;
        m_captureTimestamp = 100;
        m_sampleTimestamp = 123;
        m_priority = 0;
    }

    virtual void TearDown()
    {
        WRTP_INFOTRACE("CFrameSmoothSendingBufferASTest:teardown");
    }
public:
    CSendFramePtr CreateNewFrameUnit()
    {
        return CreateFrameUnit(m_message,
                               m_channelId,
                               m_ssrc,
                               m_payloadType,
                               m_clockRate,
                               m_timeoffsetExtId,
                               m_captureTimestamp,
                               m_sampleTimestamp,
                               m_priority,
                               m_sessionContext);

    }


public:
    static const char *m_message;
    uint32_t m_channelId;
    uint32_t m_ssrc;
    uint8_t m_payloadType;
    uint32_t m_clockRate;
    uint32_t m_timeoffsetExtId;
    uint32_t m_captureTimestamp;
    uint32_t m_sampleTimestamp;
    uint8_t m_priority;
    RTPSessionContextSharedPtr m_sessionContext;
    CCmComAutoPtr<CTokenBucket> m_parentToken;
    CSmoothSendingSchedulerSharedPtr m_smoothSendingScheduler;
};

const char *CFrameSmoothSendingBufferASTest::m_message = "a message to test frame sending controller";
#if 1
//annotated by Joyce

TEST_F(CFrameSmoothSendingBufferASTest, Test_InputData_WhenForwardDataSuccess_Then_EmptyBuffer)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendBufferAS smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.SetParentTokenBucket(m_parentToken.Get());
    auto frame = CreateNewFrameUnit();
    ASSERT_TRUE(!!frame);

    CM_INFO_TRACE("before InputData");

    int ret = smoothSendingBuffer.InputData(std::move(frame));
    ASSERT_EQ(0, ret);

    CM_INFO_TRACE("after InputData");

    WRTP_TEST_TRIGGER_ON_TIMER(20, 3);
    ASSERT_EQ(m_channelId, sink.m_channelId);
}

TEST_F(CFrameSmoothSendingBufferASTest, Test_InputData_WhenForwardDataFailed_Then_NotEmptyBuffer)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendBufferAS smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    auto frame = CreateNewFrameUnit();
    ASSERT_TRUE(!!frame);

    sink.m_forwardDataResult = -1;
    int ret = smoothSendingBuffer.InputData(std::move(frame));
    ASSERT_EQ(0, ret);
    //::SleepMs(500);
    ASSERT_EQ(0, sink.m_channelId);
    //ASSERT_EQ(1, smoothSendingBuffer.GetCurrentBufferSize());
}
#endif

class CFrameSmoothSendingBufferVideoMock
    : public CFrameSmoothSendBufferVideo
{
public:
    CFrameSmoothSendingBufferVideoMock(IFrameSendBufferSink &sendBufferSink, uint32_t initBandwidth)
        : CFrameSmoothSendBufferVideo(sendBufferSink, initBandwidth)
    {
        m_DisableOutput = false;
    }

    uint32_t DropTimeoutLowPriorityFrames(uint8_t frameType, uint8_t priority, uint32_t  timestamp, uint32_t &maxBufferedMs)
    {
        return CFrameSmoothSendBufferVideo::DropTimeoutLowPriorityFrames(frameType, priority, timestamp, maxBufferedMs);
    }

    uint32_t TryOutputDataImpl(bool probing = false)
    {
        if (true == m_DisableOutput) {
            return 0;
        }
        return CFrameSmoothSendBuffer::TryOutputDataImpl(probing);
    }

    bool GetTOBreakFlag()
    {
        return m_bTOBreak;
    }

    int GetBufferSize()
    {
        return (int)m_buffer.size();
    }

public:
    bool m_DisableOutput;
};

class CFrameSmoothSendingBufferVideoTest
    : public ::testing::Test
{
public:
    CFrameSmoothSendingBufferVideoTest()
        :m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
        m_smoothSendingScheduler = CCmMakeShared<CSmoothSendingScheduler>();
        m_smoothSendingScheduler->Start("smoothsend", false, TT_RTP_SMOOTH_SEND);
        m_sessionContext->GetOutboundConfig()->SetSmoothSendingScheduler(m_smoothSendingScheduler);
    }
    ~CFrameSmoothSendingBufferVideoTest()
    {
        if (m_smoothSendingScheduler) {
            m_smoothSendingScheduler->Stop();
            m_smoothSendingScheduler.reset();
        }
    }
    virtual void SetUp()
    {
        WRTP_INFOTRACE("CFrameSmoothSendingBufferVideoTest:setup");
        m_channelId = 3;
        m_ssrc =  123;
        m_payloadType = 89;
        m_clockRate = 90000;
        m_timeoffsetExtId = 2;
        m_sampleTimestamp = 123;
    }

    virtual void TearDown()
    {
        WRTP_INFOTRACE("CFrameSmoothSendingBufferVideoTest:teardown");

    }
public:
    CSendFramePtr CreateNewFrameUnit(uint8_t priority, uint32_t captureTimestamp)
    {
        return CreateFrameUnit(m_message,
                               m_channelId,
                               m_ssrc,
                               m_payloadType,
                               m_clockRate,
                               m_timeoffsetExtId,
                               captureTimestamp,
                               m_sampleTimestamp,
                               priority,
                               m_sessionContext);

    }

public:
    static const char *m_message;
    uint32_t m_channelId;
    uint32_t m_ssrc;
    uint8_t m_payloadType;
    uint32_t m_clockRate;
    uint8_t m_timeoffsetExtId;
    uint32_t m_sampleTimestamp;
    RTPSessionContextSharedPtr m_sessionContext;
    CSmoothSendingSchedulerSharedPtr m_smoothSendingScheduler;
};

const char *CFrameSmoothSendingBufferVideoTest::m_message = "a message to test frame sending controller";
#if 1
//annotated by Joyce
TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenBufferIsEmpty_ThenReturn0)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    uint32_t bufferedMs = 0;
    ASSERT_EQ(0, smoothSendingBuffer.DropTimeoutLowPriorityFrames(0, 0, 0, bufferedMs));
    ASSERT_EQ(0, bufferedMs);
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenBufferOnlyOneFrame_ThenReturn0)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;

    auto frame = CreateNewFrameUnit(7, TickNowMS());
    int ret = smoothSendingBuffer.InputData(std::move(frame));
    ASSERT_EQ(0, ret);
    //::SleepMs(500);
    uint32_t bufferedMs = 0;
    ASSERT_EQ(0, smoothSendingBuffer.DropTimeoutLowPriorityFrames(0, 0, 0, bufferedMs));
    ASSERT_EQ(0, bufferedMs);
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_P0_WhenBufferMoreThanOneFrame_ThenReturnDroppedFramesCount)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;

    uint32_t captureTimestamp = TickNowMS();

    auto frame = CreateNewFrameUnit(1, captureTimestamp-5);
    int ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-4);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(3, captureTimestamp-3);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-2);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    //::SleepMs(500);
    uint32_t bufferedMs = 0;
    ASSERT_EQ(4, smoothSendingBuffer.DropTimeoutLowPriorityFrames(1, 0, captureTimestamp+VIRTUAL_BUFFER_LENGTH, bufferedMs));
    ASSERT_EQ(1, smoothSendingBuffer.GetBufferSize());
    ASSERT_FALSE(smoothSendingBuffer.GetTOBreakFlag());
    ASSERT_EQ(VIRTUAL_BUFFER_LENGTH+5, bufferedMs);
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenBufferMoreThanOneFrame_ThenReturnDroppedFramesCount)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;

    uint32_t captureTimestamp = TickNowMS();

    auto frame = CreateNewFrameUnit(1, captureTimestamp-5);
    int ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-4);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(13, captureTimestamp-3);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(7, captureTimestamp-2);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(5, captureTimestamp);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    //::SleepMs(500);
    uint32_t bufferedMs = 0;
    ASSERT_EQ(2, smoothSendingBuffer.DropTimeoutLowPriorityFrames(3, 5, captureTimestamp+VIRTUAL_BUFFER_LENGTH,bufferedMs));
    ASSERT_EQ(3, smoothSendingBuffer.GetBufferSize());
    ASSERT_EQ(VIRTUAL_BUFFER_LENGTH+5, bufferedMs);
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenBufferMoreThanOneFrame_ThenReturnRightMaxBufferMs)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;

    uint32_t captureTimestamp = TickNowMS()-VIRTUAL_BUFFER_LENGTH;

    auto frame = CreateNewFrameUnit(0, captureTimestamp-500);
    int ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-400);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-300);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-200);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    uint32_t bufferedMs = 0;
    ASSERT_EQ(0, smoothSendingBuffer.DropTimeoutLowPriorityFrames(3, 5, captureTimestamp,bufferedMs));
    ASSERT_EQ(500, bufferedMs);
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenInputNotIDRAfterT0Break_ThenDropNotIDRFrames)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;
    ASSERT_FALSE(smoothSendingBuffer.GetTOBreakFlag());

    uint32_t captureTimestamp = TickNowMS();

    auto frame = CreateNewFrameUnit(1, captureTimestamp-5);
    int ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    frame = CreateNewFrameUnit(0, captureTimestamp-4);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    uint32_t bufferedMs = 0;
    ASSERT_EQ(2, smoothSendingBuffer.DropTimeoutLowPriorityFrames(0, 0, captureTimestamp+VIRTUAL_BUFFER_LENGTH, bufferedMs));
    ASSERT_TRUE(smoothSendingBuffer.GetTOBreakFlag());
    ASSERT_TRUE(sink.m_bT0FrameDropped);
    ASSERT_EQ(1, smoothSendingBuffer.GetBufferSize());

    frame = CreateNewFrameUnit(1, captureTimestamp+VIRTUAL_BUFFER_LENGTH);
    ret = smoothSendingBuffer.OnInputData(std::move(frame));
    ASSERT_EQ(0, ret);

    ASSERT_EQ(1, smoothSendingBuffer.GetBufferSize());
    ASSERT_TRUE(smoothSendingBuffer.GetTOBreakFlag());
}

TEST_F(CFrameSmoothSendingBufferVideoTest, Test_DropTimeoutLowPriorityFrames_WhenInputIDRAfterT0Break_ThenBufferIDRFrameAndToBreakRecovered)
{
    CScopedTracer test_info;
    MockFrameSendingBufferSink sink;
    CFrameSmoothSendingBufferVideoMock smoothSendingBuffer(sink, WRTP_SESSION_INITIAL_BANDWIDTH);
    smoothSendingBuffer.m_DisableOutput = true;
    ASSERT_FALSE(smoothSendingBuffer.GetTOBreakFlag());

    uint32_t captureTimestamp = TickNowMS();

    auto frame = CreateNewFrameUnit(1, captureTimestamp-5);
    ASSERT_EQ(0, smoothSendingBuffer.OnInputData(std::move(frame)));

    frame = CreateNewFrameUnit(0, captureTimestamp-4);
    ASSERT_EQ(0, smoothSendingBuffer.OnInputData(std::move(frame)));

    uint32_t bufferedMs = 0;
    ASSERT_EQ(2, smoothSendingBuffer.DropTimeoutLowPriorityFrames(0, 0, captureTimestamp+VIRTUAL_BUFFER_LENGTH, bufferedMs));
    ASSERT_TRUE(smoothSendingBuffer.GetTOBreakFlag());
    ASSERT_TRUE(sink.m_bT0FrameDropped);
    ASSERT_EQ(1, smoothSendingBuffer.GetBufferSize());

    frame = CreateNewFrameUnit(0, captureTimestamp+VIRTUAL_BUFFER_LENGTH);
    auto& metaInfo = frame->GetMediaMetaInfo();
    metaInfo.SetFrameType(1);
    ASSERT_EQ(0, smoothSendingBuffer.OnInputData(std::move(frame)));

    ASSERT_EQ(2, smoothSendingBuffer.GetBufferSize());
    ASSERT_FALSE(smoothSendingBuffer.GetTOBreakFlag());
}

#endif
