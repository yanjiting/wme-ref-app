//
//  CmDtlsTest.cpp
//  tpTest_mac
//
//  Created by Andy Hu on 9/9/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include <stdio.h>

#include "gtest/gtest.h"
#include "CmConnectionInterface.h"
#include "CmInetAddr.h"
#include "CmErrorNetwork.h"
#include "CmHttpClient.h"
#include "CmThreadManager.h"
#include <openssl/rsa.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include "SelfSignedCert.h"
#include "CmDtls.h"
#include "CmDtlsCertificate.h"
#include "CmString.h"

void print_memory(unsigned char* aBuf, int aLen, bool isServer)
{
    printf("\n");
    char log[32] = {0};
    sprintf(log, "%s: %d bytes data:", isServer? "server" : "client", aLen);
    
    if(aLen != 0){
        printf("%s\n", log);
    }
    
#if 1
    for (int i = 0; i < aLen; i++) {
        printf("%X", aBuf[i]);
    }
#endif
    
}

CCmMessageBlock* mbClientSendBuffer = NULL;
CCmMessageBlock* mbServerSendBuffer = NULL;

class CmDtlsSink : public ICmDtlsSink
{
public:
    CmDtlsSink(CmDtls* dtls)
    {
        m_pDtls = dtls;
    }
    virtual ~CmDtlsSink(){}
    
public:
    virtual int RecvDtlsData(CCmMessageBlock &mb, int aErr)
    {
        printf("Receive data from DTLS session. Data = %s, Length = %d\n", mb.GetTopLevelReadPtr(), mb.GetTopLevelLength());
    }
    
    virtual int SendDtlsData(CCmMessageBlock &mb, int &aErr)
    {
        int len = mb.GetChainedLength();
        if(m_pDtls-> m_bIsServer) {
            mbServerSendBuffer->Write(mb.GetTopLevelReadPtr(), mb.GetChainedLength());
        } else {
            mbClientSendBuffer->Write(mb.GetTopLevelReadPtr(), mb.GetChainedLength());
        }
        //print_memory(aBuf, (int)aLen, true, bIsServer);
        return len;
    }
    
    virtual int OnDtlsConnected()
    {
        printf("CmDtlsSink:OnConnect, Dtls session established, bIsServer = %d.\n", m_pDtls->m_bIsServer);
    }
    
private:
    CmDtls* m_pDtls;
};

class CCmDtlsTest : public testing::Test
{
public:
    CCmDtlsTest()
    {
    }
    
    virtual ~CCmDtlsTest()
    {
    }
    
    
    virtual void SetUp()
    {
        CCmThreadManager* pThreadManager = CCmThreadManager::Instance();
        
        m_pem = cm_test_tp_tls::m_default_cert;
        m_sault = "dkfjsdklfjlks";
        
        mbClientSendBuffer = new CCmMessageBlock(10*1024);
        mbServerSendBuffer = new CCmMessageBlock(10*1024);
    }
    
    virtual void TearDown()
    {
        delete(mbClientSendBuffer);
        delete(mbServerSendBuffer);
    }
    
protected:
    CCmString m_sault;
    CCmString m_pem;
};

TEST_F(CCmDtlsTest, TestDtlsConnect)
{
    int ret = -1;
    
    CmDtls dtls_client;
    CmDtlsSink pSinkClient(&dtls_client);
    dtls_client.InitSSl(&pSinkClient, false, m_pem, m_sault);
    CCmString client_fingerPrint = dtls_client.GetLocalFingerPrint();
    
    
    CmDtls dtls_server;
    CmDtlsSink pSinkServer(&dtls_server);
    dtls_server.InitSSl(&pSinkServer, true, m_pem, m_sault);
    CCmString server_fingerPrint = dtls_server.GetLocalFingerPrint();
    
    dtls_client.SetRemoteFingerPrint(server_fingerPrint, "");
    dtls_server.SetRemoteFingerPrint(client_fingerPrint, "");
    
    /*
     printf("client_fingerPrint: %s, length: %d\n", client_fingerPrint.c_str(), (int)client_fingerPrint.length());
     printf("server_fingerPrint: %s, length: %d\n\n\n", server_fingerPrint.c_str(), (int)server_fingerPrint.length());
     */
    
    //client send clienthello
    dtls_client.Connect();
    
    //server receive client hello
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client receive HelloVerifyRequest
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server receive client hello with cookie
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client verify server certification
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server verify client certification
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //DTLS handshake finished
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    EXPECT_TRUE(dtls_client.m_nDtlsState == CONNECTED);
    EXPECT_TRUE(dtls_server.m_nDtlsState == CONNECTED);
}

TEST_F(CCmDtlsTest, TestDtlsSendData)
{
    int ret = -1;
    
    CmDtls dtls_client;
    CmDtlsSink pSinkClient(&dtls_client);
    dtls_client.InitSSl(&pSinkClient, false, m_pem, m_sault);
    CCmString client_fingerPrint = dtls_client.GetLocalFingerPrint();
    
    
    CmDtls dtls_server;
    CmDtlsSink pSinkServer(&dtls_server);
    dtls_server.InitSSl(&pSinkServer, true, m_pem, m_sault);
    CCmString server_fingerPrint = dtls_server.GetLocalFingerPrint();
    
    dtls_client.SetRemoteFingerPrint(server_fingerPrint, "");
    dtls_server.SetRemoteFingerPrint(client_fingerPrint, "");

    
    /*
     printf("client_fingerPrint: %s, length: %d\n", client_fingerPrint.c_str(), (int)client_fingerPrint.length());
     printf("server_fingerPrint: %s, length: %d\n\n\n", server_fingerPrint.c_str(), (int)server_fingerPrint.length());
     */
    
    //client send clienthello
    dtls_client.Connect();
    
    //server receive client hello
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client receive HelloVerifyRequest
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server receive client hello with cookie
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client verify server certification
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server verify client certification
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //DTLS handshake finished
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    EXPECT_TRUE(dtls_client.m_nDtlsState == CONNECTED);
    EXPECT_TRUE(dtls_server.m_nDtlsState == CONNECTED);
    
    CCmMessageBlock mb(1024);
    char* client_test_data = "Hello, Dtls Server!";
    int len = strlen(client_test_data);
    mb.Write(client_test_data, len);
    dtls_client.WriteData(mb);

    ret = dtls_server.ReadData(*mbClientSendBuffer);
    EXPECT_TRUE(ret == len);
    
    CCmMessageBlock mb2(1024);
    char* server_test_data = "Hello, Dtls Client!";
    len = strlen(server_test_data);
    mb2.Write(server_test_data, len);
    dtls_server.WriteData(mb2);
    
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    EXPECT_TRUE(ret == len);
}

