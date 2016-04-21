
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expat.h>
#include "tinyxml.h"

#include "CmBoshDataBuild.h"
#include "CmBoshBody.h"


class CmBoshBodyTest : public testing::Test
{
public:
    CmBoshBodyTest()
    {
        m_pBoshRequest = NULL;
    }

    virtual ~CmBoshBodyTest()
    {

    }

    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
    }

protected:
    CBoshRequest*           m_pBoshRequest;
};

TEST_F(CmBoshBodyTest, StringToINT64TestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);
    
    string str0 = "0"; //0
    string strP1 = "1" ; //1
    string strP1000 = "1000";//1000
    string strP7F_14 = "9223372036854775807";//9223372036854775807
    string strN1 = "-1";//-1
    string strN1000 = "-1000";//-1000
    string strNFF_13_E = "-9223372036854775806";//-9223372036854775806

    INT64 i64Num = 0;

    EXPECT_TRUE(CM_OK == StringToINT64(str0, i64Num));
    EXPECT_EQ(0, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strP1, i64Num));
    EXPECT_EQ(1, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strP1000, i64Num));
    EXPECT_EQ(1000, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strP7F_14, i64Num));
    EXPECT_EQ(9223372036854775807, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strN1, i64Num));
    EXPECT_EQ(-1, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strN1000, i64Num));
    EXPECT_EQ(-1000, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strNFF_13_E, i64Num));
    EXPECT_EQ(-9223372036854775806, i64Num);
}

TEST_F(CmBoshBodyTest, StringToINT64TestInvalidInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    string strAllChar = "abcd";
    string strCharBegin = "a12345";
    string strCharEnd = "12345a";
    string strNumMid = "a123c";
    string strCharMid = "56a89";
    string strZeroBegin = "06789";

    INT64 i64Num = 0;

    EXPECT_TRUE(CM_OK != StringToINT64(strAllChar, i64Num));
    EXPECT_TRUE(CM_OK != StringToINT64(strCharBegin, i64Num));

    EXPECT_TRUE(CM_OK == StringToINT64(strCharEnd, i64Num));
    EXPECT_EQ(12345, i64Num);

    EXPECT_TRUE(CM_OK != StringToINT64(strNumMid, i64Num));
    
    EXPECT_TRUE(CM_OK == StringToINT64(strCharMid, i64Num));
    EXPECT_EQ(56, i64Num);

    EXPECT_TRUE(CM_OK == StringToINT64(strZeroBegin, i64Num));
    EXPECT_EQ(6789, i64Num);
}

TEST_F(CmBoshBodyTest, INT64ToStringTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    INT64 i64P0 = 0;//"0"
    INT64 i64P1000 = 1000;//"1000"
    INT64 i64P7F_14 = 9223372036854775807;//"9223372036854775807"
    INT64 i64N1 = -1;//"-1"
    INT64 i64N1000 = -1000;//"-1000"
    INT64 i64NFF_13_E = -9223372036854775806;//"-9223372036854775806"

    string strOut("");

    EXPECT_EQ(CM_OK, INT64ToString(i64P0, strOut));
    EXPECT_STREQ("0", strOut.c_str());

    EXPECT_EQ(CM_OK, INT64ToString(i64P1000, strOut));
    EXPECT_STREQ("1000", strOut.c_str());

    EXPECT_EQ(CM_OK, INT64ToString(i64P7F_14, strOut));
    EXPECT_STREQ("9223372036854775807", strOut.c_str());

    EXPECT_EQ(CM_OK, INT64ToString(i64N1000, strOut));
    EXPECT_STREQ("-1000", strOut.c_str());

    EXPECT_EQ(CM_OK, INT64ToString(i64NFF_13_E, strOut));
    EXPECT_STREQ("-9223372036854775806", strOut.c_str());
}

