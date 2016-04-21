
#include "MyMediaConnection.h"
#include "CmByteStream.h"
#include "CmInetAddr.h"
#include "timer.h"


#ifdef CREATE_NETWORD_THREAD
#include "CmThread.h"
#include "string.h"
#endif
#define DEFAULT_RECV_BUFF_LEN 2048

const char *CTRL_CONN_REQUEST = "C001";
const char *CTRL_CONN_ACK	= "C002";
const char *CTRL_CONN_FIN	= "C003";

const char *DATA_CONN_REQUEST =  "D001";
const char *DATA_CONN_ACK	= "D002";
const char *DATA_CONN_FIN	= "D003";

const char *CONN_KEEP_LIVE	= "K001";

#define MAX_TRY_TIME 5

CMyMediaConnection::CMyMediaConnection()
{

	m_pCtrlAcceptor = NULL;
	m_pDataAcceptor = NULL;
	m_pCtrlConnector = NULL;	//For control data, at client
	m_pDataConnector = NULL;	//For application data, at client
	m_pCtrlTrpt = NULL;
	m_pDataTrpt = NULL;
	m_pTmpCtrlTrpt = NULL;
	m_pTmpDataTrpt = NULL;

	m_bHost = false;
	m_bConnected = false;

	m_bHostInited = false;


	m_pConnSink = NULL;
	m_pVideoConnSink = NULL;
	m_pAudioConnSink = NULL;

	m_bAllowCtrlConn = false;

	m_CtrlConnType = CCmConnectionManager::CTYPE_TCP;
	m_DataConnType = CCmConnectionManager::CTYPE_UDP;

	m_dwLastRecvKeepLiveTime = 0;
	m_DataConnectionStatus = Status_DISCONNECTED;
	m_CtrlConnectionStatus = Status_DISCONNECTED;

	m_dwLastDataConnCtrlMsgSendTime = 0;
	m_dwLastCtrlConnCtrlMsgSendTime = 0;
	m_nDataConnCtrlMsgRetransmitTimes = 0;
	m_nCtrlConnCtrlMsgRetransmitTimes = 0;

	m_pIcerClient = NULL;
	m_bNotUseIcer = false;
	m_bUseIcer = false;
	m_pTrpt = NULL;

//	m_pRecvBuff = NULL;
	m_nRecvBuffLen = DEFAULT_RECV_BUFF_LEN;
	m_pRecvBuff = new unsigned char[m_nRecvBuffLen];
	memset(m_pRecvBuff, 0, m_nRecvBuffLen);
	ScheduleTimer();
    
    m_nCtrlRemain = 0;
    m_nDataRemain = 0;

	m_nCtrlRemain = 0;
	m_nDataRemain =0;
    
    m_bRtpConnected = false;
    m_bRtcpConnected = false;

}

CMyMediaConnection::~CMyMediaConnection()
{
	CancelTimer();
	Stop();

	if(m_pRecvBuff)
	{
		delete []m_pRecvBuff;
		m_pRecvBuff = NULL;
		m_nRecvBuffLen = DEFAULT_RECV_BUFF_LEN;
	}

}

int CMyMediaConnection::InitHost(bool bEnableCtrlConn, int hostDataPort, NetConnType connDataType, int hostCtrlPort, NetConnType connCtrlType)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::InitHost, bEnableCtrlConn = " << bEnableCtrlConn
		<< ", hostDataPort = " << hostDataPort
		<< ", connDataType = " << connDataType
		<< ", hostCtrlPort = " << hostCtrlPort
		<< ", connCtrlType = " << connCtrlType
		);

	if(m_bUseIcer)
	{
		return CM_ERROR_FAILURE;
	}

	m_bNotUseIcer = true;
	m_bUseIcer = false;

	if(m_pCtrlConnector != NULL||m_pDataConnector != NULL)
	{
		CM_ERROR_TRACE_THIS("CMyMediaConnection::InitHost, this method shoul not be called as this object is used as client!");
		return CM_ERROR_ALREADY_INITIALIZED;
	}

	if(m_bHostInited)
	{
		CM_WARNING_TRACE_THIS("CMyMediaConnection::InitHost, already inited!");
		return CM_OK;
	}

	m_bAllowCtrlConn = bEnableCtrlConn;



	if(m_bAllowCtrlConn)
		CM_ASSERTE(!m_pCtrlAcceptor);

	CM_ASSERTE(!m_pDataAcceptor);

	if(m_bAllowCtrlConn)
	{
		m_CtrlConnType = CCmConnectionManager::CTYPE_TCP;
		if(connCtrlType == CONN_UDP)
		{
			m_CtrlConnType = CCmConnectionManager::CTYPE_UDP;	
		}

		CCmInetAddr ctrlAddr;			
		CmResult rv = ctrlAddr.SetPort(hostCtrlPort);

		 rv = CCmConnectionManager::Instance()->
				CreateConnectionServer(m_CtrlConnType, m_pCtrlAcceptor.ParaOut());
		if (CM_FAILED(rv))
				return rv;
		CM_ASSERTE(m_pCtrlAcceptor);	
		m_pCtrlAcceptor->StartListen(this, ctrlAddr);
		m_CtrlConnectionStatus = Status_LISTEN;
	}


	m_DataConnType = CCmConnectionManager::CTYPE_TCP;
	if(connDataType == CONN_UDP)
	{
		m_DataConnType = CCmConnectionManager::CTYPE_UDP;	
	}

	CCmInetAddr dataAddr;			
	CmResult rv = dataAddr.SetPort(hostDataPort);
	 rv = CCmConnectionManager::Instance()->
			CreateConnectionServer(m_DataConnType, m_pDataAcceptor.ParaOut());
	if (CM_FAILED(rv))
			return rv;
	CM_ASSERTE(m_pDataAcceptor);	


	m_bHostInited = true;
	m_bHost = true;

	m_DataConnectionStatus = Status_LISTEN;

    return m_pDataAcceptor->StartListen(this, dataAddr);
}

