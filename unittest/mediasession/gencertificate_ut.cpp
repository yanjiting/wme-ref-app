//
//  gencertificate_ut.cpp
//  MediaSessionUT_mac
//
//  Created by Wilson Chen on 1/22/16.
//  Copyright © 2016 cisco spark. All rights reserved.
//

#include "gtest/gtest.h"
#include "stringhelper.h"
#include "CmUtilMisc.h"
#include "CertGenerator.h"
#include "CmThreadManager.h"

using namespace wme;

class CTestGenerateCertificate : public testing::Test, public CCertficateGenerator {
protected:
    virtual void SetUp() {
    }
    
    // Tears down the test fixture.
    virtual void TearDown() {
        Stop();
        Join();
    }
    
    virtual int CreateCertificate(CCmString &cert, CCmString &key) {
        cert = m_sCert;
        key = m_sKey;
        
        if (m_nDelay > 0) {
            SleepMs(m_nDelay);
        }
        
        return 0;
    }

    const CCmString m_sCert = "Created Cert";
    const CCmString m_sKey = "This is Key";
    int m_nDelay = 0;
    CCmString m_sActualCert, m_sActualKey;
};

TEST_F(CTestGenerateCertificate, GenerateCertAborted) {
    CCmThreadManager::Instance();
    m_nDelay = 200;
    
    SetStop();
    GenerateKey();
    ASSERT_TRUE(GenerateKey() == CM_ERROR_IN_PROGRESS);
    m_bInited = true;
    
    CCmThreadManager::CleanupOnlyOne();
}

TEST_F(CTestGenerateCertificate, GenerateCertCreated) {
    CCmThreadManager::Instance();
    
    m_nDelay = 50;
    SetStop();
    GenerateKey();
    m_bInited = true;
    
    auto onCertCreated = [this] () -> CmResult
    {
        m_sActualCert = GetCertificate();
        m_sActualKey = GetPrivateKey();
        return CM_OK;
    };
    
    ICmEvent *pCallbackEvent = new CCmInvokeEvent<decltype(onCertCreated)>(onCertCreated);
    
    RegisterObserver(pCallbackEvent, TT_MAIN);
    for (int i = 0; i < 50; i++) {
        SleepMsWithLoop(10);
        if (!m_sActualCert.empty()) {
            ASSERT_EQ(m_sActualCert, m_sCert);
            ASSERT_EQ(m_sActualKey, m_sKey);
            break;
        }

    }
    
    CCmThreadManager::CleanupOnlyOne();
}
