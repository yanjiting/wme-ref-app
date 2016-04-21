#include "gmock/gmock.h"
#include "gtest/gtest.h"
//#include "CmThreadManager.h"
#include "sendcontrol.h"
#include "rtpsessionconfig.h"
#include "testutil.h"

using namespace wrtp;

#define FRAME_UNIT_LEN (1 * 1024 * 8) // 8kbps, 1kBps
//CASE DEFINE
#define BANDWIDTH_RATE (10 * FRAME_UNIT_LEN) // 80kbps, 8kBps

#define TIME_ECLIPSE 10 //ms

static char data[FRAME_UNIT_LEN] = {0};

class CSendControlSinkMock : public ISendingControlSink
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
        m_filterType = sType;
    }

    //virtual void OnDataDropped(int32_t eFrameIdc, int32_t iPriority , uint32_t dwTimeStamp) = 0;

    virtual void OnDataDropped(const FilterVideoDataInfo &filterInfo)
    {

    }
    virtual int32_t OnOutputData(CCmMessageBlock &mb, const CRTPPacketMetaInfo& pktInfo, const CMediaMetaInfo* mediaInfo)
    {
        return 0;
    }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }

public:
    FilterResult m_filterType;
};

class CSendControlTest : public ::testing::Test
{
protected:
    CSendControlTest()
        : m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
        m_smoothSendingScheduler = CCmMakeShared<CSmoothSendingScheduler>();
        m_smoothSendingScheduler->Start("smoothsend", false, TT_RTP_SMOOTH_SEND);
        m_sessionContext->GetOutboundConfig()->SetSmoothSendingScheduler(m_smoothSendingScheduler);
    }
    ~CSendControlTest()
    {
        if (m_smoothSendingScheduler) {
            m_smoothSendingScheduler->Stop();
            m_smoothSendingScheduler.reset();
        }
    }
    virtual void SetUp()
    {
        m_pSendControl = std::unique_ptr<CSendController>(new CSendController(&sink, RTP_SESSION_WEBEX_VIDEO));
    }
    virtual void TearDown()
    {
        m_pSendControl.reset();
    }
    CSendFramePtr CreateTestFrame()
    {
        CSendFramePtr frame = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);
        auto &nal = frame->CreateNAL();
        auto &fragments = nal->CreateFragment(FRAME_UNIT_LEN, 1, m_sessionContext->GetFrameManager());
        fragments[0]->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        return frame;
    }
    
    CSendControlSinkMock sink;
    std::unique_ptr<CSendController> m_pSendControl;
    RTPSessionContextSharedPtr m_sessionContext;
    CSmoothSendingSchedulerSharedPtr m_smoothSendingScheduler;
};

TEST_F(CSendControlTest, create_sendcontrol_successful)
{
    CScopedTracer test_info;
    CSendControlSinkMock sink;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(&sink);
    EXPECT_TRUE(pSendControl != nullptr);
    if (pSendControl) {
        pSendControl->Destroy();
    }
}
TEST_F(CSendControlTest, create_sendcontrol_with_null_sink_fail)
{
    CScopedTracer test_info;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(nullptr);
    EXPECT_TRUE(pSendControl == nullptr);
}
TEST_F(CSendControlTest, SendFrameData_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    m_pSendControl->SetBandwidth(2 * BANDWIDTH_RATE);
    EXPECT_EQ(ret,0);
    //EXPECT_CALL(sink,OnPacketFiltered(_,_,_,_,_,_));
    m_pSendControl->SendFrameData(std::move(CreateTestFrame()));
    EXPECT_EQ(FILTER_RESULT_PASSED, sink.m_filterType);

}
// TEST_F(CSendControlTest, SendFrameData_filtered)
// {
//     uint32_t ret = m_pSendControl->Initalize(BANDWIDTH_RATE, 1);
//     EXPECT_EQ(ret,0);
//     //EXPECT_CALL(sink,OnPacketFiltered(_,_,_,_,_,_));
//     m_pSendControl->SendFrameData(pFrameUnit);
//     EXPECT_EQ(FILTER_RESULT_DROPPED, sink.m_filterType);
// }

