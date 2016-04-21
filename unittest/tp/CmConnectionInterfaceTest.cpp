
#include "gtest/gtest.h"
#include "CmConnectionInterface.h"
#include "CmInetAddr.h"
#include "CmErrorNetwork.h"
#include "CmHttpClient.h"
#include "CmThreadManager.h"

#include "SelfSignedCert.h"


class CCmAcceptorSink : public ICmAcceptorConnectorSink 
{
public:
	virtual void OnConnectIndication(
                CmResult aReason,
                ICmTransport *aTrpt,
                ICmAcceptorConnectorId *aRequestId)
        {
	}	
}; 

class CCmConnectionInterfaceTest : public testing::Test
{
public:
        CCmConnectionInterfaceTest()
        {
        }

        virtual ~CCmConnectionInterfaceTest()
        {
        }


        virtual void SetUp()
        {
            CCmConnectionManager::Instance()->CreateConnectionServer(CCmConnectionManager::CTYPE_UDP, m_pAcceptor.ParaOut());
        }

        virtual void TearDown()
        {
            if(m_pAcceptor) {
                m_pAcceptor->StopListen(0);
            }
        }

protected:
	CCmComAutoPtr<ICmAcceptor> m_pAcceptor;
};


TEST_F(CCmConnectionInterfaceTest, TestStartListenOK)
{
	
	CmResult ret;
	CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

	CCmAcceptorSink theSink;
	ret = m_pAcceptor->StartListen(&theSink, addr);	
		
	EXPECT_TRUE(ret == CM_OK);

}

TEST_F(CCmConnectionInterfaceTest, TestStartListenFailbyNULLSink)
{

        CmResult ret;
        CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

        CCmAcceptorSink theSink;
        ret = m_pAcceptor->StartListen(NULL, addr);

        EXPECT_TRUE(ret == CM_ERROR_INVALID_ARG);

}

TEST_F(CCmConnectionInterfaceTest, TestStartListenFailbyTwiceListen)
{

        CmResult ret;
        CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

        CCmAcceptorSink theSink;
        ret = m_pAcceptor->StartListen(&theSink, addr);

        EXPECT_TRUE(ret == CM_OK);

	ret = m_pAcceptor->StartListen(&theSink, addr);

	EXPECT_TRUE(ret == CM_ERROR_ALREADY_INITIALIZED);
}


TEST_F(CCmConnectionInterfaceTest, TestStopListenOK)
{
	
	CmResult ret;
        CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

        CCmAcceptorSink theSink;
        ret = m_pAcceptor->StartListen(&theSink, addr);

        EXPECT_TRUE(ret == CM_OK);

	ret = m_pAcceptor->StopListen(CM_OK);

	EXPECT_TRUE(ret == CM_OK);

}

TEST_F(CCmConnectionInterfaceTest, TestStopListenbyTwiceStop)
{

        CmResult ret;
        CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

        CCmAcceptorSink theSink;
        ret = m_pAcceptor->StartListen(&theSink, addr);

        EXPECT_TRUE(ret == CM_OK);

        ret = m_pAcceptor->StopListen(CM_OK);

	EXPECT_TRUE(ret == CM_OK);

	ret = m_pAcceptor->StopListen(CM_OK);

	EXPECT_TRUE(ret == CM_OK);

}


TEST_F(CCmConnectionInterfaceTest, TestConnectOK)
{
	
	CmResult ret;	
        CCmInetAddr addr;
        addr.Set("127.0.0.1", 8000);

        CCmAcceptorSink theSink;
        ret = m_pAcceptor->StartListen(&theSink, addr);
	
	EXPECT_TRUE(ret == CM_OK);


	CCmInetAddr peerAddr;
	peerAddr.Set("10.224.193.98", 9000);	
	ret = m_pAcceptor->Connect(peerAddr);

	EXPECT_TRUE(ret == CM_OK);
	
}

class CCmTLSTest : public testing::Test, public ICmAcceptorConnectorSink
{    
public:
    CCmTLSTest() {
        
    }
    
    virtual void SetUp()
    {
        CCmConnectionManager::Instance()->CreateConnectionClient(CCmConnectionManager::CTYPE_SSL_DIRECT, m_connector.ParaOut());
    }
    
    virtual void TearDown() {
        if(m_Transport)
            m_Transport->Disconnect(0);
        m_Transport = NULL;
        
        if(m_connector)
            m_connector->CancelConnect();
        m_connector = NULL;
    }

    virtual void OnConnectIndication(
                                     CmResult aReason,
                                     ICmTransport *aTrpt,
                                     ICmAcceptorConnectorId *aRequestId)
    {
        EXPECT_TRUE(CM_SUCCEEDED(aReason));
        m_Transport = aTrpt;
    }
    
protected:
    CCmComAutoPtr<ICmConnector> m_connector;
    CCmComAutoPtr<ICmTransport> m_Transport;
};