int CMyMediaConnection::Stop()
{

	CM_INFO_TRACE_THIS("CMyMediaConnection::Stop");
	DisConnect();
	if(m_pCtrlAcceptor != NULL)
	{
		m_pCtrlAcceptor->StopListen(0);
//		m_pCtrlAcceptor->ReleaseReference();
		m_pCtrlAcceptor = NULL;
	}

	if(m_pDataAcceptor != NULL)
	{
		m_pDataAcceptor->StopListen(0);
//		m_pDataAcceptor->ReleaseReference();
		m_pDataAcceptor = NULL;
	}

	m_bHostInited = false;
	m_bHost = false;


	if(m_pCtrlConnector != NULL)
	{
		m_pCtrlConnector->CancelConnect();
//		m_pCtrlConnector->ReleaseReference();
		m_pCtrlConnector = NULL;
	}

	if(m_pDataConnector != NULL)
	{
		m_pDataConnector->CancelConnect();
//		m_pDataConnector->ReleaseReference();
		m_pDataConnector = NULL;
	}

	if(m_pIcerClient)
	{
		DeleteIcerClient(m_pIcerClient);
		m_pIcerClient = NULL;
	}

	m_DataConnectionStatus = Status_DISCONNECTED;
	m_CtrlConnectionStatus = Status_DISCONNECTED;

	m_bConnected = false;
	m_bAllowCtrlConn = false;

	m_CtrlConnType = CCmConnectionManager::CTYPE_TCP;
	m_DataConnType = CCmConnectionManager::CTYPE_UDP;

	m_dwLastDataConnCtrlMsgSendTime = 0;
	m_dwLastCtrlConnCtrlMsgSendTime = 0;
	m_nDataConnCtrlMsgRetransmitTimes = 0;
	m_nCtrlConnCtrlMsgRetransmitTimes = 0;

	m_bNotUseIcer = false;
	m_bUseIcer = false;

	return CM_OK;
}

int CMyMediaConnection::ConnectTo(const char * hostIP, int hostDataPort, NetConnType connDataType, int hostCtrlPort, NetConnType connCtrlType)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::ConnectTo, begin");

	if(m_bUseIcer)
	{
	    CM_ERROR_TRACE_THIS("CMyMediaConnection::ConnectTo, error as it is ICE");
		return CM_ERROR_FAILURE;
	}

	m_bNotUseIcer = true;
	m_bUseIcer = false;

	if(m_pDataAcceptor)
	{
	    CM_ERROR_TRACE_THIS("CMyMediaConnection::ConnectTo, had m_pDataAcceptor");
		return CM_ERROR_ALREADY_INITIALIZED;
	}

	if(m_bConnected)
	{
	    CM_ERROR_TRACE_THIS("CMyMediaConnection::ConnectTo, m_bConnected="<<m_bConnected);
		return 1;
	}

	if(hostCtrlPort > 0 )
	{
		m_bAllowCtrlConn = true;
	}



	CmResult res = CM_OK;


	if(!m_pCtrlConnector && m_bAllowCtrlConn )
	{
		m_CtrlConnType = CCmConnectionManager::CTYPE_TCP;
		if(connCtrlType == CONN_UDP)
		{
			m_CtrlConnType = CCmConnectionManager::CTYPE_UDP;	
		}
		res = CCmConnectionManager::Instance()->CreateConnectionClient(m_CtrlConnType, m_pCtrlConnector.ParaOut());	
	}

	if(!m_pDataConnector)
	{
		m_DataConnType = CCmConnectionManager::CTYPE_TCP;
		if(connDataType == CONN_UDP)
		{
			m_DataConnType = CCmConnectionManager::CTYPE_UDP;	
		}
		res = CCmConnectionManager::Instance()->CreateConnectionClient(m_DataConnType, m_pDataConnector.ParaOut());
	}
		

	if(m_bAllowCtrlConn)
	{
	    CM_INFO_TRACE_THIS("CMyMediaConnection::ConnectTo, AsycConnect for CtrlPort="<<hostCtrlPort);
		CCmInetAddr aPeerAddr(hostIP, hostCtrlPort);	
		m_pCtrlConnector->AsycConnect(this, aPeerAddr);

	}

	CM_INFO_TRACE_THIS("CMyMediaConnection::ConnectTo, AsycConnect for DataPort="<<hostDataPort);
	CCmInetAddr aPeerAddr(hostIP, hostDataPort);	

	m_pDataConnector->AsycConnect(this, aPeerAddr);


	return CM_OK;

}

int CMyMediaConnection::DisConnect()
{

//	CancelTimer();

	CM_INFO_TRACE_THIS("CMyMediaConnection::DisConnect, start");

	if(!m_bUseIcer)
	{

		if(m_pCtrlTrpt)
		{
			if(m_CtrlConnType == CCmConnectionManager::CTYPE_UDP)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::DisConnect, SendConnFinMsg to remote control connection.");
				SendConnFinMsg(TYPE_VIDEO_CONTROL);		
			}

			m_pCtrlTrpt->Disconnect(0);
			m_pCtrlTrpt = NULL;
		}
		if(m_pDataTrpt)
		{
			if(m_DataConnType == CCmConnectionManager::CTYPE_UDP)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::DisConnect, SendConnFinMsg to remote data connection.");
				SendConnFinMsg(TYPE_VIDEO_DATA);		
			}
			m_pDataTrpt->Disconnect(0);
			m_pDataTrpt = NULL;
		}


		if(m_pTmpCtrlTrpt)
		{
			m_pTmpCtrlTrpt->Disconnect(0);
			m_pTmpCtrlTrpt = NULL;
		}
		if(m_pTmpDataTrpt)
		{
			m_pTmpDataTrpt->Disconnect(0);
			m_pTmpDataTrpt = NULL;
		}
	}
	else
	{
		//need to do
		if(m_pTrpt)
		{
			SendConnFinMsg(TYPE_CONN_CONTROL);
			CM_INFO_TRACE_THIS("CMyMediaConnection::DisConnect, Disconnect_ m_pTrpt " << m_pTrpt);
			m_pTrpt->Disconnect(0);
			m_pTrpt = NULL;
		}

		if(m_pIcerClient)
		{
			DeleteIcerClient(m_pIcerClient);
			m_pIcerClient = NULL;
		}
	}

	m_bConnected = false;


	return CM_OK;
}


void CMyMediaConnection::OnConnectIndication(CmResult aReason, ICmTransport *aTrpt, ICmAcceptorConnectorId *aRequestId)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, begin =aReason" << aReason);

	if(m_bUseIcer)
	{
		return;
	}

	if(CM_FAILED(aReason) || aTrpt == NULL)
	{
		if(aRequestId == m_pCtrlConnector || aRequestId == m_pCtrlAcceptor)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connect failed, aTrpt = " << aTrpt
				<< ", aReason = " << aReason);	

			if(aTrpt != NULL && aTrpt == m_pCtrlTrpt)
			{
				m_pCtrlTrpt->Disconnect(0);
				m_pCtrlTrpt =NULL;
			}
		}
		else
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, data connect failed, aTrpt = " << aTrpt
				<< ", aReason = " << aReason);	

			if(aTrpt != NULL && aTrpt == m_pDataTrpt)
			{
				m_pDataTrpt->Disconnect(0);
				m_pDataTrpt =NULL;
			}
			if(m_pConnSink)
			{
				m_pConnSink->OnConnected(false);
			}
		}

		return;
	}
	
	aTrpt->OpenWithSink(this);

	DWORD dwNow = static_cast<DWORD>(ticker::now()/1000);




