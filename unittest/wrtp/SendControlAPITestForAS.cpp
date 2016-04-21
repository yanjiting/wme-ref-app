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

#define FRAME_UNIT_LEN (640 * 1024)

#define BANDWIDTH_RATE  (1024 * 1024)

static char data[FRAME_UNIT_LEN] = {0};

class CSendControlForASSinkAPIMock : public ISendingControlSink
{
public:
    CSendControlForASSinkAPIMock()
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

    virtual void OnDataDropped(const FilterVideoDataInfo &filterInfo)
    {

    }
    virtual int32_t OnOutputData(CCmMessageBlock &, const CRTPPacketMetaInfo&, const CMediaMetaInfo*)
    {
        m_bOutput = false;
        return -1;
    }
    virtual void ForwardSendingStatusChanged(uint32_t msid, uint32_t currentBufferSize, uint32_t totalBufferSize)
    {
    }

public:
    FilterResult m_filterType;
    bool m_bOutput;
    bool m_bSmoothed;
};

class CSendControlAPITestForAS : public ::testing::Test
{
protected:
    virtual void SetUp()
    {


        m_pSendControl = ISendingControl::CreateSendingController(&m_sink, RTP_SESSION_WEBEX_AS);

    }
    virtual void TearDown()
    {
        if (nullptr != m_pSendControl) {
            m_pSendControl->Destroy();
            m_pSendControl = nullptr;
        }
    }
    CSendControlForASSinkAPIMock m_sink;
    ISendingControl *m_pSendControl;
};

TEST_F(CSendControlAPITestForAS, create_sendcontrol_successful)
{
    CScopedTracer test_info;
    CSendControlForASSinkAPIMock m_sink;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(&m_sink, RTP_SESSION_WEBEX_AS);
    EXPECT_TRUE(pSendControl != nullptr);
}

TEST_F(CSendControlAPITestForAS, create_sendcontrol_with_null_sink_fail)
{
    CScopedTracer test_info;
    ISendingControl *pSendControl = ISendingControl::CreateSendingController(nullptr);
    EXPECT_TRUE(pSendControl == nullptr);
}

TEST_F(CSendControlAPITestForAS, SendFrameData_for_AS_pass)
{
    CScopedTracer test_info;
    uint32_t ret = m_pSendControl->Initialize();
    EXPECT_EQ(ret,0);
    m_pSendControl->SetBandwidth(FRAME_UNIT_LEN);
    RTPSessionContextSharedPtr sessionConfig(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO));
    auto pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_AS,1,1,TickNowMS(),TickNowMS(),1,sessionConfig);

    auto &nal = pFrameUnit->CreateNAL();
    auto &frag = nal->CreateFragment(FRAME_UNIT_LEN, sessionConfig->GetFrameManager());
    frag->CopyDataIn((const uint8_t *)data,FRAME_UNIT_LEN);

    m_pSendControl->SendFrameData(std::move(pFrameUnit));
    EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);
}

TEST_F(CSendControlAPITestForAS, SendFrameData_for_AS_drop)
{
    CScopedTracer test_info;
#ifdef WIN32
#pragma message(INCLUDE_FILE_AND_LINE("becuase CASSendAdapter allows all frame to pass for PB integration now!!! so don't do drop test"))
#endif // ~WIN32
    //uint32_t ret = m_pSendControl->Initialize(FRAME_UNIT_LEN, true, 1000, 1);
    //EXPECT_EQ(ret,0);
    //CFrameUnit* pFrameUnit = ConstructFrameUnit(1,2,RTP_SESSION_WEBEX_AS,1,1,TickNowMS(),TickNowMS(),1);

    //CNALUnit* nal = pFrameUnit->CreateNAL();
    //CFragmentUnit* frag = nal->CreateFragment(FRAME_UNIT_LEN);
    //frag->CopyDataIn((const uint8_t*)data,FRAME_UNIT_LEN);

    //m_pSendControl->SendFrameData(pFrameUnit);
    //EXPECT_EQ(FILTER_RESULT_PASSED, m_sink.m_filterType);

    //::SleepMs(1000);

    //m_pSendControl->SendFrameData(pFrameUnit);

    //EXPECT_EQ(FILTER_RESULT_DROPPED, m_sink.m_filterType);
}
