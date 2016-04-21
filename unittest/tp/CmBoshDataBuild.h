
#ifndef __CM_BOSH_DATA_BUILD_H__
#define __CM_BOSH_DATA_BUILD_H__

#include <string>
#include "CmDataType.h"


using namespace std;

class CBoshDataBuild
{
public:
    //POST /webclient HTTP/1.1
    //Host: httpcm.example.com
    //Accept-Encoding: gzip, deflate
    //Content-Type: text/xml; charset=utf-8
    //Content-Length: 104
    // 
    //<body content='text/xml; charset=utf-8'
    //      from='user@example.com'
    //      hold='1'
    //      rid='1573741820'
    //      to='example.com'
    //      route='xmpp:example.com:9999'
    //      ver='1.6'
    //      wait='60'
    //      ack='1'
    //      xml:lang='en'
    //      xmlns='http://jabber.org/protocol/httpbind'/>
    static string XmlSessionCreateRequest();

    //POST /webclient HTTP/1.1
    //Host: httpcm.example.com
    //Accept-Encoding: gzip, deflate
    //Content-Type: text/xml; charset=utf-8
    //Content-Length: 188
    // 
    //<body rid='1249243562'
    //      sid='SomeSID'
    //      xmlns='http://jabber.org/protocol/httpbind'>
    //  <message to='contact@example.com'
    //           xmlns='jabber:client'>
    //    <body>Good morning!</body>
    //  </message>
    //  <message to='friend@example.com'
    //           xmlns='jabber:client'>
    //    <body>Hey, what&apos;s up?</body>
    //  </message>
    //</body>
    static string XmlPostDataRequest(
        const string& strSid);

    //POST /webclient HTTP/1.1
    //Host: httpcm.example.com
    //Accept-Encoding: gzip, deflate
    //Content-Type: text/xml; charset=utf-8
    //Content-Length: 88
    // 
    //<body rid='1249243563'
    //      sid='SomeSID'
    //      xmlns='http://jabber.org/protocol/httpbind'/>
    static string XmlRetrieveDataRequest(
        const string& strSid);

    //POST /webclient HTTP/1.1
    //Host: httpcm.example.com
    //Accept-Encoding: gzip, deflate
    //Content-Type: text/xml; charset=utf-8
    //Content-Length: 153
    // 
    //<body rid='1249243565'
    //      sid='SomeSID'
    //      type='terminate'
    //      xmlns='http://jabber.org/protocol/httpbind'>
    //  <presence type='unavailable'
    //            xmlns='jabber:client'/>
    //</body>
    static string XmlTerminateDataRequest(
        const string& strSid);

    static string XmlPauseDataRequest(
        const string& strSid);

    static string XmlInvalidBoshData();

    static string XmlRestartRequest(
        const string& strSid);

    static INT64 InitializeRid();

    static void RidIncreaseOne();

    static void RidDecreaseOne();

protected:
    static INT64            s_i64Rid;
    static string           s_strRid;
};

#endif