//
//  RTPStreamManagementTest.cpp
//  wrtpTest_mac
//
//  Created by hankpeng on 3/31/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rtpstream.h"
#include "videostream.h"
#include "rtpchannel.h"
#include "rtpstreammanager.h"
#include "rtpsessionconfig.h"
#include "rtpsessionclient.h"

#include <memory>

using namespace wrtp;


class CRTPMultiStreamWeakPtrManagerTest : public ::testing::Test
{
public:
    CRTPMultiStreamWeakPtrManagerTest()
        : m_multistreamManager()
        , m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VOIP))
        , m_session(nullptr)
        , m_channel(nullptr)
        , m_channelId(1000) {}

    ~CRTPMultiStreamWeakPtrManagerTest() {}
protected:
    virtual void SetUp()
    {
        m_multistreamManager.Clear();

        WRTPSessionParams sessionParams;
        sessionParams.sessionType   = RTP_SESSION_WEBEX_VOIP;
        sessionParams.enableRTCP    = true;

        m_session = WRTPCreateRTPSessionClient(sessionParams);
        m_session->IncreaseReference();

        WRTPChannelParams channelParams;
        m_channel = m_session->CreateRTPChannel(++m_channelId, channelParams);
        m_channel->IncreaseReference();
    }

    virtual void TearDown()
    {
        if (m_channel) {
            m_channel->DecreaseReference();
            m_channel = nullptr;
        }
        if (m_session) {
            m_session->DecreaseReference();
            m_session = nullptr;
        }
    }

    CRTPStreamPtr CreateStream(uint32_t ssrc)
    {
        return CRTPStreamPtr(new CRTPStreamVoIP(dynamic_cast<CRTPSessionClient *>(m_session), ssrc, 48000, wrtp::STREAM_OUT));
    }

protected:
    CRTPMultiStreamWeakPtrManager   m_multistreamManager;
    RTPSessionContextSharedPtr      m_sessionContext;
    IRTPSessionClient              *m_session;
    IRTPChannel                    *m_channel;
    uint32_t                          m_channelId;
};

TEST_F(CRTPMultiStreamWeakPtrManagerTest, Add_Remove_Test)
{
    CRTPStreamPtr stream1(CreateStream(1000));
    m_multistreamManager.AddStream(stream1);
    ASSERT_EQ(1, m_multistreamManager.GetSize());

    CRTPStreamPtr stream2(CreateStream(1000));
    m_multistreamManager.AddStream(stream2);
    ASSERT_EQ(2, m_multistreamManager.GetSize());

    CRTPStreamPtr stream3(CreateStream(1001));
    m_multistreamManager.AddStream(stream3);
    ASSERT_EQ(3, m_multistreamManager.GetSize());

    m_multistreamManager.RemoveStream(stream1);
    ASSERT_EQ(2, m_multistreamManager.GetSize());

    m_multistreamManager.RemoveStream(stream2);
    ASSERT_EQ(1, m_multistreamManager.GetSize());

    m_multistreamManager.RemoveStream(stream3);
    ASSERT_EQ(0, m_multistreamManager.GetSize());
}

TEST_F(CRTPMultiStreamWeakPtrManagerTest, Multiple_Remove_Test)
{
    CRTPStreamPtr stream1(CreateStream(1000));
    m_multistreamManager.AddStream(stream1);
    ASSERT_EQ(1, m_multistreamManager.GetSize());

    CRTPStreamPtr stream2(CreateStream(1000));
    m_multistreamManager.AddStream(stream2);
    ASSERT_EQ(2, m_multistreamManager.GetSize());

    CRTPStreamPtr stream3(CreateStream(1001));
    m_multistreamManager.AddStream(stream3);
    ASSERT_EQ(3, m_multistreamManager.GetSize());

    m_multistreamManager.RemoveMultiStream(1000);
    ASSERT_EQ(1, m_multistreamManager.GetSize());
}

TEST_F(CRTPMultiStreamWeakPtrManagerTest, Get_Streams_Test)
{
    CRTPStreamPtr stream1(CreateStream(1000));
    m_multistreamManager.AddStream(stream1);
    ASSERT_EQ(1, m_multistreamManager.GetSize());

    CRTPStreamPtr stream2(CreateStream(1000));
    m_multistreamManager.AddStream(stream2);
    ASSERT_EQ(2, m_multistreamManager.GetSize());

    CRTPStreamPtr stream3(CreateStream(1001));
    m_multistreamManager.AddStream(stream3);
    ASSERT_EQ(3, m_multistreamManager.GetSize());

    std::vector<CRTPStreamWeakPtr> streams;
    m_multistreamManager.GetStreams(streams);
    ASSERT_EQ(3, streams.size());

    m_multistreamManager.GetStreams(1000, streams);
    ASSERT_EQ(2, streams.size());

    m_multistreamManager.GetStreams(1001, streams);
    ASSERT_EQ(1, streams.size());

    m_multistreamManager.Clear();
    m_multistreamManager.GetStreams(streams);
    ASSERT_EQ(0, m_multistreamManager.GetSize());
    ASSERT_EQ(0, streams.size());
}

