#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "wrtpapi.h"
#include "wrtpwmeapi.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"
#include "rtpheaderext.h"

#include <vector>

using namespace wrtp;
using namespace wme;
using namespace std;

#ifndef DEFAULT_VIDIO_CAPTURE_CLOCK_RATE
    #define DEFAULT_VIDIO_CAPTURE_CLOCK_RATE 90000
#endif


void AssertEQ(const FrameMarkingInfo &frameMarkingInfo, const uint8_t *frameMarkingExt, uint8_t extLen)
{
    ASSERT_TRUE(frameMarkingExt != nullptr);
    EXPECT_EQ(1, extLen);
    EXPECT_EQ(frameMarkingInfo.disposable,  RFCFrameMarkingExt_GetD(frameMarkingExt[0]));
    EXPECT_EQ(frameMarkingInfo.switchable,  RFCFrameMarkingExt_GetS(frameMarkingExt[0]));
    EXPECT_EQ(frameMarkingInfo.TID,         RFCFrameMarkingExt_GetTID(frameMarkingExt[0]));
    EXPECT_EQ(frameMarkingInfo.frameType,   RFCFrameMarkingExt_GetType(frameMarkingExt[0]));
}

class CRTPHeaderExtTest : public ::testing::Test
{
public:
    CRTPHeaderExtTest(): exlength(128) {}

protected:
    virtual void SetUp()
    {
        exlength = 128;
        for (int ii = 0; ii < exlength; ++ii) {
            extBuffer[ii] = ii;
        }
    }


public:
    uint8_t extBuffer[128];
    uint8_t exlength;
};

TEST_F(CRTPHeaderExtTest, FrameMarkingHelpMacroTest)
{
    uint8_t extVal = 0;
    EXPECT_EQ(0, RFCFrameMarkingExt_GetD(extVal));
    EXPECT_EQ(0, RFCFrameMarkingExt_GetS(extVal));
    EXPECT_EQ(0, RFCFrameMarkingExt_GetTID(extVal));
    EXPECT_EQ(0, RFCFrameMarkingExt_GetType(extVal));

    uint8_t d=0, s=1, tid=1, type=2;
    RFCFrameMarkingExt_SetD(extVal, d);
    RFCFrameMarkingExt_SetS(extVal, s);
    RFCFrameMarkingExt_SetTID(extVal, tid);
    RFCFrameMarkingExt_SetType(extVal, type);

    EXPECT_EQ((d<<7) + (s<<6) + (tid<<3) + type, extVal);
    EXPECT_EQ(d,    RFCFrameMarkingExt_GetD(extVal));
    EXPECT_EQ(s,    RFCFrameMarkingExt_GetS(extVal));
    EXPECT_EQ(tid,  RFCFrameMarkingExt_GetTID(extVal));
    EXPECT_EQ(type, RFCFrameMarkingExt_GetType(extVal));

    d=1, s=0, tid=2, type=3;
    RFCFrameMarkingExt_SetD(extVal, d);
    RFCFrameMarkingExt_SetS(extVal, s);
    RFCFrameMarkingExt_SetTID(extVal, tid);
    RFCFrameMarkingExt_SetType(extVal, type);

    EXPECT_EQ((d<<7) + (s<<6) + (tid<<3) + type, extVal);
    EXPECT_EQ(d,    RFCFrameMarkingExt_GetD(extVal));
    EXPECT_EQ(s,    RFCFrameMarkingExt_GetS(extVal));
    EXPECT_EQ(tid,  RFCFrameMarkingExt_GetTID(extVal));
    EXPECT_EQ(type, RFCFrameMarkingExt_GetType(extVal));
}