#if 0
//annotated by Joyce--These cases haven't check anything,fix them later
TEST_F(CSendControlTest, SendFrameData_regularly_with_large_buffer)
{
    CScopedTracer test_info;
    uint32_t ret = pSendControl->Initialize(10 * BANDWIDTH_RATE, false, 0, 1);
    EXPECT_EQ(ret,0);

    for (int i = 1; i <= 10; ++i) {
        CFrameUnit *pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);

        CNALUnit *nal = pFrameUnit->CreateNAL();
        CFragmentUnit *frag = nal->CreateFragment(FRAME_UNIT_LEN);
        frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        m_pSendControl->SendFrameData(pFrameUnit);
        ::SleepMs(100);
    }
}

TEST_F(CSendControlTest, SendFrameData_regularly_with_small_buffer)
{
    CScopedTracer test_info;
    uint32_t ret = pSendControl->Initialize(2 * BANDWIDTH_RATE, false, 0, 1);
    EXPECT_EQ(ret,0);

    for (int i = 1; i <= 10; ++i) {
        CFrameUnit *pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);

        CNALUnit *nal = pFrameUnit->CreateNAL();
        CFragmentUnit *frag = nal->CreateFragment(FRAME_UNIT_LEN);
        frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        m_pSendControl->SendFrameData(pFrameUnit);
        ::SleepMs(100);
    }
}

TEST_F(CSendControlTest, SendFrameData_reset_rate)
{
    CScopedTracer test_info;
    uint32_t ret = pSendControl->Initialize(10 * BANDWIDTH_RATE, false, 0, 1);
    EXPECT_EQ(ret,0);

    for (int i = 1; i <= 10; ++i) {
        CFrameUnit *pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);

        CNALUnit *nal = pFrameUnit->CreateNAL();
        CFragmentUnit *frag = nal->CreateFragment(FRAME_UNIT_LEN);
        frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        m_pSendControl->SendFrameData(pFrameUnit);
        ::SleepMs(100);
        if (i ==6) {
            pSendControl->SetBandwidth(5 * BANDWIDTH_RATE);
        }
    }
}

TEST_F(CSendControlTest, SendFrameData_reset_threshold_zero)
{
    CScopedTracer test_info;
    uint32_t ret = pSendControl->Initialize(10 * BANDWIDTH_RATE, false, 0, 1);
    EXPECT_EQ(ret,0);

    for (int i = 1; i <= 10; ++i) {
        CFrameUnit *pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);

        CNALUnit *nal = pFrameUnit->CreateNAL();
        CFragmentUnit *frag = nal->CreateFragment(FRAME_UNIT_LEN);
        frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        m_pSendControl->SendFrameData(pFrameUnit);
        ::SleepMs(100);
        if (i ==5) {
            pSendControl->SetBandwidth(5 * BANDWIDTH_RATE);
        }
        if (i ==8) {
            pSendControl->ResetLevelThreshold(0);
        }
    }
}

TEST_F(CSendControlTest, SendFrameData_reset_threshold_default)
{
    CScopedTracer test_info;
    uint32_t ret = pSendControl->Initialize(10 * BANDWIDTH_RATE, false, 0, 1);
    EXPECT_EQ(ret,0);

    for (int i = 1; i <= 10; ++i) {
        CFrameUnit *pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,123,456,1,m_sessionContext);

        CNALUnit *nal = pFrameUnit->CreateNAL();
        CFragmentUnit *frag = nal->CreateFragment(FRAME_UNIT_LEN);
        frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);
        m_pSendControl->SendFrameData(pFrameUnit);
        ::SleepMs(100);
        if (i ==5) {
            pSendControl->SetBandwidth(5 * BANDWIDTH_RATE);
        }
        if (i ==8) {
            pSendControl->ResetLevelThreshold();
        }
    }
}
#endif

class MediaEngineMockForSendController : public ISendingFilterSink
{
public:
    MediaEngineMockForSendController()
    {
        m_msid = 0;
        m_result = FILTER_RESULT_UNKNOWN;
        m_totalLen = 0;
        m_infoDataLen = 0;
    }
    virtual ~MediaEngineMockForSendController()
    {
        m_infoData.reset();
    }
public:
    virtual void OnSendingFiltered(const StreamIdInfo &streamId, FilterResult result, uint32_t totalLen, uint8_t *infoData, uint32_t infoDataLen)
    {
        ASSERT_TRUE(nullptr != infoData);
        EXPECT_TRUE(0 != infoDataLen);

        m_msid = streamId.trackId;
        m_result = result;
        m_totalLen = totalLen;
        m_infoDataLen = infoDataLen;

        m_infoData.reset(new uint8_t[m_infoDataLen]);
        ASSERT_TRUE(nullptr != m_infoData);
        memcpy(m_infoData.get(), infoData, infoDataLen);
    }

