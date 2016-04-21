#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <deque>
#include <random>

#include "pktstat.h"
#include "runningstat.h"
#include "testutil.h"
#include "CmBase.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;

using namespace wrtp;

template<typename type>
bool IsInRange(type x, type y, double r)
{
    double upper = (1+r)*y;
    double lower = (1-r)*y;
    return x >= lower && x <=upper;
}

//////////////////////////////////////////////////////////////////////////

class CLossDetectionForTest: public CLossDetection
{
public:
    void Init(std::string expected)
    {
        m_expcted = expected;
        m_result = "";
        m_idx = 0;
        Reset();
    }

    std::string GetResult()
    {
        return m_result;
    }
    virtual void Notify(bool bLost, int32_t seq)
    {
        ASSERT_LT(m_idx, m_expcted.length());
        ASSERT_TRUE(m_expcted[m_idx++]==(bLost?'1':'0'));
        m_result.push_back(bLost?'1':'0');
    }

private:
    std::string m_expcted;
    unsigned m_idx;
    std::string m_result;
};

class CLossDetectionTest : public testing::Test
{
protected:

    void feedStr(std::string s)   //0 - recvd, 1 - lost
    {
        lossDectector.Init(s);
        unsigned j = s.length();
        for (unsigned i=0; i<j; ++i) {
            if (s[i] == '0') {
                lossDectector.AddSeq(i);
            } else if (s[i] == '1') {
                ;
            } else {
                assert(false);
            }
        }
    }

