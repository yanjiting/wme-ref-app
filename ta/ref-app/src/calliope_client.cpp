#include <string>
#include <iomanip>
#include <sstream>
#include "CmHttpUrl.h"

using namespace std;
#include "calliope_client.h"
#include "json/json.h"
#include "MediaConnection.h"
using namespace wme;
#include "testconfig.h"

const string ORPHEUS_URL = "https://calliope-integration.wbx2.com:443/";//
string newUUID();

COrpheusClient::COrpheusClient(IOrpheusClientSink *pSink, const char *szTrackingID)
    :m_pHttpClient(NULL), m_pSink(pSink), m_bDeleteVenue(false)
{
    m_auth = new CWX2Auth(this);
    m_state = ORPHEUS_INIT;
    if(szTrackingID == NULL) {
        m_sTrackId = "WMETEST_" + newUUID() + "_123";
    }else{
        m_sTrackId = szTrackingID;
    }
    CM_INFO_TRACE_THIS("Client TrackingId:" << m_sTrackId);
}

COrpheusClient::~COrpheusClient()
{
    if(m_auth)
    {
        delete m_auth;
        m_auth = NULL;
    }
    Close();
}

void COrpheusClient::Close()
{
    if(m_pHttpClient != NULL)
    {
        m_pHttpClient->Close();
        tp_destroy_httpclient(m_pHttpClient);
        m_pHttpClient = NULL;
    }
    if (m_auth != NULL)
        m_auth->Close();
}