TEST_F(CRTPHeaderExtTest, FrameMarkingExtensionBuilder)
{
    CScopedTracer test_info;
    FrameMarkingInfo frameMarkingInfo{0, 1, 2, 0};
    int32_t builderResult = RTPHeaderExtFrameMarkingBuilder(RTPEXT_FrameMarking, &frameMarkingInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);
    EXPECT_EQ(1, exlength);


    // Vedio frame marking extension
    //0                   1
    //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|  ID=2 |  L=0  |D|S|TID  |Type |
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    uint8_t extBufferFirst = extBuffer[0];

    uint8_t disposable    = RFCFrameMarkingExt_GetD(extBufferFirst);
    uint8_t switchable    = RFCFrameMarkingExt_GetS(extBufferFirst);
    uint8_t tid           = RFCFrameMarkingExt_GetTID(extBufferFirst);
    uint8_t type          = RFCFrameMarkingExt_GetType(extBufferFirst);

    EXPECT_EQ(frameMarkingInfo.disposable ? 1 : 0,  disposable);
    EXPECT_EQ(frameMarkingInfo.switchable ? 1 : 0,  switchable);
    EXPECT_EQ(frameMarkingInfo.TID,                 tid);
    EXPECT_EQ(frameMarkingInfo.frameType,           type);
}

TEST_F(CRTPHeaderExtTest, FrameMarkingExtensionParser)
{
    CScopedTracer test_info;
    FrameMarkingInfo frameMarkingInfo{1, 1, 2, 1};
    EXPECT_EQ(0, strcmp(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), "http://protocols.cisco.com/framemarking"));
    int32_t builderResult = RTPHeaderExtFrameMarkingBuilder(RTPEXT_FrameMarking, &frameMarkingInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);
    EXPECT_EQ(1, exlength);

    FrameMarkingInfo parsedInfo{false,false, 0, 0};
    int32_t parseResult = RTPHeaderExtFrameMarkingParser(RTPEXT_FrameMarking, &parsedInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, parseResult);

    EXPECT_EQ(frameMarkingInfo.disposable,  parsedInfo.disposable);
    EXPECT_EQ(frameMarkingInfo.switchable,  parsedInfo.switchable);
    EXPECT_EQ(frameMarkingInfo.TID,         parsedInfo.TID);
    EXPECT_EQ(frameMarkingInfo.frameType,   parsedInfo.frameType);
}

TEST_F(CRTPHeaderExtTest, AudioLevelIndicationExtensionBuilder)
{
    CScopedTracer test_info;
    AudioLevelInfo audoLevelInfo{true, 11};
    int32_t builderResult = RTPHeaderExtAudioLevelBuilder(RTPEXT_AudioLevel, &audoLevelInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);

    uint8_t v     = (extBuffer[0] & 0x80)>>7;
    uint8_t level = extBuffer[0] & 0x7F;

    EXPECT_EQ(v,        audoLevelInfo.vad ? 1 : 0);
    EXPECT_EQ(level,    audoLevelInfo.level);
}

TEST_F(CRTPHeaderExtTest, AudioLevelIndicationExtensionParser)
{
    CScopedTracer test_info;
    AudioLevelInfo audoLevelInfo{true, 11};
    int32_t builderResult = RTPHeaderExtAudioLevelBuilder(RTPEXT_AudioLevel, &audoLevelInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);

    WRTPMediaDataVoIP mediaDataVoIP;
    AudioLevelInfo parsedInfo{false, 0};
    int32_t parserResult = RTPHeaderExtAudioLevelParser(RTPEXT_AudioLevel, &parsedInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, parserResult);

    EXPECT_EQ(audoLevelInfo.vad,      parsedInfo.vad);
    EXPECT_EQ(audoLevelInfo.level,    parsedInfo.level);
}

TEST_F(CRTPHeaderExtTest, TransmissionOffsetExtensionBuilderNegative)
{
    CScopedTracer test_info;
    int32_t timeOffset = -0x0fffff;
    uint8_t buff[3];
    uint8_t len = 3;
    memset(buff, 0 , 3);
    RTPHeaderExtTimeOffsetBuilder(RTPEXT_TOFFSET, &timeOffset, buff, len);

    EXPECT_NE(0, (buff[0]&0x80));
    EXPECT_EQ(0x0f, (buff[0]&0x7f));
    EXPECT_EQ(0xff, buff[1]);
    EXPECT_EQ(0xff, buff[2]);
}

TEST_F(CRTPHeaderExtTest, TransmissionOffsetExtensionBuilderPositive)
{
    CScopedTracer test_info;
    int32_t timeOffset = 0x0fffff;
    uint8_t buff[3];
    uint8_t len = 3;
    memset(buff, 0 , 3);
    RTPHeaderExtTimeOffsetBuilder(RTPEXT_TOFFSET, &timeOffset, buff, len);

    EXPECT_EQ(0, (buff[0]&0x80));
    EXPECT_EQ(0x0f, (buff[0]&0x7f));
    EXPECT_EQ(0xff, buff[1]);
    EXPECT_EQ(0xff, buff[2]);
}

