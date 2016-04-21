#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "sendcontrol.h"
#include "rtpsessionconfig.h"
#include "CmThreadManager.h"
#include "rtputils.h"
#include "testutil.h"

using namespace wrtp;
using ::testing::_;
using ::testing::AtLeast;

//#define FRAME_UNIT_LEN_SMALL (640 * 1024)
// old value is 640*1024, we change default maxDelay from 600ms to 800ms, so new value is (640*1024*(800/600))
#define FRAME_UNIT_LEN_SMALL (640*1024*VIRTUAL_BUFFER_LENGTH/600)
#define FRAME_UNIT_LEN_LARGE (2 * FRAME_UNIT_LEN_SMALL)

#define BANDWIDTH_RATE_SMALL  (1024 * 1024)
#define BANDWIDTH_RATE_MIDDLE (2 * BANDWIDTH_RATE_SMALL)
#define BANDWIDTH_RATE_LARGE  (6 * BANDWIDTH_RATE_SMALL)

#define MAX_DELAY_SMALL   100  //  100ms
#define MAX_DELAY_LARGE   1200 //  1200ms

char data_small[FRAME_UNIT_LEN_SMALL] = {0};
char data_large[FRAME_UNIT_LEN_LARGE] = {0};

class CSendControlSinkAPIMock : public ISendingControlSink
{
public:
    CSendControlSinkAPIMock()
        : m_bOutput(false)
        , m_bSmoothed(false)
    {
    }

    virtual int32_t OnPacketSmoothed(uint32_t msid)
    {
        m_bSmoothed = true;
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
    virtual int32_t OnOutputData(CCmMessageBlock &, const CRTPPacketMetaInfo&, const CMediaMetaInfo*)
    {
        m_bOutput = true;
        return 0;
    }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }
public:
    FilterResult m_filterType;
    bool m_bOutput;
    bool m_bSmoothed;
};

class CSendControlAPITest : public ::testing::Test
{
protected:
    CSendControlAPITest()
        : m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
        m_smoothSendingScheduler = CCmMakeShared<CSmoothSendingScheduler>();
        m_smoothSendingScheduler->Start("smoothsend", false, TT_RTP_SMOOTH_SEND);
        m_sessionContext->GetOutboundConfig()->SetSmoothSendingScheduler(m_smoothSendingScheduler);
    }
    ~CSendControlAPITest()
    {
        if (m_smoothSendingScheduler) {
            m_smoothSendingScheduler->Stop();
            m_smoothSendingScheduler.reset();
        }
    }
    virtual void SetUp()
    {
        m_pSendControl = ISendingControl::CreateSendingController(&m_sink);
    }
    virtual void TearDown()
    {
        if (nullptr != m_pSendControl) {
            m_pSendControl->Destroy();
            m_pSendControl = nullptr;
        }
    }
    CSendControlSinkAPIMock m_sink;
    ISendingControl *m_pSendControl;
    RTPSessionContextSharedPtr m_sessionContext;
    CSmoothSendingSchedulerSharedPtr m_smoothSendingScheduler;
};

TEST_F(CSendControlAPITest, create_sendcontrol_successful)
{
    CScopedTracer test_info;
    CSendControlSinkAPIMock m_sink;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(&m_sink);
    EXPECT_TRUE(pSendControl != nullptr);
}

TEST_F(CSendControlAPITest, create_sendcontrol_with_null_sink_fail)
{
    CScopedTracer test_info;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(nullptr);
    EXPECT_TRUE(pSendControl == nullptr);
}

TEST_F(CSendControlAPITest, SendFrameData_small_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_large_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);
    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_0F_unknown)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);
    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0x0F,m_sessionContext);

    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_UNKNOWN, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_large_with_largeBW_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_LARGE);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_small_with_smallBW_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_SMALL);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_p0_with_disable_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    m_pSendControl->EnableDropP0(false);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_p0_with_enable_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    m_pSendControl->EnableDropP0(true);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_drop_after_p0_with_enable_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    m_pSendControl->EnableDropP0(true);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_p0_pass_after_p0_with_enable_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    m_pSendControl->EnableDropP0(true);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_pass_after_p0_pass_after_p0_with_enable_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    m_pSendControl->EnableDropP0(true);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    //::SleepMs(1000);
    WRTP_TEST_TRIGGER_ON_TIMER(1000, 1);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),0,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    //::SleepMs(1000);
    WRTP_TEST_TRIGGER_ON_TIMER(1000, 1);

    auto pFrameUnit3 = ConstructFrameUnit(1,4,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal3 = pFrameUnit3->CreateNAL();
    nal3->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag3 = nal3->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag3->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit3));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_large_with_reset_large_threshold_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    m_pSendControl->ResetLevelThreshold(BANDWIDTH_RATE_LARGE);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_large_with_reset_small_threshold_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    m_pSendControl->ResetLevelThreshold(BANDWIDTH_RATE_SMALL);

    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_large_with_large_delay_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);

    m_pSendControl->SetMaxDelay(MAX_DELAY_LARGE);

    //::SleepMs(500);
    WRTP_TEST_TRIGGER_ON_TIMER(500, 1);
    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    //::SleepMs(500);
    WRTP_TEST_TRIGGER_ON_TIMER(500, 1);
    auto pFrameUnit3 = ConstructFrameUnit(1,4,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal3 = pFrameUnit3->CreateNAL();
    nal3->SetDataSize(FRAME_UNIT_LEN_LARGE);
    auto &frag3 = nal3->CreateFragment(FRAME_UNIT_LEN_LARGE, m_sessionContext->GetFrameManager());
    frag3->CopyDataIn((const uint8_t *)data_large,FRAME_UNIT_LEN_LARGE);

    m_pSendControl->SendFrameData(std::move(pFrameUnit3));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_small_with_small_delay_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    //::SleepMs(500);
    WRTP_TEST_TRIGGER_ON_TIMER(500, 1);
    auto pFrameUnit2 = ConstructFrameUnit(1,3,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    m_pSendControl->SetMaxDelay(MAX_DELAY_SMALL);

    auto pFrameUnit3 = ConstructFrameUnit(1,4,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal3 = pFrameUnit3->CreateNAL();
    nal3->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag3 = nal3->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag3->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);

    m_pSendControl->SendFrameData(std::move(pFrameUnit3));
    EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}

#define MAX_PACKET_SIZE 1200
TEST_F(CSendControlAPITest, SendFrameData_small_outputdata)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);
    m_pSendControl->SetMaxPacketSize(MAX_PACKET_SIZE);//Set MTU

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);
    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(MAX_PACKET_SIZE);
    auto &frag = nal->CreateFragment(MAX_PACKET_SIZE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,MAX_PACKET_SIZE);

    ::SleepMs(100);
    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
    //::SleepMs(200);
    //EXPECT_EQ(true, m_sink.m_bOutput);
    //WRTP_TEST_WAIT_UNTIL(20, 50, (true == m_sink.m_bOutput));
    EXPECT_TRUE(true == m_sink.m_bOutput);
}

