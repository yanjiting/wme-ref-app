#ifndef MYMEDIACONNECTION_H
#define MYMEDIACONNECTION_H

#include "MediaConnection.h"
#include "CmConnectionInterface.h"

#include "IcerClientInterface.h"

class CMyMediaConnection: public IMediaConnection,
				public ICmAcceptorConnectorSink,
				public ICmTransportSink,
				public CCmTimerWrapperIDSink,
				public IIcerSink
{
public:

	enum ConnectStatus
	{
		Status_DISCONNECTED = 0,
		Status_LISTEN = 1,
		Status_SEND_REQUEST = 2,
		Status_SEND_ACK = 3,
		Status_CONNECTED = 4,
	};

	enum DataAction
	{
		DROP_DATA = 0,
		SUBMIT_DATA = 1,
	};

	CMyMediaConnection();
	~CMyMediaConnection();

    ///
    /// for IMediaConnection

    // for normal connect and server
	virtual int InitHost(bool enableCtrl, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType);
	virtual int ConnectTo(const char *hostIP, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType);
	virtual void SetSink(IMediaConnectionSink* pSink);

	virtual int Stop();
	virtual int DisConnect();

	virtual unsigned short GetBindPort();
	virtual bool IsConnected();

	virtual int	SendData(CCmMessageBlock &aData, NetDataType datatype);
	virtual int	SendData(unsigned char *pData, int nDataLen, NetDataType datatype);

    // for ICE connect and server
	virtual int InitHost(const char *myName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort);
	virtual int ConnectTo(const char *myName, const char *hostName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort);
	virtual void SetSink(IMediaConnectionSink* pSink, ConnSinkType sinkType);


	virtual void OnConnectIndication(CmResult aReason, ICmTransport *aTrpt, ICmAcceptorConnectorId *aRequestId);
	virtual void OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
	virtual void OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
	virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId);
	virtual void OnTimer(CCmTimerWrapperID *aId);

	void ScheduleTimer();
	void CancelTimer();

	DataAction ProcessConnCtrlMsg(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara);
	DataAction ProcessConnCtrlMsg(unsigned char *pData, int nDataLen, ICmTransport *aTrptId, CCmTransportParameter *aPara);

    ///
	/// for IIcerSink
	virtual void OnIceConcluded( const IIcer* pIcer);
	virtual void OnValidTransport( const IIcer* pIcer, const ICmTransport* pTransport, int iComponentId, UINT64 transportPriority);
	virtual void OnIceError( const IIcer* pIcer, int iError );
	virtual void OnLocalCandidateInfo( const IIcer* pIcer,
									   const IceCandidate** pLocalCandidates, 
									   int iCandidatesSize, bool bCompleted = false);

protected:
    void OnDataReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara);
    void OnCtrlReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara);
    void OnReceiveOneBlock(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara);
    
    char m_dataBuffer[64*1024];
    int m_nDataRemain;
    char m_ctrlBuffer[8*1024];
    int m_nCtrlRemain;

public:
//	int SendConnCtrlMsg(NetDataType datatype);
	int SendConnAckMsg(NetDataType datatype);
	int SendConnFinMsg(NetDataType datatype);
	int SendConnReqMsg(NetDataType datatype);

private:
	CCmComAutoPtr<ICmAcceptor> m_pCtrlAcceptor;
	CCmComAutoPtr<ICmAcceptor> m_pDataAcceptor;
	CCmComAutoPtr<ICmConnector> m_pCtrlConnector;	//For control data, at client
	CCmComAutoPtr<ICmConnector> m_pDataConnector;	//For application data, at client
	CCmComAutoPtr<ICmTransport> m_pCtrlTrpt;
	CCmComAutoPtr<ICmTransport> m_pDataTrpt;
	CCmComAutoPtr<ICmTransport> m_pTmpCtrlTrpt;
	CCmComAutoPtr<ICmTransport> m_pTmpDataTrpt;

	CCmTimerWrapperID m_TimerId;

	bool m_bHost;
	bool m_bConnected;

	bool m_bHostInited;

	IMediaConnectionSink* m_pConnSink;

	bool m_bAllowCtrlConn;
	CCmConnectionManager::CType m_DataConnType;
	CCmConnectionManager::CType m_CtrlConnType;

	DWORD m_dwLastRecvKeepLiveTime;
	ConnectStatus m_CtrlConnectionStatus;
	ConnectStatus m_DataConnectionStatus;

	DWORD m_dwLastDataConnCtrlMsgSendTime;
	DWORD m_dwLastCtrlConnCtrlMsgSendTime;
	int m_nDataConnCtrlMsgRetransmitTimes;
	int m_nCtrlConnCtrlMsgRetransmitTimes;

/////////////////////////////////////////
	//for using ICER
	IIcerClient *m_pIcerClient;
	bool m_bUseIcer;
	bool m_bNotUseIcer;
	CCmComAutoPtr<ICmTransport> m_pTrpt;
	IMediaConnectionSink* m_pVideoConnSink;
	IMediaConnectionSink* m_pAudioConnSink;

	unsigned char *m_pRecvBuff;
	int m_nRecvBuffLen;
    
    bool m_bRtpConnected;
    bool m_bRtcpConnected;
};

#endif