TEST_F(CCmDtlsTest, TestDtlsExportSSLKey)
{
    int ret = -1;
    
    CmDtls dtls_client;
    CmDtlsSink pSinkClient(&dtls_client);
    dtls_client.InitSSl(&pSinkClient, false, m_pem, m_sault);
    CCmString client_fingerPrint = dtls_client.GetLocalFingerPrint();
    
    
    CmDtls dtls_server;
    CmDtlsSink pSinkServer(&dtls_server);
    dtls_server.InitSSl(&pSinkServer, true, m_pem, m_sault);
    CCmString server_fingerPrint = dtls_server.GetLocalFingerPrint();
    
    dtls_client.SetRemoteFingerPrint(server_fingerPrint, "");
    dtls_server.SetRemoteFingerPrint(client_fingerPrint, "");
    
    /*
     printf("client_fingerPrint: %s, length: %d\n", client_fingerPrint.c_str(), (int)client_fingerPrint.length());
     printf("server_fingerPrint: %s, length: %d\n\n\n", server_fingerPrint.c_str(), (int)server_fingerPrint.length());
     */
    
    //client send clienthello
    dtls_client.Connect();
    
    //server receive client hello
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client receive HelloVerifyRequest
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server receive client hello with cookie
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client verify server certification
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server verify client certification
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //DTLS handshake finished
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    EXPECT_TRUE(dtls_client.m_nDtlsState == CONNECTED);
    EXPECT_TRUE(dtls_server.m_nDtlsState == CONNECTED);
    
    
    DtlsSrtpDerivedKeySalt clientKey;
    
    ret = dtls_client.ExportKey(clientKey);
    
    DtlsSrtpDerivedKeySalt serverKey;
    ret = dtls_server.ExportKey(serverKey);
    
    ret = memcmp(clientKey.clientKey, serverKey.clientKey, sizeof(clientKey.clientKey));
    EXPECT_TRUE(ret == 0);

    ret = memcmp(clientKey.serverKey, serverKey.serverKey, sizeof(clientKey.serverKey));
    EXPECT_TRUE(ret == 0);
    
    ret = memcmp(clientKey.clientSalt, serverKey.clientSalt, sizeof(clientKey.clientSalt));
    EXPECT_TRUE(ret == 0);
    
    ret = memcmp(clientKey.serverSalt, serverKey.serverSalt, sizeof(clientKey.serverSalt));
    EXPECT_TRUE(ret == 0);
}

TEST_F(CCmDtlsTest, TestDtlsCreateCertification)
{
    int ret = -1;
    BIO *bio_err;
    CCmString x509;
    CCmString pkey;
    
    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
    
    bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
    
    ret = CreateCert(x509, pkey, 1024, 0, 365);
    
    EXPECT_TRUE(ret == 0);

    
#ifndef OPENSSL_NO_ENGINE
    ENGINE_cleanup();
#endif
    CRYPTO_cleanup_all_ex_data();

    CRYPTO_mem_leaks(bio_err);
    BIO_free(bio_err);

}

TEST_F(CCmDtlsTest, TestDtlsCertificationFingerPrint)
{
    int ret = -1;
    
    CmDtls dtls_client;
    CmDtlsSink pSinkClient(&dtls_client);
    dtls_client.InitSSl(&pSinkClient, false, m_pem, m_sault);
    CCmString client_fingerPrint = dtls_client.GetLocalFingerPrint();
    
    
    CmDtls dtls_server;
    CmDtlsSink pSinkServer(&dtls_server);
    dtls_server.InitSSl(&pSinkServer, true, m_pem, m_sault);
    CCmString server_fingerPrint = dtls_server.GetLocalFingerPrint();
    
    dtls_client.SetRemoteFingerPrint(server_fingerPrint, "");
    dtls_server.SetRemoteFingerPrint(client_fingerPrint, "");
    
    /*
    printf("client_fingerPrint: %s, length: %d\n", client_fingerPrint.c_str(), (int)client_fingerPrint.length());
    printf("server_fingerPrint: %s, length: %d\n\n\n", server_fingerPrint.c_str(), (int)server_fingerPrint.length());
    */
    
    //client send clienthello
    dtls_client.Connect();
    
    //server receive client hello
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client receive HelloVerifyRequest
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server receive client hello with cookie
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //client verify server certification
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    //server verify client certification
    ret = dtls_server.ReadData(*mbClientSendBuffer);
    
    //DTLS handshake finished
    ret = dtls_client.ReadData(*mbServerSendBuffer);
    
    EXPECT_TRUE(dtls_client.m_nDtlsState == CONNECTED);
    EXPECT_TRUE(dtls_server.m_nDtlsState == CONNECTED);
}