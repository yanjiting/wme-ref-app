#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "wrtpmediaapi.h"
#include "wrtpapi.h"
#include "testutil.h"
#include "fragmentop.h"

using namespace wrtp;

class CRTPMediaChannelTest : public ::testing::Test
{
public:
    CRTPMediaChannelTest() : m_mediaChannel(nullptr) {}

protected:
    virtual void SetUp()
    {
        // create RTP session
        m_sessionParam.sessionType  = RTP_SESSION_WEBEX_VIDEO;
        m_sessionParam.enableRTCP   = true;

        m_videoSession = WRTPCreateRTPSessionClient(m_sessionParam);
        ASSERT_TRUE(nullptr != m_videoSession);
        m_videoSession->IncreaseReference();

        int32_t ret = m_videoSession->RegisterPayloadType(60, 20, 90000);
        ASSERT_TRUE(0 == ret);

        // create RTP channel
        m_channelParam.pPacketizer  = nullptr;

        m_channelId = 111;
        m_rtpChannel = m_videoSession->CreateRTPChannel(m_channelId, m_channelParam);
        ASSERT_TRUE(nullptr != m_rtpChannel);
        m_rtpChannel->IncreaseReference();

        // cast the RTP channel to media Channel
        m_mediaChannel = m_rtpChannel;
    }

    virtual void TearDown()
    {
        if (m_mediaChannel) {
            m_mediaChannel = nullptr;
        }

        if (m_rtpChannel) {
            m_rtpChannel->Close();
            m_rtpChannel->DecreaseReference();
            m_rtpChannel = nullptr;
        }

        if (m_videoSession) {
            m_videoSession->DecreaseReference();
            m_videoSession = nullptr;
        }
    }

protected:
    WRTPSessionParams m_sessionParam;
    WRTPChannelParams m_channelParam;
    uint32_t m_channelId;

    IRTPSessionClient *m_videoSession;
    IRTPChannel *m_rtpChannel;
    IRTPMediaChannel *m_mediaChannel;
};

TEST_F(CRTPMediaChannelTest, FragmentOperator_Test)
{
    CScopedTracer test_info;
    EXPECT_EQ(nullptr, m_mediaChannel->GetPacketizationOperator());

    m_mediaChannel->SetPacketizationOperator(nullptr);
    EXPECT_EQ(nullptr, m_mediaChannel->GetPacketizationOperator());

    CRTPPacketizationMock fragmentOp;
    m_mediaChannel->SetPacketizationOperator(&fragmentOp);
    EXPECT_EQ(&fragmentOp, m_mediaChannel->GetPacketizationOperator());
}