TEST_F(CmBoshBodyTest, StringToUShortTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    unsigned short usOut = 0;
    string strP0 = "0";//0
    string strP100 = "100";//100
    string strP65534 = "65534";//65534
    string strP65535 = "65535";//65535

    EXPECT_TRUE(CM_OK == StringToUShort(strP0, usOut));
    EXPECT_EQ(0, usOut);

    EXPECT_TRUE(CM_OK == StringToUShort(strP100, usOut));
    EXPECT_EQ(100, usOut);

    EXPECT_TRUE(CM_OK == StringToUShort(strP65534, usOut));
    EXPECT_EQ(65534, usOut);

    EXPECT_TRUE(CM_OK == StringToUShort(strP65535, usOut));
    EXPECT_EQ(65535, usOut);
}

TEST_F(CmBoshBodyTest, StringToUShortTestInvalidInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    unsigned short usOut = 0;

    string strAllChar = "abcd";
    string strCharBegin = "a123";
    string strCharEnd = "198a";
    string strCharMid = "1a34";
    string strNumMid = "a10b";
    string strZeroBegin = "05678";

    EXPECT_TRUE(CM_OK != StringToUShort(strAllChar, usOut));
    EXPECT_TRUE(CM_OK != StringToUShort(strCharBegin, usOut));

    EXPECT_TRUE(CM_OK == StringToUShort(strCharEnd, usOut));
    EXPECT_EQ(198, usOut);

    EXPECT_TRUE(CM_OK == StringToUShort(strCharMid, usOut));
    EXPECT_EQ(1, usOut);

    EXPECT_TRUE(CM_OK != StringToUShort(strNumMid, usOut));

    EXPECT_TRUE(CM_OK == StringToUShort(strZeroBegin, usOut));
    EXPECT_EQ(5678, usOut);
}

TEST_F(CmBoshBodyTest, UShortToStringTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    string strOut("");

    unsigned short usP0 = 0;//"0"
    unsigned short usP100 = 100;//"100"
    unsigned short usP65534 = 65534;//"65534"
    unsigned short usP65535 = 65535;//"65535"

    EXPECT_TRUE(CM_OK == UShortToString(usP0, strOut));
    EXPECT_STREQ("0", strOut.c_str());

    EXPECT_TRUE(CM_OK == UShortToString(usP100, strOut));
    EXPECT_STREQ("100", strOut.c_str());

    EXPECT_TRUE(CM_OK == UShortToString(usP65534, strOut));
    EXPECT_STREQ("65534", strOut.c_str());

    EXPECT_TRUE(CM_OK == UShortToString(usP65535, strOut));
    EXPECT_STREQ("65535", strOut.c_str());
}

TEST_F(CmBoshBodyTest, StringToUCharTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    string strP0 = "0";//0
    string strP127 = "127";//127
    string strP254 = "254";//254
    string strP255 = "255";//255

    unsigned char ucOut = 0;

    EXPECT_TRUE(CM_OK == StringToUChar(strP0, ucOut));
    EXPECT_EQ(0, ucOut);

    EXPECT_TRUE(CM_OK == StringToUChar(strP127, ucOut));
    EXPECT_EQ(127, ucOut);

    EXPECT_TRUE(CM_OK == StringToUChar(strP254, ucOut));
    EXPECT_EQ(254, ucOut);

    EXPECT_TRUE(CM_OK == StringToUChar(strP255, ucOut));
    EXPECT_EQ(255, ucOut);
}

TEST_F(CmBoshBodyTest, StringToUCharTestInvalidInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    unsigned char ucOut = 0;

    string strAllChar = "abc";
    string strCharBegin = "a123";
    string strCharEnd = "123b";
    string strCharMid = "12T4";
    string strNumMid = "a12d";
    string strZeroBegin = "0129";

    EXPECT_TRUE(CM_OK != StringToUChar(strAllChar, ucOut));
    EXPECT_TRUE(CM_OK != StringToUChar(strCharBegin, ucOut));

    EXPECT_TRUE(CM_OK == StringToUChar(strCharEnd, ucOut));
    EXPECT_EQ(123, ucOut);

    EXPECT_TRUE(CM_OK == StringToUChar(strCharMid, ucOut));
    EXPECT_EQ(12, ucOut);

    EXPECT_TRUE(CM_OK != StringToUChar(strNumMid, ucOut));

    EXPECT_TRUE(CM_OK == StringToUChar(strZeroBegin, ucOut));
    EXPECT_EQ(129, ucOut);
}

