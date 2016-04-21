#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
using namespace wme;

#include <vector>
using namespace std;

typedef vector<wme::IWmeMediaPackage*> WmeMediaPackageContainer;

class CWmeMediaTransportMockForH264Packetization : public wme::IWmeMediaTransport
{
public:
    CWmeMediaTransportMockForH264Packetization() : m_sentPackages()
    {}

    ~CWmeMediaTransportMockForH264Packetization() 
    {
        for (WmeMediaPackageContainer::iterator iter = m_sentPackages.begin(); iter != m_sentPackages.end(); ++iter)
        {
            (*iter)->Release();
        }
        m_sentPackages.clear();
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(NULL != pRTPPackage);
        WmeMediaFormatType eType = WmeMediaFormatRTP;
        WMERESULT wmeRet = pRTPPackage->GetFormatType(eType);
        EXPECT_TRUE(WME_SUCCEEDED(wmeRet));
        EXPECT_EQ(WmeMediaFormatRTP, eType);

        pRTPPackage->AddRef();
        m_sentPackages.push_back(pRTPPackage);

        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        EXPECT_TRUE(NULL != pRTCPPackage);
        WmeMediaFormatType eType = WmeMediaFormatRTCP;
        WMERESULT wmeRet = pRTCPPackage->GetFormatType(eType);
        EXPECT_TRUE(WME_SUCCEEDED(wmeRet));
        EXPECT_EQ(WmeMediaFormatRTCP, eType);

        return WME_S_OK;
    }

    WmeMediaPackageContainer& GetSentPackages() { return m_sentPackages; }

private:
    WmeMediaPackageContainer m_sentPackages;
};

class CWmeH264PacketizationSendingTest : public testing::Test
{
public:
    CWmeH264PacketizationSendingTest() : m_pEngine(NULL), m_pVideoSession(NULL), m_pLocalVideoExternalTrack(NULL), 
        m_pMediaTransport(NULL), m_uTrackLabel(12345), m_uMaxPacketSize(1300)
    {
    }

    ~CWmeH264PacketizationSendingTest()
    {
    }

protected:
    virtual void SetUp() 
    {
        // create media engine
        WMERESULT wmeRet = wme::WmeCreateMediaEngine(&m_pEngine);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
        ASSERT_TRUE(NULL != m_pEngine);

        // create media session
        wmeRet = m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoSession);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
        ASSERT_TRUE(NULL != m_pVideoSession);

        // create local external track
        wmeRet = m_pEngine->CreateLocalVideoExternalTrack(&m_pLocalVideoExternalTrack, Wme_SourceType_Video_Sharing);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
        ASSERT_TRUE(NULL != m_pLocalVideoExternalTrack);

        // set media transport
        m_pMediaTransport = new CWmeMediaTransportMockForH264Packetization();
        wmeRet = m_pVideoSession->SetTransport(m_pMediaTransport);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

        // configure the media session: Codec/Payload type
        WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {WmeCodecType_SVC, 98, 90000};
        wmeRet = m_pVideoSession->SetOption(WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(WmeCodecPayloadTypeMap));
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

        // configure the media session: RTP header Extension
        WmeRTPExtension stToffsetExtension;
        stToffsetExtension.sExtURI = (char*)("urn:ietf:params:rtp-hdrext:toffset");
        stToffsetExtension.uExtID  = 1;
        wmeRet = m_pVideoSession->SetOption(WmeSessionOption_RTPExtension, &stToffsetExtension, sizeof(WmeRTPExtension));
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

        WmeRTPExtensionEx stVidExtension = {};
        stVidExtension.sExtURI  = (char*)("http://protocols.cisco.com/virtualid");
        stVidExtension.uExtID   = 2;
        stVidExtension.uDirection   = STREAM_INOUT;
        wmeRet = m_pVideoSession->SetOption(WmeSessionOption_RTPExtensionEx, &stVidExtension, sizeof(WmeRTPExtensionEx));
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
        
