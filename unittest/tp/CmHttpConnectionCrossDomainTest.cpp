
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "CmHttpHead.h"
#include "CmChannelHttpServer.h"
#include "CmTransportTcp.h"

const std::string strOrigin = "http://api.bob.com";

class CCmTransportTest : public ICmTransport
{
public:
	CCmTransportTest(){}
	virtual ~CCmTransportTest(){}

	virtual DWORD AddReference(){ return 0;}
	virtual DWORD ReleaseReference(){return 0;}
	virtual CmResult OpenWithSink(ICmTransportSink *aSink)
	{
		return CM_OK;
	}

	virtual ICmTransportSink* GetSink()
	{
		return CM_OK;
	}

	/// If success, fill <aPara->m_dwHaveSent> if <aPara> is not NULL:
	///    if <aData> has sent completely, return CM_OK;
	///    else return CM_ERROR_PARTIAL_DATA;
	/// Note: <aData> has been advanced <aPara->m_dwHaveSent> bytes in this function.
	virtual CmResult SendData(CCmMessageBlock &aData, CCmTransportParameter *aPara = NULL)
	{
		return CM_OK;
	}

	/// the <aCommand>s are all listed in file CmErrorNetwork.h
	virtual CmResult SetOption(DWORD aCommand, LPVOID aArg)
	{
		return CM_OK;
	}
	virtual CmResult GetOption(DWORD aCommand, LPVOID aArg)
	{
		return CM_OK;
	}

	/// Disconnect the connection, and will not callback <ICmTransportSink> longer.
	virtual CmResult Disconnect(CmResult aReason)
	{
		return CM_OK;
	}
};
class CCmChannelHttpServerTest : public CCmChannelHttpServer
{
public:
	CCmChannelHttpServerTest(ICmTransport *aTrpt) : CCmChannelHttpServer(aTrpt)
	{
		
	}
	~CCmChannelHttpServerTest()
	{
	}
	void InitSimpleRequest()
	{
		m_RequestHeader.SetVersion(HttpVersion_1_1);
		m_RequestHeader.SetMethod(CCmHttpAtomList::Get);
		m_RequestHeader.SetRequestURI("/");
		SetOrAddRequestHeader("Origin", strOrigin);
		m_RequestHeader.SetHeader(CCmHttpAtomList::Host, "api.alice.com");
		m_RequestHeader.SetHeader(CCmHttpAtomList::Accept, "*/*");
		m_RequestHeader.SetHeader(CCmHttpAtomList::User_Agent, "webex utiltp");
	}
	/*void InitPreflightRequest()
	{
		m_RequestHeader.SetVersion(HttpVersion_1_1);
		m_RequestHeader.SetMethod(CCmHttpAtomList::Options);
		SetOrAddRequestHeader("Origin", "http://api.bob.com");
		SetOrAddRequestHeader("Access-Control-Request-Method", "PUT");
		SetOrAddRequestHeader("Access-Control-Request-Headers", "X-Custom-Header");
		m_RequestHeader.SetHeader(CCmHttpAtomList::Host, "api.alice.com");*/
	//	m_RequestHeader.SetHeader(CCmHttpAtomList::Accept, "*/*");
	//	m_RequestHeader.SetHeader(CCmHttpAtomList::User_Agent, "webex utiltp");
	//}*/
	CmResult SetOrAddRequestHeader(const CCmString &aHeader, const CCmString &aValue)
	{
		CCmHttpAtom atHead = CCmHttpAtomList::Instance()->ResolveAtom(aHeader);

		BOOL bRet = m_RequestHeader.SetHeader(atHead, aValue);
		return bRet ? CM_OK : CM_ERROR_FAILURE;
	}
	CmResult GetResponseHeader(const CCmString &aHeader, CCmString &aValue)
	{
		CCmHttpAtom atHead = CCmHttpAtomList::Instance()->ResolveAtom(aHeader);

		BOOL bRet = m_ResponseHeader.GetHeader(atHead, aValue);
		return bRet ? CM_OK : CM_ERROR_FAILURE;
	}
	CmResult ParserHttpProtocol_test(CCmMessageBlock &aData)
	{
		return ParserHttpProtocol_t(aData);
	}
};
class CCmHttpConnectionCrossDomainTest : public testing::Test
{
public:
	CCmHttpConnectionCrossDomainTest()
	{
	}

	virtual ~CCmHttpConnectionCrossDomainTest()
	{
	}


	virtual void SetUp()
	{
		InitPtr(m_pHttpServer.ParaOut());
	}

	virtual void TearDown()
	{
		if (NULL != m_pTransportTest)
		{
			delete m_pTransportTest;
			m_pTransportTest = NULL;
		}
		
	}
	void GetPreflightRequest(CCmHttpRequestHead & RequestHeader)
	{
		RequestHeader.SetVersion(HttpVersion_1_1);
		RequestHeader.SetMethod(CCmHttpAtomList::Options);
		RequestHeader.SetRequestURI("/");
		RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Origin"), strOrigin);
		//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Method"), "PUT");
		//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Headers"), "X-Custom-Header");
		RequestHeader.SetHeader(CCmHttpAtomList::Host, "api.alice.com");
		RequestHeader.SetHeader(CCmHttpAtomList::Accept, "*/*");
		RequestHeader.SetHeader(CCmHttpAtomList::User_Agent, "webex utiltp");
	}
private:
	void InitPtr(CCmChannelHttpServerTest *& p)
	{
		m_pTransportTest = new CCmTransportTest();
		CCmChannelHttpServerTest * temp = new CCmChannelHttpServerTest(m_pTransportTest);
		p = temp;
		p->AddReference();
	}
protected:
	CCmComAutoPtr<CCmChannelHttpServerTest> m_pHttpServer;
	CCmTransportTest * m_pTransportTest;
};

