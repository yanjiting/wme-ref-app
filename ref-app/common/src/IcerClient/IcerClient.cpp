//
//  MessageClient.cpp
//  ForwardClient
//
//  Created by kejni on 13-7-4.
//  Copyright (c) 2013年 Cisco. All rights reserved.
//

#include "tinyxml.h"

#include "IcerClient.h"
#include "CandidateParse.h"


class CIcerTransport : public ICmTransportSink
{
public:
	// interface of ICmTransportSink
	virtual void OnReceive(
		CCmMessageBlock &aData,
		ICmTransport *aTrptId,
		CCmTransportParameter *aPara = NULL)
	{
		CM_INFO_TRACE("CIcerTransport::OnReceive: aData = " << aData.FlattenChained().c_str() << ", aTrptId = " << aTrptId << ", aPara = " << aPara);
	}

	virtual void OnSend(
		ICmTransport *aTrptId,
		CCmTransportParameter *aPara = NULL)
	{
		CM_INFO_TRACE("CIcerTransport::OnSend");
	}

	virtual void OnDisconnect(
		CmResult aReason,
		ICmTransport *aTrptId)
	{
		CM_INFO_TRACE("CIcerTransport::OnDisconnect");
	}

	CmResult SetTransport(ICmTransport* pTransport)
	{
		CM_INFO_TRACE("CIcerTransport::SetTransport, pTransport="<<(void*)pTransport);

		if (m_pTransUdp.Get() != NULL) {
			m_pTransUdp->Disconnect(CM_OK);
			m_pTransUdp = NULL;
		}
		CmResult nResult = pTransport->OpenWithSink(this);
		CM_ASSERTE_RETURN(CM_SUCCEEDED(nResult), nResult);

		m_pTransUdp = pTransport;

		char szBuff[] = "hello,P2P";
		int nStrlen = strnlen(szBuff,1024);
		CCmMessageBlock aData(nStrlen, szBuff, CCmMessageBlock::DONT_DELETE, nStrlen);
		m_pTransUdp->SendData(aData);

		return CM_OK;
	}
private:
	CCmComAutoPtr<ICmTransport> m_pTransUdp;
};


CIcerTransport g_IcerTransport;

CIcerClient::CIcerClient(bool bIsCaller,int nComponent)
:m_pClient(0)
,m_pIcer(NULL)
,m_eNominationMode(ICE_NOMINATION_MODE_REGULAR)
,m_eWorkMode(ICE_MODE_VANILLA)
,m_bIsCaller(bIsCaller)
,m_nComponent(nComponent)
,m_pIcerSink(NULL)
{
    m_pCandidateParse = new CCandidateParse;
}

CIcerClient::~CIcerClient()
{
    if( m_pCandidateParse)
    {
        delete m_pCandidateParse;
        m_pCandidateParse = NULL;
    }
	if(m_pClient)
	{
		m_pClient->Close(0);
		delete m_pClient;
		m_pClient = NULL;
	}
    
#if 0
	if(m_pIcer)
	{
		m_pIcer->Stop();
		p2p_destroy_ice(m_pIcer);
		m_pIcer = NULL;
	}
#endif
}

 void CIcerClient::OnClientSessionJoin(CmResult aReason)
{
    //Start ICE

	std::string strSetNameCmd = "bind";
	if( m_pClient )
	{
		strSetNameCmd += "#";
		strSetNameCmd += m_strMyName;
		if(m_bIsCaller)
		{
			//client
			strSetNameCmd += "$";
			strSetNameCmd += m_strPeerName;

		}
#if 0
		else

		{
			//client
			strSetNameCmd += "$";
			strSetNameCmd += "testc";

		}
#endif
//		std::string strSetNameCmd = "bind#tom$jerry";

		CCmMessageBlock aMB(strSetNameCmd.length(),
							strSetNameCmd.c_str(),
							0,
							strSetNameCmd.length());

		m_pClient->SendData(aMB);
		fprintf(stderr, "CIcerClient::OnClientSessionJoin, send binding info to jingle server, %s\n", strSetNameCmd.c_str());

	}


	fprintf(stderr, "CIcerClient::OnClientSessionJoin, StartICE\n");
    StartICE();
}

 void CIcerClient::OnClientSessionLeave(CmResult aReason)
{

}

 void CIcerClient::OnClientReceive(CCmString& aData)
{
    if( aData.length() && m_pCandidateParse )
    {
        std::vector< IceCandidate*> vecCandidates;
        eIceRole eRole = ICE_ROLE_CONTROLLED;
		fprintf(stderr, "CIcerClient::OnClientReceive, recept data:\n%s\n", aData.c_str());
 
		m_pCandidateParse->ParseCandidate( aData.c_str(), eRole, vecCandidates);
        
        const IceCandidate **ppCan = new const IceCandidate*[ vecCandidates.size()];
		int i = 0;
		for (CandidateLst::iterator it = vecCandidates.begin(); it != vecCandidates.end(); ++it)
		{
			ppCan[i++] = (*it);
		}

		fprintf(stderr, "CIcerClient::OnClientReceive, receive %d remote candidates\n", vecCandidates.size());
		
        if( m_pIcer)
        {
			if(vecCandidates.size() > 0)
			{
				m_pIcer->AddRemoteCandidates(ppCan, vecCandidates.size(), true);
				if(m_bIsCaller == false)
				{
					fprintf(stderr, "CIcerClient::OnClientReceive, start ICER\n");
					m_pIcer->Start();	
				}
			}
			else
			{
					fprintf(stderr, "CIcerClient::OnClientReceive, vecCandidates.size() = 0\n");
			
			}

        }
        
        delete [] ppCan;
    }
}

