//
//  stuntrace_ut.cpp
//  MediaSessionUT_mac
//
//  Created by Wilson Chen on 1/14/16.
//  Copyright © 2016 cisco spark. All rights reserved.
//

#include "gtest/gtest.h"
#include "MediaConnection.h"
#include "StunTrace.h"
#include "TraceServer.h"
#include "json.h"
#include <functional>

using namespace wme;

class CTraceServerTest : public ::testing::Test, public CTraceServer, public IWmeTraceServerSink
{
protected:
    virtual void SetUp() {
        InitMediaEngine(false);
        pSink = this;
    }
    
    // Tears down the test fixture.
    virtual void TearDown() {
        auto l = std::bind(&CTraceServerTest::CleanUp, this);
        CmInvokeInThread(GetThread(TT_NETWORK), false, l);
        UninitMediaEngine();
    }
    
    virtual WMERESULT OnTraceServerResult(WmeStunTraceResult reason, const char* szDetail) {
        if (szDetail) {
            m_sResult = szDetail;
            m_aResultJson = json::Deserialize(m_sResult);
        }
        return 0;
    }
    
    void CheckComplete(int nMilliSeconds) {
        for (int i = 0; i < nMilliSeconds / 50; i++) {
            if (m_bRunning) {
                SleepMsWithLoop(50);
            } else {
                break;
            }
        }
    }
    
    std::string m_sResult;
    json::Value m_aResultJson;
};

TEST_F(CTraceServerTest, TraceServer_SetNode) {
    std::string input;
    StartTrace("", 0);
    EXPECT_TRUE(m_hashedInput.size() == 0);
     
    m_nMaxKickCount = 20;
    input = "{\"a\": {\"https\": [\"https://lqtservice-web.wbx2.com/lqt/api/v1/ping\"]}}";
    StartTrace(input.c_str(), input.length());
    CheckComplete(1000);
    //EXPECT_TRUE(m_aResultJson["a"]["https"]["reachable"].ToString() == "true");
     
    input = "{\"b\": {\"udp\": [\"stun:biston-docker-int-02-public-direct-600.wbx2.com:33434\"]}}";
    StartTrace(input.c_str(), input.length());
    CheckComplete(1000);
    //EXPECT_TRUE(m_aResultJson["b"]["udp"]["reachable"].ToString() == "true");

    /*
    input = "{\"c\": {\"tcp\": [\"stun:lqtservice-web.wbx2.com:33434\"]}}";
    StartTrace(input.c_str(), input.length());
    CheckComplete(3200);
    EXPECT_TRUE(m_aResultJson["c"]["tcp"]["reachable"].ToString() == "true");

    input = "{\"d\": {\"xtls\": [\"stun:lqtservice-web.wbx2.com\"]}}";
    StartTrace(input.c_str(), input.length());
    CheckComplete(3200);
    EXPECT_TRUE(m_aResultJson["d"]["xtls"]["reachable"].ToString() == "true");
     */

    input = "{\"squared.US-EAST.*\": {\"udp\": [\"stun:biston-docker-int-02-public-direct-600.wbx2.com:33434\", \"stun:biston-docker-int-02-public-direct-600.wbx2.com:3478\"], \"tcp\": [\"stun:biston-docker-int-02-public-direct-600.wbx2.com:33434\"], \"https\": [\"https://sx-controller-19.cisco.com/api/v1/ping\"], \"xtls\": [\"stun:biston-docker-int-02-public-direct-600.wbx2.com:33434\"] }, \"squared.US-WEST.*\": {\"udp\": [ \"stun:104.130.225.108:3478\"],\"tcp\": [ \"stun:lqtservice-lqtproda1-403-riad-public.wbx2.com:33434\"],\"https\": [\"https://lqtservice-web.wbx2.com/lqt/api/v1/ping\"],\"xtls\": [ \"stun:lqtservice-lqtproda1-403-riad-public.wbx2.com:33434\"]}}";
    StartTrace(input.c_str(), input.length());
    EXPECT_TRUE(m_hashedInput.size() == 2);
    CheckComplete(1000);
}
