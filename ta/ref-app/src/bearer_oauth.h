#ifndef __BEARER_OAUTH_WEBEX_SQUARE_H__
#define __BEARER_OAUTH_WEBEX_SQUARE_H__

#pragma once

#include "CmBase.h"
#include "CmHttpClient.h"

using namespace std;

class IWx2AuthSink
{
public:
    virtual ~IWx2AuthSink() {}
    virtual void OnAuthenicateHeader(const string &headers) = 0;
};

class CWX2Auth : public ICmHttpClientSink, public ICmEvent
{
public:
    CWX2Auth(IWx2AuthSink *pSink);
    virtual ~CWX2Auth();

public:
    void GetAuthenticationHeader();
    void Close();

protected:
    void GetBearerToken();
    void GetAccessToken();
	virtual void OnReceive(CmResult nResult, CCmMessageBlock *aData);
	virtual CmResult OnEventFire();
    virtual void OnDestorySelf() {}

    ICmHttpClient *m_pHttpClient;
    IWx2AuthSink *m_pSink;
    enum{
        AUTH_INITIALIZED = 0,
        AUTH_GET_BEARER_TOKEN = 1,
        AUTH_GET_ACCESS_TOKEN = 2,
        AUTH_DONE
    } m_state;

    string m_oBearerToken;
    string m_sAccessToken;
};

#endif //!define __BEARER_OAUTH_WEBEX_SQUARE_H__