void COrpheusClient::DoTask()
{
    Close();

    std::string sOrpheusUrl = TestConfig::i().m_sOrpheusUrl.empty() ? ORPHEUS_URL : TestConfig::i().m_sOrpheusUrl;
	bool bDelete = false;
    string sSend;
    Json::Value root(Json::objectValue);
    BOOL bVerify = FALSE;
    if(ORPHEUS_CREATE_VENUE == m_state)
    {
        string url = sOrpheusUrl + "calliope/api/v1/venues";
        tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_CREATE_VENUE url=" << url);
        m_pHttpClient->Open("post", url.c_str(), this, 1);
        m_pHttpClient->SetRequestHeader("Content-Type", "application/json");
        m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());

        root["event_url"] = Json::Value("127.0.0.1/wme_calliope_test");
        Json::Value &features = root["features"] = Json::Value(Json::objectValue);
        features["agent_version"] = Json::Value("v2");
        features["max_size"] = Json::Value(2);
        features["test_name"] = Json::Value("wme_calliope_test");
    }
    else if(ORPHEUS_CREATE_FLOW == m_state)
    {
        CCmUri uri;
        CCmString sVenueID;
        if (uri.Parse(m_sVenueUrl)) {
            CCmString::size_type pos = uri.m_sPath.rfind('/');
            if (pos != CCmString::npos) {
                sVenueID = uri.m_sPath.substr(pos + 1);
            } else {
                sVenueID = uri.m_sPath;
            }
        }
        string url = sOrpheusUrl + "calliope/api/v1/venues/" + sVenueID + "/confluences";
        tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_CREATE_FLOW url=" << url);
        m_pHttpClient->Open("post", url.c_str(), this, 1);
        m_pHttpClient->SetRequestHeader("Content-Type", "application/json");
        m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());

        root["venueUrl"] = Json::Value(m_sVenueUrl);
        root["sdpOffer"] = Json::Value(m_sOffer);
        root["correlationId"] = Json::Value(m_sUuid);
		root["floorControlWebhook"] = std::string("http://10.224.203.40/");
    }
    else if(ORPHEUS_CREATE_DELETE_VENUE == m_state)
    {
		bDelete = true;
        tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        if (m_sVenueUrl.find_first_of("http") != m_sVenueUrl.npos)
            m_pHttpClient->Open("delete", m_sVenueUrl.c_str(), this, 1);
        else {
            CM_INFO_TRACE_THIS("COrpheusClient,calliop, venue is not a URL, venue is:" << m_sVenueUrl);
            return;
        }
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_CREATE_DELETE_VENUE m_sVenueUrl="
            << m_sVenueUrl << ", m_pHttpClient=" << m_pHttpClient);
        m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());
    }
    else if(ORPHEUS_DELETE_CONFLUENCE == m_state)
    {
		bDelete = true;
		tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        m_pHttpClient->Open("delete", m_sUrlForDelete.c_str(), this, 1);
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_DELETE_CONFLUENCE m_sUrlForDelete=" 
            << m_sUrlForDelete << ", m_pHttpClient=" << m_pHttpClient);
        if(!m_sAuth.empty())
            m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());
    }
    else if(ORPHEUS_CREATE_LINUS_CONFLUENCE == m_state)
    {
        string url = m_sLinusAddr + "calliope/api/v1/confluences";
        tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        m_pHttpClient->Open("post", url.c_str(), this, 1);
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_CREATE_LINUS_CONFLUENCE url=" 
            << url << ", m_pHttpClient=" << m_pHttpClient);
        m_pHttpClient->SetRequestHeader("Content-Type", "application/json");
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());
        if(!TestConfig::i().m_sAuthHeader.empty() && TestConfig::i().m_sAuthHeader != "auto")
            m_sAuth = TestConfig::i().m_sAuthHeader;
        if(!m_sAuth.empty())
            m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());

        if(m_sVenueUrl.find("http") != m_sVenueUrl.npos)
            root["venueUrl"] = Json::Value(m_sVenueUrl);
        else
            root["venueUrl"] = Json::Value(string("http://localhost/calliope/api/v1/venues/") + m_sVenueUrl);
        root["venue"] = Json::Value(m_sVenueUrl);
        root["sdp"] = Json::Value(m_sOffer);
        root["correlationId"] = Json::Value(m_sUuid);
        Json::Value &opt = root["confluenceOptions"] = Json::Value(Json::objectValue);
        opt["metrixPostfix"] = Json::Value("test");
		opt["floorControlWebHook"] = std::string("test");
        
        Json::Value &feature_toggles = opt["featureToggles"] = Json::Value(Json::arrayValue);
        Json::Value calliope_media_balance_cluster = Json::Value(Json::objectValue);
        calliope_media_balance_cluster["key"] = "calliope-media-balance-cluster";
        calliope_media_balance_cluster["val"] = (int)30;
        feature_toggles.append(calliope_media_balance_cluster);
        
        Json::Value &trans = root["venueOptions"] = Json::Value(Json::objectValue);
        trans["forceVideoTranscode"] = Json::Value(false);

        if (!TestConfig::i().m_bgSdpOffer.empty()) {

            root["sdps"] = Json::Value(Json::arrayValue);
            root["sdps"].append(Json::Value(m_sOffer));
            root["sdps"].append(Json::Value(TestConfig::i().m_bgSdpOffer));
        }
	}
	else if (ORPHEUS_REQUEST_FLOOR == m_state){
		tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        m_pHttpClient->Open("put", m_sFloorRequestUrl.c_str(), this, 1);
		CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_REQUEST_FLOOR url=" << m_sFloorRequestUrl);
		m_pHttpClient->SetRequestHeader("Content-Type", "application/json");
		m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());
		if (!m_sAuth.empty())
			m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
	}
    else if (ORPHEUS_RELEASE_FLOOR == m_state){
        tp_create_httpclient(&m_pHttpClient);
        m_pHttpClient->SetOption(CM_OPT_TLS_VERIFY_PEER_CERTIFICATE, &bVerify);
        m_pHttpClient->Open("put", m_sFloorReleaseUrl.c_str(), this, 1);
        CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint],httpclient ORPHEUS_RELEASE_FLOOR url=" << m_sFloorRequestUrl);
        m_pHttpClient->SetRequestHeader("Content-Type", "application/json");
        m_pHttpClient->SetRequestHeader("TrackingID", m_sTrackId.c_str());
        if (!m_sAuth.empty())
            m_pHttpClient->SetRequestHeader("Authorization", m_sAuth.c_str());
    }

    Json::FastWriter w;
	sSend = w.write(root);

	CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint], m_state=" << m_state << ", send=" << sSend);
	CCmMessageBlock mb(sSend.length() + 1);
	if (!bDelete)
		mb.Write(sSend.c_str(), sSend.length());

    CM_INFO_TRACE_THIS("debug=="<<sSend.c_str());
    
    m_pHttpClient->Send(&mb);
}

void COrpheusClient::CreateLinusConfluence(string linusIp, 
                                           string venueUrl, 
                                           string offer, 
                                           string uuid)
{
    m_sLinusAddr = linusIp;
    m_sVenueUrl = venueUrl;
    m_sOffer = offer;
    m_sUuid = uuid;
    m_state = ORPHEUS_CREATE_LINUS_CONFLUENCE;
    if(!TestConfig::i().m_sAuthHeader.empty() && TestConfig::i().m_sAuthHeader == "auto") {
        m_auth->GetAuthenticationHeader();
    }
    else {
        DoTask();
    }
}

void COrpheusClient::DeleteConfluence(bool deleteVenue, string url)
{
    m_state = ORPHEUS_DELETE_CONFLUENCE;
	if (!url.empty())
		m_sUrlForDelete = url;
	else
		m_sUrlForDelete = m_sConfluenceUrl;
	
	if(!m_sUrlForDelete.empty())
		DoTask();

    m_sConfluenceUrl.clear();
    m_bDeleteVenue = deleteVenue;
}

void COrpheusClient::OnAuthenicateHeader(const string &headers)
{
    m_sAuth = headers;
    DoTask();
}

void COrpheusClient::DeleteVenue()
{
    m_state = ORPHEUS_CREATE_DELETE_VENUE;
    DoTask();
}