TEST_F(CRTPHeaderExtTest, TransmissionOffsetExtensionParserNegative)
{
    CScopedTracer test_info;
    int32_t timeOffset = -0x0fffff;
    uint8_t buff[3];
    uint8_t len = 3;
    memset(buff, 0 , 3);
    RTPHeaderExtTimeOffsetBuilder(RTPEXT_TOFFSET, &timeOffset, buff, len);

    int32_t parseValue = 0;
    RTPHeaderExtTimeOffsetParser(RTPEXT_TOFFSET, &parseValue, buff, len);
    EXPECT_EQ(timeOffset, parseValue);
}

TEST_F(CRTPHeaderExtTest, TransmissionOffsetExtensionParserPositive)
{
    CScopedTracer test_info;
    int32_t timeOffset = 0x0fffff;
    uint8_t buff[3];
    uint8_t len = 3;
    memset(buff, 0 , 3);
    RTPHeaderExtTimeOffsetBuilder(RTPEXT_TOFFSET, &timeOffset, buff, len);

    int32_t parseValue = 0;
    RTPHeaderExtTimeOffsetParser(RTPEXT_TOFFSET, &parseValue, buff, len);
    EXPECT_EQ(timeOffset, parseValue);
}

TEST_F(CRTPHeaderExtTest, RTPHeaderExtMariSeqTimestampBuilder)
{

    CScopedTracer test_info;

    MariExtensionInfo mariInfo{3, 2};
    int32_t builderResult = RTPHeaderExtMariSeqTimestampBuilder(RTPEXT_MARITimestamp, &mariInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);

    MariExtensionInfo *mariInfoExpect;
    mariInfoExpect = reinterpret_cast<MariExtensionInfo *>(&extBuffer[1]);

    EXPECT_EQ(extBuffer[0], 0);
    uint16_t seq = ntohs(mariInfoExpect->sequence);
    uint32_t timestamp = ntohl(mariInfoExpect->timestamp);



    EXPECT_EQ(seq, mariInfo.sequence);

    EXPECT_EQ(timestamp, mariInfo.timestamp);
    EXPECT_EQ(exlength, 7);

}

TEST_F(CRTPHeaderExtTest, RTPHeaderExtMariSeqTimestampParser)
{
    CScopedTracer test_info;

    MariExtensionInfo mariInfo{3, 2};
    int32_t builderResult = RTPHeaderExtMariSeqTimestampBuilder(RTPEXT_MARITimestamp, &mariInfo, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, builderResult);

    MariExtensionInfo mariInfoDecoded;

    RTPHeaderExtMariSeqTimestampParser(RTPEXT_MARITimestamp, &mariInfoDecoded, extBuffer, exlength);

    EXPECT_EQ(mariInfoDecoded.sequence, mariInfo.sequence);

    EXPECT_EQ(mariInfoDecoded.timestamp, mariInfo.timestamp);
}

TEST_F(CRTPHeaderExtTest, RTPHeaderExt64bitsNTPBuilder)
{
    CScopedTracer test_info;

    NTP64bitsInfo ntp{1234, 5678};
    int32_t ret = RTPHeaderExt64bitsNTPBuilder(RTPEXT_RapicSync64, &ntp, extBuffer, exlength);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);
    ASSERT_EQ(8, exlength);

    NTP64bitsInfo *ntpExpect = reinterpret_cast<NTP64bitsInfo *>(&extBuffer[0]);
    uint32_t sec = ntohl(ntpExpect->seconds);
    uint32_t fra = ntohl(ntpExpect->fraction);

    EXPECT_EQ(ntp.seconds,  sec);
    EXPECT_EQ(ntp.fraction, fra);
}