    virtual void OnSendingStatusChanged(const StreamIdInfo &streamId, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {

    }
public:
    uint32_t m_msid;
    FilterResult m_result;
    uint32_t m_totalLen;
    std::unique_ptr<uint8_t[]> m_infoData;
    uint32_t m_infoDataLen;
};

class SendingControlSinkMock_978: public ISendingControlSink
{
public:
    SendingControlSinkMock_978(ISendingFilterSink *filterSink) : m_filterSink(filterSink) {}
    virtual int32_t OnPacketSmoothed(uint32_t msid) { return 0; }
    virtual int32_t OnT0FrameDropped(uint32_t ssrc) { return 0; }
    virtual void OnPacketFiltered(uint32_t msid, int32_t dwTimeStamp, FilterResult sType, int32_t nLen, uint8_t ucPriority, FilterVideoDataInfo *infoData, BOOL bFECFlag)
    {
        if (m_filterSink) {
            uint32_t infoDataLen = sizeof(*infoData);
            StreamIdInfo streamId;
            streamId.trackId = msid;
            m_filterSink->OnSendingFiltered(streamId, sType, nLen, (uint8_t *)infoData, infoDataLen);
        }
    }
    virtual void OnDataDropped(const FilterVideoDataInfo &filterInfo) {}
    virtual int32_t OnOutputData(CCmMessageBlock &, const CRTPPacketMetaInfo& , const CMediaMetaInfo* ) { return 0; }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
        if (m_filterSink) {
            StreamIdInfo streamId;
            streamId.trackId = msid;
            m_filterSink->OnSendingStatusChanged(streamId, currentBufferSize, totalBufferSize);
        }
    }

private:
    ISendingFilterSink *m_filterSink;
};

TEST_F(CSendControlTest, Test_SendingFilter__PASSED_callback_one_NAL)
{
    CScopedTracer test_info;

    MediaEngineMockForSendController meMock;
    SendingControlSinkMock_978 sendControllerSink{&meMock};
    std::unique_ptr<CSendController> sendController(new CSendController(&sendControllerSink, RTP_SESSION_WEBEX_VIDEO));
    sendController->Initialize();
    sendController->EnableDropP0(true);

    uint32_t channleId = 1;
    uint32_t ssrc = 2;
    uint8_t payloadType = RTP_SESSION_WEBEX_VIDEO;
    uint32_t clockRate = 1;
    uint8_t timeoffsetExtId = 1;
    uint32_t captureTimestamp = 123;
    uint32_t sampleTimestamp = 456;
    uint8_t priority = 1;

    auto pFrameUnit = ConstructFrameUnit(channleId, ssrc, payloadType, clockRate, timeoffsetExtId, captureTimestamp, sampleTimestamp, priority, m_sessionContext);
    auto& metaInfo = pFrameUnit->GetMediaMetaInfo();
    ASSERT_TRUE(nullptr != pFrameUnit);
    auto &nal = pFrameUnit->CreateNAL();
    ASSERT_TRUE(nullptr != nal);

    auto& fragments = nal->CreateFragment(234, 2, m_sessionContext->GetFrameManager());
    ASSERT_TRUE(nullptr != fragments[0]);
    fragments[0]->SetDataSize(15);
    ASSERT_TRUE(nullptr != fragments[1]);
    fragments[1]->SetDataSize(25);

    nal->SetDataSize(40);
    metaInfo.SetFrameIDC(1);

    uint32_t meFrame_idc  = metaInfo.GetFrameIDC();
    uint32_t meTimestamp  = metaInfo.GetCaptureTickMS();
    uint8_t meDID         = metaInfo.GetDID();
    uint8_t mePriority    = metaInfo.GetPriority();
    uint32_t meTotalDatalength = pFrameUnit->GetTotalDataLength();

    int res = sendController->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(0, res);

    FilterVideoDataInfo *infoData = (FilterVideoDataInfo *)meMock.m_infoData.get();
    ASSERT_TRUE(nullptr != infoData);

    EXPECT_EQ(meDID, infoData->DID);
    EXPECT_EQ(meFrame_idc, infoData->frame_idc);
    EXPECT_EQ(meTimestamp, infoData->timestamp);
    EXPECT_EQ(mePriority, infoData->priority);
    EXPECT_EQ(meTotalDatalength, infoData->dataLength);

    EXPECT_EQ(channleId, meMock.m_msid);
    EXPECT_EQ(FILTER_RESULT_PASSED, meMock.m_result);

    EXPECT_EQ(meTotalDatalength, meMock.m_totalLen);
    EXPECT_EQ(sizeof(FilterVideoDataInfo), meMock.m_infoDataLen);
}

