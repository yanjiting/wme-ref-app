#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "stringhelper.h"
#include "NetworkWarning.h"
#include "MediaConnection.hpp"

using namespace testing;
using namespace wme;

/************************************************************************/
/* CNetworkWarningStateTest                                             */
/************************************************************************/

TEST(CNetworkWarningStateTest, TestBadState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    for( unsigned i=0; i<3; ++i){
        st.Update(NETWORK_SEVERE, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
    }

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);
}

TEST(CNetworkWarningStateTest, TestRecoverFromBadState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    unsigned i=0;
    for(; i<4; ++i){
        st.Update(NETWORK_SEVERE, false);
    }

    for(i=0; i<3; ++i){
        st.Update(NETWORK_VERY_GOOD, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);
    }

    st.Update(NETWORK_VERY_GOOD, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
}

TEST(CNetworkWarningStateTest, TestMinimumState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    for( unsigned i=0; i<3; ++i){
        st.Update(NETWORK_SEVERE, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
    }

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);

    st.Update(NETWORK_SEVERE, true);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);
}

TEST(CNetworkWarningStateTest, TestRecoverFromMinimumState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    unsigned i=0;
    for(; i<4; ++i){
        st.Update(NETWORK_SEVERE, false);
    }

    st.Update(NETWORK_SEVERE, true);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);

    for(i=0; i<3; ++i){
        st.Update(NETWORK_VERY_GOOD, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);
    }

    st.Update(NETWORK_VERY_GOOD, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
}


TEST(CNetworkWarningStateTest, TestBreakState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    for( unsigned i=0; i<3; ++i){
        st.Update(NETWORK_SEVERE, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
    }

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);


    st.Update(NETWORK_SEVERE, true);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);

    for( unsigned i=0; i<3; ++i){
        st.Update(NETWORK_SEVERE, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);
    }

    st.Update(NETWORK_SEVERE, true);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BREAKER);
}

TEST(CNetworkWarningStateTest, TestRecoverFromBreakState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    unsigned i=0;
    for(; i<4; ++i){
        st.Update(NETWORK_SEVERE, false);
    }

    st.Update(NETWORK_SEVERE, true);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);

    for(i=0; i<4; ++i){
        st.Update(NETWORK_SEVERE, false);
    }
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BREAKER);

    for(i=0; i<3; ++i){
        st.Update(NETWORK_VERY_GOOD, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BREAKER);
    }
    st.Update(NETWORK_VERY_GOOD, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
}

TEST(CNetworkWarningStateTest, TestNoSessionAfterBadState)
{
    CNetworkWarningState st(3U, 3U, 3U, 0, DIRECTION_BOTHLINK);

    for( unsigned i=0; i<3; ++i){
        st.Update(NETWORK_SEVERE, false);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
    }

    st.Update(NETWORK_SEVERE, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);

    //no session will update with NETWORK_UNKNOWN
    st.Update(NETWORK_UNKNOWN, false);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
}

/************************************************************************/
/* CNetworkWarningTest                                                  */
/************************************************************************/

TEST(CNetworkWarningTest, TestInvalidInterval)
{
    CNetworkWarning warn(NULL, NULL, 0, 5000U, 5000U, 5000U, 10U);
    ASSERT_EQ(warn.GetUpdateIntervalInMills(), NETWORKWARNING_MINIMUM_UPDATE_INTERVAL);
}

TEST(CNetworkWarningTest, TestCounterThreshold)
{
    ASSERT_EQ(10U, CNetworkWarning::GetCounterThreshold(10000U, 1000U));
    ASSERT_EQ(6U, CNetworkWarning::GetCounterThreshold(5500U, 1000U));
    ASSERT_EQ(1U, CNetworkWarning::GetCounterThreshold(5500U, 0U));
}