TEST_F(CRTPHeaderExtTest, RTPHeaderExt64bitsNTPParser)
{
    CScopedTracer test_info;

    NTP64bitsInfo ntp{12345, 23456};
    int32_t ret = RTPHeaderExt64bitsNTPBuilder(RTPEXT_RapicSync64, &ntp, extBuffer, exlength);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    ASSERT_EQ(8, exlength);

    NTP64bitsInfo ntpDecoded{0, 0};
    ret = RTPHeaderExt64bitsNTPParser(RTPEXT_RapicSync64, &ntpDecoded, extBuffer, exlength);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    EXPECT_EQ(ntp.seconds,  ntpDecoded.seconds);
    EXPECT_EQ(ntp.fraction, ntpDecoded.fraction);
}

TEST_F(CRTPHeaderExtTest, RTPExt_Index_Test)
{
    ASSERT_EQ(0, RTPEXT_VID);
    ASSERT_EQ(1, RTPEXT_AudioLevel);
    ASSERT_EQ(1, RTPEXT_FrameMarking);
    ASSERT_EQ(2, RTPEXT_TOFFSET);
    ASSERT_EQ(3, RTPEXT_MARITimestamp);
    ASSERT_EQ(4, RTPEXT_Priority);
    ASSERT_EQ(5, RTPEXT_RapicSync64);
    ASSERT_EQ(6, RTPEXT_CVO);
}

TEST_F(CRTPHeaderExtTest, RTPExt_Name_Test)
{
    // Video
    ASSERT_STREQ("http://protocols.cisco.com/virtualid",        GetVideoRTPHeaderExtName(RTPEXT_VID));
    ASSERT_STREQ("http://protocols.cisco.com/framemarking",     GetVideoRTPHeaderExtName(RTPEXT_FrameMarking));
    ASSERT_STREQ("urn:ietf:params:rtp-hdrext:toffset",          GetVideoRTPHeaderExtName(RTPEXT_TOFFSET));
    ASSERT_STREQ("http://protocols.cisco.com/timestamp#100us",  GetVideoRTPHeaderExtName(RTPEXT_MARITimestamp));
    ASSERT_STREQ("urn:ietf:params:rtp-hdrext:ntp-64",           GetVideoRTPHeaderExtName(RTPEXT_RapicSync64));
    ASSERT_STREQ("http://protocols.cisco.com/priority",         GetVideoRTPHeaderExtName(RTPEXT_Priority));

    // Audio
    ASSERT_STREQ("http://protocols.cisco.com/virtualid",        GetAudioRTPHeaderExtName(RTPEXT_VID));
    ASSERT_STREQ("urn:ietf:params:rtp-hdrext:ssrc-audio-level", GetAudioRTPHeaderExtName(RTPEXT_AudioLevel));
    ASSERT_STREQ("urn:ietf:params:rtp-hdrext:toffset",          GetAudioRTPHeaderExtName(RTPEXT_TOFFSET));
    ASSERT_STREQ("http://protocols.cisco.com/timestamp#100us",  GetAudioRTPHeaderExtName(RTPEXT_MARITimestamp));
    ASSERT_STREQ("urn:ietf:params:rtp-hdrext:ntp-64",           GetAudioRTPHeaderExtName(RTPEXT_RapicSync64));
}