TEST_F(CSendControlTest, Test_SendingFilter__PASSED_callback_Multi_NAL)
{
    CScopedTracer test_info;

    MediaEngineMockForSendController meMock;
    SendingControlSinkMock_978 sendControllerSink{&meMock};
    std::unique_ptr<CSendController> sendController(new CSendController(&sendControllerSink, RTP_SESSION_WEBEX_VIDEO));
    sendController->Initialize();
    sendController->EnableDropP0(true);

    uint32_t channleId = 1;
    uint32_t ssrc = 2;
    uint8_t payloadType = RTP_SESSION_WEBEX_VIDEO;
    uint32_t clockRate = 1;
    uint8_t timeoffsetExtId = 1;
    uint32_t captureTimestamp = 123;
    uint32_t sampleTimestamp = 456;
    uint8_t priority = 1;
    auto pFrameUnit = ConstructFrameUnit(channleId, ssrc, payloadType, clockRate, timeoffsetExtId, captureTimestamp, sampleTimestamp, priority, m_sessionContext);
    auto& metaInfo = pFrameUnit->GetMediaMetaInfo();
    ASSERT_TRUE(nullptr != pFrameUnit);

    auto &nal1 = pFrameUnit->CreateNAL();
    ASSERT_TRUE(nullptr != nal1);
    nal1->SetDataSize(5);

    auto& fragments1 = nal1->CreateFragment(234, 2, m_sessionContext->GetFrameManager());
    ASSERT_TRUE(nullptr != fragments1[0]);
    fragments1[0]->SetDataSize(5);
    ASSERT_TRUE(nullptr != fragments1[1]);
    fragments1[1]->SetDataSize(5);

    auto &nal2 = pFrameUnit->CreateNAL();
    ASSERT_TRUE(nullptr != nal2);
    nal2->SetDataSize(5);

    auto& fragments2 = nal2->CreateFragment(234, 2, m_sessionContext->GetFrameManager());
    ASSERT_TRUE(nullptr != fragments2[0]);
    fragments2[0]->SetDataSize(5);
    ASSERT_TRUE(nullptr != fragments2[1]);
    fragments2[1]->SetDataSize(5);

    auto &nal3 = pFrameUnit->CreateNAL();
    ASSERT_TRUE(nullptr != nal3);
    nal3->SetDataSize(10);

    auto& fragments3 = nal3->CreateFragment(234, 2, m_sessionContext->GetFrameManager());
    ASSERT_TRUE(nullptr != fragments3[0]);
    fragments3[0]->SetDataSize(5);
    ASSERT_TRUE(nullptr != fragments3[1]);
    fragments3[1]->SetDataSize(5);

    ASSERT_FALSE(pFrameUnit->IsEmpty());
    //EXPECT_EQ(3, pFrameUnit->GetNALs().size());

    metaInfo.SetFrameIDC(1);

    uint32_t meFrame_idc  = metaInfo.GetFrameIDC();
    uint32_t meTimestamp  = metaInfo.GetCaptureTickMS();
    uint8_t meDID         = metaInfo.GetDID();
    uint8_t mePriority    = metaInfo.GetPriority();
    uint32_t meTotalDatalength = pFrameUnit->GetTotalDataLength();

    int res = sendController->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(0, res);

    FilterVideoDataInfo *infoData = (FilterVideoDataInfo *)meMock.m_infoData.get();
    ASSERT_TRUE(nullptr != infoData);

    EXPECT_EQ(meDID, infoData->DID);
    EXPECT_EQ(meFrame_idc, infoData->frame_idc);
    EXPECT_EQ(meTimestamp, infoData->timestamp);
    EXPECT_EQ(mePriority, infoData->priority);
    EXPECT_EQ(meTotalDatalength, infoData->dataLength);

    EXPECT_EQ(channleId, meMock.m_msid);
    EXPECT_EQ(FILTER_RESULT_PASSED, meMock.m_result);
    EXPECT_EQ(meTotalDatalength, meMock.m_totalLen);
    EXPECT_EQ(sizeof(FilterVideoDataInfo), meMock.m_infoDataLen);
}