        // set the max payload size
        wmeRet = m_pVideoSession->SetOption(WmeSessionOption_MaxPacketSize, &m_uMaxPacketSize, sizeof(m_uMaxPacketSize));
        EXPECT_TRUE(WME_SUCCEEDED(wmeRet));

        wmeRet = m_pLocalVideoExternalTrack->SetTrackLabel(m_uTrackLabel);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

        // ready to go
        wmeRet = m_pVideoSession->AddTrack(m_pLocalVideoExternalTrack);
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

        wmeRet = m_pLocalVideoExternalTrack->Start();
        ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    }
    
    virtual void TearDown() 
    {
        if (m_pLocalVideoExternalTrack) 
        {
            m_pLocalVideoExternalTrack->Release();
            m_pLocalVideoExternalTrack = NULL;
        }

        if (m_pVideoSession)
        {
            m_pVideoSession->Release();
            m_pVideoSession = NULL;
        }

        if (m_pEngine) 
        {
            m_pEngine->Release();
            m_pEngine = NULL;
        }

        if (m_pMediaTransport) 
        {
            delete m_pMediaTransport;
            m_pMediaTransport = NULL;
        }
    }

protected:
    wme::IWmeMediaEngine*                       m_pEngine;
    wme::IWmeMediaSession*                      m_pVideoSession;
    wme::IWmeLocalVideoExternalTrack*           m_pLocalVideoExternalTrack;
    
    CWmeMediaTransportMockForH264Packetization* m_pMediaTransport;
    uint32_t                                    m_uTrackLabel;
    uint32_t                                    m_uMaxPacketSize;
};

// forward declaration
extern "C" void SleepMs(uint32_t aMsec);

#ifndef WME_TEST_LIMITED_WAIT
#define WME_TEST_LIMITED_WAIT(interval, maxTimes, prediction)   \
for (int ii = 0; ii < (maxTimes); ++ii) {                       \
    if (prediction) {                                           \
        break;                                                  \
    }                                                           \
    SleepMs(interval);                                          \
}

#endif // ~WME_TEST_LIMITED_WAIT

#ifndef WRTP_TEST_INIT_NAL_HEADER
#define WRTP_TEST_INIT_NAL_HEADER(pNAL, type) (*pNAL = (type && 0x1F))
#endif // ~WRTP_TEST_INIT_NAL_HEADER

static void RTPPacketQuerierCheck(wme::IWmeMediaSession* pVideoSession, uint32_t uTrackLable, unsigned char*& pRTPHeader, uint16_t& uPacketLen)
{
    ASSERT_TRUE(NULL != pVideoSession);
    ASSERT_TRUE(NULL != pRTPHeader);
    ASSERT_TRUE(uPacketLen > 0);
    
    IWmeRTPPacketQuerier* querier = NULL;
    WMERESULT wmeRet = pVideoSession->GetRTPPacketQuerier(STREAM_OUT, &querier);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    
    wmeRet = querier->Bind(pRTPHeader, uPacketLen);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    uint16_t uCSICount = querier->GetCSICount();
    ASSERT_EQ(1, uCSICount);
    
    uint32_t uCSI = 0;
    wmeRet = querier->GetCSIAt(0, uCSI);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_EQ(uTrackLable, uCSI);
    
    uint32_t pCSIArray[2] = {};
    uCSICount = 2;
    wmeRet = querier->GetMultiCSI(pCSIArray, uCSICount);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_EQ(1, uCSICount);
    ASSERT_EQ(uTrackLable, pCSIArray[0]);
}