//	if(aRequestId == m_pCtrlAcceptor || aRequestId == m_pCtrlConnector)
	if(aRequestId == m_pCtrlConnector)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, receive a ConnectIndication from m_pCtrlConnector, aTrpt = " << aTrpt
			<< ", aRequestId = " << aRequestId);

		if(m_pCtrlTrpt && m_pCtrlTrpt != aTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, the control connection is already exist, disconnect it first, m_pCtrlTrpt = " << m_pCtrlTrpt);
			m_pCtrlTrpt->Disconnect(0);
			m_pCtrlTrpt = NULL;
		}
		m_pCtrlTrpt = aTrpt;
		if(m_CtrlConnType == CCmConnectionManager::CTYPE_TCP)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connecton type is CTYPE_TCP, now it is connected!");
			m_DataConnectionStatus = Status_CONNECTED;
            
            m_bRtcpConnected = true;
			if(m_pConnSink && m_bRtpConnected && m_bRtcpConnected)
			{
				m_pConnSink->OnConnected(true);
			}
			return;
		}

		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connecton type is CTYPE_UDP, now SendConnReqMsg!");
		SendConnReqMsg(TYPE_VIDEO_CONTROL);	
		m_CtrlConnectionStatus = Status_SEND_REQUEST;
		m_dwLastCtrlConnCtrlMsgSendTime = dwNow;
		m_nCtrlConnCtrlMsgRetransmitTimes = 1;
        m_nCtrlRemain = 0;

	}
	else if(aRequestId == m_pDataConnector)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, receive a ConnectIndication from m_pDataConnector, aTrpt = " << aTrpt
			<< ", aRequestId = " << aRequestId);

        m_nDataRemain = 0;
		if(m_pDataTrpt&& m_pDataTrpt != aTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, the data connection is already exist, disconnect it first, m_pDataTrpt = " << m_pDataTrpt);
			m_pDataTrpt->Disconnect(0);
			m_pDataTrpt = NULL;
		}

		m_pDataTrpt = aTrpt;
        
		if(m_DataConnType == CCmConnectionManager::CTYPE_TCP)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, data connecton type is CTYPE_TCP, now it is connected!");
			m_DataConnectionStatus = Status_CONNECTED;
            m_bRtpConnected = true;
			if(m_pConnSink && m_bRtpConnected && m_bRtcpConnected)
			{
				m_pConnSink->OnConnected(true);
			}
			return;
		}

		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, SendConnReqMsg for data connecton ");
		SendConnReqMsg(TYPE_VIDEO_DATA);	

		m_DataConnectionStatus = Status_SEND_REQUEST;

		m_dwLastDataConnCtrlMsgSendTime = dwNow;
		m_nDataConnCtrlMsgRetransmitTimes = 1;

	}

	else if(aRequestId == m_pCtrlAcceptor)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, receive a new connection indication from m_pCtrlAcceptor, aTrpt = " << aTrpt
			<< ", aRequestId = " << aRequestId);

        m_nCtrlRemain = 0;
		if(m_CtrlConnType == CCmConnectionManager::CTYPE_TCP)
		{
			if(m_pCtrlTrpt != NULL && m_pCtrlTrpt != aTrpt)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connecton is already exist, Disconnect it first, m_pCtrlTrpt = " << m_pCtrlTrpt);
				m_pCtrlTrpt->Disconnect(0);
				m_pCtrlTrpt = 0;
			}
			m_pCtrlTrpt = aTrpt;
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connecton type is CTYPE_TCP, now it is connected, m_pCtrlTrpt = " << m_pCtrlTrpt);
			m_DataConnectionStatus = Status_CONNECTED;
            
            m_bRtcpConnected = true;
			if(m_pConnSink && m_bRtpConnected && m_bRtcpConnected)
			{
				m_pConnSink->OnConnected(true);
			}
            
			return;
		}

		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, control connecton type is CTYPE_UDP, now SendConnReqMsg!");
	
		
		if(m_pTmpCtrlTrpt)
		{
			m_pTmpCtrlTrpt->Disconnect(0);
			m_pTmpCtrlTrpt = NULL;
		}

		m_pTmpCtrlTrpt = aTrpt;
	}

	else if(aRequestId == m_pDataAcceptor)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, receive a new connection indication from m_pDataAcceptor, aTrpt = " << aTrpt
			<< ", aRequestId = " << aRequestId);

        m_nDataRemain = 0;
		if(m_DataConnType == CCmConnectionManager::CTYPE_TCP)
		{
			if(m_pDataTrpt != NULL && m_pDataTrpt != aTrpt)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, data connecton is already exist, Disconnect it first, m_pDataTrpt = " << m_pDataTrpt);
				m_pDataTrpt->Disconnect(0);
				m_pDataTrpt = 0;
			}
			m_pDataTrpt = aTrpt;
			CM_INFO_TRACE_THIS("CMyMediaConnection::OnConnectIndication, data connecton type is CTYPE_TCP, now it is connected, m_pDataTrpt = " << m_pDataTrpt);
			m_DataConnectionStatus = Status_CONNECTED;
            
            m_bRtpConnected = true;
			if(m_pConnSink && m_bRtpConnected && m_bRtcpConnected)
			{
				m_pConnSink->OnConnected(true);
			}
			return;
		}

		if(m_pTmpDataTrpt)
		{
			m_pTmpDataTrpt->Disconnect(0);
			m_pTmpDataTrpt = NULL;
		}
        
		m_pTmpDataTrpt = aTrpt;
		m_pDataTrpt = aTrpt;
	}

	return;
}

CMyMediaConnection::DataAction CMyMediaConnection::ProcessConnCtrlMsg(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
	if(aData.GetChainedLength() != 4)
	{
		return SUBMIT_DATA;
	}

	if(m_bUseIcer)
	{
		if(aTrptId != m_pTrpt)
		{
			return DROP_DATA;
		}
	}

	char szBuff[16];
	CCmByteStreamNetwork _os(aData);
	_os.Read(szBuff, 4);

	return ProcessConnCtrlMsg((unsigned char *)szBuff, 4, aTrptId, aPara);
}

