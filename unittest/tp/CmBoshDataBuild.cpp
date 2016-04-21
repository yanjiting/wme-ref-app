
#include "tinyxml.h"

#include "CmBase.h"
#include "CmBoshConstStringDefine.h"
#include "CmBoshBody.h"
#include "CmBoshDataBuild.h"


INT64 CBoshDataBuild::s_i64Rid = 0;
string CBoshDataBuild::s_strRid = "0";

string CBoshDataBuild::XmlSessionCreateRequest()
{
    CBoshDataBuild::InitializeRid();

    TiXmlElement xmlEle(CSZ_ELEM_BODY);
    xmlEle.SetAttribute(CSZ_ATTR_CONTENT, "text/xml; charset=utf-8");
    xmlEle.SetAttribute(CSZ_ATTR_FROM, "user@example.com");
    xmlEle.SetAttribute(CSZ_ATTR_HOLD, "1");
    xmlEle.SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    xmlEle.SetAttribute(CSZ_ATTR_TO, "example.com");
    xmlEle.SetAttribute(CSZ_ATTR_ROUTE, "xmpp:example.com:9999");
    xmlEle.SetAttribute(CSZ_ATTR_VER, "1.6");
    xmlEle.SetAttribute(CSZ_ATTR_WAIT, "20");
    xmlEle.SetAttribute(CSZ_ATTR_ACK, "1");
    xmlEle.SetAttribute(CSZ_ATTR_LANG, "en");
    xmlEle.SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlPrinter xmlPrinter;
    xmlEle.Accept(&xmlPrinter);
    string strXml = xmlPrinter.CStr();

    return strXml;
}

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
string CBoshDataBuild::XmlPostDataRequest(
    const string& strSid)
{
    CBoshDataBuild::RidIncreaseOne();

    TiXmlElement* pRootElem = new TiXmlElement(CSZ_ELEM_BODY);
    pRootElem->SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_SID, strSid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlElement* pXmlChild1 = new TiXmlElement("message");
    pXmlChild1->SetAttribute("to", "contact@example.com");
    pXmlChild1->SetAttribute("xmlns", "jabber:client");

    TiXmlElement* pXmlChild1Sub = new TiXmlElement("body");
    TiXmlText* pXmlText1 = new TiXmlText("Good morning!");
    pXmlChild1Sub->LinkEndChild(pXmlText1);
    pXmlChild1->LinkEndChild(pXmlChild1Sub);

    TiXmlElement* pXmlChild2 = new TiXmlElement("message");
    pXmlChild2->SetAttribute("to", "friend@example.com");
    pXmlChild2->SetAttribute("xmlns", "jabber:client");

    TiXmlElement* pXmlChild2Sub = new TiXmlElement("body");
    TiXmlText* pXmlText2 = new TiXmlText("Hey, what&apos;s up?");
    pXmlChild2Sub->LinkEndChild(pXmlText2);
    pXmlChild2->LinkEndChild(pXmlChild2Sub);

    pRootElem->LinkEndChild(pXmlChild1);
    pRootElem->LinkEndChild(pXmlChild2);

    TiXmlPrinter xmlPrinter;
    pRootElem->Accept(&xmlPrinter);
    string strXml(xmlPrinter.CStr());

    delete pRootElem;
    return strXml;
}

//<body rid='1249243563'
//      sid='SomeSID'
//      xmlns='http://jabber.org/protocol/httpbind'/>
string CBoshDataBuild::XmlRetrieveDataRequest(
    const string& strSid)
{
    CBoshDataBuild::RidIncreaseOne();

    TiXmlElement* pRootElem = new TiXmlElement(CSZ_ELEM_BODY);
    pRootElem->SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_SID, strSid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlPrinter xmlPrinter;
    pRootElem->Accept(&xmlPrinter);
    string strXml(xmlPrinter.CStr());

    delete pRootElem;
    return strXml;
}