TEST_F(CRTPHeaderExtTest, Audio_RTPExt_Manager_Test)
{
    RTPHeaderExtType    extType;
    uint8_t               id;
    int32_t               ret;
    CRTPHeaderExtManager    builder(false, true);
    CRTPHeaderExtManager    parser(false, false);

    // RTPEXT_VID
    extType = RTPEXT_VID;
    id      = 1;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(1,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(1,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtVidBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtVidParser  == parser .GetExtHandler(extType));

    // RTPEXT_AudioLevel
    extType = RTPEXT_AudioLevel;
    id      = 2;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(1,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(1,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtAudioLevelBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtAudioLevelParser  == parser .GetExtHandler(extType));

    // RTPEXT_TOFFSET
    extType = RTPEXT_TOFFSET;
    id      = 3;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(3,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(3,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtTimeOffsetBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtTimeOffsetParser  == parser .GetExtHandler(extType));

    // RTPEXT_MARITimestamp
    extType = RTPEXT_MARITimestamp;
    id      = 4;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(7,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(7,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtMariSeqTimestampBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtMariSeqTimestampParser  == parser .GetExtHandler(extType));

    // RTPEXT_64bitsNTP
    extType = RTPEXT_RapicSync64;
    id      = 5;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(8,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(8,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExt64bitsNTPBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExt64bitsNTPParser  == parser .GetExtHandler(extType));

}

TEST_F(CRTPHeaderExtTest, Video_RTPExt_Manager_Test)
{
    RTPHeaderExtType    extType;
    uint8_t               id;
    int32_t               ret;
    CRTPHeaderExtManager    builder(true, true);
    CRTPHeaderExtManager    parser(true, false);

    // RTPEXT_VID
    extType = RTPEXT_VID;
    id      = 1;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(1,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(1,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtVidBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtVidParser  == parser .GetExtHandler(extType));

    // RTPEXT_FrameMarking
    extType = RTPEXT_FrameMarking;
    id      = 2;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(1,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(1,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtFrameMarkingBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtFrameMarkingParser  == parser .GetExtHandler(extType));

    // RTPEXT_TOFFSET
    extType = RTPEXT_TOFFSET;
    id      = 3;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(3,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(3,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtTimeOffsetBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtTimeOffsetParser  == parser .GetExtHandler(extType));

    // RTPEXT_MARITimestamp
    extType = RTPEXT_MARITimestamp;
    id      = 4;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(7,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(7,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtMariSeqTimestampBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtMariSeqTimestampParser  == parser .GetExtHandler(extType));

    // RTPEXT_RapicSync64
    extType = RTPEXT_RapicSync64;
    id      = 5;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(8,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(8,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExt64bitsNTPBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExt64bitsNTPParser  == parser .GetExtHandler(extType));

    // RTPEXT_Priority
    extType = RTPEXT_Priority;
    id      = 6;

    ret = builder.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   builder.GetExtId(extType));
    ASSERT_EQ(1,    builder.GetExtMinLen(extType));

    ret = parser.RegisterExt(extType, id);
    ASSERT_EQ(0,    ret);
    ASSERT_EQ(id,   parser.GetExtId(extType));
    ASSERT_EQ(1,    parser.GetExtMinLen(extType));

    ASSERT_TRUE(RTPHeaderExtPriorityBuilder == builder.GetExtHandler(extType));
    ASSERT_TRUE(RTPHeaderExtPriorityParser  == parser .GetExtHandler(extType));
}

static int32_t AudioRTPExtIterFunc(const char *extURI)
{
    RTPHeaderExtType enabled[] = {RTPEXT_VID, RTPEXT_AudioLevel, RTPEXT_TOFFSET, RTPEXT_MARITimestamp};
    static int index = -1;
    ++index;

    EXPECT_GE(index, 0);
    EXPECT_LE(index, sizeof(enabled)/sizeof(enabled[0]));
    EXPECT_STREQ(extURI, GetAudioRTPHeaderExtName(enabled[index]));

    return 0;
}

static int32_t VideoRTPExtIterFunc(const char *extURI)
{
    RTPHeaderExtType enabled[] = {RTPEXT_VID, RTPEXT_FrameMarking, RTPEXT_TOFFSET, RTPEXT_MARITimestamp, RTPEXT_CVO};
    static int index = -1;
    ++index;

    EXPECT_GE(index, 0);
    EXPECT_LE(index, sizeof(enabled)/sizeof(enabled[0]));
    EXPECT_STREQ(extURI, GetVideoRTPHeaderExtName(enabled[index]));

    return 0;
}

TEST_F(CRTPHeaderExtTest, Default_RTPExt_Test)
{
    EnumBuiltinRTPHeaderExtensions(RTP_SESSION_WEBEX_VOIP,  AudioRTPExtIterFunc);
    EnumBuiltinRTPHeaderExtensions(RTP_SESSION_WEBEX_VIDEO, VideoRTPExtIterFunc);
}

TEST_F(CRTPHeaderExtTest, Enumeration_Value_Test)
{
    ASSERT_EQ(WME_RTPEXT_VID,            RTPEXT_VID);
    ASSERT_EQ(WME_RTPEXT_AudioLevel,     RTPEXT_AudioLevel);
    ASSERT_EQ(WME_RTPEXT_FrameMarking,   RTPEXT_FrameMarking);
    ASSERT_EQ(WME_RTPEXT_TOFFSET,        RTPEXT_TOFFSET);
    ASSERT_EQ(WME_RTPEXT_MARITimestamp,  RTPEXT_MARITimestamp);
    ASSERT_EQ(WME_RTPEXT_Priority,       RTPEXT_Priority);
    ASSERT_EQ(WME_RTPEXT_RapicSync64,    RTPEXT_RapicSync64);
    ASSERT_EQ(WME_RTPEXT_MaxCount,          RTPEXT_MaxCount);
}

static uint32_t AbsoluteDiff(uint32_t x, uint32_t y)
{
    return (x > y) ? (x - y) : (y - x);
}

TEST_F(CRTPHeaderExtTest, NTP_MS_Convert_Test)
{
    {
        // ms -> ntp -> ms
        const uint32_t ms     = 123456;
        NTP64bitsInfo ntp   = NTP64bitsInfoFromTickMS(ms);
        uint32_t newMS        = NTP64bitsInfoToTickMS(ntp);
        ASSERT_LE(AbsoluteDiff(ms, newMS), 1);
    }

    {
        // ntp -> ms -> ntp
        const NTP64bitsInfo ntp {1234, 65536*1000};
        uint32_t ms               = NTP64bitsInfoToTickMS(ntp);
        NTP64bitsInfo newNtp    = NTP64bitsInfoFromTickMS(ms);
        uint32_t newMS            = NTP64bitsInfoToTickMS(newNtp);

        ASSERT_EQ(ntp.seconds,  newNtp.seconds);
        ASSERT_LE(AbsoluteDiff(ms, newMS), 1);
    }

    {
        // ms -> ntp
        const uint32_t ms     = 123456;
        NTP64bitsInfo ntp   = NTP64bitsInfoFromTickMS(ms);

        CNTPTime ntpTime(ntp.seconds, ntp.fraction);
        uint32_t newMS        = ntpTime.ToClockTime().ToMilliseconds();

        ASSERT_LE(AbsoluteDiff(ms, newMS), 1);

    }

    {
        // ntp -> ms
        const NTP64bitsInfo ntp {1234, 65536*1000};
        uint32_t ms           = NTP64bitsInfoToTickMS(ntp);

        CNTPTime ntpTime(ntp.seconds, ntp.fraction);
        uint32_t newMS        = ntpTime.ToClockTime().ToMilliseconds();
        ASSERT_LE(AbsoluteDiff(ms, newMS), 1);
    }
}

//////////////////////////////////////////////////////
typedef vector<IWmeMediaPackage *> VideoRTPPackageVector;

class CVideoHeaderExtensionTransportMock : public IRTPMediaTransport
{
public:
    CVideoHeaderExtensionTransportMock() : m_videoPackages() {}
    ~CVideoHeaderExtensionTransportMock()
    {
        for (VideoRTPPackageVector::iterator iter = m_videoPackages.begin(); iter != m_videoPackages.end(); ++iter) {
            IWmeMediaPackage *package = (*iter);
            package->Release();
        }
        m_videoPackages.clear();

    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(pRTPPackage);
        pRTPPackage->AddRef();
        m_videoPackages.push_back(pRTPPackage);

        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }

    const VideoRTPPackageVector &GetVideoPackages()
    {
        return m_videoPackages;
    }

private:
    VideoRTPPackageVector m_videoPackages;
};

class CRTPHeaderExtensionVideoTest :public ::testing::Test
{
public:
    CRTPHeaderExtensionVideoTest(): m_SendSession(nullptr)
        , m_SendChannel(nullptr)
        , m_mediaSourceId(123)
        , m_framemarkingURI("http://protocols.cisco.com/framemarking")
        , m_framemarkingId(2)
    {
    }

protected:
    virtual void SetUp()
    {
        InitSender();
    }

    virtual void TearDown()
    {
        m_SendSession->SetMediaTransport(nullptr);

        m_SendChannel->Close();
        m_SendChannel->DecreaseReference();
        m_SendChannel = nullptr;

        m_SendSession->DecreaseReference();
        m_SendSession = nullptr;

        m_transport.reset();

    }
    void InitSender()
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP  = true;

        m_SendSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_SendSession);
        m_SendSession->IncreaseReference();

        int32_t ret = 0;
        ret = m_SendSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
        EXPECT_TRUE(0 == ret);

        ret = m_SendSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

        // create video sending channel
        WRTPChannelParams channelParams;

        m_SendChannel = m_SendSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_SendChannel);
        m_SendChannel->IncreaseReference();

        // sending sink for sending session
        m_transport.reset(new CVideoHeaderExtensionTransportMock());
        ASSERT_TRUE(nullptr != m_transport.get());
        ret = m_SendSession->SetMediaTransport(m_transport.get());
        EXPECT_TRUE(0 == ret);

        ret = m_SendSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        EXPECT_TRUE(0 == ret);

    }

protected:
    IRTPSessionClient        *m_SendSession;
    IRTPChannel              *m_SendChannel;

    uint32_t                    m_mediaSourceId;
    const char               *m_framemarkingURI;
    uint8_t                     m_framemarkingId;

    std::unique_ptr<CVideoHeaderExtensionTransportMock> m_transport;
};

TEST_F(CRTPHeaderExtensionVideoTest, Framemarking_RTP_Header_Extension_Single_Packet_Not_Switchable_Frame_Test)
{
    CScopedTracer test_info;

    uint32_t timestamp        = 10;
    uint32_t sampleTimestamp  = 10*DEFAULT_VIDIO_CAPTURE_CLOCK_RATE/1000;
    uint32_t dataLen          = 1024;
    uint8_t DID               = 0;
    uint8_t marker            = 1;
    CodecType codecType     = CODEC_TYPE_VIDEO_TEST;

    FrameMarkingInfo frameMarkingInfo = {1, 0, 0, 1};

    WRTPMediaData *mediaData    = CreateVideoData(timestamp, sampleTimestamp, dataLen, DID, marker, codecType);
    WRTPMediaDataVideo *mdVideo = static_cast<WRTPMediaDataVideo *>(mediaData);
    mdVideo->disposable     = frameMarkingInfo.disposable;
    mdVideo->switchable     = frameMarkingInfo.switchable;
    mdVideo->TID            = frameMarkingInfo.TID;
    mdVideo->frameType      = frameMarkingInfo.frameType;

    int32_t ret = m_SendChannel->SendMediaData(mdVideo);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    const VideoRTPPackageVector &rtpPackets = m_transport->GetVideoPackages();
    WRTP_TEST_TRIGGER_ON_TIMER(100, 2);
    ASSERT_EQ(1, rtpPackets.size());

    IWmeMediaPackage *pRTPPackage = rtpPackets.front();
    CCmMessageBlock *mb = nullptr;
    ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
    EXPECT_TRUE(mb != nullptr);

    CRTPPacketLazy rtpPacketLazy;
    rtpPacketLazy.Bind(*mb);

    uint8_t *frameMarkingExt = nullptr;
    uint8_t extLen = sizeof(frameMarkingExt);
    ret = rtpPacketLazy.GetExtensionElement(m_framemarkingId, frameMarkingExt, extLen);
    ASSERT_EQ(0, ret);

    AssertEQ(frameMarkingInfo, frameMarkingExt, extLen);

    DestroyMediaData(mediaData);
}

TEST_F(CRTPHeaderExtensionVideoTest, Framemarking_RTP_Header_Extension_Single_Packet_Switchable_Frame_Test)
{
    CScopedTracer test_info;

    uint32_t timestamp        = 10;
    uint32_t sampleTimestamp  = 10*DEFAULT_VIDIO_CAPTURE_CLOCK_RATE/1000;
    uint32_t dataLen          = 1024;
    uint8_t DID               = 0;
    uint8_t marker            = 1;
    CodecType codecType     = CODEC_TYPE_VIDEO_TEST;

    FrameMarkingInfo frameMarkingInfo = {0, 1, 0, 1};

    WRTPMediaData *mediaData    = CreateVideoData(timestamp, sampleTimestamp, dataLen, DID, marker, codecType);
    WRTPMediaDataVideo *mdVideo = static_cast<WRTPMediaDataVideo *>(mediaData);
    mdVideo->disposable     = frameMarkingInfo.disposable;
    mdVideo->switchable     = frameMarkingInfo.switchable;
    mdVideo->TID            = frameMarkingInfo.TID;
    mdVideo->frameType      = frameMarkingInfo.frameType;

    int32_t ret = m_SendChannel->SendMediaData(mdVideo);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    const VideoRTPPackageVector &rtpPackets = m_transport->GetVideoPackages();
    WRTP_TEST_TRIGGER_ON_TIMER(100, 2);
    ASSERT_EQ(1, rtpPackets.size());

    IWmeMediaPackage *pRTPPackage = rtpPackets.front();
    CCmMessageBlock *mb = nullptr;
    ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
    EXPECT_TRUE(mb != nullptr);

    CRTPPacketLazy rtpPacketLazy;
    rtpPacketLazy.Bind(*mb);

    uint8_t *frameMarkingExt = nullptr;
    uint8_t extLen = sizeof(frameMarkingExt);
    ret = rtpPacketLazy.GetExtensionElement(m_framemarkingId, frameMarkingExt, extLen);
    ASSERT_EQ(0, ret);

    AssertEQ(frameMarkingInfo, frameMarkingExt, extLen);

    DestroyMediaData(mediaData);
}

TEST_F(CRTPHeaderExtensionVideoTest, Framemarking_RTP_Header_Extension_Multiple_Packets_Switchable_Frame_Test)
{
    CScopedTracer test_info;
    uint32_t timestamp        = 10;
    uint32_t sampleTimestamp  = 10*DEFAULT_VIDIO_CAPTURE_CLOCK_RATE/1000;
    uint32_t dataLen          = 1024;
    uint8_t DID               = 0;
    CodecType codecType     = CODEC_TYPE_VIDEO_TEST;

    FrameMarkingInfo frameMarkingInfo = {0, 1, 0, 1};

    WRTPMediaData *mediaData1    = CreateVideoData(timestamp, sampleTimestamp, dataLen, DID, 0, codecType);
    WRTPMediaDataVideo *mdVideo = static_cast<WRTPMediaDataVideo *>(mediaData1);
    mdVideo->disposable     = frameMarkingInfo.disposable;
    mdVideo->switchable     = frameMarkingInfo.switchable;
    mdVideo->TID            = frameMarkingInfo.TID;
    mdVideo->frameType      = frameMarkingInfo.frameType;

    int32_t ret = m_SendChannel->SendMediaData(mdVideo);
    DestroyMediaData(mediaData1);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    WRTPMediaData *mediaData2    = CreateVideoData(timestamp, sampleTimestamp, dataLen, DID, 1, codecType);
    mdVideo = static_cast<WRTPMediaDataVideo *>(mediaData2);
    mdVideo->disposable     = frameMarkingInfo.disposable;
    mdVideo->switchable     = frameMarkingInfo.switchable;
    mdVideo->TID            = frameMarkingInfo.TID;
    mdVideo->frameType      = frameMarkingInfo.frameType;

    ret = m_SendChannel->SendMediaData(mdVideo);
    DestroyMediaData(mediaData2);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 2);

    const VideoRTPPackageVector &rtpPackets = m_transport->GetVideoPackages();
    ASSERT_EQ(2, rtpPackets.size());

    {
        // first packet: the switchable bit shoulb be 1
        IWmeMediaPackage *pRTPPackage = rtpPackets.front();
        CCmMessageBlock *mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        EXPECT_TRUE(mb != nullptr);

        CRTPPacketLazy rtpPacketLazy;
        rtpPacketLazy.Bind(*mb);

        uint8_t *frameMarkingExt = nullptr;
        uint8_t extLen = sizeof(frameMarkingExt);
        ret = rtpPacketLazy.GetExtensionElement(m_framemarkingId, frameMarkingExt, extLen);
        ASSERT_EQ(0, ret);

        AssertEQ(frameMarkingInfo, frameMarkingExt, extLen);
    }

    {
        // second packet: the switchable bit shoulb be 0
        IWmeMediaPackage *pRTPPackage = rtpPackets.back();
        CCmMessageBlock *mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        EXPECT_TRUE(mb != nullptr);

        CRTPPacketLazy rtpPacketLazy;
        rtpPacketLazy.Bind(*mb);

        uint8_t *frameMarkingExt = nullptr;
        uint8_t extLen = sizeof(frameMarkingExt);
        ret = rtpPacketLazy.GetExtensionElement(m_framemarkingId, frameMarkingExt, extLen);
        ASSERT_EQ(0, ret);

        frameMarkingInfo.switchable = false;
        AssertEQ(frameMarkingInfo, frameMarkingExt, extLen);
    }
}