CMyMediaConnection::DataAction CMyMediaConnection::ProcessConnCtrlMsg(unsigned char *pData, int nDataLen, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
	if(nDataLen != 4 || !pData)
	{
		return SUBMIT_DATA;
	}

	char szBuff[16];
	memcpy(szBuff, pData, 4);
	
	if(m_bUseIcer)
	{
		if(aTrptId != m_pTrpt)
		{
			return DROP_DATA;
		}


		if(memcmp(szBuff, DATA_CONN_FIN, 4) == 0)
		{
			m_pTrpt->Disconnect(0);
			m_pTrpt = NULL;

			if(m_pVideoConnSink)
			{
				m_pVideoConnSink->OnDisconnected();
			}

			if(m_pAudioConnSink)
			{
				m_pAudioConnSink->OnDisconnected();			
			}
		}

		return DROP_DATA;
	}


	//CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive connection control message from remote, aTrptId = " << aTrptId);

	if(memcmp(szBuff, CONN_KEEP_LIVE, 4) == 0)
	{
		if( aTrptId == m_pDataTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CONN_KEEP_LIVE message from m_pDataTrpt, m_pDataTrpt = " << m_pDataTrpt);

			m_dwLastRecvKeepLiveTime = static_cast<DWORD>(ticker::now()/1000);
		}
		return DROP_DATA;
	}

	if(memcmp(szBuff, DATA_CONN_FIN, 4) == 0)
	{

		if( aTrptId == m_pDataTrpt)
		{

			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive DATA_CONN_FIN message from m_pDataTrpt, m_pDataTrpt = " << m_pDataTrpt
				<<", disconnect data connection, and notice m_pConnSink");
			m_pDataTrpt->Disconnect(0);
			m_pDataTrpt = NULL;
			m_bConnected = false;
			m_DataConnectionStatus = Status_DISCONNECTED;

			DisConnect();

			if(m_pConnSink)
			{
				m_pConnSink->OnDisconnected();
			}
		}
		else if(aTrptId == m_pTmpDataTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive DATA_CONN_FIN message from m_pTmpDataTrpt, m_pTmpDataTrpt = " << m_pTmpDataTrpt
				<<", disconnect m_pTmpDataTrpt");
			m_pTmpDataTrpt->Disconnect(0);
			m_pTmpDataTrpt = NULL;
		}
		return DROP_DATA;
	}

	if(memcmp(szBuff, CTRL_CONN_FIN, 4) == 0)
	{
		if( aTrptId == m_pCtrlTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CTRL_CONN_FIN message from m_pCtrlTrpt, m_pCtrlTrpt = " << m_pCtrlTrpt
				<<", disconnect control connection");
			m_pCtrlTrpt->Disconnect(0);
			m_pCtrlTrpt = NULL;
			m_CtrlConnectionStatus = Status_DISCONNECTED;

		}
		else if(aTrptId == m_pTmpCtrlTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CTRL_CONN_FIN message from m_pTmpCtrlTrpt, m_pTmpCtrlTrpt = " << m_pTmpCtrlTrpt
				<<", disconnect m_pTmpCtrlTrpt");

			m_pTmpCtrlTrpt->Disconnect(0);
			m_pTmpCtrlTrpt = NULL;
		}
		return DROP_DATA;	
	}


	if(memcmp(szBuff, DATA_CONN_REQUEST, 4) == 0)
	{

		if(aTrptId == m_pDataTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive DATA_CONN_REQUEST message from m_pDataTrpt, m_pDataTrpt = " << m_pDataTrpt
				<<", data connection is connected, SendConnAckMsg and notice m_pConnSink");
			m_DataConnectionStatus = Status_CONNECTED;

			SendConnAckMsg(TYPE_VIDEO_DATA);
			if(m_pConnSink)
			{
				m_pConnSink->OnConnected(true);
			}
		}

		else if(aTrptId == m_pTmpDataTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive DATA_CONN_REQUEST message from m_pTmpDataTrpt, m_pTmpDataTrpt = " << m_pTmpDataTrpt);

			if(m_pDataTrpt)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, the data connection is already exist, disconnect it first, m_pDataTrpt  = " << m_pDataTrpt);

				m_pDataTrpt->Disconnect(0);
				m_pDataTrpt = NULL;
			}
			m_pDataTrpt = m_pTmpDataTrpt;
			m_pTmpDataTrpt = NULL;

			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, SendConnAckMsg and notice m_pConnSink, new data connection is connected, m_pDataTrpt = " << m_pDataTrpt);
			m_DataConnectionStatus = Status_CONNECTED;

			SendConnAckMsg(TYPE_VIDEO_DATA);
			if(m_pConnSink)
			{
				m_pConnSink->OnConnected(true);
			}

			m_bConnected = true;

		}
		return DROP_DATA;		
	}

	if(memcmp(szBuff, CTRL_CONN_REQUEST, 4) == 0)
	{
		if(aTrptId == m_pCtrlTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CTRL_CONN_REQUEST message from m_pCtrlTrpt, m_pCtrlTrpt = " << m_pCtrlTrpt
					<<", SendConnAckMsg");

			SendConnAckMsg(TYPE_VIDEO_CONTROL);
		}

		else if(aTrptId == m_pTmpCtrlTrpt)
		{
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CTRL_CONN_REQUEST message from m_pTmpCtrlTrpt, m_pTmpCtrlTrpt = " << m_pTmpCtrlTrpt);
			if(m_pCtrlTrpt)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, the control connection is already exist, disconnect it first, m_pCtrlTrpt  = " << m_pCtrlTrpt);
				m_pCtrlTrpt->Disconnect(0);
				m_pCtrlTrpt = NULL;
			}
			m_pCtrlTrpt = m_pTmpCtrlTrpt;
			m_pTmpCtrlTrpt = NULL;

			m_CtrlConnectionStatus = Status_CONNECTED;

			SendConnAckMsg(TYPE_VIDEO_CONTROL);
			CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, SendConnAckMsg, new control connection is connected,  m_pCtrlTrpt = " << m_pCtrlTrpt);

		}
		return DROP_DATA;		
	}

	if(memcmp(szBuff, DATA_CONN_ACK, 4) == 0)
	{
		m_DataConnectionStatus = Status_CONNECTED;
		CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive DATA_CONN_ACK message, data connection is connected, m_pDataTrpt = " << m_pDataTrpt);

		if(m_pConnSink)
		{
			m_pConnSink->OnConnected(true);
		}

		m_bConnected = true;

		return DROP_DATA;	
	}

	if(memcmp(szBuff, CTRL_CONN_ACK, 4) == 0)
	{
		
		CM_INFO_TRACE_THIS("CMyMediaConnection::ProcessConnCtrlMsg, receive CTRL_CONN_ACK message, control connection is connected, m_pCtrlTrpt = " << m_pCtrlTrpt);
		m_CtrlConnectionStatus = Status_CONNECTED;

		return DROP_DATA;
	}

	return SUBMIT_DATA;

}

static  CmResult PeekType( const CCmMessageBlock& mbBlock, BYTE& cType /* out */ )
{
	BYTE pBuf[1];
	CmResult cmResult = const_cast<CCmMessageBlock&>( mbBlock ).Read(
		pBuf, 
		sizeof(pBuf), 
		NULL, 
		FALSE );

	if ( CM_FAILED(cmResult) ) 
	{
		return cmResult;
	}

	cType = pBuf[0];

	return CM_OK;
}

