
#ifndef __WBX_SIMPLECLIENTSESSION_H__
#define __WBX_SIMPLECLIENTSESSION_H__

#include "CmStdCpp.h"
#include "CmBase.h"
#include "CmConnectionInterface.h"
#include <list>
#include "SimpleClientSessionSink.h"

#define CM_OPT_SESSION_BASE              500

#define CM_OPT_SESSION_RECONNECT_TIME    (CM_OPT_SESSION_BASE+1)

using namespace std;


class CSimpleClientSession : public ICmAcceptorConnectorSink,
					public CCmTimerWrapperIDSink,
					public ICmTransportSink
{
public:
	CSimpleClientSession(
				const CCmString& strSvrIP,
				WORD	nSvrPort,
				BOOL	bReconnect,
				ISimpleClientSessionSink* pSink);
	virtual ~CSimpleClientSession();

	CmResult Open(DWORD dwType, BOOL bAlone);
	void Close(int iReason);

public: // from ICmAcceptorConnectorSink
	virtual void OnConnectIndication(CmResult aReason,
									ICmTransport *aTrpt,
									ICmAcceptorConnectorId *aRequestId);

public: // from CCmTimerWrapperIDSink
	virtual void OnTimer(CCmTimerWrapperID* aId);

public: // from ICmTransportSink
	virtual void OnReceive(CCmMessageBlock &aData, 
							ICmTransport *aTrptId,
							CCmTransportParameter *aPara = NULL);
	virtual void OnSend(ICmTransport *aTrptId,
						CCmTransportParameter *aPara = NULL);
	virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId);

public:
	CmResult Connect();
	CmResult CreateConnector(DWORD dwType, BOOL bAlone = FALSE);
	CmResult SetTransport(ICmTransport* pTransport);
	CmResult ScheduleReconnectTimer();
	CmResult CancelReconnectTimer();
	CmResult SendData(CCmMessageBlock &aMB);
	CmResult Resend();
	CmResult ReLoad();

    /// the <aCommand>s are all listed in file CmErrorNetwork.h
    CmResult SetOption(DWORD aCommand, LPVOID aArg);
    CmResult GetOption(DWORD aCommand, LPVOID aArg);

private:
	void RemoveMBCache();// Release cached message blocks.

private:
	CCmComAutoPtr<ICmConnector>	m_pConnector;
	CCmComAutoPtr<ICmTransport>	m_pTransport;
	BOOL m_bUpperRegistered; // Indicate if upper layer has registered.
	CCmTimerWrapperID m_reconnectTimer;
	
private:
	CCmString	m_strSvrIP;
	WORD		m_nSvrPort;
	CCmString	m_strBindIP;
	WORD		m_nBindTcpPort;
	BOOL		m_bAlone;
	DWORD		m_dwConnType;
	BOOL		m_bReconnect;
	ISimpleClientSessionSink*		m_pSink;
	BOOL		m_bConnected;
    long        m_lReconnectSec;

private: // Unsent Message block cache.
	typedef list<CCmMessageBlock*>::iterator MBIterator;
	list<CCmMessageBlock*> m_MBCache;
};

#endif //!__WBX_SIMPLECLIENTSESSION_H__