TEST_F(CmBoshBodyTest, UCharToStringTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    string strOut("");

    unsigned char ucP0 = 0;//"0"
    unsigned char ucP127 = 127;//"127"
    unsigned char ucP254 = 254;//"254"
    unsigned char ucP255 = 255;//"255"

    EXPECT_TRUE(CM_OK == UCharToString(ucP0, strOut));
    EXPECT_STREQ("0", strOut.c_str());

    EXPECT_TRUE(CM_OK == UCharToString(ucP127, strOut));
    EXPECT_STREQ("127", strOut.c_str());

    EXPECT_TRUE(CM_OK == UCharToString(ucP254, strOut));
    EXPECT_STREQ("254", strOut.c_str());

    EXPECT_TRUE(CM_OK == UCharToString(ucP255, strOut));
    EXPECT_STREQ("255", strOut.c_str());
}

TEST_F(CmBoshBodyTest, StrCompareNoCaseTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    string str0 = "abcdefghijklm";
    string str1 = "abcdefghijklm";
    string str2 = "ABCDefghijklm";
    string str3 = "abcdefghiJKLM";
    string str4 = "ABCDEFghiJKLM";
    string str5 = "abcdef";
    string str6 = "abcdefghijklm0";

    EXPECT_TRUE(0 == StrCompareNoCase(str0, str1));
    EXPECT_TRUE(0 == StrCompareNoCase(str0, str2));
    EXPECT_TRUE(0 == StrCompareNoCase(str0, str3));
    EXPECT_TRUE(0 == StrCompareNoCase(str0, str4));

    EXPECT_TRUE(0 < StrCompareNoCase(str0, str5));
    EXPECT_TRUE(0 > StrCompareNoCase(str0, str6));
}

TEST_F(CmBoshBodyTest,CBoshRequestTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest();

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(CM_OK == m_pBoshRequest->ParseXml(strXml));
    }
}

TEST_F(CmBoshBodyTest, ReferenceTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        m_pBoshRequest = new CBoshRequest();

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        DWORD dwRef1 = m_pBoshRequest->GetReference();

        int nTimes = 100;
        for(int i=0; i<nTimes; i++)
        {
            m_pBoshRequest->AddReference();
        }

        EXPECT_EQ(dwRef1 + nTimes, m_pBoshRequest->GetReference());

        for(int i=0; i<nTimes; i++)
        {
            m_pBoshRequest->ReleaseReference();
        }
        EXPECT_EQ(dwRef1, m_pBoshRequest->GetReference());
    }
}

TEST_F(CmBoshBodyTest, ParseXmlTestNormal)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        string strBody = m_pBoshRequest->GetBodyText();
        EXPECT_TRUE(strBody.empty());

        string strNonExist = m_pBoshRequest->GetAttribute("non_exist");
        EXPECT_TRUE(strNonExist.empty());

        strXml = CBoshDataBuild::XmlPostDataRequest("SomeSID");
        EXPECT_TRUE(CM_OK == m_pBoshRequest->ParseXml(strXml));

        strBody = m_pBoshRequest->GetBodyText();
        EXPECT_TRUE(!strBody.empty());
    }
}

TEST_F(CmBoshBodyTest, ParseXmlTestInvalidInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(m_pBoshRequest->IsSessionCreateRequest());

        strXml = "";
        EXPECT_TRUE(CM_OK != m_pBoshRequest->ParseXml(strXml));

        strXml = "asdfds";
        EXPECT_TRUE(CM_OK != m_pBoshRequest->ParseXml(strXml));

        strXml =  CBoshDataBuild::XmlInvalidBoshData();
        EXPECT_TRUE(CM_OK != m_pBoshRequest->ParseXml(strXml));

        strXml = CBoshDataBuild::XmlSessionCreateRequest() + CBoshDataBuild::XmlInvalidBoshData();
        EXPECT_TRUE(CM_OK != m_pBoshRequest->ParseXml(strXml));
    }
}