    void feedSeqLst(unsigned *seqLst, unsigned len, std::string expected)
    {
        lossDectector.Init(expected);
        for (unsigned i=0; i<len; ++i) {
            lossDectector.AddSeq(seqLst[i]);
        }
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    CLossDetectionForTest lossDectector;
};

TEST_F(CLossDetectionTest, TestLossDectection)
{
    CScopedTracer test_info;
    std::string s = "0101011000101";
    feedStr(s);
    std::string r = "0101011000";
    ASSERT_EQ(lossDectector.GetResult(),r);

    s = "0000100000";
    feedStr(s);
    r = s;
    ASSERT_EQ(lossDectector.GetResult(),r);

    s = "0101011000101101010101001111001100011000010001111100000";
    feedStr(s);
    r = s;
    ASSERT_EQ(lossDectector.GetResult(),r);
}

TEST_F(CLossDetectionTest, TestDisorderAndLoss)
{
    CScopedTracer test_info;
    {
        unsigned seqLst[] = {1,2,5,3,4,9,10,11,6,7,8};
        std::string expected = "00000000000";
        feedSeqLst(seqLst, sizeof(seqLst)/sizeof(unsigned), expected);
        ASSERT_EQ(lossDectector.GetResult(),expected);
    }

    {
        unsigned seqLst[] = {1,2,5,3,4,9,10,11,7,6,8};
        std::string expected = "00000100000";
        feedSeqLst(seqLst, sizeof(seqLst)/sizeof(unsigned), expected);
        ASSERT_EQ(lossDectector.GetResult(),expected);
    }

    {
        unsigned seqLst[] = {1,2,5,3,4,9,10,11};
        std::string expected = "00000";
        feedSeqLst(seqLst, sizeof(seqLst)/sizeof(unsigned), expected);
        ASSERT_EQ(lossDectector.GetResult(),expected);
    }
}

//////////////////////////////////////////////////////////////////////////

TEST(RunningStatTest, TestStats)
{
    CScopedTracer test_info;
    RunningStat st;
    st.Push(1);
    st.Push(2);
    st.Push(3);
    st.Push(4);

    ASSERT_EQ(st.NumDataValues(), 4);
    ASSERT_TRUE(IsInRange(st.Mean(), 2.5, 0.01));
    ASSERT_TRUE(IsInRange(st.Max(), 4.0, 0.01));
    ASSERT_TRUE(IsInRange(st.Min(), 1.0, 0.01));
    ASSERT_TRUE(IsInRange(st.StandardDeviation(), 1.29099, 0.01));
    ASSERT_TRUE(IsInRange(st.Variance(), 1.66666, 0.01));
}

//////////////////////////////////////////////////////////////////////////
TEST(CLossBurstLenBucketTest, Test_CLossBurstLenBucket)
{
    CLossBurstLenBucket bucket(5, 10);
    bucket.CheckAdd(5);
    bucket.CheckAdd(8);
    bucket.CheckAdd(10);
    bucket.CheckAdd(4);
    EXPECT_EQ(2, bucket.GetCount());
    EXPECT_EQ(5, bucket.GetValue().floor);
    EXPECT_EQ(10, bucket.GetValue().ceil);
    EXPECT_EQ(2, bucket.GetValue().cnt);
}

class CLossBurstLenMetricsMock: public CLossBurstLenMetrics
{
public:
    CLossBurstLenMetricsMock()
    {}
    ~CLossBurstLenMetricsMock()
    {
        m_lossInfo.clear();
    }
    void Kick(const LossBurstLenItem &lossInfo)
    {
        m_lossInfo.push_back(lossInfo);
    }
    LossBurstLenItem &GetLossInfo()
    {
        return m_lossInfo.front();
    }
    void PopFrontLossInfo()
    {
        m_lossInfo.pop_front();
    }
    uint32_t GetKickCnt()
    {
        return (uint32_t)m_lossInfo.size();
    }
private:
    std::list <LossBurstLenItem> m_lossInfo;
};

class CLossBurstLenTest : public testing::Test
{
public:
    CLossBurstLenTest()
    {
        m_lossBurstLen.SetLossBurstLenMetrics(&m_metrics);
    }
    ~CLossBurstLenTest() {}

public:
    CLossBurstLen m_lossBurstLen;
    CLossBurstLenMetricsMock m_metrics;
};

TEST_F(CLossBurstLenTest, Test_SingleStream)
{
    // test recv single stream with status 1101111
    m_metrics.UpdateSeq(68);
    m_metrics.UpdateSeq(69);
    m_metrics.UpdateSeq(71);
    m_metrics.UpdateSeq(72);
    m_metrics.UpdateSeq(73);
    m_metrics.UpdateSeq(74);

    EXPECT_EQ(2, m_metrics.GetKickCnt());
    EXPECT_EQ(2, m_metrics.GetLossInfo().len);
    EXPECT_EQ(LBL_GAP, m_metrics.GetLossInfo().lossStat);
    m_metrics.PopFrontLossInfo();
    EXPECT_EQ(1, m_metrics.GetLossInfo().len);
    EXPECT_EQ(LBL_BURST, m_metrics.GetLossInfo().lossStat);
}

TEST_F(CLossBurstLenTest, Test_SingleStream_disorder)
{
    // test recv single stream with status 1101111(disorder)
    m_metrics.UpdateSeq(68);
    m_metrics.UpdateSeq(69);
    m_metrics.UpdateSeq(71);
    m_metrics.UpdateSeq(73);
    m_metrics.UpdateSeq(72);
    m_metrics.UpdateSeq(74);

    EXPECT_EQ(2, m_metrics.GetKickCnt());
    EXPECT_EQ(2, m_metrics.GetLossInfo().len);
    EXPECT_EQ(LBL_GAP, m_metrics.GetLossInfo().lossStat);
    m_metrics.PopFrontLossInfo();
    EXPECT_EQ(1, m_metrics.GetLossInfo().len);
    EXPECT_EQ(LBL_BURST, m_metrics.GetLossInfo().lossStat);

}

//bucket for loss burst:
// 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, >10 packets
//
//bucket for loss gap:
//0~3, 3~5, 5~8, 8~10, 10~15, 15~20, 20~30, 30~100, >100 packets
class CLossBurstListenerMock: public ILossBurstListener
{
public:
    CLossBurstListenerMock()
        : m_status(LBL_INIT)
        , m_len(0)
        , m_index(0)
    {}

    void OnLossBurstStateIndentified(LBL_STATE status, uint32_t len, uint32_t index)
    {
        m_status = status;
        m_len    = len;
        m_index  = index;
    }