//<body rid='1249243565'
//sid='SomeSID'
//type='terminate'
//xmlns='http://jabber.org/protocol/httpbind'>
//<presence type='unavailable'
//xmlns='jabber:client'/>
//</body>
string CBoshDataBuild::XmlTerminateDataRequest(
    const string& strSid)
{
    CBoshDataBuild::RidIncreaseOne();

    TiXmlElement* pRootElem = new TiXmlElement(CSZ_ELEM_BODY);
    pRootElem->SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_SID, strSid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_TYPE, "terminate");
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlElement* pXmlChild = new TiXmlElement("presence");
    pXmlChild->SetAttribute(CSZ_ATTR_TYPE, "unavailable");
    pXmlChild->SetAttribute(CSZ_ATTR_XMLNS, "jabber:client");

    pRootElem->LinkEndChild(pXmlChild);

    TiXmlPrinter xmlPrinter;
    pRootElem->Accept(&xmlPrinter);
    string strXml(xmlPrinter.CStr());

    delete pRootElem;
    return strXml;
}

string CBoshDataBuild::XmlPauseDataRequest(
    const string& strSid)
{
    CBoshDataBuild::RidIncreaseOne();

    TiXmlElement* pRootElem = new TiXmlElement(CSZ_ELEM_BODY);
    pRootElem->SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_SID, strSid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_TYPE, "pause");
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlPrinter xmlPrinter;
    pRootElem->Accept(&xmlPrinter);
    string strXml(xmlPrinter.CStr());

    delete pRootElem;
    return strXml;
}

string CBoshDataBuild::XmlInvalidBoshData()
{
    TiXmlElement xmlEle("testdata");
    xmlEle.SetAttribute(CSZ_ATTR_RID, "1249243562");
    xmlEle.SetAttribute(CSZ_ATTR_SID, "SomeSID");
    xmlEle.SetAttribute(CSZ_ATTR_XMLNS, "http://jabber.org/protocol/httpbind");

    TiXmlPrinter xmlPrinter;
    xmlEle.Accept(&xmlPrinter);
    string strXml = xmlPrinter.CStr();

    return strXml;
}

//<body rid='1573741824'
//      sid='SomeSID'
//      to='example.com'
//      xml:lang='en'
//      xmpp:restart='true'
//      xmlns='http://jabber.org/protocol/httpbind'
//      xmlns:xmpp='urn:xmpp:xbosh'/>

//<body xmlns="http://jabber.org/protocol/httpbind" 
//sid="rnjixjaQ05rXns91ACiT64==" 
//xml:lang="zh-CN" 
//xmlns:xml="http://www.w3.org/XML/1998/namespace" 
//xmlns:xmpp="urn:xmpp:xbosh" 
//xmpp:restart="true" 
//to="whfdevwcs173-ms-vip.qa.webex.com" 
//rid="2656116092"/>

string CBoshDataBuild::XmlRestartRequest(
    const string& strSid)
{
    CBoshDataBuild::RidIncreaseOne();

    TiXmlElement* pRootElem = new TiXmlElement(CSZ_ELEM_BODY);
    pRootElem->SetAttribute(CSZ_ATTR_RID, s_strRid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_SID, strSid.c_str());
    pRootElem->SetAttribute(CSZ_ATTR_LANG, "zh-CN");
    pRootElem->SetAttribute(CSZ_ATTR_TO, "whfdevwcs173-ms-vip.qa.webex.com");
    pRootElem->SetAttribute("xmlns:xml", "http://www.w3.org/XML/1998/namespace");
    pRootElem->SetAttribute(CSZ_ATTR_XMPP_RESTART, CSZ_TRUE);
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS, CSZ_XMLNS_HTTPBIND);
    pRootElem->SetAttribute(CSZ_ATTR_XMLNS_XMPP, CSZ_XMPP_XBOSH);

    TiXmlPrinter xmlPrinter;
    pRootElem->Accept(&xmlPrinter);
    string strXml(xmlPrinter.CStr());

    delete pRootElem;
    return strXml;
}


INT64 CBoshDataBuild::InitializeRid()
{
    srand((unsigned int)time(NULL));
    s_i64Rid = rand();
    INT64ToString(s_i64Rid, s_strRid);
    
    return s_i64Rid;
}

void CBoshDataBuild::RidIncreaseOne()
{
    s_i64Rid++;
    INT64ToString(s_i64Rid, s_strRid);
}

void CBoshDataBuild::RidDecreaseOne()
{
    s_i64Rid--;
    INT64ToString(s_i64Rid, s_strRid);
}