/*
TEST_F(CCmTLSTest, Test_ECDH_AES256) {
    BOOL bVerifyPeerCertificate = FALSE;
    m_connector->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerifyPeerCertificate);
    m_connector->AsycConnect(this, CCmInetAddr("173.39.168.45:443"));
    EXPECT_TRUE(TRUE);
    while(m_Transport == NULL)
    {
        SleepMsWithLoop(100);
    }
}
*/

class CCmHTTPClientTest : public testing::Test, public ICmHttpClientSink
{
public:
    CCmHTTPClientTest() {
        m_bReceived = FALSE;
    }
    
    virtual void SetUp()
    {
        tp_create_httpclient(&m_httpClient);
    }
    
    virtual void TearDown() {
        if(m_httpClient)
            tp_destroy_httpclient(m_httpClient);
        m_httpClient = NULL;
    }
    
    virtual void OnReceive(CmResult nResult, CCmMessageBlock *aData)
    {
        EXPECT_TRUE(CM_SUCCEEDED(nResult));
        EXPECT_TRUE(aData && aData->GetChainedLength() > 0);
        if(aData && aData->GetChainedLength())
        {
            char szData[1024] = {0};
            aData->Read(szData, 1024);
        }
        m_bReceived = TRUE;
    }
    
protected:
    BOOL m_bReceived;
    ICmHttpClient* m_httpClient;
};

/*
TEST_F(CCmHTTPClientTest, Test_HTTPS) {
    BOOL bVerifyPeerCertificate = FALSE;
    EXPECT_TRUE(m_httpClient != NULL);
    m_httpClient->Open("GET", "https://173.39.168.45/", this, true);
    
    //cisco.com which can pass the case
    //m_httpClient->Open("GET", "https://173.37.145.84/", this, true);
    m_httpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerifyPeerCertificate);
    CCmMessageBlock mb(1);
    m_httpClient->Send(&mb);
    while(!m_bReceived)
    {
        SleepMsWithLoop(100);
    }
}


TEST_F(CCmHTTPClientTest, Test_HTTPS_VERIFY_FINGERPRINT) {
    BOOL bVerifyPeerCertificate = TRUE;
    EXPECT_TRUE(m_httpClient != NULL);
    
    m_httpClient->Open("GET", "https://173.39.168.45/", this, true);
    m_httpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerifyPeerCertificate);
    m_httpClient->SetOption(CM_OPT_TLS_PEER_CERT_FINGERPRINT,
                            (void*)"SHA-1 A4:59:27:58:70:F1:8A:7B:C8:BD:54:55:DF:64:B6:D2:E0:1D:09:E2");
    CCmMessageBlock mb(1);
    m_httpClient->Send(&mb);
    while(!m_bReceived)
    {
        SleepMsWithLoop(100);
    }
}
*/


class CCmTLSAcceptorTest : public testing::Test,
                           public ICmAcceptorConnectorSink,
                           public ICmTransportSink
{
public:
    CCmTLSAcceptorTest() {
        
    }
    
    virtual void SetUp()
    {
        m_bDone = false;
        CCmConnectionManager::Instance()->CreateConnectionServer(CCmConnectionManager::CTYPE_SSL_DIRECT, m_acceptor.ParaOut());
    }
    
    virtual void TearDown() {
        if(m_Transport)
            m_Transport->Disconnect(0);
        m_Transport = NULL;
        
        if(m_acceptor)
            m_acceptor->StopListen(0);
        m_acceptor = NULL;
        m_bDone = true;
    }
    
    virtual void OnReceive(CCmMessageBlock &aData,
                           ICmTransport *aTrptId,
                           CCmTransportParameter *aPara = NULL)
    {
        CCmString sHeader = "HTTP/1.x 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n";
        CCmMessageBlock aSendHeader(1024);
        aSendHeader.Write(sHeader.c_str(), (DWORD)sHeader.size());
        aTrptId->SendData(aSendHeader);
        
        CmResult rv = CM_OK;
        while(CM_SUCCEEDED(rv)) {
            CCmString sData = cm_test_tp_tls::m_default_cert;
            CCmMessageBlock aSendData(4096);
            aSendData.Write(sData.c_str(), (DWORD)sData.size());
            rv = aTrptId->SendData(aSendData);
        }
        
    }
    
    virtual void OnSend(ICmTransport *aTrptId,
                        CCmTransportParameter *aPara = NULL)
    {
        CCmString sHeader = "================Done===================";
        CCmMessageBlock aSendHeader(1024);
        aSendHeader.Write(sHeader.c_str(), (DWORD)sHeader.size());
        aTrptId->SendData(aSendHeader);
    }
    
    virtual void OnDisconnect(CmResult aReason,
                              ICmTransport *aTrptId)
    {
        m_bDone = true;
    }

    
    virtual void OnConnectIndication(CmResult aReason,
                                     ICmTransport *aTrpt,
                                     ICmAcceptorConnectorId *aRequestId)
    {
        EXPECT_TRUE(CM_SUCCEEDED(aReason));
        m_Transport = aTrpt;
        m_Transport->OpenWithSink(this);
    }
    
protected:
    CCmComAutoPtr<ICmAcceptor> m_acceptor;
    CCmComAutoPtr<ICmTransport> m_Transport;
    bool m_bDone;
};
/*
TEST_F(CCmTLSAcceptorTest, Test_TLS_Server)
{
    CCmString sPem = cm_test_tp_tls::m_default_cert;
    CmResult rv = m_acceptor->SetOption(CM_OPT_TLS_SET_CERTIFICATE, &sPem);
    EXPECT_TRUE(rv == 0);
    m_acceptor->StartListen(this, CCmInetAddr("0.0.0.0", 8443));
    while(!m_bDone)
    {
        SleepMsWithLoop(100);
    }
}
*/
TEST(Test_InetAddr, Test_XLAT64Sync) {
    CCmInetAddr addr("180.149.132.47", 5000);
    EXPECT_TRUE(CM_SUCCEEDED(addr.XLAT46Sync()));
    CCmString str = addr.GetIpDisplayName();
    EXPECT_TRUE(str == "64:ff9b::b495:842f");
}