void CMyMediaConnection::OnDataReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    DWORD nRead = 0;
    aData.Read(m_dataBuffer + m_nDataRemain, sizeof(m_dataBuffer)-m_nDataRemain, &nRead);
    char *pBuffer;
    char *pEnd;
    DWORD len;
    while( nRead > 0 )
    {
        pBuffer = m_dataBuffer;
        pEnd = m_dataBuffer+m_nDataRemain+nRead;
        while (pBuffer < pEnd)
        {
            len = ntohl(*((DWORD *)pBuffer));
            if( pBuffer+sizeof(DWORD) > pEnd || pBuffer+sizeof(DWORD)+len > pEnd )
            {
                m_nDataRemain = pEnd-pBuffer;
                if( m_nDataRemain && pBuffer > m_dataBuffer )
                    memmove(m_dataBuffer, pBuffer, pEnd-pBuffer);
                break;
            }
            pBuffer += sizeof(DWORD);
            if( len > 0 )
            {
                CCmMessageBlock mb(len, pBuffer,CCmMessageBlock::DONT_DELETE, len);
                OnReceiveOneBlock(mb, aTrptId, aPara);
                pBuffer += len;
            }
            m_nDataRemain = 0;
        };
        
        nRead = 0;
        aData.Read(m_dataBuffer + m_nDataRemain, sizeof(m_dataBuffer)-m_nDataRemain, &nRead);
    }
}

void CMyMediaConnection::OnCtrlReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    DWORD nRead(0);
    aData.Read(m_ctrlBuffer + m_nCtrlRemain, sizeof(m_ctrlBuffer)-m_nCtrlRemain, &nRead);
    char *pBuffer;
    char *pEnd;
    DWORD len;
    while( nRead > 0 )
    {
        pBuffer = m_ctrlBuffer;
        pEnd = m_ctrlBuffer+m_nCtrlRemain+nRead;
        while(pBuffer<pEnd){
            len = ntohl(*((DWORD *)pBuffer));
            if( pBuffer+sizeof(DWORD) > pEnd || pBuffer+sizeof(DWORD)+len > pEnd )
            {
                m_nCtrlRemain = pEnd-pBuffer;
                if( m_nCtrlRemain && pBuffer > m_ctrlBuffer )
                    memmove(m_ctrlBuffer, pBuffer, pEnd-pBuffer);
                break;
            }
            pBuffer += sizeof(DWORD);
            if( len > 0 )
            {
                CCmMessageBlock mb(len, pBuffer,CCmMessageBlock::DONT_DELETE, len);
                OnReceiveOneBlock(mb, aTrptId, aPara);
                pBuffer+=len;
            }
            m_nCtrlRemain = 0;
        }
        nRead = 0;
        aData.Read(m_ctrlBuffer + m_nCtrlRemain, sizeof(m_ctrlBuffer)-m_nCtrlRemain, &nRead);
    }
}

void CMyMediaConnection::OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    long lptr = (long) aTrptId; // fix 64bit

    if( CCmConnectionManager::CTYPE_TCP == m_CtrlConnType)
    {
        if( aTrptId == m_pDataTrpt || m_pTmpDataTrpt == aTrptId )
            OnDataReceive( aData, aTrptId, aPara );
        else if( aTrptId == m_pCtrlTrpt || m_pTmpCtrlTrpt == aTrptId )
            OnCtrlReceive( aData, aTrptId, aPara );
        else
            OnReceiveOneBlock(aData, aTrptId, aPara );
    }
    else
        OnReceiveOneBlock(aData, aTrptId, aPara);
}

void CMyMediaConnection::OnReceiveOneBlock(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    long lptr = (long) aTrptId; // fix 64bit

	int nLen = aData.GetChainedLength();

	if(!m_pRecvBuff)
	{
		if(m_nRecvBuffLen < nLen)
		{
			m_nRecvBuffLen = nLen;
		}

		m_pRecvBuff = new unsigned char [m_nRecvBuffLen];

	}

	if(m_pRecvBuff)
	{
		if(m_nRecvBuffLen < nLen)
		{
			delete m_pRecvBuff;
			m_nRecvBuffLen = nLen;
			m_pRecvBuff = new unsigned char [m_nRecvBuffLen];
		}
	}

	if(!m_pRecvBuff)
	{
		return;
	}


	if(!m_bUseIcer)
	{
		if(ProcessConnCtrlMsg(aData, aTrptId, aPara) == DROP_DATA)
		{
			return;
		}

		if(!m_pConnSink)
		{
			return;
		}

		DWORD nRead = 0;
		aData.Read(m_pRecvBuff, nLen, &nRead);
#if 0
		if (aTrptId == m_pDataTrpt)
		{
			m_pConnSink->OnReceiveData(aData, TYPE_VIDEO_DATA);
		}
		else if (aTrptId == m_pCtrlTrpt)
		{
			m_pConnSink->OnReceiveData(aData, TYPE_VIDEO_CONTROL);
		}
#endif
		if (aTrptId == m_pDataTrpt)
		{
			m_pConnSink->OnReceiveData(m_pRecvBuff, nLen, TYPE_VIDEO_DATA);
		}
		else if (aTrptId == m_pCtrlTrpt)
		{
			m_pConnSink->OnReceiveData(m_pRecvBuff, nLen, TYPE_VIDEO_CONTROL);
		}
		return;
	}
	else
	{
#if 0
		int nLen = aData.GetChainedLength();

		BYTE byType = 0; 
		PeekType(aData, byType);

		if(byType == TYPE_VIDEO_DATA)
		{
			if(m_pVideoConnSink)
			{
				m_pVideoConnSink->OnReceiveData(aData, TYPE_VIDEO_DATA);
			}
		}
		else if(byType == TYPE_VIDEO_CONTROL)
		{
			if(m_pVideoConnSink)
			{
				m_pVideoConnSink->OnReceiveData(aData, TYPE_VIDEO_CONTROL);
			}		
		}
		else if(byType == TYPE_AUDIO_DATA)
		{
			if(m_pAudioConnSink)
			{
				m_pAudioConnSink->OnReceiveData(aData, TYPE_AUDIO_DATA);
			}		
		}
		else if(byType == TYPE_AUDIO_CONTROL)
		{
			if(m_pAudioConnSink)
			{
				m_pAudioConnSink->OnReceiveData(aData, TYPE_AUDIO_CONTROL);
			}		
		}
		else if(byType == TYPE_CONN_CONTROL)
		{
			ProcessConnCtrlMsg(aData, aTrptId, aPara);			
		}
#endif

		DWORD nRead = 0;
//
		BYTE byType = 0; 
//		PeekType(aData, byType);

		aData.Read(m_pRecvBuff, nLen, &nRead);

		byType = (BYTE)m_pRecvBuff[0]; 
//		PeekType(aData, byType);
#ifdef OUTPUT_MORE_LOG
		CM_INFO_TRACE_THIS("EngineDemo, CMyMediaConnection::OnReceive, byType = " << byType
			<< ", nDataLen = " << nLen
			<< ", ");
#endif

		if(byType == TYPE_VIDEO_DATA)
		{
			if(m_pVideoConnSink)
			{
				m_pVideoConnSink->OnReceiveData(m_pRecvBuff+1, nLen-1, TYPE_VIDEO_DATA);
			}
		}
		else if(byType == TYPE_VIDEO_CONTROL)
		{
			if(m_pVideoConnSink)
			{
				m_pVideoConnSink->OnReceiveData(m_pRecvBuff+1, nLen-1,  TYPE_VIDEO_CONTROL);
			}		
		}
		else if(byType == TYPE_AUDIO_DATA)
		{
			if(m_pAudioConnSink)
			{
				m_pAudioConnSink->OnReceiveData(m_pRecvBuff+1, nLen-1, TYPE_AUDIO_DATA);
			}		
		}
		else if(byType == TYPE_AUDIO_CONTROL)
		{
			if(m_pAudioConnSink)
			{
				m_pAudioConnSink->OnReceiveData(m_pRecvBuff+1, nLen-1, TYPE_AUDIO_CONTROL);
			}		
		}
		else if(byType == TYPE_CONN_CONTROL)
		{
			ProcessConnCtrlMsg(m_pRecvBuff+1, nLen-1,aTrptId, aPara);			
		}
	}

	return;
}