TEST(CNetworkWarningTest, TestSetWaitForBadInMills)
{
    uint32_t interval = 1000U;
    CNetworkWarning warn(NULL, NULL, 0, 5000U, 5000U, 5000U, interval);

    uint32_t wait = 10000U;
    uint32_t thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);

    warn.SetWaitForBadInMills(DIRECTION_UPLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForBadCounterThreshold(), thresholdCounter);

    wait = 20000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForBadInMills(DIRECTION_DOWNLINK, wait);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForBadCounterThreshold(), thresholdCounter);

    wait = 25000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForBadInMills(DIRECTION_BOTHLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForBadCounterThreshold(), thresholdCounter);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForBadCounterThreshold(), thresholdCounter);
}

TEST(CNetworkWarningTest, TestSetWaitForBreakInMills)
{
    uint32_t interval = 1000U;
    CNetworkWarning warn(NULL, NULL, 0, 5000U, 5000U, 5000U, interval);

    uint32_t wait = 10000U;
    uint32_t thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);

    warn.SetWaitForBreakInMills(DIRECTION_UPLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForBreakCounterThreshold(), thresholdCounter);

    wait = 20000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForBreakInMills(DIRECTION_DOWNLINK, wait);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForBreakCounterThreshold(), thresholdCounter);

    wait = 25000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForBreakInMills(DIRECTION_BOTHLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForBreakCounterThreshold(), thresholdCounter);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForBreakCounterThreshold(), thresholdCounter);
}

TEST(CNetworkWarningTest, TestSetWaitForRecoveredInMills)
{
    uint32_t interval = 1000U;
    CNetworkWarning warn(NULL, NULL, 0, 5000U, 5000U, 5000U, interval);

    uint32_t wait = 10000U;
    uint32_t thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);

    warn.SetWaitForRecoveredInMills(DIRECTION_UPLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForRecoveredCounterThreshold(), thresholdCounter);

    wait = 20000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForRecoveredInMills(DIRECTION_DOWNLINK, wait);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForRecoveredCounterThreshold(), thresholdCounter);

    wait = 25000U;
    thresholdCounter = CNetworkWarning::GetCounterThreshold(wait, interval);
    warn.SetWaitForRecoveredInMills(DIRECTION_BOTHLINK, wait);
    ASSERT_EQ(warn.GetUplinkState().GetWaitForRecoveredCounterThreshold(), thresholdCounter);
    ASSERT_EQ(warn.GetDownlinkState().GetWaitForRecoveredCounterThreshold(), thresholdCounter);
}

class CWmeNetworkIndicatorMock: public IWmeNetworkIndicator
{
public:
    MOCK_METHOD1(GetNetworkIndex, WmeNetworkIndex(WmeNetworkDirection));
#ifdef WIN32
	MOCK_METHOD0_WITH_CALLTYPE(WMECALLTYPE, AddRef, uint32_t());
	MOCK_METHOD0_WITH_CALLTYPE(WMECALLTYPE, Release, uint32_t());
    MOCK_METHOD0_WITH_CALLTYPE(WMECALLTYPE, GetReference, uint32_t());
	MOCK_METHOD2_WITH_CALLTYPE(WMECALLTYPE, QueryInterface, WMERESULT(REFWMEIID, void**));
#else
	MOCK_METHOD0(AddRef, uint32_t());
	MOCK_METHOD0(Release, uint32_t());
    MOCK_METHOD0(GetReference, uint32_t());
	MOCK_METHOD2(QueryInterface, WMERESULT(REFWMEIID, void**));
#endif
};

