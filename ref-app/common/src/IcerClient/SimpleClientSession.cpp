
#include "CmBase.h"
#include "CmByteStream.h"
#include "CmMessageBlock.h"
#include "CmInetAddr.h"
#include "CmTimerWrapperID.h"
#include "CmTimeValue.h"

#include "SimpleClientSession.h"

class DeleteMessageBlock
{
public:
	void operator()(CCmMessageBlock* pMB)
	{
		pMB->DestroyChained();
	}
};

//////////////////////////////////////////////////////////////
// class CSimpleClientSession
//////////////////////////////////////////////////////////////

CSimpleClientSession::CSimpleClientSession(
						const CCmString& strSvrIP,
						WORD			nSvrPort,
						BOOL			bReconnect,
						ISimpleClientSessionSink* pSink)
: m_strSvrIP(strSvrIP)
, m_nSvrPort(nSvrPort)
, m_bReconnect(bReconnect)
, m_pSink( pSink )
, m_lReconnectSec(20)
{
	m_bAlone = FALSE;
	m_pTransport = NULL;
	m_pConnector = NULL;
	m_dwConnType = CCmConnectionManager::CTYPE_TCP;

	m_bConnected = FALSE;
	DETAILTRACE_THIS("CSimpleClientSession::CSimpleClientSession");
}

CSimpleClientSession::~CSimpleClientSession()
{
	DETAILTRACE("CSimpleClientSession::~CSimpleClientSession");
	m_bConnected = FALSE;

	Close(CM_OK);
	
}

CmResult CSimpleClientSession::CreateConnector(DWORD dwType, BOOL bAlone)
{
	m_bAlone = bAlone;
	CCmConnectionManager* pConnMgr = CCmConnectionManager::Instance();
	CM_ASSERTE_RETURN(pConnMgr, CM_ERROR_NULL_POINTER);

/*	
	CCmConnectionManager::CType connType = 
		CCmConnectionManager::CTYPE_TCP
		|CCmConnectionManager::CTYPE_PDU_LENGTH;
//		|CCmConnectionManager::CTYPE_PDU_PACKAGE
//		|CCmConnectionManager::CTYPE_PDU_KEEPALIVE;
*/
	m_dwConnType = dwType;
	if(bAlone)
		m_dwConnType |= CCmConnectionManager::CTYPE_INVOKE_MULTITHREAD;

	return pConnMgr->CreateConnectionClient(m_dwConnType, m_pConnector.ParaOut());
}

CmResult CSimpleClientSession::Connect()
{
	DETAILTRACE("CSimpleClientSession::Connect");

	if (m_pConnector.Get()) {
		m_pConnector->CancelConnect();
	}
	else {
		CmResult nResult = CreateConnector(m_dwConnType, m_bAlone);
		if (CM_FAILED(nResult))
		{
			CM_ERROR_TRACE_THIS("CSimpleClientSession::Connect, "
				"create connector fail, errno="<<nResult);
			return nResult;
		}
	}

	// Connection timeout: 20 seconds.
	CCmTimeValue tvTimeOut(20L);
    CCmInetAddr aInetAddr( m_strSvrIP.c_str(), m_nSvrPort);
	m_pConnector->AsycConnect(this, 
							aInetAddr,//CCmInetAddr(m_strSvrIP.c_str(), m_nSvrPort),
							&tvTimeOut);
	return CM_OK;
}

CmResult CSimpleClientSession::Open(DWORD dwType, BOOL bAlone)
{
	m_dwConnType = dwType;
	m_bAlone = bAlone;
	return Connect();
}

void CSimpleClientSession::Close(int iReason)
{
	DETAILTRACE("CSimpleClientSession::Close");
	m_bConnected = FALSE;

	CancelReconnectTimer();
	RemoveMBCache();

	if (m_pTransport.Get()) {
		m_pTransport->Disconnect(iReason);
		m_pTransport = NULL;
	}
	if (m_pConnector.Get()) {
		m_pConnector->CancelConnect();
		m_pConnector = NULL;
	}
}

