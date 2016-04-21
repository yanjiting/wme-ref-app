#include "wsclient.h"
#include "json/json.h"

CWsClient::CWsClient(ISignalHook *pSink)
    :m_pSink(pSink),
     m_nMyId(-1),
     m_bStartedCall(false)
{
}

void CWsClient::Open(const char *url)
{
    m_sUrl.Init(url);
    CmResult rv = CCmChannelManager::Instance()->CreateChannelWSClient(
            m_wsChannel.ParaOut(),
            &m_sUrl,
            CCmChannelManager::CFLAG_WITH_BROWER_PROXY);

    CM_ASSERTE(CM_SUCCEEDED(rv));
    CM_ASSERTE(m_wsChannel);
    rv = m_wsChannel->AsyncOpen(this);
}

void CWsClient::Stop()
{
	m_timerKeepAlive.Cancel();
    if(m_wsChannel)
        m_wsChannel->Disconnect(CM_OK);
    m_bStartedCall = false;
}

void CWsClient::OnConnect(CmResult aReason, ICmChannel *aChannelId, long id)
{
     if(m_pSink && CM_SUCCEEDED(aReason))
    {
        m_timerKeepAlive.Schedule(this, CCmTimeValue(5l));
        m_pSink->OnConnected();
    }
}

void CWsClient::OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    int nDataLen = aData.GetChainedLength();
    char *szData = new char[nDataLen + 1];
    szData[nDataLen] = 0;
    aData.Read(szData, nDataLen);

    Json::Reader r;
    Json::Value root;
    r.parse(szData, root, true);
    string sType = root["type"].asString();
    if(sType.compare("join") == 0){
        int nCount = root["count"].asInt();
        CM_INFO_TRACE_THIS("Endpoint, stype:join, count=" << nCount);

        m_nMyId = nCount;
        if(m_pSink && !m_bStartedCall)
        {
            m_pSink->OnStartCall(nCount);
            m_bStartedCall = true;
        }
    }
    else if(sType == "echo"){
        m_nMyId = root["count"].asInt();
        return;
    }
    else{
        CM_INFO_TRACE_THIS("Endpoint, stype:" << root["type"].asString());
        m_pSink->OnSDPReceived(root["type"].asString(), root["sdp"].asString());
    }
    delete [] szData;
}

void CWsClient::SendTo(string type, string text, int index)
{
    Json::Value root(Json::objectValue);
    root["type"] = Json::Value(type);
    root["sdp"] = Json::Value(text);
    Json::FastWriter w;
    string sTxt = w.write(root);

    CCmMessageBlock mb(sTxt.length() + 1);
    mb.Write(sTxt.c_str(), sTxt.length());
    m_wsChannel->SendData(mb);
}

void CWsClient::OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
}

void CWsClient::OnDisconnect(CmResult aReason, ICmTransport *aTrptId)
{
}

void CWsClient::OnTimer(CCmTimerWrapperID* aId)
{
    Json::Value root(Json::objectValue);
    root["type"] = Json::Value("echo");
    Json::FastWriter w;
    string sTxt = w.write(root);

    CCmMessageBlock mb(sTxt.length() + 1);
    mb.Write(sTxt.c_str(), sTxt.length());
    m_wsChannel->SendData(mb);
}