    void CompareResults(LBL_STATE status, uint32_t len, uint32_t index)
    {
        EXPECT_TRUE(status == m_status);
        EXPECT_EQ(len, m_len);
        EXPECT_EQ(index, m_index);
    }
private:
    LBL_STATE m_status;
    uint32_t    m_len;
    uint32_t    m_index;
};
class CLossBurstLenMetricsTest: public testing::Test
{
public:
    CLossBurstLenMetricsTest() {}
    ~CLossBurstLenMetricsTest() {}
    void SetUp()
    {
        m_metrics.SetListener(&m_listener);
    }
public:
    CLossBurstLenMetrics m_metrics;
    CLossBurstListenerMock m_listener;
};

TEST_F(CLossBurstLenMetricsTest, Test_Kick_GetMetrics)
{
    LossBurstLenItem item1 = {2, LBL_BURST};
    LossBurstLenItem item2 = {6, LBL_GAP};
    LossBurstLenItem item3 = {2, LBL_BURST};
    LossBurstLenItem item4 = {26, LBL_GAP};
    LossBurstLenItem item5 = {11, LBL_BURST};
    LossBurstLenItem item6 = {23, LBL_GAP};
    LossBurstLenItem item7 = {8, LBL_BURST};
    LossBurstLenItem item8 = {110, LBL_GAP};
    LossBurstLenItem item9 = {87, LBL_GAP};

    m_metrics.Kick(item1);
    m_listener.CompareResults(LBL_BURST, 2, 1);
    m_metrics.Kick(item2);
    m_listener.CompareResults(LBL_GAP, 6, 2);
    m_metrics.Kick(item3);
    m_listener.CompareResults(LBL_BURST, 2, 1);
    m_metrics.Kick(item4);
    m_listener.CompareResults(LBL_GAP, 26, 6);
    m_metrics.Kick(item5);
    m_listener.CompareResults(LBL_BURST, 11, 10);
    m_metrics.Kick(item6);
    m_listener.CompareResults(LBL_GAP, 23, 6);
    m_metrics.Kick(item7);
    m_listener.CompareResults(LBL_BURST, 8, 7);
    m_metrics.Kick(item8);
    m_listener.CompareResults(LBL_GAP, 110, 8);
    m_metrics.Kick(item9);
    m_listener.CompareResults(LBL_GAP, 87, 7);

    LossBurstLenStat totalSt;
    EXPECT_EQ(WRTP_ERR_NOERR, m_metrics.GetMetrics(totalSt));
    EXPECT_EQ(2, totalSt.burstLen[1].cnt);
    EXPECT_EQ(1, totalSt.burstLen[7].cnt);
    EXPECT_EQ(1, totalSt.burstLen[10].cnt);
    EXPECT_EQ(1, totalSt.gapLen[2].cnt);
    EXPECT_EQ(2, totalSt.gapLen[6].cnt);
    EXPECT_EQ(1, totalSt.gapLen[7].cnt);
    EXPECT_EQ(1, totalSt.gapLen[8].cnt);
}

TEST_F(CLossBurstLenMetricsTest, Test_SingleStream)
{
    //ssrc1:5(1),1(0),18(1), 10(0),38(1), 5(0), 110(1), 1(0)
    uint16_t seq_ssrc1 = 8854;

    for (uint16_t cnt = 0; cnt < 5; ++cnt) {
        m_metrics.UpdateSeq(++seq_ssrc1);
    }
    //ssrc1 loss 1 packet
    ++seq_ssrc1;
    for (uint16_t cnt = 0; cnt < 18; ++cnt) {
        m_metrics.UpdateSeq(++seq_ssrc1);

    }
    //ssrc1 loss 10 packets
    seq_ssrc1 += 10;
    for (uint16_t cnt = 0; cnt < 38; ++cnt) {
        m_metrics.UpdateSeq(++seq_ssrc1);
    }

    //ssrc1 loss 5 packets
    seq_ssrc1 += 5;
    for (uint16_t cnt = 0; cnt < 110; ++cnt) {
        m_metrics.UpdateSeq(++seq_ssrc1);

    }
    //ssrc1 loss 1 packets
    ++seq_ssrc1;
    //use 4 packets to refresh status
    for (uint16_t cnt = 0; cnt < 4; ++cnt) {
        m_metrics.UpdateSeq(++seq_ssrc1);
    }

    LossBurstLenStat totalSt;
    EXPECT_EQ(WRTP_ERR_NOERR, m_metrics.GetMetrics(totalSt));
    EXPECT_EQ(2, totalSt.burstLen[0].cnt);
    EXPECT_EQ(1, totalSt.burstLen[4].cnt);
    EXPECT_EQ(1, totalSt.burstLen[9].cnt);

    EXPECT_EQ(1, totalSt.gapLen[1].cnt);
    EXPECT_EQ(1, totalSt.gapLen[5].cnt);
    EXPECT_EQ(1, totalSt.gapLen[7].cnt);
    EXPECT_EQ(1, totalSt.gapLen[8].cnt);
}

class COutOfOrderListenerMock: public IOutOfOrderListener
{
public:
    COutOfOrderListenerMock()
        : m_gapLen(0)
        , m_index(0)
    {}