void GetRTPInfoFromMediaPackage(IWmeMediaPackage* pPackage, unsigned char*& pRTPHeader, uint16_t& uPacketLen, unsigned char*& pPayload, uint16_t& uPayloadLen)
{
    ASSERT_TRUE(NULL != pPackage);

    WmeMediaFormatType eType;
    WMERESULT wmeRet = pPackage->GetFormatType(eType);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_EQ(WmeMediaFormatRTP, eType);

    unsigned char *pBuffer = NULL;
    uint32_t uBufferOffset = 0;
    uint32_t uBufferLen = 0;
    wmeRet = pPackage->GetDataPointer(&pBuffer);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pBuffer);

    wmeRet = pPackage->GetDataOffset(uBufferOffset);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

    wmeRet = pPackage->GetDataLength(uBufferLen);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(uBufferLen > 0);

    pRTPHeader                  = pBuffer + uBufferOffset;
    uPacketLen                  = uBufferLen;
    
    uint8_t uCSRCCount          = (*pRTPHeader) & 0x0F;
    bool bHasHeaderExtension    = (*pRTPHeader) & 0x10;
    uint16_t* pHeaderExtLen     = bHasHeaderExtension ? (uint16_t*)(pRTPHeader + 12 + uCSRCCount*4 + 2) : NULL;
    uint16_t uHeaderExtLen      = pHeaderExtLen ? (ntohs(*pHeaderExtLen)+1) * 4 : 0;

    uint16_t uRTPHeaderLen      = 12 + uCSRCCount*4 + uHeaderExtLen;
    pPayload    = pRTPHeader + uRTPHeaderLen;
    uPayloadLen = uBufferLen - uBufferOffset - uRTPHeaderLen;
}

TEST_F(CWmeH264PacketizationSendingTest, Mode0_send_payload_shortter_than_max_payload_size)
{
    WMERESULT wmeRet = WME_S_OK;

    // set the packetization-mode=0
    WmePacketizationMode eMode = WmePacketizationMode_0;
    wmeRet = m_pVideoSession->SetOption(WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode));
    EXPECT_TRUE(WME_SUCCEEDED(wmeRet));


    IWmeExternalInputter* pVideoInputter = NULL;
    wmeRet = m_pLocalVideoExternalTrack->GetExternalInputter(&pVideoInputter);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pVideoInputter);

    uint32_t uTimestamp                 = 0;
    WmeMediaFormatType eFormatType      = WmeMediaFormatVideoCodec;
    WmeVideoCodecFormat stVideoCodecFormat = {};
    memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
    stVideoCodecFormat.eCodecType       = WmeCodecType_SVC;
    stVideoCodecFormat.uTimestamp       = 1000;
    stVideoCodecFormat.uSampleTimestamp = 90000;
    stVideoCodecFormat.uMarker          = 1;
    stVideoCodecFormat.uPriority        = 0;
    const int iLen = 1024;
    unsigned char pVideoData[iLen];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData, iLen);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));

    // wait until the packet is sent
    WME_TEST_LIMITED_WAIT(10, 50, m_pMediaTransport->GetSentPackages().size() > 0);
    ASSERT_EQ(1, m_pMediaTransport->GetSentPackages().size());

    // check the sent packet
    IWmeMediaPackage* pPackage  = m_pMediaTransport->GetSentPackages().front();
    unsigned char* pRTPHeader   = NULL;
    uint16_t uPacketLen         = 0;
    unsigned char* pPayload     = NULL;
    uint16_t uPayloadLen        = 0;
    GetRTPInfoFromMediaPackage(pPackage, pRTPHeader, uPacketLen, pPayload, uPayloadLen);

    ASSERT_TRUE(NULL != pPayload);
    ASSERT_EQ(iLen, uPayloadLen);
    ASSERT_EQ(0, memcmp(pVideoData, pPayload, iLen));
    
    RTPPacketQuerierCheck(m_pVideoSession, m_uTrackLabel, pRTPHeader, uPacketLen);
    pVideoInputter->Release();
}

