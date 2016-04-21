#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "streamstats.h"

NS_WRTP_BEGIN // namespace wrtp

class CRTPLossRatioStatsTest : public testing::Test
{
public:
    CRTPLossRatioStatsTest() {}
protected:
    //will be called before running every case
    virtual void SetUp()
    {
    }
    //will be called after running every case
    virtual void TearDown()
    {
    }
public:
    CRTPLossRatioStats m_loss;
};

TEST_F(CRTPLossRatioStatsTest, test_lossratio_without_packet_recovered)
{
    //interval 1, seq:4567-4575
    m_loss.NotifyPacketRecved(4567, false);
    m_loss.NotifyPacketRecved(4570, false);
    m_loss.NotifyPacketRecved(4569, false);
    m_loss.NotifyPacketRecved(4575, false);
    m_loss.NotifyPacketRecved(4573, false);
    m_loss.NotifyPacketRecved(4574, false);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(3)/9, m_loss.GetLossRatio());
    EXPECT_EQ(float(3)/9, m_loss.GetFecResidualessLossRatio());

    //interval 2, seq:4576-4578
    m_loss.NotifyPacketRecved(4576, false);
    // these two packets will not be calculated because of they are in early interval
    m_loss.NotifyPacketRecved(4572, false);
    m_loss.NotifyPacketRecved(4568, false);
    m_loss.NotifyPacketRecved(4578, false);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(1)/3, m_loss.GetLossRatio());
    EXPECT_EQ(float(1)/3, m_loss.GetFecResidualessLossRatio());

    //interval 3, seq:4579-4588
    m_loss.NotifyPacketRecved(4580, false);
    m_loss.NotifyPacketRecved(4588, false);
    //reduplicated packet
    m_loss.NotifyPacketRecved(4580, false);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(8)/10, m_loss.GetLossRatio());
    EXPECT_EQ(float(8)/10, m_loss.GetFecResidualessLossRatio());
}

TEST_F(CRTPLossRatioStatsTest, test_lossratio_with_packet_recovered_seq_rollback)
{
    //interval 1, seq:65530-4
    m_loss.NotifyPacketRecved(65530, false);
    m_loss.NotifyPacketRecved(3, false);
    m_loss.NotifyPacketRecved(0, true);
    m_loss.NotifyPacketRecved(4, false);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(8)/11, m_loss.GetLossRatio());
    EXPECT_EQ(float(7)/11, m_loss.GetFecResidualessLossRatio());

    //interval 2, seq:5-10
    m_loss.NotifyPacketRecved(65535, false);
    m_loss.NotifyPacketRecved(65534, true);
    m_loss.NotifyPacketRecved(10, false);
    m_loss.NotifyPacketRecved(8, true);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(5)/6, m_loss.GetLossRatio());
    EXPECT_EQ(float(4)/6, m_loss.GetFecResidualessLossRatio());
}

TEST_F(CRTPLossRatioStatsTest, test_lossratio_with_packet_recovered)
{
    //interval 1, seq:4567-4575
    m_loss.NotifyPacketRecved(4567, false);
    m_loss.NotifyPacketRecved(4570, false);
    m_loss.NotifyPacketRecved(4569, false);
    m_loss.NotifyPacketRecved(4575, false);
    m_loss.NotifyPacketRecved(4573, false);
    m_loss.NotifyPacketRecved(4574, false);
    // notify recovered packet
    m_loss.NotifyPacketRecved(4571, true);
    m_loss.NotifyPacketRecved(4575, true);//this packet is already received, shouldn't be calculated
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(3)/9, m_loss.GetLossRatio());
    EXPECT_EQ(float(2)/9, m_loss.GetFecResidualessLossRatio());

    //interval 2, seq:4576-4578
    m_loss.NotifyPacketRecved(4572, true);//early interval
    m_loss.NotifyPacketRecved(4576, false);
    // these two packets will not be calculated, early interval
    m_loss.NotifyPacketRecved(4572, false);
    m_loss.NotifyPacketRecved(4568, false);
    m_loss.NotifyPacketRecved(4578, false);
    m_loss.NotifyPacketRecved(4578, false);//reduplicated packet
    m_loss.NotifyPacketRecved(4577, true);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(1)/3, m_loss.GetLossRatio());
    EXPECT_EQ(0.0f, m_loss.GetFecResidualessLossRatio());

    //interval 3, seq:4579-4588
    m_loss.NotifyPacketRecved(4579, true);
    m_loss.NotifyPacketRecved(4588, false);
    m_loss.NotifyPacketRecved(4586, false);
    m_loss.CalcInLastInterval();
    EXPECT_EQ(float(8)/10, m_loss.GetLossRatio());
    EXPECT_EQ(float(7)/10, m_loss.GetFecResidualessLossRatio());
}



NS_WRTP_END // ~namespace wrtp