    void OnOutOfOrderStateIndentified(uint32_t len, uint16_t index)
    {
        m_gapLen = len;
        m_index  = index;
    }

    void CompareResults(uint32_t len, uint16_t index)
    {
        EXPECT_EQ(len, m_gapLen);
        EXPECT_EQ(index, m_index);
    }
private:
    uint32_t m_gapLen;
    uint16_t m_index;

};
class COutOfOrderMetricsTest: public testing::Test
{
public:
    void SetUp()
    {
        m_metrics.SetListener(&m_listener);
    }

public:
    COutOfOrderMetrics m_metrics;
    COutOfOrderListenerMock m_listener;
};
TEST_F(COutOfOrderMetricsTest, test_outOfOrderMetrics)
{
    m_metrics.NotifyOutOfOrder(3);
    m_listener.CompareResults(3, 2);
    m_metrics.NotifyOutOfOrder(3);
    m_listener.CompareResults(3, 2);
    m_metrics.NotifyOutOfOrder(1);
    m_listener.CompareResults(1, 0);
    m_metrics.NotifyOutOfOrder(13);
    m_listener.CompareResults(13, 7);

    OutOfOrderStat stat;
    m_metrics.GetMetrics(stat);
    EXPECT_EQ(2, stat.gapLen[2].cnt);
    EXPECT_EQ(1, stat.gapLen[0].cnt);
    EXPECT_EQ(1, stat.gapLen[7].cnt);
    EXPECT_EQ(0, stat.gapLen[3].cnt);

}


////////////////////////////////////////////////////////////////////
#if defined(CM_MAC) || defined (CM_IOS) || defined (CM_ANDROID)

//  the purpose of this class is to use CPacketLossRecorder's protected methods.
class CPacketLossRecorderChild: public CPacketLossRecorder
{
public:
    CPacketLossRecorderChild(uint32_t bufferByteLen = 512):CPacketLossRecorder(bufferByteLen)
    {

    }
    ~CPacketLossRecorderChild() {}

    void WriteFlagsToLog()
    {
        CPacketLossRecorder::WriteFlagsToLog();
    }

    uint32_t GetTotalRawFlagsByteLen()
    {
        return CPacketLossRecorder::GetTotalRawFlagsByteLen();
    }
};

class CPacketLossRecorderTest : public testing::Test
{
public:
    CPacketLossRecorderTest()
    {

    }
    ~CPacketLossRecorderTest() {}
};


// process each byte, start from byte 0,
// for each byte, start from bit 7 to bit 0
// for each bit, check its value, if it is 1, push true to vector, otherwise push false
void ConvertFromByteToBools(std::vector<bool> *flags, const BYTE *inbuf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            unsigned char x = 1;
            x = x << (7 - j);
            bool flag = x & inbuf[i];
            flags->push_back(flag);
        }
    }
}

string ConvertFromBoolsToBytes(const std::vector<bool> &flags)
{
    size_t pktlen = flags.size();
    size_t bytelen = (pktlen % 8) ? (pktlen / 8 + 1) : pktlen / 8;
    BYTE *buf = new BYTE[bytelen];
    memset(buf, 0, bytelen);

    for (int ipkt = 0; ipkt < pktlen; ipkt++) {
        if (flags[ipkt]) {
            size_t kbyte = ipkt / 8;
            size_t kbit = 7 - ipkt % 8;

            unsigned char x = 1;
            x = x << kbit;
            buf[kbyte] += x;
        }
    }

    string ret((char *)buf, bytelen);

    delete [] buf;

    return ret;
}