CmResult CSimpleClientSession::ScheduleReconnectTimer()
{
	if(m_bReconnect)
	{
		CCmTimeValue aTimeValue(m_lReconnectSec);
		CmResult ret = m_reconnectTimer.Schedule(this, aTimeValue, 1);
		if(CM_FAILED(ret))
		{
			CM_ERROR_TRACE_THIS("CSimpleClientSession::ScheduleReconnectTimer, "
				"schedule timer fail, errno="<<ret);
			return ret;
		}

		DETAILTRACE("CSimpleClientSession::ScheduleReconnectTimer");
	}
	return CM_OK;
}

CmResult CSimpleClientSession::CancelReconnectTimer()
{
//	CM_INFO_TRACE("[Server Proxy Manager] CSimpleClientSession::CancelReconnectTimer");
	return m_reconnectTimer.Cancel();
}

CmResult CSimpleClientSession::SetTransport(ICmTransport* pTransport)
{
	CM_ASSERTE_RETURN(pTransport, CM_ERROR_FAILURE);
	CM_INFO_TRACE("CSimpleClientSession::SetTransport, pTransport="<<pTransport);
	
	if (m_pTransport.Get() != NULL) {
		m_pTransport->Disconnect(CM_OK);
		m_pTransport = NULL;
	}
	CmResult nResult = pTransport->OpenWithSink(this);
	CM_ASSERTE_RETURN(CM_SUCCEEDED(nResult), nResult);

	m_pTransport = pTransport;

	/* peida added at 2009/04/23. For SO_KEEP_ALIVE option */
	//int keepAlive = 1;
	//int keepIdle = 60;
	//int keepInterval = 5;
	//int keepCount = 3;
	//m_pTransport->SetOption(CM_OPT_TRANSPORT_TCP_KEEPALIVE, (void *)&keepIdle);
	//m_pTransport->SetOption(SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	//m_pTransport->SetOption(SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	//m_pTransport->SetOption(SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	//m_pTransport->SetOption(SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	return CM_OK;
}

void CSimpleClientSession::OnConnectIndication(
						CmResult aReason,
						ICmTransport *aTrpt,
						ICmAcceptorConnectorId *aRequestId)
{
	// Must be the same connector.
	CM_ASSERTE_RETURN_VOID(aRequestId == m_pConnector.Get());
	if (CM_FAILED(aReason))
	{
		//DETAILTRACE(" CSimpleClientSession::OnConnectIndication,  connect to server manager fail, aReason="<<aReason);
		if( m_bReconnect )
 		{
			m_bConnected = FALSE;
			ScheduleReconnectTimer();
		}
		else
		{
			if(m_pSink)
				m_pSink->OnClientSessionJoin(aReason);
		}
		return;
	}

	DETAILTRACE("CSimpleClientSession::OnConnectIndication,"
		" connect to server successfully.");

	CM_ASSERTE_RETURN_VOID(aTrpt);
	CmResult nResult = SetTransport(aTrpt);
	CM_ASSERTE_RETURN_VOID(CM_SUCCEEDED(nResult));
	Resend();
	m_bConnected = TRUE;
			
	if(m_pSink)
		m_pSink->OnClientSessionJoin(aReason);
}

void CSimpleClientSession::OnTimer(CCmTimerWrapperID* aId)
{
	// Reconnect timer.
	CM_ASSERTE_RETURN_VOID(aId == &m_reconnectTimer);
	DETAILTRACE("CSimpleClientSession::OnTimer,"
		" now reconnect to server.");

	Connect();
}

void CSimpleClientSession::OnReceive(CCmMessageBlock &aData, 
							ICmTransport *aTrptId,
							CCmTransportParameter *aPara)
{
	CM_ASSERTE_RETURN_VOID(aTrptId && aTrptId == m_pTransport.Get());
	//CM_INFO_TRACE("SimpleClientSession::OnReceive");
	CCmString str = aData.FlattenChained();
	if(m_pSink)
		m_pSink->OnClientReceive(str);

}

void CSimpleClientSession::OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
	DETAILTRACE("CSimpleClientSession::OnSend");
	// Must be the same transport.
	CM_ASSERTE_RETURN_VOID(aTrptId && aTrptId == m_pTransport.Get());

	Resend();
}

