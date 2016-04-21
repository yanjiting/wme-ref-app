#ifndef __CALLIOPE_REST_API_WEBEX_SQUARE_H__
#define __CALLIOPE_REST_API_WEBEX_SQUARE_H__

#pragma once

#include "CmBase.h"
#include "CmHttpClient.h"
#include "bearer_oauth.h"

class COrpheusClient;
class IOrpheusClientSink
{
public:
    virtual ~IOrpheusClientSink() {}

public:
	virtual void OnVenue(const string &venueUrl, COrpheusClient* pOrigin) = 0;
	virtual void OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin) = 0;
};

class COrpheusClient : public IWx2AuthSink,
                       public ICmHttpClientSink, 
                       public ICmEvent
{
public:
    COrpheusClient(IOrpheusClientSink *pSink, const char *szTrackingID = NULL);
    virtual ~COrpheusClient();
    
public:
    void CreateVenue();
    void CreateLinusConfluence(string linusIp, string venueUrl, string offer, string uuid);
    void DeleteConfluence(bool deleteVenue, string url = "");
    void CreateFlow(string venueUrl, string offer, string uuid);
    void DeleteVenue();
    void Close();
    void RequestFloor();
    void ReleaseFloor();

protected:
    virtual void OnAuthenicateHeader(const string &headers);

	virtual CmResult OnEventFire();
    virtual void OnDestorySelf() {}
    virtual void OnReceive(CmResult nResult, CCmMessageBlock *aData);

    void DoTask();

protected:
    enum{
        ORPHEUS_INIT = 0,
        ORPHEUS_CREATE_VENUE,
        ORPHEUS_CREATE_FLOW,
        ORPHEUS_CREATE_CONFLUENCE,
        ORPHEUS_CREATE_DELETE_VENUE,
        ORPHEUS_CREATE_LINUS_CONFLUENCE,
        ORPHEUS_DELETE_CONFLUENCE,
        ORPHEUS_REQUEST_FLOOR,
        ORPHEUS_RELEASE_FLOOR,
        ORPHEUS_DONE
    }m_state;
    ICmHttpClient *m_pHttpClient;
    CWX2Auth *m_auth;
    IOrpheusClientSink *m_pSink;
    string m_sAuth;
    string m_sVenueUrl;
    string m_sVenueUuid;
    string m_sRemoteSDP;
    string m_sOffer;
    string m_sUuid;
    string m_sTrackId;
    string m_sLinusAddr;
    string m_sUrlForDelete;
    string m_sConfluenceUrl;
    string m_sFloorRequestUrl;
    string m_sFloorReleaseUrl;
    bool m_bDeleteVenue;
};

#endif //!define __CALLIOPE_REST_API_WEBEX_SQUARE_H__
