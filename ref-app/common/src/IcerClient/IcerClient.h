//
//  MessageClient.h
//  ForwardClient
//
//  Created by kejni on 13-7-4.
//  Copyright (c) 2013年 Cisco. All rights reserved.
//

#ifndef __ForwardClient__MessageClient__
#define __ForwardClient__MessageClient__

#include "cstdefine.h"
#include "csttransport.h"
#include "CmConnectionInterface.h"

#include "SimpleClientSession.h"
#include "IcerClientInterface.h"

class CCandidateParse;

typedef std::vector<IceCandidate*> CandidateLst;

class CIcerClient:public ISimpleClientSessionSink,public IIcerSink, public IIcerClient
{
public:
    
    CIcerClient(bool bIsCaller,int);
    
    ~CIcerClient();
    
	virtual void SetName(const char* szMyName);
	virtual void SetPeerName(const char* szPeerName);
	virtual void SetStunServer(const char* szSver, UINT nPort);
    virtual void ConnectToServer(const char* szServer, int nPort);
	virtual void SetSink(IIcerSink *pIcerSink);

    virtual void OnClientSessionJoin(CmResult aReason);
	virtual void OnClientSessionLeave(CmResult aReason);
	virtual void OnClientReceive(CCmString& aData);

	virtual void OnIceConcluded( const IIcer* pIcer);
  	virtual void OnValidTransport( const IIcer* pIcer, const ICmTransport* pTransport, int iComponentId, UINT64 transportPriority);
	virtual void OnIceError( const IIcer* pIcer, int iError );
 	virtual void OnLocalCandidateInfo( const IIcer* pIcer,
                                      const IceCandidate** pLocalCandidates,
                                      int iCandidatesSize, bool bCompleted = false);



private:
	void StartICE();
    void ParseCandidatesFromXML();
    void PackCandidates(char* pszXML, int& nLen);

    
private:
    //TCP Connection 
    CSimpleClientSession* m_pClient;
    
    //Pack/unpack candidate information for test
    CCandidateParse*      m_pCandidateParse;
    
    //ICE related
	IIcer*		  m_pIcer;
	bool   		  m_bIsCaller;
	int           m_nComponent;
	eIceWorkMode  m_eWorkMode;
	eIceNominationMode m_eNominationMode;
    
    std::vector<const IceCandidate*> m_vecCandidates;

	std::string m_strStunServerAddr;
	std::string m_strMyName;
	std::string m_strPeerName;
	UINT		m_uStunServerPort;
	IIcerSink *m_pIcerSink;
};
#endif /* defined(__ForwardClient__MessageClient__) */