TEST_F(CRTPMultiStreamWeakPtrManagerTest, Expiration_Test)
{
    CRTPStreamPtr stream1(CreateStream(1000));
    m_multistreamManager.AddStream(stream1);
    ASSERT_EQ(1, m_multistreamManager.GetSize());

    CRTPStreamPtr stream2(CreateStream(1000));
    m_multistreamManager.AddStream(stream2);
    ASSERT_EQ(2, m_multistreamManager.GetSize());

    CRTPStreamPtr stream3(CreateStream(1001));
    m_multistreamManager.AddStream(stream3);
    ASSERT_EQ(3, m_multistreamManager.GetSize());

    std::vector<CRTPStreamWeakPtr> streams;
    m_multistreamManager.GetStreams(streams);
    ASSERT_EQ(3, streams.size());

    stream1.reset();
    ASSERT_EQ(2, m_multistreamManager.GetSize());
    m_multistreamManager.GetStreams(1000, streams);
    ASSERT_EQ(1, streams.size());

    stream2.reset();
    stream3.reset();
    m_multistreamManager.ExpirationCollect();
    ASSERT_EQ(0, m_multistreamManager.GetSize());
}

//////////////////////////////////////////////////////////////
using VideoStreamManager = CChannelSendingStreamManager<CVideoStreamClassifier>;
class CChannelSendingStreamManagerTest : public ::testing::Test
{
public:
    CChannelSendingStreamManagerTest(): m_manger(), m_session(nullptr)
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP  = true;
        
        m_session = WRTPCreateRTPSessionClient(sessionParams);
        EXPECT_TRUE(nullptr != m_session);
        m_session->IncreaseReference();
    }
    
    ~CChannelSendingStreamManagerTest() {
        m_session->DecreaseReference();
        m_session = nullptr;
    }

protected:
    virtual void SetUp() {
        m_manger.reset(new VideoStreamManager());
    }
    
    virtual void TearDown() {
    }
    
    void AddStream(uint8_t DID, uint32_t ssrc) {
        CVideoStreamClassifier classifier(DID, nullptr, 0);
        auto stream = std::make_shared<CVideoStream>((CRTPSessionBase*)m_session, ssrc, 9000, STREAM_OUT);
        m_manger->AddStream(classifier, stream);
    }
   
    CRTPStreamPtr QueryByDID(uint8_t DID) {
        CVideoStreamClassifier classifier(DID, nullptr, 0);
        return m_manger->GetStream(classifier);
    }
    
    CRTPStreamPtr QueryBySSRC(uint32_t ssrc) {
        return m_manger->GetStream(ssrc);
    }
    
    void DeleteByDID(uint8_t DID) {
        CVideoStreamClassifier classifier(DID, nullptr, 0);
        return m_manger->RemoveStream(classifier);
    }
    
    void DeleteBySSRC(uint32_t ssrc) {
        return m_manger->RemoveStream(ssrc);
    }
    
protected:
    std::unique_ptr<VideoStreamManager> m_manger;
    IRTPSessionClient*                  m_session;
};

TEST_F(CChannelSendingStreamManagerTest, Test_Original_State)
{
    ASSERT_EQ(0, m_manger->GetStreamCount());
    ASSERT_EQ(0, m_manger->GetQueryCount());
    ASSERT_EQ(0, m_manger->GetHitCount());
}

TEST_F(CChannelSendingStreamManagerTest, Test_Query_Cache_Hit)
{
    uint queryCount = 0;
    uint hitCount   = 0;

    // Note: Each time a stream is added, we will query first to see if it already exists
    AddStream(1, 1000); { ++queryCount; }
    AddStream(2, 2000); { ++queryCount; }
    AddStream(3, 3000); { ++queryCount; }
    
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    
    auto stream = QueryByDID(3); { ++queryCount; ++hitCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryByDID(3); { ++queryCount; ++hitCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryByDID(2); { ++queryCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(2000); { ++queryCount; ++hitCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(2000); { ++queryCount; ++hitCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryByDID(-1); { ++queryCount; }
    ASSERT_TRUE(!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
}

TEST_F(CChannelSendingStreamManagerTest, Test_Query_Cache_Hit_with_Remove)
{
    uint queryCount = 0;
    uint hitCount   = 0;
    
    AddStream(1, 1000); { ++queryCount; }
    AddStream(2, 2000); { ++queryCount; }
    AddStream(3, 3000); { ++queryCount; }
    
    DeleteBySSRC(3000);
    // {1: 1000, 2: 2000}
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    ASSERT_EQ(2,          m_manger->GetStreamCount());
   
    auto stream = QueryByDID(2); { ++queryCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(2000); { ++queryCount; ++hitCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    DeleteByDID(2);
    // {1: 1000}
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    ASSERT_EQ(1,          m_manger->GetStreamCount());
    
    stream = QueryByDID(2); { ++queryCount; }
    ASSERT_TRUE(!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(2000); { ++queryCount; }
    ASSERT_TRUE(!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(1000); { ++queryCount; }
    ASSERT_TRUE(!!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    m_manger->Clear();
    // {}
    stream = QueryByDID(1); { ++queryCount; }
    ASSERT_TRUE(!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
    
    stream = QueryBySSRC(1000); { ++queryCount; }
    ASSERT_TRUE(!stream);
    ASSERT_EQ(queryCount, m_manger->GetQueryCount());
    ASSERT_EQ(hitCount,   m_manger->GetHitCount());
}