TEST_F(CmBoshBodyTest, GetAttributeTestErrorInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strNonExist = m_pBoshRequest->GetAttribute("");
        EXPECT_TRUE(strNonExist.empty());
    }
}

TEST_F(CmBoshBodyTest, IsSessionCreateRequestTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(m_pBoshRequest->IsSessionCreateRequest());

        strXml = CBoshDataBuild::XmlPostDataRequest("SomeSID");
        EXPECT_EQ(CM_OK, m_pBoshRequest->ParseXml(strXml));
        
        mapBodyAttr bodyAttr = m_pBoshRequest->GetAttribute();
        EXPECT_TRUE(0 != bodyAttr.size());
        
        EXPECT_TRUE(!m_pBoshRequest->IsSessionCreateRequest());
    }
}

TEST_F(CmBoshBodyTest, IsTerminateRequestTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(!m_pBoshRequest->IsTerminateRequest());

        strXml = CBoshDataBuild::XmlTerminateDataRequest("SomeSID");
        EXPECT_EQ(CM_OK, m_pBoshRequest->ParseXml(strXml));
        EXPECT_TRUE(m_pBoshRequest->IsTerminateRequest());
    }
}

TEST_F(CmBoshBodyTest, IsPauseRequestTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(!m_pBoshRequest->IsPauseRequest());
    }
}

TEST_F(CmBoshBodyTest, IsXMPPRestartTestNormalInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlRestartRequest("Sid123456");
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(m_pBoshRequest->IsXMPPRestart());
        EXPECT_TRUE(m_pBoshRequest->IsXMPPBosh());
    }
}

TEST_F(CmBoshBodyTest, IsXMPPRestartTestErrorInput)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        m_pBoshRequest = new CBoshRequest(strXml);

        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_TRUE(!m_pBoshRequest->IsXMPPRestart());
        EXPECT_TRUE(!m_pBoshRequest->IsXMPPBosh());
    }
}

TEST_F(CmBoshBodyTest, GetAttrCountTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        m_pBoshRequest = new CBoshRequest();
        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        EXPECT_EQ(0, m_pBoshRequest->GetAttrCount());
        string strXml = CBoshDataBuild::XmlSessionCreateRequest();
        EXPECT_EQ(CM_OK, m_pBoshRequest->ParseXml(strXml));
        EXPECT_EQ(11, m_pBoshRequest->GetAttrCount());
    }
}

TEST_F(CmBoshBodyTest, GetBodyTextTest)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        m_pBoshRequest = new CBoshRequest();
        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        string strXml ="<body xmlns=\"http://jabber.org/protocol/httpbind\" sid=\"VLt6QKBE9PSAK7J0aOJV+I==\" rid=\"2231363250\">\
            <presence id=\"cwxl13727573098520\" xmlns=\"jabber:client\" from=\"65976012_781@whitney.cisco.com/gmxzryxpqh\" to=\"65976012@conf.whitney.cisco.com/web_781_36807fa2-3559-4c4d-8c28-6ed9357e597d\">\
            <nick xmlns=\"http://jabber.org/protocol/nick\">No Name(495)</nick>\
            <usertag xmlns=\"http://protocols.cisco.com/usertag\">No Name(495)</usertag>\
            <c xmlns=\"http://jabber.org/protocol/caps\" hash=\"sha-1\" node=\"http://cisco.com/MMUCCAXL\" ver=\"LpsNlRBH/qn4e3q2+MfW9550uMA=\"></c>\
            </presence>\
            <iq id=\"cwxl13727573098551\" xmlns=\"jabber:client\" from=\"65976012_781@whitney.cisco.com/gmxzryxpqh\" to=\"65976012@conf.whitney.cisco.com\" type=\"set\">\
            <query xmlns=\"http://jabber.org/protocol/muc#owner\">\
            <x xmlns=\"jabber:x:data\" type=\"submit\">\
            <field var=\"FORM_TYPE\">\
            <value>http://cisco.com/protocol/webex#meetingconfig</value>\
        </field>\
            <field var=\"webex#meetingconfig_teleParameters\">\
            <value></value>\
            </field>\
            <field var=\"webex#meetingconfig_userData\">\
            <value>No Name(495)</value>\
            </field>\
            </x>\
            </query>\
            </iq>\
            </body>";

        EXPECT_EQ(CM_OK, m_pBoshRequest->ParseXml(strXml));
        
        vecString vecStr;
        m_pBoshRequest->GetBodyText(vecStr);
        vecString_IT itNext = vecStr.begin();
        while(vecStr.end() != itNext)
        {
            itNext++;
        }
    }
}

