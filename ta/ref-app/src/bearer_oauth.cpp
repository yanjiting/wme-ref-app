#include <string>
#include <iomanip>
#include <sstream>
#include "ClickCall.h"
#include "testconfig.h"

using namespace std;
#include "bearer_oauth.h"
#include "json/json.h"

const string BROKER_URL = "https://idbroker.webex.com/idb";
const string BEARER_URL = BROKER_URL + "/token/v1/actions/GetBearerToken/invoke";

const string ACCESS_URL = BROKER_URL + "/oauth2/v1/access_token";
// TODO: we could do with a Calliope scope, how?
//const string SCOPE = "webexsquare:get_conversation";
//const string CLIENT_ID = "Cd6966e933eea56cb9fc3dabfd3d7dd51dc9594ce857bc81f173eae82da8348be";
//const string CLIENT_SECRET = "4fc884b0c4695b515cb117719ef48e1fd988f2973e59810a70ea7c0c136a47f5";
//Linus scope
const string SCOPE = "webexsquare:create_venue";
const string CLIENT_ID = "Ccebba9b7f96c8f6a0d0586359962a8e9faab518640a8de66f9e19d46453c548a";
const string CLIENT_SECRET = "19b128fe7285ec3c67daf8135ae691c991b7706b0ff00f82f7aa6bbe88fcb609";

const string GRANT_TYPE = "urn:ietf:params:oauth:grant-type:saml2-bearer";
const string HEALTH_CHECK_URL = BROKER_URL + "/HealthCheck.jsp";

const string FILES_URL = "https://beta.webex.com/files/api/v1/authenticate";
const string FILES_PASSWORD = "SecretPassw0rd!";
const string FILES_EMAIL = "calliope@wx2.example.com";

string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else if (c == ' ')  {
            escaped << '+';
        }
        else {
            escaped << '%' << setw(2) << ((int) c) << setw(0);
        }
    }

    return escaped.str();
}

////////////////////////
//CWX2Auth
////////////////////////
CWX2Auth::CWX2Auth(IWx2AuthSink *pSink)
    :m_pHttpClient(NULL),
     m_state(AUTH_INITIALIZED),
     m_pSink(pSink)
{
}

CWX2Auth::~CWX2Auth()
{
    Close();
}

void CWX2Auth::Close()
{
    if(m_pHttpClient != NULL)
    {
        m_pHttpClient->Close();
        tp_destroy_httpclient(m_pHttpClient);
        m_pHttpClient = NULL;
    }
}

void CWX2Auth::GetBearerToken()
{
    Close();

    tp_create_httpclient(&m_pHttpClient);
    m_state = AUTH_GET_BEARER_TOKEN;
    BOOL bVerify = FALSE;
    m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
    m_pHttpClient->Open("post", BEARER_URL.c_str(), this, 1);
    m_pHttpClient->SetRequestHeader("Content-Type", "application/json");

    Json::Value root(Json::objectValue);
    root["uid"] = Json::Value(TestConfig::i().m_sOAuthUID);
    root["password"] = Json::Value(TestConfig::i().m_sOAuthPasswd);
    Json::FastWriter w;
    string sTxt = w.write(root);

    CCmMessageBlock mb((DWORD)sTxt.length() + 1);
    mb.Write(sTxt.c_str(), (DWORD)sTxt.length());

    m_pHttpClient->Send(&mb);
}

void CWX2Auth::GetAccessToken()
{
    Close();

    tp_create_httpclient(&m_pHttpClient);
    m_pHttpClient->Open("post", ACCESS_URL.c_str(), this, 1);
    m_pHttpClient->SetRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    m_pHttpClient->SetRequestHeader("Accept", "application/json");

    string sSending;
    sSending += ("grant_type=" + url_encode(GRANT_TYPE) + "&");
    sSending += ("scope=" + url_encode(SCOPE) + "&");
    sSending += ("client_id=" + url_encode(CLIENT_ID) + "&");
    sSending += ("client_secret=" + url_encode(CLIENT_SECRET) + "&");
    sSending += ("assertion=" + url_encode(m_oBearerToken) + "&");

	CM_INFO_TRACE_THIS("HTTPContent, auth, sending=" << sSending);
    CCmMessageBlock mb((DWORD)sSending.length() + 1);
    mb.Write(sSending.c_str(), (DWORD)sSending.length());

    m_pHttpClient->Send(&mb);
}

void CWX2Auth::GetAuthenticationHeader()
{
    GetBearerToken();
}

CmResult CWX2Auth::OnEventFire()
{
    if(m_state == AUTH_GET_ACCESS_TOKEN)
        GetAccessToken();
    else if(m_state == AUTH_DONE)
    {
        Close();
        if(m_pSink)
            m_pSink->OnAuthenicateHeader(m_sAccessToken);
    }
    return CM_OK;
}

void CWX2Auth::OnReceive(CmResult nResult, CCmMessageBlock *aData)
{
    CM_ASSERTE_RETURN_VOID(aData);
    int nDataLen = aData->GetChainedLength();
    char *szData = new char[nDataLen + 1];
    szData[nDataLen] = 0;
    aData->Read(szData, nDataLen);

	CM_INFO_TRACE_THIS("HTTPContent, auth, received=" << szData);
    Json::Reader r;
    Json::Value root;
    r.parse(szData, root, true);

    if(m_state == AUTH_GET_BEARER_TOKEN)
    {
        m_oBearerToken = root["BearerToken"].asString();
        m_state = AUTH_GET_ACCESS_TOKEN;
        CCmThreadManager::Instance()->GetThread(TT_MAIN)->GetEventQueue()->PostEvent(this);
    }
    else if(m_state == AUTH_GET_ACCESS_TOKEN)
    {
        m_state = AUTH_DONE;
        m_sAccessToken = "Bearer " + root["access_token"].asString();
        CCmThreadManager::Instance()->GetThread(TT_MAIN)->GetEventQueue()->PostEvent(this);
    }
    delete [] szData;
}