TEST_F(CWmeH264PacketizationSendingTest, Mode0_send_payload_longer_than_max_payload_size)
{
    WMERESULT wmeRet = WME_S_OK;

    // set the packetization-mode=0
    WmePacketizationMode eMode = WmePacketizationMode_0;
    wmeRet = m_pVideoSession->SetOption(WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode));
    EXPECT_TRUE(WME_SUCCEEDED(wmeRet));


    IWmeExternalInputter* pVideoInputter = NULL;
    wmeRet = m_pLocalVideoExternalTrack->GetExternalInputter(&pVideoInputter);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pVideoInputter);

    uint32_t uTimestamp                 = 0;
    WmeMediaFormatType eFormatType      = WmeMediaFormatVideoCodec;
    WmeVideoCodecFormat stVideoCodecFormat = {};
    memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
    stVideoCodecFormat.eCodecType       = WmeCodecType_SVC;
    stVideoCodecFormat.uTimestamp       = 1000;
    stVideoCodecFormat.uSampleTimestamp = 90000;
    stVideoCodecFormat.uMarker          = 1;
    stVideoCodecFormat.uPriority        = 0;
    const int iLen = 2048;
    unsigned char pVideoData[iLen];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData, iLen);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));

    // wait until the packet is sent
    WME_TEST_LIMITED_WAIT(10, 50, m_pMediaTransport->GetSentPackages().size() > 0);
    ASSERT_EQ(1, m_pMediaTransport->GetSentPackages().size());

    // check the sent packet
    IWmeMediaPackage* pPackage  = m_pMediaTransport->GetSentPackages().front();
    unsigned char* pRTPHeader   = NULL;
    uint16_t uPacketLen         = 0;
    unsigned char* pPayload     = NULL;
    uint16_t uPayloadLen        = 0;
    GetRTPInfoFromMediaPackage(pPackage, pRTPHeader, uPacketLen, pPayload, uPayloadLen);

    ASSERT_TRUE(NULL != pPayload);
    ASSERT_EQ(iLen, uPayloadLen);
    ASSERT_EQ(0, memcmp(pVideoData, pPayload, iLen));
    
    RTPPacketQuerierCheck(m_pVideoSession, m_uTrackLabel, pRTPHeader, uPacketLen);
    pVideoInputter->Release();
}


TEST_F(CWmeH264PacketizationSendingTest, Mode1_send_Single_NAL_Unit_Packet)
{
    WMERESULT wmeRet = WME_S_OK;

    // set the packetization-mode=0
    WmePacketizationMode eMode = WmePacketizationMode_1;
    wmeRet = m_pVideoSession->SetOption(WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode));
    EXPECT_TRUE(WME_SUCCEEDED(wmeRet));


    IWmeExternalInputter* pVideoInputter = NULL;
    wmeRet = m_pLocalVideoExternalTrack->GetExternalInputter(&pVideoInputter);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pVideoInputter);

    uint32_t uTimestamp                 = 0;
    WmeMediaFormatType eFormatType      = WmeMediaFormatVideoCodec;
    WmeVideoCodecFormat stVideoCodecFormat = {};
    memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
    stVideoCodecFormat.eCodecType       = WmeCodecType_SVC;
    stVideoCodecFormat.uTimestamp       = 1000;
    stVideoCodecFormat.uSampleTimestamp = 90000;
    stVideoCodecFormat.uMarker          = 1;
    stVideoCodecFormat.uPriority        = 0;
    const int iLen = 1024;
    unsigned char pVideoData[iLen];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData, iLen);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));

    // wait until the packet is sent
    WME_TEST_LIMITED_WAIT(10, 50, m_pMediaTransport->GetSentPackages().size() > 0);
    ASSERT_EQ(1, m_pMediaTransport->GetSentPackages().size());

    // check the sent packet
    IWmeMediaPackage* pPackage  = m_pMediaTransport->GetSentPackages().front();
    unsigned char* pRTPHeader   = NULL;
    uint16_t uPacketLen         = 0;
    unsigned char* pPayload     = NULL;
    uint16_t uPayloadLen        = 0;
    GetRTPInfoFromMediaPackage(pPackage, pRTPHeader, uPacketLen, pPayload, uPayloadLen);

    ASSERT_TRUE(NULL != pPayload);
    ASSERT_EQ(iLen, uPayloadLen);
    ASSERT_EQ(0, memcmp(pVideoData, pPayload, iLen));
    
    RTPPacketQuerierCheck(m_pVideoSession, m_uTrackLabel, pRTPHeader, uPacketLen);
    pVideoInputter->Release();
}