TEST_F(CmBoshBodyTest, GetBodyTextTest2)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        m_pBoshRequest = new CBoshRequest();
        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        string strXml ="<body xmlns=\"http://jabber.org/protocol/httpbind\" \
            sid=\"3EP2Rmw8VQAPEIkdfipK7X==\" rid=\"2743999650\">\
        <message xmlns=\"jabber:client\" from=\"121_690@whitney.cisco.com/ueobrreuiz\" \
        to=\"121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421\">\
            <sox xmlns=\"urn:xmpp:sox\">INVITE sip:121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421:5064 SIP/2.0 Via: SIP/2.0/UDP 192.1.2.3;rport=1078;branch=0 From: \
            <sip:121_690@whitney.cisco.com/ueobrreuiz> To: \
            <sip:121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421> \
            Call-ID: CwxlSoXSipCallID13739537843640 CSeq: 1 INVITE Content-Type: application/sdp Content-Length: \
            417 v=0 o=CMS 496886 497562 IN IP4 127.0.0.1 s=ciscowebex_cms_session c=IN IP4 127.0.0.1 t=0 0 m=audio \
            18276 RTP/AVP 102 13 98 99 a=sprop-source:1 csi=1746220032;simul=1 a=sprop-simul:1 \
            1 * a=recv-source 1,2,3 a=rtpmap:102 iLBC/8000 a=rtpmap:13 CN/8000 a=rtpmap:98 CN/16000 a=rtpmap:99 \
            CN/32000 a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level a=extmap:2 urn:ietf:params:rtp-hdrext:toffset\
            a=ptime:20</sox>\
            </message>\
            </body>";

        EXPECT_TRUE(CM_OK != m_pBoshRequest->ParseXml(strXml));

        vecString vecStr;
        m_pBoshRequest->GetBodyText(vecStr);
        vecString_IT itNext = vecStr.begin();
        while(vecStr.end() != itNext)
        {
            itNext++;
        }
    }
}

