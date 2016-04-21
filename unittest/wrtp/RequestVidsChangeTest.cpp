//
//  RequestVidsChangeTest.cpp
//  wrtpTest_mac
//
//  Created by Hank Peng on 1/14/16.
//  Copyright © 2016 cisco. All rights reserved.
//

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>
#include <memory>
#include <functional>

#include "simulcastresponser.h"

using namespace wrtp;

class CRequestVidsChangeTest : public ::testing::Test
{
public:
    CRequestVidsChangeTest()
    : m_requestSubChannels()
    , m_currentVids()
    , m_callbackCount(0)
    {}
    ~CRequestVidsChangeTest() = default;
    
    void OnVidChanged(const uint8_t* newVidArray, uint8_t newVidCount) {
        ++ m_callbackCount;
        
        if (!newVidArray || !newVidCount) {
            m_currentVids.clear();
        }
        m_currentVids.assign(newVidArray, newVidArray + newVidCount);
    }
    
protected:
    virtual void SetUp() {
        m_requestSubChannels.reset(new CSimulcastRequestSubchannels());
        m_requestSubChannels->SetVidChangedCallback(std::bind(&CRequestVidsChangeTest::OnVidChanged, this, std::placeholders::_1, std::placeholders::_2));
        
        m_currentVids.clear();
        m_callbackCount = 0;
        
    }
    
    virtual void TearDown() {
    }
    
protected:
    std::unique_ptr<CSimulcastRequestSubchannels>   m_requestSubChannels;
    VidVector                                       m_currentVids;
    int                                             m_callbackCount;
};

TEST_F(CRequestVidsChangeTest, Test_VID_Change)
{
    VidVector requestedVids = {3, 2, 1, 0};
    VidVector invalidVids   = {0, 3};
    VidVector validVids     = {1, 2};
    
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    ASSERT_EQ(0, m_currentVids.size());
    ASSERT_EQ(0, m_callbackCount);
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(1, m_callbackCount);
}

TEST_F(CRequestVidsChangeTest, Test_VID_Multiple_Invalid_Changes)
{
    VidVector requestedVids = {3, 2, 1, 0};
    VidVector invalidVids   = {0, 3};
    VidVector validVids     = {1, 2};
    
    int callbackCount = 0;
    
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    
    // Nothing changed
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    // More VIDs are invalid
    invalidVids.assign({0, 3, 1});
    validVids.assign({2});
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    // Less VIDs are invalid
    invalidVids.assign({0});
    validVids.assign({1, 2, 3});
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    // All VIDs are valid
    invalidVids.assign({});
    validVids.assign({0, 1, 2, 3});
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
}

TEST_F(CRequestVidsChangeTest, Test_VID_Multiple_Request_Invalid_Changes)
{
    VidVector requestedVids = {3, 2, 1, 0};
    VidVector invalidVids   = {0, 3};
    VidVector validVids     = {1, 2};
    
    int callbackCount = 0;
    
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
   
    // Nothing changed
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    // Request & invalid changed
    requestedVids.assign({4, 3 ,2 ,1, 0});
    invalidVids.assign({0, 2, 4});
    validVids.assign({1, 3});
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    // Reset to empty
    requestedVids.assign({});
    invalidVids.assign({});
    validVids.assign({});
    
    m_requestSubChannels->UpdateRequestedVids(requestedVids.data(), requestedVids.size());
    ASSERT_EQ(callbackCount, m_callbackCount);
    
    m_requestSubChannels->UpdateInvalidVids(invalidVids.data(), invalidVids.size());
    ++ callbackCount;
    ASSERT_EQ(validVids, m_currentVids);
    ASSERT_EQ(callbackCount, m_callbackCount);
}