void CIcerClient::ConnectToServer(const char* szServer, int nPort)
{
   
    if( !m_pClient )
    {

        m_pClient = new CSimpleClientSession(szServer,
                                             nPort,
                                             TRUE,
                                             this);
    }
    
     m_pClient->Connect();    

}


//callback functions of IIcerSink
/**
 * Callback notification of an ICE conclusion being reached.
 *
 */
void CIcerClient::OnIceConcluded( const IIcer* pIcer)
{
    CM_INFO_TRACE("CIcerClient::OnIceConcluded");
	if(m_pIcerSink)
	{
		m_pIcerSink->OnIceConcluded(pIcer);
	}
}

/**
 * Callback notification of an ICE conclusion being reached.
 *
 */
void CIcerClient::OnValidTransport( const IIcer* pIcer, const ICmTransport* pTransport, int iComponentId, UINT64 transportPriority)
{
	 CM_INFO_TRACE("CIcerClient::OnValidTransport");
	fprintf(stderr, "CIcerClient::OnValidTransport\n");
//	 g_IcerTransport.SetTransport(const_cast<ICmTransport*>(pTransport));
	if(m_pIcerSink)
	{
		m_pIcerSink->OnValidTransport(pIcer, pTransport, iComponentId, transportPriority);
	}
}

/**
 * Callback notification of an ICE occur errors.
 *
 */
void CIcerClient::OnIceError( const IIcer* pIcer, int iError )
{
    if(m_pIcerSink)
	{
		m_pIcerSink->OnIceError(pIcer, iError);
	}
}

/**
 * Callback notification of all local ICE candidates for a media
 * @param iIndex the index of media line
 */
void CIcerClient::OnLocalCandidateInfo( const IIcer* pIcer,
                                  const IceCandidate** pLocalCandidates,
                                  int iCandidatesSize, bool bCompleted /*= false*/)
{
    
    
    if( !iCandidatesSize )
    {
		fprintf(stderr, "CIcerClient::OnLocalCandidateInfo, get no candidates\n");
        return;
    }
  
	fprintf(stderr, "CIcerClient::OnLocalCandidateInfo, get %d candidates\n", iCandidatesSize);

    for( int i = 0; i < iCandidatesSize; i++)
    {
        const IceCandidate* pCandidate = pLocalCandidates[i];
        m_vecCandidates.push_back(pCandidate);
    }
    
    if( bCompleted && m_pCandidateParse)
    {
        std::string strXML;
		eIceRole eRole = ICE_ROLE_CONTROLLING;
		if ( !m_bIsCaller )
		{
			eRole = ICE_ROLE_CONTROLLED;
		}
		
        m_pCandidateParse->PackCandidate(eRole,m_vecCandidates, strXML);

		fprintf(stderr, "detail candidates info:\n%s\n", strXML.c_str());

        
        if( m_pClient)
        {
            CCmMessageBlock aMB(strXML.length(),
                                strXML.c_str(),
                                0,
                                strXML.length());
            
            m_pClient->SendData(aMB);
        }
    }
}

void CIcerClient::SetSink(IIcerSink *pIcerSink)
{
	m_pIcerSink = pIcerSink;
}

void CIcerClient::SetStunServer(const char* szSver, UINT nPort)
{
	if( !szSver )
		return;
	m_strStunServerAddr = szSver;
	m_uStunServerPort = nPort;
}

void CIcerClient::SetName(const char* szMyName)
{
	if(!szMyName)
	{
		return;
	}

	m_strMyName = szMyName;
}

void CIcerClient::SetPeerName(const char* szPeerName)
{
	if(!szPeerName)
	{
		return;
	}

	m_strPeerName = szPeerName;
}


void CIcerClient::StartICE()
{
#if 0
	IceMlineInfo imi;
	imi.m_iMaxComponent = 1;
	imi.m_transType = CST_TRANSPORT_TYPE_UDP;
	imi.m_nominationMode = (eIceNominationMode)m_eNominationMode;
	imi.m_workMode = (eIceWorkMode)m_eWorkMode;
	if(m_bIsCaller != false)
		imi.m_initialRole = ICE_ROLE_CONTROLLING;
	else
		imi.m_initialRole = ICE_ROLE_CONTROLLED;

	imi.m_iMaxComponent = m_nComponent;
    
	if(m_pIcer)
	{
		m_pIcer->Stop();
		p2p_destroy_ice(m_pIcer);
	}
	p2p_create_ice( &m_pIcer);
	if(m_pIcer != NULL)
	{
		CCmInetAddr addr(m_strStunServerAddr.c_str(), m_uStunServerPort);
		m_pIcer->Init(this,imi, addr);
		if(m_bIsCaller != false)
			m_pIcer->Start();
	}
#endif
}

extern "C" int  CreateIcerClient(IIcerClient **ppClient, bool bIsCaller,int nComponent )
{
	if(!ppClient)
		return - 1;
	*ppClient = NULL;

	CIcerClient *pIcerClient = new CIcerClient(bIsCaller, nComponent);

	if(!pIcerClient)
		return -1;

	*ppClient = (IIcerClient *)pIcerClient;
	return 0;

}
extern "C" int  DeleteIcerClient(IIcerClient *pClient)
{
	if(!pClient)
		return -1;

	delete ((CIcerClient *)pClient);
	return 0;
}