#define BUF_LEN 500
TEST_F(CPacketLossRecorderTest, Test_NotifyToWrite)
{
    vector<bool> flags_a, flags_b, flags_c;

    int byteLen = BUF_LEN;

    std::random_device rd;
    mt19937_64 generator{rd()};
    uniform_int_distribution<> dist{0,1};

    //  init flags_a
    for (int iByte = 0; iByte < byteLen; iByte++) {
        for (int i = 0; i < 8; i++) {
            flags_a.push_back(dist(generator));
        }
    }
    // reason: prevent from all flags being 0, otherwise, there will be no compress anymore -- the design is: it will not compress when all flags are 0.
    flags_a[0] = true;

    //  init flags_b
    for (int iByte = 0; iByte < byteLen; iByte++) {
        for (int i = 0; i < 8; i++) {
            flags_b.push_back(dist(generator));
        }
    }
    flags_b[0] = true;

    //  init flags_c, not one byte
    for (int i = 0; i < 7; i++) {
        flags_c.push_back(dist(generator));
    }
    flags_c[0] = true;

    CPacketLossRecorderChild lossRawInfo(byteLen);

    vector<CCmString> totalCompressed;
    lossRawInfo.SetOnEncodedCallback([&totalCompressed](const CCmString& encoded) {
        CCmString decoded;
        if (encoded.length()) {
            CM_Base64Decode(encoded.c_str(), decoded);
            totalCompressed.push_back(decoded);
        }
    });

    lossRawInfo.SetQueryCallback([] {
        return true;
    });

    int32_t unused = 0;

    // benchmark
    std::clock_t startTick=std::clock();

    for (int i = 0; i < flags_a.size(); i++) {
        lossRawInfo.Notify(flags_a[i], unused);
    }
    for (int i = 0; i < flags_b.size(); i++) {
        lossRawInfo.Notify(flags_b[i], unused);
    }
    for (int i = 0; i < flags_c.size(); i++) {
        lossRawInfo.Notify(flags_c[i], unused);
    }
    // last call to WriteFlagsToLog
    lossRawInfo.WriteFlagsToLog();

    std::clock_t endTick = std::clock();
    double costInMS = 1000.0 * (endTick - startTick) / CLOCKS_PER_SEC;

    // both flags_a and flags_b have been compressed and encoded,
    // I bet the total time used by them is very small, otherwise, warn me: ukison@cisco.com
    // note that:
    // the single time used by flags_a/flags_b needs be further divided by 2 -- even small.
#if defined(CM_MAC)
    EXPECT_LT(costInMS, 2);
#elif defined (CM_IOS)
    // Expected: (costInMS) < (4), actual: 8.7 vs 4, tested on iphone 5C
    EXPECT_LT(costInMS, 10);
#elif defined (CM_ANDROID)
    // Expected: (costInMS) < (4), actual: 5.98 vs 4
    EXPECT_LT(costInMS, 10);
#endif
    
    EXPECT_EQ(totalCompressed.size(), 3);

    BYTE uncompressOutBuffer[BUF_LEN*3];
    uint32_t outlen = BUF_LEN*3;

    // verify flag_a
    uint32_t uncompressLen = outlen;
    CPacketLossRecorder::Uncompress((BYTE *)totalCompressed[0].c_str(), (uint32_t)totalCompressed[0].length(), uncompressOutBuffer, uncompressLen);
    EXPECT_EQ(uncompressLen, byteLen);
    std::vector<bool> flags;
    ConvertFromByteToBools(&flags, uncompressOutBuffer, uncompressLen);
    for (int i=0; i<flags_a.size(); i++) {
        EXPECT_TRUE(flags[i]==flags_a[i]);
    }

    // verify flag_b
    uncompressLen = outlen;
    CPacketLossRecorder::Uncompress((BYTE *)totalCompressed[1].c_str(), (uint32_t)totalCompressed[1].length(), uncompressOutBuffer, uncompressLen);
    EXPECT_EQ(uncompressLen, byteLen);
    flags.clear();
    ConvertFromByteToBools(&flags, uncompressOutBuffer, uncompressLen);
    for (int i=0; i<flags_b.size(); i++) {
        EXPECT_TRUE(flags[i]==flags_b[i]);
    }

    // verify flag_c
    uncompressLen = outlen;
    CPacketLossRecorder::Uncompress((BYTE *)totalCompressed[2].c_str(), (uint32_t)totalCompressed[2].length(), uncompressOutBuffer, uncompressLen);
    EXPECT_EQ(uncompressLen, 1);
    flags.clear();
    ConvertFromByteToBools(&flags, uncompressOutBuffer, uncompressLen);
    for (int i=0; i<flags_c.size(); i++) {
        EXPECT_TRUE(flags[i]==flags_c[i]);
    }

    EXPECT_TRUE(lossRawInfo.GetTotalRawFlagsByteLen()==2*byteLen+1);
}