TEST_F(CCmHttpConnectionCrossDomainTest,TestSimpleRequest)
{
	CCmMessageBlock aData(0,NULL,0,0);
	m_pHttpServer->InitSimpleRequest();
	CmResult rv = m_pHttpServer->SendData(aData);
	EXPECT_TRUE(CM_OK == rv);
	CCmString aValue;
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Origin",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_EQ(strOrigin,aValue);

}
TEST_F(CCmHttpConnectionCrossDomainTest,TestPreflightRequest)
{
	CCmHttpRequestHead RequestHeader;
	GetPreflightRequest(RequestHeader);
	RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Method"), "PUT");
	RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Headers"), "X-Custom-Header");
	CCmString strRequest = RequestHeader.Flatten();
	strRequest += s_cm_szHttpHeaderNewLine;
	CCmMessageBlock mbHead(strRequest.length(),
		const_cast<LPSTR>(strRequest.c_str()),
		CCmMessageBlock::DONT_DELETE,
		strRequest.length());
	CmResult rv = m_pHttpServer->ParserHttpProtocol_test(mbHead);
	EXPECT_TRUE(CM_OK == rv);
	CCmString aValue;
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Origin",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_EQ(strOrigin,aValue);

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Methods",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_TRUE(!aValue.empty());

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Headers",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_TRUE(!aValue.empty());
}

TEST_F(CCmHttpConnectionCrossDomainTest,TestPreflightRequestHasNoAccessControlRequestMethod)
{
	CCmHttpRequestHead RequestHeader;
	GetPreflightRequest(RequestHeader);
	//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Method"), "PUT");
	RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Headers"), "X-Custom-Header");
	CCmString strRequest = RequestHeader.Flatten();
	strRequest += s_cm_szHttpHeaderNewLine;
	CCmMessageBlock mbHead(strRequest.length(),
		const_cast<LPSTR>(strRequest.c_str()),
		CCmMessageBlock::DONT_DELETE,
		strRequest.length());
	CmResult rv = m_pHttpServer->ParserHttpProtocol_test(mbHead);
	EXPECT_TRUE(CM_OK == rv);
	CCmString aValue;
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Origin",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_EQ(strOrigin,aValue);

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Methods",aValue);
	EXPECT_TRUE(CM_ERROR_FAILURE == rv);
}
TEST_F(CCmHttpConnectionCrossDomainTest,TestPreflightRequestHasNoAccessControlRequestHeaders)
{
	CCmHttpRequestHead RequestHeader;
	GetPreflightRequest(RequestHeader);
	RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Method"), "PUT");
	//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Headers"), "X-Custom-Header");
	CCmString strRequest = RequestHeader.Flatten();
	strRequest += s_cm_szHttpHeaderNewLine;
	CCmMessageBlock mbHead(strRequest.length(),
		const_cast<LPSTR>(strRequest.c_str()),
		CCmMessageBlock::DONT_DELETE,
		strRequest.length());
	CmResult rv = m_pHttpServer->ParserHttpProtocol_test(mbHead);
	EXPECT_TRUE(CM_OK == rv);
	CCmString aValue;
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Origin",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_EQ(strOrigin,aValue);

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Headers",aValue);
	EXPECT_TRUE(CM_ERROR_FAILURE == rv);
}
TEST_F(CCmHttpConnectionCrossDomainTest,TestPreflightRequestHasNoMethodAndHeaders)
{
	CCmHttpRequestHead RequestHeader;
	GetPreflightRequest(RequestHeader);
	//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Method"), "PUT");
	//RequestHeader.SetHeader(CCmHttpAtomList::Instance()->ResolveAtom("Access-Control-Request-Headers"), "X-Custom-Header");
	CCmString strRequest = RequestHeader.Flatten();
	strRequest += s_cm_szHttpHeaderNewLine;
	CCmMessageBlock mbHead(strRequest.length(),
		const_cast<LPSTR>(strRequest.c_str()),
		CCmMessageBlock::DONT_DELETE,
		strRequest.length());
	CmResult rv = m_pHttpServer->ParserHttpProtocol_test(mbHead);
	EXPECT_TRUE(CM_OK == rv);
	CCmString aValue;
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Origin",aValue);
	EXPECT_TRUE(CM_OK == rv);
	EXPECT_EQ(strOrigin,aValue);

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Methods",aValue);
	EXPECT_TRUE(CM_ERROR_FAILURE == rv);

	aValue.clear();
	rv = m_pHttpServer->GetResponseHeader("Access-Control-Allow-Headers",aValue);
	EXPECT_TRUE(CM_ERROR_FAILURE == rv);
}

TEST_F(CCmHttpConnectionCrossDomainTest,TestPreflightRequestIsNull)
{
	CCmMessageBlock aData(500,NULL,0,500);
	CmResult rv = m_pHttpServer->ParserHttpProtocol_test(aData);
	EXPECT_TRUE(CM_OK != rv);
}