TEST_F(CWmeH264PacketizationSendingTest, Mode1_send_FU_A_Packet)
{
    WMERESULT wmeRet = WME_S_OK;

    // set the packetization-mode=0
    WmePacketizationMode eMode = WmePacketizationMode_1;
    wmeRet = m_pVideoSession->SetOption(WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode));
    EXPECT_TRUE(WME_SUCCEEDED(wmeRet));


    IWmeExternalInputter* pVideoInputter = NULL;
    wmeRet = m_pLocalVideoExternalTrack->GetExternalInputter(&pVideoInputter);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pVideoInputter);

    uint32_t uTimestamp                 = 0;
    WmeMediaFormatType eFormatType      = WmeMediaFormatVideoCodec;
    WmeVideoCodecFormat stVideoCodecFormat = {};
    memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
    stVideoCodecFormat.eCodecType       = WmeCodecType_SVC;
    stVideoCodecFormat.uTimestamp       = 1000;
    stVideoCodecFormat.uSampleTimestamp = 90000;
    stVideoCodecFormat.uMarker          = 1;
    stVideoCodecFormat.uPriority        = 0;
    const int iLen = 3000;
    unsigned char pVideoData[iLen];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData, iLen);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));

    // wait until the packet is sent
    WME_TEST_LIMITED_WAIT(10, 50, m_pMediaTransport->GetSentPackages().size() > 2);
    ASSERT_EQ(3, m_pMediaTransport->GetSentPackages().size());

    // get the RTP payload
    struct RTPPayload
    {
        unsigned char*  pPayload;
        uint16_t        uPayloadLen;
    };
    RTPPayload stRTPPayloads[3];
    for (uint32_t ii = 0; ii < m_pMediaTransport->GetSentPackages().size(); ++ii)
    {
        IWmeMediaPackage* pPackage =  m_pMediaTransport->GetSentPackages()[ii];
        unsigned char* pRTPHeader   = NULL;
        uint16_t uPacketLen         = 0;
        GetRTPInfoFromMediaPackage(pPackage, pRTPHeader, uPacketLen, stRTPPayloads[ii].pPayload, stRTPPayloads[ii].uPayloadLen);
        ASSERT_TRUE(NULL != stRTPPayloads[ii].pPayload);
        
        RTPPacketQuerierCheck(m_pVideoSession, m_uTrackLabel, pRTPHeader, uPacketLen);
    }

    unsigned char cFuIndicator  = 0;
    unsigned char cFuHeader     = 0;
    
    // check the first payload
    cFuIndicator    = stRTPPayloads[0].pPayload[0];
    cFuHeader       = stRTPPayloads[0].pPayload[1];
    EXPECT_EQ(28, cFuIndicator);
    EXPECT_EQ(0x81, cFuHeader);     // 0x81: 10000001

    // check the middle payload
    cFuIndicator    = stRTPPayloads[1].pPayload[0];
    cFuHeader       = stRTPPayloads[1].pPayload[1];
    EXPECT_EQ(28, cFuIndicator);
    EXPECT_EQ(0x01, cFuHeader);     // 0x01: 00000001

    // check the last payload
    cFuIndicator    = stRTPPayloads[2].pPayload[0];
    cFuHeader       = stRTPPayloads[2].pPayload[1];
    EXPECT_EQ(28, cFuIndicator);
    EXPECT_EQ(0x41, cFuHeader);     // 0x41 : 01000001

    // check the NAL content
    EXPECT_EQ(iLen-1, stRTPPayloads[0].uPayloadLen-2 + stRTPPayloads[1].uPayloadLen-2 + stRTPPayloads[2].uPayloadLen-2);
    int cmpRet = 0;
    cmpRet = memcmp(pVideoData+1, stRTPPayloads[0].pPayload+2, stRTPPayloads[0].uPayloadLen-2);
    ASSERT_EQ(0, cmpRet);
    cmpRet = memcmp(pVideoData+1 + (stRTPPayloads[0].uPayloadLen-2), stRTPPayloads[1].pPayload+2, stRTPPayloads[1].uPayloadLen-2);
    ASSERT_EQ(0, cmpRet);
    cmpRet = memcmp(pVideoData+iLen - (stRTPPayloads[2].uPayloadLen-2), stRTPPayloads[2].pPayload+2, stRTPPayloads[2].uPayloadLen-2);
    ASSERT_EQ(0, cmpRet);
    pVideoInputter->Release();
}