TEST_F(CPacketLossRecorderTest, Test_DefaultFeatureToggle)
{
    std::shared_ptr<CPacketLossRecordController> control(new CPacketLossRecordController("invalid_trace_tag"));
    CPacketLossRecorder record;

    record.SetOnEncodedCallback(control->CreateOnEncodedCallback());
    record.SetQueryCallback(control->CreateQueryCallback());

    //  init input flags
    vector<bool> flags;
    int byteLen = BUF_LEN * 10;

    std::random_device rd;
    mt19937_64 generator{rd()};
    uniform_int_distribution<> dist{0,1};

    for (int iByte = 0; iByte < byteLen; iByte++) {
        for (int i = 0; i < 8; i++) {
            flags.push_back(dist(generator));
        }
    }

    //  feed
    int32_t unused=0;
    for (int i=0; i<flags.size(); i++) {
        record.Notify(flags[i], unused);
    }

    //  verify
    EXPECT_EQ(0, control->GetBytesEncoded());
    EXPECT_EQ(0, control->GetBytesLastSingleTime());
    EXPECT_EQ(0, control->GetBytesBeforeExceed());
}

TEST_F(CPacketLossRecorderTest, Test_EnableFeatureToggle)
{
    std::shared_ptr<CPacketLossRecordController> control(new CPacketLossRecordController("invalid_trace_tag"));
    CPacketLossRecorder record;

    record.SetQueryCallback(control->CreateQueryCallback());
    record.SetOnEncodedCallback(control->CreateOnEncodedCallback());

    uint32_t max = 500;
    control->SetFeatureToggles(true, max);

    //  init input flags
    vector<bool> flags;
    int byteLen = BUF_LEN * 10;

    std::random_device rd;
    mt19937_64 generator{rd()};
    uniform_int_distribution<> dist{0,1};

    for (int iByte = 0; iByte < byteLen; iByte++) {
        for (int i = 0; i < 8; i++) {
            flags.push_back(dist(generator));
        }
    }

    //  feed
    int32_t unused=0;
    for (int i=0; i<flags.size(); i++) {
        record.Notify(flags[i], unused);
    }

    //  verify
    EXPECT_FALSE(control->GetEnabledNow());
    EXPECT_GE(control->GetBytesEncoded(), max);
    EXPECT_LE(control->GetBytesBeforeExceed(), max);
    EXPECT_EQ(control->GetBytesLastSingleTime() + control->GetBytesBeforeExceed(), control->GetBytesEncoded());
}

TEST_F(CPacketLossRecorderTest, Test_Lambda)
{
    std::shared_ptr<CPacketLossRecordController> control(new CPacketLossRecordController("invalid_trace_tag"));
    CPacketLossRecorderChild record;

    record.SetQueryCallback(control->CreateQueryCallback());
    record.SetOnEncodedCallback(control->CreateOnEncodedCallback());

    // key point
    control = nullptr;

    //  init input flags
    vector<bool> flags;
    int byteLen = BUF_LEN * 10;

    std::random_device rd;
    mt19937_64 generator{rd()};
    uniform_int_distribution<> dist{0,1};

    for (int iByte = 0; iByte < byteLen; iByte++) {
        for (int i = 0; i < 8; i++) {
            flags.push_back(dist(generator));
        }
    }

    //  feed
    int32_t unused=0;
    for (int i=0; i<flags.size(); i++) {
        record.Notify(flags[i], unused);
    }

    EXPECT_EQ(0, record.GetTotalRawFlagsByteLen());
}