#if 1
//annotated by Joyce---Fix it later(time reference)
TEST_F(CSendControlAPITest, SendFrameData_small_smoothed)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_MIDDLE);

    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

    auto &nal = pFrameUnit->CreateNAL();
    nal->SetDataSize(MAX_PACKET_SIZE);
    auto &frag = nal->CreateFragment(MAX_PACKET_SIZE, m_sessionContext->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data_small,MAX_PACKET_SIZE);

    ::SleepMs(100);
    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
    //    ::SleepMs(100);
    //    EXPECT_EQ(true, m_sink.m_bSmoothed);
    //WRTP_TEST_WAIT_UNTIL(20, 50, (true == m_sink.m_bSmoothed));
    EXPECT_TRUE(true == m_sink.m_bSmoothed);
}
#endif
TEST_F(CSendControlAPITest, SendFrameData_small_regularly_slow_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_LARGE);

    for (int i = 1; i <= 10; ++i) {
        auto pFrameUnit = ConstructFrameUnit(i,2,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

        auto &nal = pFrameUnit->CreateNAL();
        nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
        auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
        frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
        m_pSendControl->SendFrameData(std::move(pFrameUnit));
        EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
        //::SleepMs(100);
        WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    }
}

TEST_F(CSendControlAPITest, SendFrameData_small_regularly_fast_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_LARGE);

    bool bDrop = false;
    for (int i = 1; i <= 10; ++i) {
        auto pFrameUnit = ConstructFrameUnit(1,i,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

        auto &nal = pFrameUnit->CreateNAL();
        nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
        auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
        frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
        m_pSendControl->SendFrameData(std::move(pFrameUnit));
        if (m_sink.m_filterType == FILTER_RESULT_DROPPED) {
            bDrop = true;
            break;
        }
        ::SleepMs(10);
    }
    EXPECT_EQ(true, bDrop);
}

TEST_F(CSendControlAPITest, SendFrameData_later_pass_after_small_regularly_fast_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_LARGE);

    bool bDrop = false;
    for (int i = 1; i <= 10; ++i) {
        auto pFrameUnit = ConstructFrameUnit(1,i,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

        auto &nal = pFrameUnit->CreateNAL();
        nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
        auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
        frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
        m_pSendControl->SendFrameData(std::move(pFrameUnit));
        if (m_sink.m_filterType == FILTER_RESULT_DROPPED) {
            bDrop = true;
            break;
        }
        ::SleepMs(10);
    }
    EXPECT_EQ(true, bDrop);

    //::SleepMs(500);
    WRTP_TEST_TRIGGER_ON_TIMER(500, 1);
    auto pFrameUnit2 = ConstructFrameUnit(1,11,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITest, SendFrameData_high_priority_pass_after_low_priority_regularly_fast_drop)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(BANDWIDTH_RATE_LARGE);

    bool bDrop = false;
    for (int i = 1; i <= 10; ++i) {
        auto pFrameUnit = ConstructFrameUnit(1,i,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),8,m_sessionContext);

        auto &nal = pFrameUnit->CreateNAL();
        nal->SetDataSize(FRAME_UNIT_LEN_SMALL);
        auto &frag = nal->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
        frag->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
        m_pSendControl->SendFrameData(std::move(pFrameUnit));
        if (m_sink.m_filterType == FILTER_RESULT_DROPPED) {
            bDrop = true;
            break;
        }
        ::SleepMs(10);
    }
    EXPECT_EQ(true, bDrop);

    auto pFrameUnit2 = ConstructFrameUnit(1,11,RTP_SESSION_WEBEX_VIDEO,1,1,TickNowMS(),TickNowMS(),1,m_sessionContext);

    auto &nal2 = pFrameUnit2->CreateNAL();
    nal2->SetDataSize(FRAME_UNIT_LEN_SMALL);
    auto &frag2 = nal2->CreateFragment(FRAME_UNIT_LEN_SMALL, m_sessionContext->GetFrameManager());
    frag2->CopyDataIn((const uint8_t *)data_small,FRAME_UNIT_LEN_SMALL);
    m_pSendControl->SendFrameData(std::move(pFrameUnit2));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}