void CSimpleClientSession::OnDisconnect(CmResult aReason, ICmTransport *aTrptId)
{
//	CM_ASSERTE_RETURN_VOID(m_pTransport.Get() == aTrptId);
//	DETAILTRACE("CSimpleClientSession::OnDisconnect,"
//		" aReason="<<aReason);
	m_pTransport = NULL;
	if(m_bReconnect)
	{
		if(m_bConnected == TRUE)
		{
			if(m_pSink)
				m_pSink->OnClientSessionLeave(aReason);
		}
		m_bConnected = FALSE;
        ScheduleReconnectTimer(); // Soya: reconnect after onsessionleave. so application could clear cache manually first.
	}
	else
	{
		if(m_pSink)
			m_pSink->OnClientSessionLeave(aReason);
	}
}

CmResult CSimpleClientSession::Resend()
{
	CM_INFO_TRACE("CSimpleClientSession::Resend,MB cache size="<<m_MBCache.size());
	if (!m_MBCache.empty()) {
		MBIterator it = m_MBCache.begin();
		while(it != m_MBCache.end())
		{
			CmResult ret = m_pTransport->SendData(*(*it), NULL);
			if(CM_FAILED(ret))
			{
				CM_ERROR_TRACE_THIS("CSimpleClientSession::Resend, resend data fail, errno="<<ret);
				return ret;
			}
			CM_INFO_TRACE("CSimpleClientSession::Resend successful");

			delete *it;
			it = m_MBCache.erase(it);
		}
	}
	return CM_OK;
}

CmResult CSimpleClientSession::SendData(CCmMessageBlock &aMB)
{
	if(!m_pTransport.Get()) {
		CM_WARNING_TRACE("CSimpleClientSession::SendData, invalid transport handle.");
		
		CCmMessageBlock* ppMB = aMB.DuplicateChained();
		CM_ASSERTE_RETURN(ppMB, CM_ERROR_OUT_OF_MEMORY);
		m_MBCache.push_back(ppMB);
		CM_INFO_TRACE("CSimpleClientSession::SendData failed, add to MB cache="<<m_MBCache.size());

		return  CM_ERROR_NULL_POINTER;
	}
	
	CmResult ret = m_pTransport->SendData(aMB, NULL);
	if(CM_FAILED(ret))
	{
		CCmMessageBlock* pMB = aMB.DuplicateChained();
		CM_ASSERTE_RETURN(pMB, CM_ERROR_OUT_OF_MEMORY);
		m_MBCache.push_back(pMB);
		CM_INFO_TRACE("CSimpleClientSession::SendData failed, add to MB cache="<<m_MBCache.size());

		return ret;
	}
	return CM_OK;
}

void CSimpleClientSession::RemoveMBCache()
{
	if(!m_MBCache.empty())
	{
		for_each(m_MBCache.begin(), m_MBCache.end(), DeleteMessageBlock());
		m_MBCache.clear();
	}
}

CmResult CSimpleClientSession::ReLoad()
{
	CM_INFO_TRACE("CSimpleClientSession::ReLoad()");
	Close(CM_OK);
	return Connect();
}

CmResult CSimpleClientSession::SetOption(DWORD aCommand, LPVOID aArg)
{
    CM_ASSERTE_RETURN(aArg, CM_ERROR_INVALID_ARG);
    switch(aCommand)
    {
    case CM_OPT_SESSION_RECONNECT_TIME:
        {
            m_lReconnectSec = *static_cast<long*>(aArg);
            return CM_OK;
        }
    default:
        if(m_pTransport.Get())
        {
            return m_pTransport->SetOption(aCommand, aArg);
        }
        return CM_ERROR_NULL_POINTER;
    }
}

CmResult CSimpleClientSession::GetOption(DWORD aCommand, LPVOID aArg)
{
    CM_ASSERTE_RETURN(aArg, CM_ERROR_INVALID_ARG);
    switch(aCommand) 
    {
    case CM_OPT_SESSION_RECONNECT_TIME:
        {
            *static_cast<long*>(aArg) = m_lReconnectSec;
            return CM_OK;
        }
    default:
        if(m_pTransport.Get())
        {
            return m_pTransport->GetOption(aCommand, aArg);
        }
        return CM_ERROR_NULL_POINTER;
    }
}