void CMyMediaConnection::OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
{

}

void CMyMediaConnection::OnDisconnect(CmResult aReason, ICmTransport *aTrptId)
{

	CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect, begin aReason=" <<aReason);

	if(aTrptId == m_pCtrlTrpt)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect,aTrptId == m_pCtrlTrpt,  aTrptId = " << aTrptId);
		m_pCtrlTrpt = NULL;

		m_CtrlConnectionStatus = Status_DISCONNECTED;
		m_dwLastCtrlConnCtrlMsgSendTime = 0;
		m_nCtrlConnCtrlMsgRetransmitTimes = 0;
        m_bRtcpConnected = false;
		return;
	}

	if(aTrptId == m_pDataTrpt)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect, aTrptId == m_pDataTrpt,  aTrptId = " << aTrptId);

		m_pDataTrpt = NULL;
		m_bConnected = false;
		m_DataConnectionStatus = Status_DISCONNECTED;
        m_bRtpConnected = false;
		m_dwLastDataConnCtrlMsgSendTime = 0;
		m_nDataConnCtrlMsgRetransmitTimes = 0;
        m_bRtpConnected = false;
		if(m_pConnSink)
		{
			m_pConnSink->OnDisconnected();
		}
		return;
	}

	if(aTrptId == m_pTmpDataTrpt)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect, aTrptId == m_pTmpDataTrpt,  aTrptId = " << aTrptId);
		m_pTmpDataTrpt = NULL;
		return;
	}

	if(aTrptId == m_pTmpCtrlTrpt)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect, aTrptId == m_pTmpCtrlTrpt,  aTrptId = " << aTrptId);
		m_pTmpCtrlTrpt = NULL;
		return;
	}

	if(aTrptId == m_pTrpt)
	{
		CM_INFO_TRACE_THIS("CMyMediaConnection::OnDisconnect, aTrptId == m_pTrpt,  aTrptId = " << aTrptId);
		m_pTrpt = NULL;	
		if(m_pVideoConnSink)
		{
			m_pConnSink->OnDisconnected();
		}
		if(m_pAudioConnSink)
		{
			m_pConnSink->OnDisconnected();
		}
		m_bConnected = false;

	}

	return;
}

void CMyMediaConnection::SetSink(IMediaConnectionSink* pSink)
{
	m_pConnSink = pSink;
}

void CMyMediaConnection::SetSink(IMediaConnectionSink* pSink, ConnSinkType sinkType)
{
//	return CM_OK;
	if(sinkType == CONN_SINK_VIDEO)
	{
		m_pVideoConnSink = pSink;
	}
	else if(sinkType == CONN_SINK_AUDIO)
	{
		m_pAudioConnSink = pSink;
	}

}


unsigned short CMyMediaConnection::GetBindPort()
{
	return 0;

}

bool CMyMediaConnection::IsConnected()
{
	return m_bConnected;

}

int	CMyMediaConnection::SendData(CCmMessageBlock &aData, NetDataType dataType)
{
	if(!m_bUseIcer)
	{

        DWORD dwLen = aData.GetTopLevelLength();
        CCmMessageBlock mb(dwLen+sizeof(DWORD));
        if( CCmConnectionManager::CTYPE_TCP == m_CtrlConnType)
        {
            dwLen = htonl(dwLen);
            mb.Write(&dwLen, sizeof(DWORD));
        }
        mb.Write(aData.GetTopLevelReadPtr(), aData.GetTopLevelLength());
		if ((dataType == TYPE_VIDEO_DATA || dataType == TYPE_AUDIO_DATA)&& m_pDataTrpt)
		{
	#if 0
			static int nCount = 0;

			nCount++;
			if(nCount %100 ==0 )
				return 0;
	#endif
			return m_pDataTrpt->SendData(mb);
		}
		else if ((dataType == TYPE_VIDEO_CONTROL || dataType == TYPE_AUDIO_CONTROL )&& m_pCtrlTrpt)
		{
			return m_pCtrlTrpt->SendData(mb);
		}
	}
	else
	{
		BYTE tmpBuff[2048];
		DWORD nRead = 0;
        
        if( CCmConnectionManager::CTYPE_TCP == m_CtrlConnType)
        {
            tmpBuff[sizeof(DWORD)] = dataType;
            aData.Read(tmpBuff+1+sizeof(DWORD), sizeof(tmpBuff)-1-sizeof(DWORD), &nRead);
            DWORD *pData = (DWORD *)tmpBuff;
            *pData = ntohl(nRead+1);
        }
        else {
            tmpBuff[0] = dataType;
            aData.Read(tmpBuff+1, sizeof(tmpBuff)-1, &nRead);
        }
		CCmMessageBlock mbSend(nRead+1+sizeof(DWORD), (LPCSTR)tmpBuff, 0, nRead+1+sizeof(DWORD));

		int nLen = mbSend.GetChainedLength();
		if(m_pTrpt)
		{
			return m_pTrpt->SendData(mbSend);
		}
	}

	return CM_ERROR_FAILURE;
}