TEST_F(CPacketLossRecorderTest, Test_Zlib)
{
    // there is loss within 50 bytes
    vector<bool> packets;
    // the first 100 bytes has no loss
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 8; j++) {
            packets.push_back(false);
        }
    }
    // then, there is loss within 50 bytes
    std::random_device rd;
    mt19937_64 generator{rd()};
    uniform_int_distribution<> dist{0,1};
    for (int iByte = 0; iByte < 50; iByte++) {
        for (int i = 0; i < 8; i++) {
            packets.push_back(dist(generator));
        }
    }
    // the last 350 bytes has no loss
    for (int i = 0; i < 350; i++) {
        for (int j = 0; j < 8; j++) {
            packets.push_back(false);
        }
    }

    // convert the vector<bool> to byte buffer
    string toBeCompress = ConvertFromBoolsToBytes(packets);

    // compress the whole buffer
    {
        z_stream *strm = CPacketLossRecorder::CompressInit();
        BYTE tempbuf[500] = {0};
        string afterCompress;
        CPacketLossRecorder::Compress(strm, (unsigned char *)toBeCompress.c_str(), (uint32_t)toBeCompress.length(), tempbuf, 500, &afterCompress);
        CPacketLossRecorder::CompressEnd(strm);

        // see if the uncompressed value is as same as the value before compressed
        memset(tempbuf, 0, 500);
        UINT32 outbuflen = 500;
        CPacketLossRecorder::Uncompress((unsigned char *)afterCompress.c_str(), (uint32_t)afterCompress.length(), tempbuf, outbuflen);
        string uncompressed((char *)tempbuf, outbuflen);
        EXPECT_EQ(uncompressed, toBeCompress);

        vector<bool> restoredPackets;
        ConvertFromByteToBools(&restoredPackets, (const BYTE *)uncompressed.c_str(), (uint32_t)uncompressed.length());
        EXPECT_EQ(restoredPackets, packets);
    }

    // only compress bytes from 100~150
    {
        z_stream *strm = CPacketLossRecorder::CompressInit();
        BYTE tempbuf[500] = {0};
        string afterCompress;
        CPacketLossRecorder::Compress(strm, (unsigned char *)toBeCompress.c_str() + 100, 50, tempbuf, 500, &afterCompress);
        CPacketLossRecorder::CompressEnd(strm);

        // see if the uncompressed value is as same as the value before compressed
        memset(tempbuf, 0, 500);
        UINT32 outbuflen = 500;
        CPacketLossRecorder::Uncompress((unsigned char *)afterCompress.c_str(), (uint32_t)afterCompress.length(), tempbuf, outbuflen);
        string uncompressed((char *)tempbuf, outbuflen);
        EXPECT_EQ(uncompressed, toBeCompress.substr(100, 50));

        vector<bool> restoredPackets;
        ConvertFromByteToBools(&restoredPackets, (const BYTE *)uncompressed.c_str(), (uint32_t)uncompressed.length());
        for (int i = 0; i < 50*8; i++) {
            EXPECT_EQ(restoredPackets[i], packets[100*8 + i]);
        }
    }
}

TEST_F(CPacketLossRecorderTest, Test_NoLoss)
{
    // when no loss, expect no compress and encode

    vector<bool> restoredPackets;
    {
        CPacketLossRecorder record(1);

        record.SetQueryCallback([] {
            return true;
        });

        record.SetOnEncodedCallback([&restoredPackets](const CCmString& encoded) {
            CCmString decoded;
            if (encoded.length()) {
                CM_Base64Decode(encoded.c_str(), decoded);
                BYTE packets[3] = {0};
                uint32_t len = 3;
                CPacketLossRecorder::Uncompress((BYTE *)decoded.c_str(), (uint32_t)decoded.length(), packets, len);
                EXPECT_EQ(1, len);
                ConvertFromByteToBools(&restoredPackets, packets, 1);
            }
        });

        int32_t useless = 0;

        record.Notify(false, useless);
        record.Notify(true, useless);
        for (int i = 0; i < 6; i++) {
            record.Notify(false, useless);
        }

        for (int i = 0; i < 8; i++) {
            record.Notify(false, useless);
        }

        record.Notify(false, useless);
        record.Notify(true, useless);
    }

    EXPECT_EQ(restoredPackets[0], false);
    EXPECT_EQ(restoredPackets[1], true);
    for (int i = 2; i < 8; i++) {
        EXPECT_EQ(restoredPackets[i], false);
    }
    EXPECT_EQ(restoredPackets[8], false);
    EXPECT_EQ(restoredPackets[9], true);
    EXPECT_EQ(restoredPackets.size(), 16);
}

#endif //#if defined(CM_MAC) || defined (CM_IOS) || defined (CM_ANDROID)