TEST_F(CmBoshBodyTest, GetBodyTextTest3)
{
    ASSERT_TRUE(NULL == m_pBoshRequest);

    {
        m_pBoshRequest = new CBoshRequest();
        ASSERT_TRUE(NULL != m_pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(m_pBoshRequest);

        string strMsg = "<message xmlns='jabber:client' from='121_690@whitney.cisco.com/ueobrreuiz' to='121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421'>";
        strMsg += "<sox xmlns='urn:xmpp:sox'>INVITE sip:121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421:5064 SIP/2.0\n\
                        Via: SIP/2.0/UDP 192.1.2.3;rport=1078;branch=0\n\
                        From: &lt;sip:121_690@whitney.cisco.com/ueobrreuiz&gt;\n\
                        To: &lt;sip:121@conf.whitney.cisco.com/web_690_30c822f0-736c-4b75-bb84-9398172c7421&gt;\n\
                        Call-ID: CwxlSoXSipCallID13739537843640\n\
                        CSeq: 1 INVITE\n\
                        Content-Type: application/sdp\n\
                        Content-Length: 417\n\
                        \n\
                        v=0\n\
                        o=CMS 496886 497562 IN IP4 127.0.0.1\n\
                        s=ciscowebex_cms_session\n\
                        c=IN IP4 127.0.0.1\n\
                        t=0 0\n\
                        m=audio 18276 RTP/AVP 102 13 98 99\n\
                        a=sprop-source:1 csi=1746220032;simul=1\n\
                        a=sprop-simul:1 1 *\n\
                        a=recv-source 1,2,3\n\
                        a=rtpmap:102 iLBC/8000\n\
                        a=rtpmap:13 CN/8000\n\
                        a=rtpmap:98 CN/16000\n\
                        a=rtpmap:99 CN/32000\n\
                        a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\n\
                        a=extmap:2 urn:ietf:params:rtp-hdrext:toffset\n\
                        a=ptime:20</sox></message>";

        string strXml ="<body xmlns=\"http://jabber.org/protocol/httpbind\" sid=\"3EP2Rmw8VQAPEIkdfipK7X==\" rid=\"2743999650\">" + strMsg;
        strXml += "</body>";

        EXPECT_EQ(CM_OK, m_pBoshRequest->ParseXml(strXml));

        string strMsg2 = m_pBoshRequest->GetBodyText();
        
        size_t szLen1 = strMsg.length();
        size_t szLen2 = strMsg2.length();

        EXPECT_EQ(szLen1, szLen2);
    }
}

//{{{---------------------------------------------------------------------
//CBoshBodyBuild
class CBoshBodyBuildTest : public testing::Test
{
public:
    CBoshBodyBuildTest()
    {
        m_pBodyBuild = NULL;
    }

    virtual ~CBoshBodyBuildTest()
    {

    }

    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
    }

protected:
    CBoshBodyBuild*           m_pBodyBuild;
};

TEST_F(CBoshBodyBuildTest, ReferenceTest)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        DWORD dwRef1 = m_pBodyBuild->GetReference();

        int nTimes = 100;
        for(int i=0; i<nTimes; i++)
        {
            m_pBodyBuild->AddReference();
        }

        EXPECT_EQ(dwRef1 + nTimes, m_pBodyBuild->GetReference());

        for(int i=0; i<nTimes; i++)
        {
            m_pBodyBuild->ReleaseReference();
        }
        EXPECT_EQ(dwRef1, m_pBodyBuild->GetReference());
    }
}

TEST_F(CBoshBodyBuildTest, AttributeTest)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        EXPECT_TRUE(CM_OK != m_pBodyBuild->DelAttribute("non exist"));

        EXPECT_TRUE(CM_OK == m_pBodyBuild->SetAttribute("key1", "value1"));
        EXPECT_TRUE(CM_OK == m_pBodyBuild->SetAttribute("key1", "value1_1"));
        EXPECT_TRUE(CM_OK == m_pBodyBuild->SetAttribute("key2", "value2"));
        EXPECT_TRUE(CM_OK == m_pBodyBuild->SetAttribute("key3", "value3"));

        EXPECT_TRUE(CM_OK == m_pBodyBuild->DelAttribute("key2"));

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());

        m_pBodyBuild->ClearAll();
    }
}

TEST_F(CBoshBodyBuildTest, SetBodyTextTest)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        m_pBodyBuild->SetBodyText("body text");

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());
    }
}

TEST_F(CBoshBodyBuildTest, SetBodyTextTest2)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        string strText = "<message from='904@conf.whitney.cisco.com' \
                         to='904_695@whitney.cisco.com/eklanrrfol' \
                         type='headline'><event xmlns='http://jabber.org/protocol/pubsub#event'>\
                         <items node='urn:ietf:params:xml:ns:conference-info'><item>\
                         <conference-info state='partial' xmlns='urn:ietf:params:xml:ns:conference-info'>\
                         <users><user entity='904@conf.whitney.cisco.com/web_695_11e8af59-51cc-436a-baff-fc6e6533bae9'\
                         state='partial'><roles><entry>http://protocols.cisco.com/roles#participant</entry>\
                         <entry>http://protocols.cisco.com/roles#presenter</entry></roles></user></users>\
                         </conference-info></item></items></event></message>" ;
        m_pBodyBuild->SetBodyText(strText);

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());
    }
}