class CWmeMediaConnectionSinkMock: public IWmeMediaConnectionSink
{
public:
    MOCK_METHOD4(OnMediaReady, void(unsigned long, WmeDirection, WmeSessionType, IWmeMediaTrackBase *));
    MOCK_METHOD2(OnSDPReady, void(WmeSdpType, const char *));
    MOCK_METHOD3(OnMediaBlocked, void(unsigned long,unsigned long, bool));
    MOCK_METHOD4(OnDecodeSizeChanged, void(unsigned long,unsigned long, uint32_t, uint32_t));
    MOCK_METHOD3(OnEncodeSizeChanged, void(unsigned long, uint32_t, uint32_t));
    MOCK_METHOD3(OnSessionStatus, void(unsigned long, WmeSessionType, WmeSessionStatus));
    MOCK_METHOD2(OnNetworkStatus, void(WmeNetworkStatus, WmeNetworkDirection));
    MOCK_METHOD5(OnMediaStatus, void(unsigned long, unsigned long, WmeMediaStatus, bool, uint32_t));
    MOCK_METHOD1(OnError, void(int));
};

class CMediaConnectionMock: public CMediaConnection
{
public:
    CMediaConnectionMock(long callid) : CMediaConnection(0, false, false, false){
        Reset();
    }

    void Reset(){
        m_minimum = false;
    }

    void SetExpectedForIsVideoResolutionAndBitrateDropToMininum(bool minimum){
        m_minimum = minimum;
    }

    virtual bool IsVideoResolutionAndBitrateDropToMininum(WmeNetworkDirection direction, uint32_t minBitrate, uint32_t minResolution){
        return m_minimum;
    }

    virtual bool IsVideoResolutionAndDownlinkBWDropToMininum(uint32_t minBWOfDownlink, uint32_t minResolution){
        return false;
    }
    

private:
    bool m_minimum;
};


TEST(CNetworkWarningTest, TestUpdateLinkState)
{
    CWmeMediaConnectionSinkMock m_sink;
    CMediaConnectionMock m_media(0);
    m_media.SetSink(&m_sink);
    m_media.SetExpectedForIsVideoResolutionAndBitrateDropToMininum(false);
    CWmeNetworkIndicatorMock m_indicator;

    CNetworkWarning warn(&m_media, &m_indicator, 0);

    WmeNetworkDirection direct = DIRECTION_UPLINK;
    CNetworkWarningState st(3U, 3U, 3U, 0, direct);

    // Test WmeNetwork_bad
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_bad, direct)).WillOnce(Return());
    EXPECT_CALL(m_indicator, GetNetworkIndex(direct)).WillRepeatedly(Return(NETWORK_SEVERE));

    unsigned i = 0;
    for(; i<3; ++i){
        warn.UpdateLinkState(direct, st);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);
    }

    warn.UpdateLinkState(direct, st);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BAD);

    //Later bad state will not trigger OnNetworkStatus
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_bad, direct)).Times(0);
    warn.UpdateLinkState(direct, st);

    // Test WmeNetwork_video_off
    m_media.SetExpectedForIsVideoResolutionAndBitrateDropToMininum(true);
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_video_off, direct)).WillOnce(Return());
    EXPECT_CALL(m_indicator, GetNetworkIndex(direct)).WillRepeatedly(Return(NETWORK_SEVERE));

    warn.UpdateLinkState(direct, st);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);

    for(i=0; i<3; ++i){
        warn.UpdateLinkState(direct, st);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_MINIUM);
    }

    warn.UpdateLinkState(direct, st);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BREAKER);

    // Later bad state will not trigger OnNetworkStatus
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_video_off, direct)).Times(0);
    warn.UpdateLinkState(direct, st);

    // Test WmeNetwork_recovered
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_recovered, direct)).WillOnce(Return());
    EXPECT_CALL(m_indicator, GetNetworkIndex(direct)).WillRepeatedly(Return(NETWORK_VERY_GOOD));
    for(i=0; i<3; ++i){
        warn.UpdateLinkState(direct, st);
        ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_BREAKER);
    }
    warn.UpdateLinkState(direct, st);
    ASSERT_EQ(st.GetState(), CNetworkWarningState::ST_INIT);

    //Later good state will not trigger OnNetworkStatus
    EXPECT_CALL(m_sink, OnNetworkStatus(WmeNetwork_recovered, direct)).Times(0);
    warn.UpdateLinkState(direct, st);
}