int	CMyMediaConnection::SendData(unsigned char *pData, int nDataLen, NetDataType dataType)
{
	if(!m_bUseIcer)
	{

#ifdef OUTPUT_MORE_LOG
		CM_INFO_TRACE_THIS("EngineDemo, CMyMediaConnection::SendData, dataType = " << dataType
			<< ", nDataLen = " << nDataLen
			<< ", ");
#endif
		CCmMessageBlock mbSend(nDataLen+sizeof(DWORD));//, (LPCSTR)pData, CCmMessageBlock::DONT_DELETE, nDataLen);
        if( CCmConnectionManager::CTYPE_TCP == m_CtrlConnType)
        {
        DWORD dwLen = htonl(nDataLen);
        mbSend.Write(&dwLen, sizeof(DWORD));
        }
        mbSend.Write(pData, nDataLen);

		if ((dataType == TYPE_VIDEO_DATA || dataType == TYPE_AUDIO_DATA)&& m_pDataTrpt)
		{
			return m_pDataTrpt->SendData(mbSend);
		}
		else if ((dataType == TYPE_VIDEO_CONTROL || dataType == TYPE_AUDIO_CONTROL )&& m_pCtrlTrpt)
		{
			return m_pCtrlTrpt->SendData(mbSend);
		}
	}
	else
	{
		BYTE tmpBuff[16];
		tmpBuff[0] = dataType;

		CCmMessageBlock mbSend(nDataLen+1+sizeof(DWORD));
        if( CCmConnectionManager::CTYPE_TCP == m_CtrlConnType)
        {
            DWORD dwLen = nDataLen+1;
            dwLen = htonl(dwLen);
            mbSend.Write(&dwLen, sizeof(DWORD));
        }
		DWORD dwWrite = 1;
		mbSend.Write(tmpBuff, 1, &dwWrite);
		mbSend.Write(pData, nDataLen, &dwWrite);
//		CCmMessageBlock mbSend(nRead+1, (LPCSTR)tmpBuff, 0, nRead+1);

#ifdef OUTPUT_MORE_LOG
		CM_INFO_TRACE_THIS("EngineDemo, CMyMediaConnection::SendData, dataType = " << dataType
			<< ", nDataLen = " << nDataLen + 1
			<< ", ");
#endif

		int nLen = mbSend.GetChainedLength();
		if(m_pTrpt)
		{
			return m_pTrpt->SendData(mbSend);
		}
	}

	return CM_ERROR_FAILURE;
}

#if 0
int SendConnKeepLiveMsg(NetDataType datatype)
{
	CCmMessageBlock aData(4);
	CCmByteStreamNetwork _os(aData);

	_os.Write(CONN_KEEP_LIVE, 4);	


	return SendData(aData, datatype);	

}
#endif
int CMyMediaConnection::SendConnReqMsg(NetDataType datatype)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::SendConnReqMsg, datatype = " << datatype);

	CCmMessageBlock aData(4);
	CCmByteStreamNetwork _os(aData);
	if(datatype == TYPE_VIDEO_DATA)
	{
		_os.Write(DATA_CONN_REQUEST, 4);		
	}

	else
	{
		_os.Write(CTRL_CONN_REQUEST, 4);		
	}

	return SendData(aData, datatype);		
}

int CMyMediaConnection::SendConnFinMsg(NetDataType datatype)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::SendConnFinMsg, datatype = " << datatype);
	CCmMessageBlock aData(4);
	CCmByteStreamNetwork _os(aData);
	if(datatype == TYPE_VIDEO_DATA)
	{
		_os.Write(DATA_CONN_FIN, 4);		
	}

	else if(datatype == TYPE_VIDEO_CONTROL)
	{
		_os.Write(CTRL_CONN_FIN, 4);		
	}
	else
	{
		_os.Write(DATA_CONN_FIN, 4);	
	}

	if(!m_bUseIcer)
		return SendData(aData, datatype);

	return SendData(aData, TYPE_CONN_CONTROL);
}

int CMyMediaConnection::SendConnAckMsg(NetDataType datatype)
{
	CM_INFO_TRACE_THIS("CMyMediaConnection::SendConnAckMsg, datatype = " << datatype);
	CCmMessageBlock aData(4);
	CCmByteStreamNetwork _os(aData);
	if(datatype == TYPE_VIDEO_DATA)
	{
		_os.Write(DATA_CONN_ACK, 4);		
	}

	else
	{
		_os.Write(CTRL_CONN_ACK, 4);		
	}

	return SendData(aData, datatype);
}

#if 0
int CMyMediaConnection::SendConnCtrlMsg(NetDataType datatype)
{
	if(datatype == TYPE_VIDEO_DATA)
	{
		if(Status_DISCONNECTED == m_DataConnectionStatus || Status_SEND_REQUEST == m_DataConnectionStatus)
		{
			_os.Write(DATA_CONN_REQUEST, 4);		
		}
		else if(Status_CONNECTED == m_DataConnectionStatus)
		{
			_os.Write(DATA_CONN_ACK, 4);			
		}
		
		return SendData(aData, TYPE_VIDEO_DATA);	
	}

	if(datatype == TYPE_VIDEO_CONTROL)
	{
		if(Status_DISCONNECTED == m_CtrlConnectionStatus || Status_SEND_REQUEST == m_CtrlConnectionStatus)
		{
			_os.Write(CTRL_CONN_REQUEST, 4);		
		}
		else if(Status_CONNECTED == m_CtrlConnectionStatus)
		{
			_os.Write(CTRL_CONN_ACK, 4);			
		}


		return SendData(aData, TYPE_VIDEO_CONTROL);	
	}

	return CM_ERROR_FAILURE;
}
#endif

void CMyMediaConnection::OnTimer(CCmTimerWrapperID *aId)
{
	DWORD dwNow = static_cast<DWORD>(ticker::now()/1000);
	const DWORD dwMaxGap = 300000;
	const DWORD dwConnectTimeout = 1000;

#if 0
	if(m_dwLastRecvKeepLiveTime > 0 && dwNow > m_dwLastRecvKeepLiveTime + dwMaxGap)
	{
		if(m_DataConnType == CCmConnectionManager::CTYPE_UDP && m_pDataTrpt)
		{
			m_pDataTrpt->Disconnect(0);
			m_pDataTrpt = NULL;
		}
		if(m_CtrlConnType == CCmConnectionManager::CTYPE_UDP && m_pCtrlTrpt)
		{
			m_pCtrlTrpt->Disconnect(0);
			m_pCtrlTrpt = NULL;
		}

		if(m_pConnSink)
		{
			m_pConnSink->OnDisconnected();
		}

		m_bConnected = false;

		CancelTimer();
	}
#endif

	if(m_dwLastCtrlConnCtrlMsgSendTime > 0 && m_CtrlConnectionStatus == Status_SEND_REQUEST)
	{
		if(dwNow > m_dwLastCtrlConnCtrlMsgSendTime + dwConnectTimeout)
		{
			if(m_nCtrlConnCtrlMsgRetransmitTimes < MAX_TRY_TIME)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnTimer, SendConnReqMsg for control connection, m_nCtrlConnCtrlMsgRetransmitTimes = " << m_nCtrlConnCtrlMsgRetransmitTimes);

				SendConnReqMsg(TYPE_VIDEO_CONTROL);
				m_dwLastCtrlConnCtrlMsgSendTime = dwNow;
				m_nCtrlConnCtrlMsgRetransmitTimes++;
			}
			else
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnTimer, Already SendConnReqMsg for control connection "  << m_nCtrlConnCtrlMsgRetransmitTimes << " times, failed to build control connection");
				m_CtrlConnectionStatus = Status_DISCONNECTED;
				m_dwLastCtrlConnCtrlMsgSendTime = 0;
				m_nCtrlConnCtrlMsgRetransmitTimes = 0;	
			}

		}
	}

	if(m_dwLastDataConnCtrlMsgSendTime > 0 && m_DataConnectionStatus == Status_SEND_REQUEST)
	{
		if(dwNow > m_dwLastDataConnCtrlMsgSendTime + dwConnectTimeout)
		{
			if(m_nDataConnCtrlMsgRetransmitTimes < MAX_TRY_TIME)
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnTimer, SendConnReqMsg for data connection, m_nDataConnCtrlMsgRetransmitTimes = " << m_nDataConnCtrlMsgRetransmitTimes);
				SendConnReqMsg(TYPE_VIDEO_DATA);
				m_dwLastDataConnCtrlMsgSendTime = dwNow;
				m_nDataConnCtrlMsgRetransmitTimes++;
			}
			else
			{
				CM_INFO_TRACE_THIS("CMyMediaConnection::OnTimer, Already SendConnReqMsg for data connection "  << m_nDataConnCtrlMsgRetransmitTimes << " times, failed to build data connection");
				m_DataConnectionStatus = Status_DISCONNECTED;
				m_dwLastDataConnCtrlMsgSendTime = 0;
				m_nDataConnCtrlMsgRetransmitTimes = 0;

				if(m_CtrlConnectionStatus == Status_CONNECTED)
				{
					m_pCtrlTrpt->Disconnect(0);
					m_pCtrlTrpt = NULL;
					m_CtrlConnectionStatus = Status_DISCONNECTED;
				}

				if(m_pConnSink)
				{
					m_pConnSink->OnConnected(false);
				}			
			}
		}
	}