void COrpheusClient::RequestFloor()
{
	m_state = ORPHEUS_REQUEST_FLOOR;
	DoTask();
}

void COrpheusClient::ReleaseFloor()
{
    m_state = ORPHEUS_RELEASE_FLOOR;
    DoTask();
}

void COrpheusClient::CreateFlow(string venueUrl, string offer, string uuid)
{
    if(!venueUrl.empty())
        m_sVenueUrl = venueUrl;

    m_sUuid = uuid;
    m_sOffer = offer;
    m_state = ORPHEUS_CREATE_FLOW;
    if(m_sAuth.empty())
        m_auth->GetAuthenticationHeader();
    else
        DoTask();
}

CmResult COrpheusClient::OnEventFire()
{
    Close();
    if(m_pSink){
        int state = m_state;
        m_state = ORPHEUS_DONE;
        if(ORPHEUS_CREATE_VENUE == state)
        {
            m_pSink->OnVenue(m_sVenueUrl, this);
        }else if(ORPHEUS_CREATE_FLOW == state || 
                 ORPHEUS_CREATE_CONFLUENCE == state || 
                 ORPHEUS_CREATE_LINUS_CONFLUENCE == state)
        {
			CM_INFO_TRACE_THIS("COrpheusClient::OnEventFire, remoteSDP=" << m_sRemoteSDP);
            m_pSink->OnFlowParameters(m_sRemoteSDP, m_sConfluenceUrl, this);
        }
    }
    return CM_OK;
}

void COrpheusClient::OnReceive(CmResult nResult, CCmMessageBlock *aData)
{
    if(aData == NULL)
        return;

    int nDataLen = aData->GetChainedLength();
    char *szData = new char[nDataLen + 1];
    szData[nDataLen] = 0;
    aData->Read(szData, nDataLen);
	CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint], received=" << szData);

    Json::Reader r;
    Json::Value root;
    r.parse(szData, root, true);

    switch(m_state){
        case ORPHEUS_CREATE_VENUE:
        {
            m_sVenueUrl = root["url"].asString();
            m_sVenueUuid = root["uuid"].asString();
            break;
        }
        case ORPHEUS_CREATE_FLOW:
        case ORPHEUS_CREATE_CONFLUENCE:
        {
            m_sRemoteSDP = root["sdpAnswer"].asString();
			CM_ASSERTE(!m_sRemoteSDP.empty());
            m_sConfluenceUrl = root["url"].asString();
			m_sFloorRequestUrl = root["floorRequestUrl"].asString();
            m_sFloorReleaseUrl = root["floorReleaseUrl"].asString();
            break;
        }
        case ORPHEUS_CREATE_LINUS_CONFLUENCE:
        {
            m_sRemoteSDP = root["sdp"].asString();
			CM_ASSERTE(!m_sRemoteSDP.empty());
			m_sConfluenceUrl = root["url"].asString();
            m_sFloorRequestUrl = root["actionsFloorRequest"].asString();
            m_sFloorReleaseUrl = root["actionsFloorRelease"].asString();

            if (root.isMember("sdps") && root["sdps"].size() >=2) {
                m_sRemoteSDP = root["sdps"][0].asString();
                TestConfig::i().m_bgSdpAnwser = root["sdps"][1].asString();
            }
			CM_INFO_TRACE_THIS("COrpheusClient,calliop,[CheckPoint], FloorRequestUrl=" << m_sFloorRequestUrl);
			break;
        }
		case ORPHEUS_REQUEST_FLOOR:
		{
            //Linus's reponse don't has content,just Orepheus's reponse has it.
            //"{\"statusCode\":202,\"reasonPhrase\":\"ACCEPTED\"}",
            //if(!(root["reasonPhrase"].asString()=="ACCEPTED")){
            //    CM_ERROR_TRACE_THIS("COrpheusClient,calliop,ORPHEUS_REQUEST_FLOOR FAIL,[CheckPoint]");
            //}
            CM_INFO_TRACE_THIS("COrpheusClient,calliop,ORPHEUS_REQUEST_FLOOR SUCESS,[CheckPoint]");
			break;
		}
        case ORPHEUS_RELEASE_FLOOR:
        {
            CM_INFO_TRACE_THIS("COrpheusClient,calliop,ORPHEUS_RELEASE_FLOOR SUCESS,[CheckPoint]");
            break;
        }
        case ORPHEUS_DELETE_CONFLUENCE:
        {
            if(m_bDeleteVenue)
            {
                m_bDeleteVenue = false;
                DeleteVenue();
            }
        }
        default:
            break;
    }
    CCmThreadManager::Instance()->GetThread(TT_MAIN)->GetEventQueue()->PostEvent(this);
    delete [] szData;
}

void COrpheusClient::CreateVenue()
{
    m_state = ORPHEUS_CREATE_VENUE;
    m_auth->GetAuthenticationHeader();
}