TEST_F(CWmeH264PacketizationSendingTest, Mode1_STAP_A_Packet)
{
    WMERESULT wmeRet = WME_S_OK;

    // set the packetization-mode=0
    WmePacketizationMode eMode = WmePacketizationMode_1;
    wmeRet = m_pVideoSession->SetOption(WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode));
    EXPECT_TRUE(WME_SUCCEEDED(wmeRet));


    IWmeExternalInputter* pVideoInputter = NULL;
    wmeRet = m_pLocalVideoExternalTrack->GetExternalInputter(&pVideoInputter);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    ASSERT_TRUE(NULL != pVideoInputter);

    uint32_t uTimestamp                 = 0;
    WmeMediaFormatType eFormatType      = WmeMediaFormatVideoCodec;
    WmeVideoCodecFormat stVideoCodecFormat = {};
    memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
    stVideoCodecFormat.eCodecType       = WmeCodecType_SVC;
    stVideoCodecFormat.uTimestamp       = 1000;
    stVideoCodecFormat.uSampleTimestamp = 90000;
    stVideoCodecFormat.uMarker          = 0;
    stVideoCodecFormat.uPriority        = 0;

    // first video NAL
    const int iVideoDataLLen0           = 100;
    unsigned char pVideoData0[iVideoDataLLen0];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData0, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData0, iVideoDataLLen0);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));

    // second video NAL
    stVideoCodecFormat.uMarker          = 1;
    const int iVideoDataLLen1           = 200;
    unsigned char pVideoData1[iVideoDataLLen1];
    WRTP_TEST_INIT_NAL_HEADER(pVideoData1, 1);

    wmeRet = pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pVideoData1, iVideoDataLLen1);
    ASSERT_TRUE((WME_SUCCEEDED(wmeRet)));


    // wait until the packet is sent
    WME_TEST_LIMITED_WAIT(10, 50, m_pMediaTransport->GetSentPackages().size() > 0);
    ASSERT_EQ(1, m_pMediaTransport->GetSentPackages().size());

    // get RTP payload
    IWmeMediaPackage* pPackage  = m_pMediaTransport->GetSentPackages().front();
    unsigned char* pRTPHeader   = NULL;
    uint16_t uPacketLen         = 0;
    unsigned char* pPayload     = NULL;
    uint16_t uPayloadLen        = 0;
    GetRTPInfoFromMediaPackage(pPackage, pRTPHeader, uPacketLen, pPayload, uPayloadLen);

    EXPECT_EQ(uPayloadLen, iVideoDataLLen0+2 + iVideoDataLLen1+2 + 1);

    // check the first NAL
    unsigned char cSTAPANalHdr  = pPayload[0];
    uint16_t* pNALU0Size        = reinterpret_cast<uint16_t*>(pPayload + 1);
    unsigned char* pNALU0       = pPayload + 3;
    EXPECT_EQ(24, cSTAPANalHdr);
    ASSERT_EQ(iVideoDataLLen0, ntohs(*pNALU0Size));
    EXPECT_EQ(0, memcmp(pNALU0, pVideoData0, iVideoDataLLen0));

    // check the second NAL
    uint16_t* pNALU1Size        = reinterpret_cast<uint16_t*>(pNALU0 + ntohs(*pNALU0Size));
    unsigned char* pNALU1       = pNALU0 + ntohs(*pNALU0Size) + 2;
    ASSERT_EQ(iVideoDataLLen1, ntohs(*pNALU1Size));
    EXPECT_EQ(0, memcmp(pNALU1, pVideoData1, iVideoDataLLen1));
    
    RTPPacketQuerierCheck(m_pVideoSession, m_uTrackLabel, pRTPHeader, uPacketLen);
    pVideoInputter->Release();
}