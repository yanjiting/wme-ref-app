#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "CsLenPkg.h"
#include "CmUtil.h"

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
		return NULL;
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
class CCmCsLenPkgHeadLen16BitTest : public testing::Test{
public:
	CCmCsLenPkgHeadLen16BitTest()
	{
	}

	virtual ~CCmCsLenPkgHeadLen16BitTest()
	{
	}

	virtual void SetUp()
	{
		m_pTransportTest = new CCmTransportTest();
		CM_ASSERTE(m_pTransportTest);
		m_pLenPkgConn = new CLenPkgConn();
		CM_ASSERTE(m_pLenPkgConn);
		m_pLenPkgConn->SetTPTransport(m_pTransportTest);
		m_pLenPkgConn->SetCurrStatus(STATUS_DATA_CAN_SEND);
		CmUtilInit();
	}

	virtual void TearDown()
	{
		if (NULL != m_pLenPkgConn)
		{
			delete m_pLenPkgConn;
			m_pLenPkgConn = NULL;
		}
		if (NULL != m_pTransportTest)
		{
			delete m_pTransportTest;
			m_pTransportTest = NULL;
		}
		
	}
protected:
	CLenPkgConn * m_pLenPkgConn;
	CCmTransportTest * m_pTransportTest;
};

TEST_F(CCmCsLenPkgHeadLen16BitTest,SendDataWithLenNormalOK){
	CCmMessageBlock aData(0xffff,NULL,0,0xffff);
	CmResult rv = m_pLenPkgConn->SendData(aData);
	EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmCsLenPkgHeadLen16BitTest,SendDataWithLenBiggerThanMax){
	CCmMessageBlock aData(0x10000,NULL,0,0x10000);
	CmResult rv = m_pLenPkgConn->SendData(aData);
	EXPECT_TRUE(CM_ERROR_INVALID_ARG == rv);
}