#if 0
	if(m_dwLastCtrlConnCtrlMsgSendTime > 0 && m_CtrlConnectionStatus == Status_CONNECTED)
	{
		if(dwNow > m_dwLastCtrlConnCtrlMsgSendTime + dwConnectTimeout)
		{
			SendConnKeepLiveMsg(TYPE_VIDEO_CONTROL);
			m_dwLastCtrlConnCtrlMsgSendTime = dwNow;
		}	
	}

	if(m_dwLastDataConnCtrlMsgSendTime > 0 && m_DataConnectionStatus == Status_CONNECTED)
	{
		if(dwNow > m_dwLastDataConnCtrlMsgSendTime + dwConnectTimeout)
		{
			SendConnKeepLiveMsg(TYPE_VIDEO_DATA);
			m_dwLastDataConnCtrlMsgSendTime = dwNow;
		}
	}
#endif
	return;
}

void CMyMediaConnection::CancelTimer()
{
	m_dwLastRecvKeepLiveTime = 0;
	m_dwLastDataConnCtrlMsgSendTime = 0;
	m_dwLastCtrlConnCtrlMsgSendTime = 0;

	m_TimerId.Cancel();
}

void CMyMediaConnection::ScheduleTimer()
{

	CancelTimer();
	if(m_DataConnType == CCmConnectionManager::CTYPE_UDP)
	{
		m_TimerId.Schedule(this, CCmTimeValue(1, 0));
	}
}

int CMyMediaConnection::InitHost(const char *pMyName,const char * JingleServerIP, int JingleServerPort, const char *StunServerIP, int StunServerPort)
{
	if(m_bNotUseIcer)
	{
		return CM_ERROR_FAILURE;
	}

	m_bNotUseIcer = false;
	m_bUseIcer = true;

	if(m_pIcerClient)
	{
		return 0;
	}

	bool bIsCaller = false;
	CreateIcerClient(&m_pIcerClient, bIsCaller, 1);

	if(!m_pIcerClient)
	{
		return -1;
	}
	m_pIcerClient->SetSink(this);
	m_pIcerClient->SetName(pMyName);

	m_pIcerClient->SetStunServer(StunServerIP, StunServerPort);
	m_pIcerClient->ConnectToServer(JingleServerIP,JingleServerPort);

	return 0;
}

int CMyMediaConnection::ConnectTo(const char *pMyName,const char *pHostName,const char * JingleServerIP, int JingleServerPort, const char *StunServerIP, int StunServerPort)
{

	if(m_bNotUseIcer)
	{
		return CM_ERROR_FAILURE;
	}

	m_bNotUseIcer = false;
	m_bUseIcer = true;

	if(m_pIcerClient)
	{
		return 0;
	}

	bool bIsCaller = true;
	CreateIcerClient(&m_pIcerClient, bIsCaller, 1);

	if(!m_pIcerClient)
	{
		return -1;
	}
	m_pIcerClient->SetSink(this);
	m_pIcerClient->SetName(pMyName);
	m_pIcerClient->SetPeerName(pHostName);
	m_pIcerClient->SetStunServer(StunServerIP, StunServerPort);	
	m_pIcerClient->ConnectToServer(JingleServerIP,JingleServerPort);

	return 0;
}

void  CMyMediaConnection::OnIceConcluded( const IIcer* pIcer)
{

}
void  CMyMediaConnection::OnValidTransport( const IIcer* pIcer, const ICmTransport* pTransport, 
							  int iComponentId, UINT64 transportPriority)
{
	if(!m_pTrpt && pTransport)
	{
		m_pTrpt = (ICmTransport*)pTransport;
		m_pTrpt->OpenWithSink(this);
		if(m_pVideoConnSink)
		{
			m_pVideoConnSink->OnConnected(true);
		}

		if(m_pAudioConnSink)
		{
			m_pAudioConnSink->OnConnected(true);
		}
		m_bConnected = true;

	}
}

void  CMyMediaConnection::OnIceError( const IIcer* pIcer, int iError )
{

}

void  CMyMediaConnection::OnLocalCandidateInfo( const IIcer* pIcer,
								   const IceCandidate** pLocalCandidates, 
								   int iCandidatesSize, bool bCompleted)
{

}


/////////////////////////////////////////////////////////////////////////////////////////
int CreateMediaConnection(IMediaConnection **ppConn)
{
	*ppConn = new CMyMediaConnection();
	if(*ppConn)
	{
		return 0;
	}

	return -1;
}

int DeleteMediaConnection(IMediaConnection *pConn)
{
    CM_INFO_TRACE("DeleteMediaConnection, pConn="<<(void*)pConn);
	if(pConn)
	{
		delete pConn;
	}

	return 0;
}

static IMediaConnection *s_pInstance = NULL;
static int s_nRef = 0;

IMediaConnection* GetMediaConnectionInstance()
{
	if(s_pInstance)
	{
		s_nRef++;
		return s_pInstance;
	}

	int ret = CreateMediaConnection(&s_pInstance);

	if(ret == 0)
	{
		s_nRef++;
		return s_pInstance;
	}

	return NULL;
}

void DeleteMediaConnectionInstance()
{
	s_nRef--;
	if(s_nRef <= 0)
	{
		if(s_pInstance)
		{
			DeleteMediaConnection(s_pInstance);
			s_pInstance = NULL;
		}
		s_nRef = 0;
	}
}