TEST(Test_InetAddr, Test_XLAT64) {
    CCmInetAddr addrDns("webex.com", 80);
    CCmInetAddr addrV6("2001:db8:c000:221::1", 5000);
    EXPECT_FALSE(CM_SUCCEEDED(addrDns.XLAT46({{{ 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}, 96)));
    EXPECT_FALSE(CM_SUCCEEDED(addrV6.XLAT46({{{ 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}, 96)));
    
    CCmInetAddr addr("180.149.132.47", 5000);
    
    EXPECT_TRUE(CM_SUCCEEDED(addr.XLAT46({{{ 0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}, 96)));
    CCmString str = addr.GetIpDisplayName();
    EXPECT_TRUE(str == "64:ff9b::b495:842f");
    EXPECT_TRUE(5000 == addr.GetPort());
    
    struct sockaddr_storage sock_storage;
    struct in6_addr prefix, ip6addr;
    unsigned int nPrefixLen;
    struct sockaddr_in6 *sa = (sockaddr_in6 *)&sock_storage;
    
    inet_pton(AF_INET6, "2001:db8:122:344::192.0.0.171", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 96);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    str = addr.GetIpDisplayName();
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:122:344::b495:842f");
    
    inet_pton(AF_INET6, "2001:db8:122:344:c0:00:aa00::", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 64);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:122:344:b4:9584:2f00::" || addr.GetIpDisplayName() == "2001:db8:122:344:b4:9584:2f00:0");

    inet_pton(AF_INET6, "2001:db8:122:3c0:0:0ab::", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 56);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:122:3b4:95:842f::");

    inet_pton(AF_INET6, "2001:db8:122:c000:0:ab00::", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 48);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:122:b495:84:2f00::");

    inet_pton(AF_INET6, "2001:db8:1c0:0:ab::", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 40);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:1b4:9584:2f::");

    inet_pton(AF_INET6, "2001:db8:c000:ab::", &ip6addr);
    memcpy( &(sa->sin6_addr.s6_addr), &ip6addr, sizeof(struct in6_addr));
    addr.ParsePrefixLength(&sock_storage, sizeof(sockaddr_in6), prefix, nPrefixLen);
    EXPECT_TRUE(nPrefixLen == 32);
    addr.SetType(AF_INET);
    addr.XLAT46(prefix, nPrefixLen);
    EXPECT_TRUE(addr.GetIpDisplayName() == "2001:db8:b495:842f::");
}

TEST(inet_np_test, test_inetnp)
{
    CCmThreadManager::Instance();
    struct in6_addr ip6addr;
    struct in_addr ipaddr;

    inet_pton(AF_INET6, "2001:db8:122:344:c0:4:aa00:1", &ip6addr);
    char szAddr[1024] = { 0 };
    inet_ntop(AF_INET6, &ip6addr, szAddr, sizeof(szAddr));
    EXPECT_TRUE(std::string("2001:db8:122:344:c0:4:aa00:1") == szAddr);

    inet_pton(AF_INET, "173.39.168.45", &ipaddr);
    inet_ntop(AF_INET, &ipaddr, szAddr, sizeof(szAddr));
    EXPECT_TRUE(std::string("173.39.168.45") == szAddr);
}