TEST_F(CBoshBodyBuildTest, SetBodyTextTest3)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        string strText = "<message from='904@conf.whitney.cisco.com' \
                         to='904_695@whitney.cisco.com/eklanrrfol' \
                         type='headline'><event xmlns='http://jabber.org/protocol/pubsub#event'>\
                         <items node='urn:ietf:params:xml:ns:conference-info'><item>\
                         <conference-info state='partial' xmlns='urn:ietf:params:xml:ns:conference-info'>\
                         <users><user entity='904@conf.whitney.cisco.com/web_695_11e8af59-51cc-436a-baff-fc6e6533bae9'\
                         state='partial'><roles><entry>http://protocols.cisco.com/roles#participant</entry>\
                         <entry>http://protocols.cisco.com/roles#presenter</entry></roles></user></users>\
                         </conference-info></item></items></event></message>" ;

        strText += "<iq a='asd'>tesfdfad</iq>";
        m_pBodyBuild->SetBodyText(strText);

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());
    }
}

TEST_F(CBoshBodyBuildTest, SetBodyTextTest4)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        string strText = "<message from='904@conf.whitney.cisco.com' \
                         to='904_695@whitney.cisco.com/eklanrrfol' \
                         type='headline'><event xmlns='http://jabber.org/protocol/pubsub#event'>\
                         <items node='urn:ietf:params:xml:ns:conference-info'><item>\
                         <conference-info state='partial' xmlns='urn:ietf:params:xml:ns:conference-info'>\
                         <users><user entity='904@conf.whitney.cisco.com/web_695_11e8af59-51cc-436a-baff-fc6e6533bae9'\
                         state='partial'><roles><entry>http://protocols.cisco.com/roles#participant</entry>\
                         <entry>http://protocols.cisco.com/roles#presenter</entry></roles></user></users>\
                         </conference-info></item></items></event></message>" ;

        strText += "<iq a='asd'>tesfdfad</iq>hello world";
        m_pBodyBuild->SetBodyText(strText);

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());
    }
}

TEST_F(CBoshBodyBuildTest, SetBodyTextTest5)
{
    ASSERT_TRUE(NULL == m_pBodyBuild);

    {
        m_pBodyBuild = new CBoshBodyBuild();

        ASSERT_TRUE(NULL != m_pBodyBuild);
        CCmComAutoPtr<CBoshBodyBuild> autoPtrBodyBuild(m_pBodyBuild);

        string strText = "hello world<message from='904@conf.whitney.cisco.com' \
                         to='904_695@whitney.cisco.com/eklanrrfol' \
                         type='headline'><event xmlns='http://jabber.org/protocol/pubsub#event'>\
                         <items node='urn:ietf:params:xml:ns:conference-info'><item>\
                         <conference-info state='partial' xmlns='urn:ietf:params:xml:ns:conference-info'>\
                         <users><user entity='904@conf.whitney.cisco.com/web_695_11e8af59-51cc-436a-baff-fc6e6533bae9'\
                         state='partial'><roles><entry>http://protocols.cisco.com/roles#participant</entry>\
                         <entry>http://protocols.cisco.com/roles#presenter</entry></roles></user></users>\
                         </conference-info></item></items></event></message>" ;

        strText += "<iq a='asd'>tesfdfad</iq>";
        m_pBodyBuild->SetBodyText(strText);

        string strXml = m_pBodyBuild->ToXmlString();
        EXPECT_TRUE(!strXml.empty());
    }
}

//}}}---------------------------------------------------------------------