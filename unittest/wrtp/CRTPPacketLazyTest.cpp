#include "gmock/gmock.h"
#include "gtest/gtest.h"
//#include "wbxmock/wbxmock.h"
#include "CmMessageBlock.h"

#include <map>
#include <vector>
#include <string>

using namespace std;

#include "wrtppacket.h"
#include "testutil.h"

using namespace wrtp;

class CRTPPacketLazyTEST : public ::testing::Test
{
protected:
    typedef map<uint16_t, vector<uint8_t> > RTPExtensionMap;

    virtual void SetUp()
    {
        expect_true = true;
        expect_false = false;
        expect_one = 1;
        expect_zero = 0;
        expect_padding = 200;

        expect_version = WRTP_VERSION;

        expect_csrcNumber = 15;
        expect_payloadType = 111;
        expect_sequence = 123;
        expect_timestamp = 234;
        expect_ssrc = 345;
        for (int i = 0; i < expect_csrcNumber; ++i) {
            expect_csrcs.push_back(i + 100);
        }
        expect_extensionId.id = RTP_ONE_BYTE_HEADER_EXTENSION.id;
        expect_TwoByteId.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
        expect_extensionLength = 7;
        vector<uint8_t> tmpBuff(1, 1);
        expect_extensionMap[1] = tmpBuff;
        vector<uint8_t> tmpBuff2(8, 8);
        expect_extensionMap[8] = tmpBuff2;
        vector<uint8_t> tmpBuff3(14, 14);
        expect_extensionMap[14] = tmpBuff3;

        CCmMessageBlock mb(1000);
        // strlen is 100
        char payloadData[] = "This is payload test data, it will repeat until payload length reach to 1000  abcdefghigklm123456789";
        expect_payload = (uint8_t *)mb.GetTopLevelReadPtr();
        expect_payloadLength = 1000;
        uint32_t written = 0;
        for (int i = 0; i < 10; ++i) {
            mb.Write(payloadData, 100, (DWORD *)&written);
        }
        expect_payload[999] = '\0';
        expect_refPayload = mb.DuplicateChained();

        expect_payloadOneByteLength = 1;
        CCmMessageBlock mb2(1);
        mb2.Write("a", 1, (DWORD *)&written);
        exptect_oneBytePayload = (uint8_t *)mb2.GetTopLevelReadPtr();
        expect_refOneBytePayload = mb2.DuplicateChained();
    }

    virtual void TearDown()
    {
        expect_refPayload->DestroyChained();
        expect_refOneBytePayload->DestroyChained();
    }

    typedef map<string, void *> RTPParaMap;
    void CheckRTPMember(CRTPPacketLazy *rtpPackPtr, bool checkBasic = false, RTPParaMap *para = nullptr,
                        bool hasCSRC = false, bool hasExt = false)
    {
        uint32_t rv = 0;

        RTPParaMap expectResult;

        uint8_t GetVersion = 0;
        bool HasPadding = false;
        bool HasExtension = false;
        if (hasExt) {
            HasExtension = true;
        }
        uint8_t GetCSRCCount = 0;
        if (hasCSRC) {
            GetCSRCCount = expect_csrcNumber;
        }
        bool HasMarker = false;
        uint8_t GetPayloadType = 0;
        uint16_t GetSequenceNumber = 0;
        uint32_t GetTimestamp = 0;
        uint32_t GetSSRC = 0;
        vector<uint32_t> GetCSRC;
        RTPExtensionProfileID GetExtensionProfileID;
        GetExtensionProfileID.id = 0;
        uint16_t GetExtensionLength = 0;
        if (hasExt) {
            GetExtensionProfileID.id= RTP_ONE_BYTE_HEADER_EXTENSION.id;
            GetExtensionLength = expect_extensionLength;
        }
        //uint8_t *GetExtensionData = nullptr;
        RTPExtensionMap GetExtensionElement;
        uint32_t GetPayloadLength = 0;
        uint8_t *GetPayloadData = nullptr;

        if (checkBasic) {
            GetVersion = WRTP_VERSION;
            GetPayloadType = expect_payloadType;
            GetSequenceNumber = expect_sequence;
            GetTimestamp = expect_timestamp;
            GetSSRC = expect_ssrc;
            GetCSRC = expect_csrcs;

            GetExtensionElement = expect_extensionMap;
            GetPayloadLength = expect_payloadLength;
            GetPayloadData = expect_payload;
        }

        expectResult["GetVersion"] = &GetVersion;
        expectResult["HasPadding"] = &HasPadding;
        expectResult["HasExtension"] = &HasExtension;
        expectResult["GetCSRCCount"] = &GetCSRCCount;
        expectResult["HasMarker"] = &HasMarker;
        expectResult["GetPayloadType"] = &GetPayloadType;
        expectResult["GetSequenceNumber"] = &GetSequenceNumber;
        expectResult["GetTimestamp"] = &GetTimestamp;
        expectResult["GetSSRC"] = &GetSSRC;
        expectResult["GetCSRC"] = &GetCSRC;
        expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;
        expectResult["GetExtensionLength"] = &GetExtensionLength;
        expectResult["GetExtensionElement"] = &GetExtensionElement;
        expectResult["GetPayloadLength"] = &GetPayloadLength;
        expectResult["GetPayloadData"] = &GetPayloadData;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], rtpPackPtr->GetVersion());
        EXPECT_EQ(*(bool *)expectResult["HasPadding"], rtpPackPtr->HasPadding());
        EXPECT_EQ(*(bool *)expectResult["HasExtension"], rtpPackPtr->HasExtension());
        EXPECT_EQ(*(uint8_t *)expectResult["GetCSRCCount"], rtpPackPtr->GetCSRCCount());
        EXPECT_EQ(*(bool *)expectResult["HasMarker"], rtpPackPtr->HasMarker());
        EXPECT_EQ(*(uint8_t *)expectResult["GetPayloadType"], rtpPackPtr->GetPayloadType());
        EXPECT_EQ(*(uint16_t *)expectResult["GetSequenceNumber"], rtpPackPtr->GetSequenceNumber());
        EXPECT_EQ(*(uint32_t *)expectResult["GetTimestamp"], rtpPackPtr->GetTimestamp());
        EXPECT_EQ(*(uint32_t *)expectResult["GetSSRC"], rtpPackPtr->GetSSRC());
        EXPECT_EQ((*(RTPExtensionProfileID *)expectResult["GetExtensionProfileID"]).id, rtpPackPtr->GetExtensionProfileID().id);
        EXPECT_EQ(*(uint16_t *)expectResult["GetExtensionLength"], rtpPackPtr->GetExtensionLength());
        EXPECT_EQ(*(uint32_t *)expectResult["GetPayloadLength"], rtpPackPtr->GetPayloadLength());
        EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetPayloadData"], rtpPackPtr->GetPayloadData(), rtpPackPtr->GetPayloadLength()));

        // check invalid csrc index, no matter what CSRC number is, it should OK
        EXPECT_EQ((uint32_t)0, rtpPackPtr->GetCSRC(100));

        if (hasCSRC) {
            rv = rtpPackPtr->UpdateCSRC(100, 999);
            if (WRTP_ERR_INVALID_BUFFER!= rv) {
                EXPECT_EQ(WRTP_ERR_INVALID_CSRC_INDEX, rv);
            }

            for (uint8_t i = 0; i < *(uint8_t *)expectResult["GetCSRCCount"]; ++i) {
                EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetCSRC"])[i], rtpPackPtr->GetCSRC(i));
            }
        } else {
            rv = rtpPackPtr->UpdateCSRC(100, 999);
            if (WRTP_ERR_INVALID_BUFFER!= rv) {
                EXPECT_EQ(WRTP_ERR_NO_CSRC, rv);
            }
        }

        uint8_t *elementData = (uint8_t *)888;
        uint8_t elementLength = 222;
        // not exist test
        rv = rtpPackPtr->GetExtensionElement(999, elementData, elementLength);
        EXPECT_EQ(nullptr, elementData);
        EXPECT_EQ(0, elementLength);

        if (hasExt) {
            EXPECT_EQ((uint32_t)WRTP_ERR_NO_EXTENSION_ELEMENT, rv);
            for (RTPExtensionMap::iterator it = (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).begin();
                    it != (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).end(); ++it) {
                rv = rtpPackPtr->GetExtensionElement(it->first, elementData, elementLength);
                EXPECT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
                EXPECT_EQ(it->second.size(), elementLength);
                EXPECT_EQ(0, memcmp(&*(it->second.begin()), elementData, elementLength));
            }
        } else {
            if (WRTP_ERR_INVALID_BUFFER!= rv) {
                EXPECT_EQ((uint32_t)WRTP_ERR_NO_EXTENSION, rv);
            }
        }
    }

    void CheckPackedMember(CRTPPacket *rtpPackPtr, bool checkBasic = false, RTPParaMap *para = nullptr,
                           bool hasCSRC = false, bool hasExt = false)
    {
        RTPParaMap expectResult;

        bool IsValid = false;
        uint8_t GetVersion = WRTP_VERSION;
        bool HasPadding = false;
        bool HasExtension = false;
        if (hasExt) {
            HasExtension = true;
        }
        uint8_t GetCSRCCount = 0;
        if (hasCSRC) {
            GetCSRCCount = expect_csrcNumber;
        }
        bool HasMarker = false;
        uint8_t GetPayloadType = 0;
        uint16_t GetSequenceNumber = 0;
        uint32_t GetTimestamp = 0;
        uint32_t GetSSRC = 0;
        vector<uint32_t> GetCSRC;
        RTPExtensionProfileID GetExtensionProfileID;
        GetExtensionProfileID = expect_extensionId;
        RTPExtensionMap GetExtensionElement;
        uint32_t CalcPacketLength = 0;
        uint32_t GetPayloadLength = 0;
        uint8_t *GetPayloadData = nullptr;

        if (checkBasic) {
            IsValid = expect_true;
            GetPayloadType = expect_payloadType;
            GetSequenceNumber = expect_sequence;
            GetTimestamp = expect_timestamp;
            GetSSRC = expect_ssrc;
            GetCSRC = expect_csrcs;
            GetExtensionElement = expect_extensionMap;
            CalcPacketLength = sizeof(RTPHeader)+expect_payloadLength;
            GetPayloadLength = expect_payloadLength;
            GetPayloadData = expect_payload;
        }

        expectResult["IsValid"] = &IsValid;
        expectResult["GetVersion"] = &GetVersion;
        expectResult["HasPadding"] = &HasPadding;
        expectResult["HasExtension"] = &HasExtension;
        expectResult["GetCSRCCount"] = &GetCSRCCount;
        expectResult["HasMarker"] = &HasMarker;
        expectResult["GetPayloadType"] = &GetPayloadType;
        expectResult["GetSequenceNumber"] = &GetSequenceNumber;
        expectResult["GetTimestamp"] = &GetTimestamp;
        expectResult["GetSSRC"] = &GetSSRC;
        expectResult["GetCSRC"] = &GetCSRC;
        expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;
        expectResult["GetExtensionElement"] = &GetExtensionElement;
        expectResult["CalcPacketLength"] = &CalcPacketLength;
        expectResult["GetPayloadLength"] = &GetPayloadLength;
        expectResult["GetPayloadData"] = &GetPayloadData;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        EXPECT_EQ(*(bool *)expectResult["IsValid"], rtpPackPtr->IsValid());
        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], rtpPackPtr->GetVersion());
        EXPECT_EQ(*(bool *)expectResult["HasPadding"], rtpPackPtr->HasPadding());
        EXPECT_EQ(*(bool *)expectResult["HasExtension"], rtpPackPtr->HasExtension());
        EXPECT_EQ(*(uint8_t *)expectResult["GetCSRCCount"], rtpPackPtr->GetCSRCCount());
        EXPECT_EQ(*(bool *)expectResult["HasMarker"], rtpPackPtr->HasMarker());
        EXPECT_EQ(*(uint8_t *)expectResult["GetPayloadType"], rtpPackPtr->GetPayloadType());
        EXPECT_EQ(*(uint16_t *)expectResult["GetSequenceNumber"], rtpPackPtr->GetSequenceNumber());
        EXPECT_EQ(*(uint32_t *)expectResult["GetTimestamp"], rtpPackPtr->GetTimestamp());
        EXPECT_EQ(*(uint32_t *)expectResult["GetSSRC"], rtpPackPtr->GetSSRC());
        EXPECT_EQ((*(RTPExtensionProfileID *)expectResult["GetExtensionProfileID"]).id, rtpPackPtr->GetExtensionProfileID().id);
        EXPECT_EQ(*(uint32_t *)expectResult["CalcPacketLength"], rtpPackPtr->CalcPacketLength());
        EXPECT_EQ(*(uint32_t *)expectResult["GetPayloadLength"], rtpPackPtr->GetPayloadLength());
        EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetPayloadData"], rtpPackPtr->GetPayloadData(), rtpPackPtr->GetPayloadLength()));

        // check invalid csrc index, no matter what CSRC number is, it should OK
        EXPECT_EQ((uint32_t)0, rtpPackPtr->GetCSRC(100));
        EXPECT_EQ(WRTP_ERR_INVALID_CSRC_INDEX, rtpPackPtr->UpdateCSRC(100, 999));

        if (hasCSRC) {
            for (uint8_t i = 0; i < *(uint8_t *)expectResult["GetCSRCCount"]; ++i) {
                EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetCSRC"])[i], rtpPackPtr->GetCSRC(i));
            }
        } else {
            // if has no csrc, update invalid csrc should return error
            EXPECT_EQ(WRTP_ERR_INVALID_CSRC_INDEX, rtpPackPtr->UpdateCSRC(8, 999));
        }

        uint8_t *elementData = (uint8_t *)888;
        uint8_t elementLength = 222;
        uint32_t rv = 0;
        // not exist test
        rv = rtpPackPtr->GetExtensionElement(999, elementData, elementLength);
        EXPECT_EQ(nullptr, elementData);
        EXPECT_EQ(0, elementLength);

        if (hasExt) {
            EXPECT_EQ((uint32_t)WRTP_ERR_INVALID_EXTENSION_ELEMENT_ID, rv);
            for (RTPExtensionMap::iterator it = (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).begin();
                    it != (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).end(); ++it) {
                rv = rtpPackPtr->GetExtensionElement(it->first, elementData, elementLength);
                EXPECT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
                EXPECT_EQ(it->second.size(), elementLength);
                EXPECT_EQ(0, memcmp(&*(it->second.begin()), elementData, it->second.size()));
            }
        } else {
            if (WRTP_ERR_INVALID_PACKAGE != rv) {
                EXPECT_EQ((uint32_t)WRTP_ERR_NO_EXTENSION, rv);
            }
        }
    }

    bool expect_true;
    bool expect_false;
    uint8_t expect_one;
    uint8_t expect_zero;
    uint8_t expect_padding;

    uint8_t expect_version;

    uint8_t expect_csrcNumber;
    uint8_t expect_payloadType;
    uint16_t expect_sequence;
    uint32_t expect_timestamp;
    uint32_t expect_ssrc;
    vector<uint32_t> expect_csrcs;
    RTPExtensionProfileID expect_extensionId;
    RTPExtensionProfileID expect_TwoByteId;
    uint16_t expect_extensionLength;
    RTPExtensionMap expect_extensionMap;

    uint32_t expect_payloadLength;
    uint32_t expect_payloadOneByteLength;
    uint8_t *expect_payload;
    uint8_t *exptect_oneBytePayload;

    CCmMessageBlock *expect_refPayload;
    CCmMessageBlock *expect_refOneBytePayload;

};

TEST_F(CRTPPacketLazyTEST, Bind_InvalidBufferTest)
{
    CScopedTracer test_info;
    // prepare test data
    CCmMessageBlock mb(2000);
    CRTPPacketLazy rtpPacketLazy;

    EXPECT_EQ(WRTP_ERR_INVALID_BUFFER, rtpPacketLazy.Bind(mb));
    CheckRTPMember(&rtpPacketLazy);
}

TEST_F(CRTPPacketLazyTEST, Bind_InvalidPacketTest)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    mb.RetreatTopLevelWritePtr(mb.GetTopLevelLength()-30);
    CRTPPacketLazy rtpPacketLazy;

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, rtpPacketLazy.Bind(mb));

    delete rtpPackPtr;
}

TEST_F(CRTPPacketLazyTEST, Bind_ResetNormalTest)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));
    CheckRTPMember(&rtpPacketLazy, true);

    rtpPacketLazy.Reset();
    CheckRTPMember(&rtpPacketLazy);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

TEST_F(CRTPPacketLazyTEST, Bind_ResetEmptyPacketTest)
{
    CScopedTracer test_info;
    CRTPPacketLazy rtpPacketLazy;

    rtpPacketLazy.Reset();
    CheckRTPMember(&rtpPacketLazy);
}

#define xxBind_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRTPPacketLazyTEST, Bind_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));
    CheckRTPMember(&rtpPacketLazy, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBindOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRTPPacketLazyTEST, BindOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refOneBytePayload);
    rtpPackPtr->SetPayload(*expect_refOneBytePayload);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 1;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["GetPayloadLength"] = &expect_payloadOneByteLength;
    expectResult["GetPayloadData"] = &exptect_oneBytePayload;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_NoPadding_NoExt_NoMarker_SetCSRC
TEST_F(CRTPPacketLazyTEST, Bind_NoPadding_NoExt_NoMarker_SetCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_NoPadding_NoMarker_CSRC_SetExtension
TEST_F(CRTPPacketLazyTEST, Bind_NoPadding_NoMarker_CSRC_SetExtension)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 +
                                sizeof(RTPHeaderExtension) + expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_NoPadding_Marker_CSRC_Extension
TEST_F(CRTPPacketLazyTEST, Bind_NoPadding_Marker_CSRC_Extension)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) + expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_Marker_CSRC_Extension_SetPadding
TEST_F(CRTPPacketLazyTEST, Bind_Marker_CSRC_Extension_SetPadding)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_Marker_CSRC_Extension_SetPadding_Nopayload
TEST_F(CRTPPacketLazyTEST, Bind_Marker_CSRC_Extension_SetPadding_Nopayload)
{
    CScopedTracer test_info;
    // prepare test data
    //Get Basic No payload RTP Packet
    CRTPPacket *rtpPackPtr = GetBasicNopayloadRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    uint32_t GetPayloadLength = 0;
    uint8_t *GetPayloadData = nullptr;
    expectResult["GetPayloadLength"] = &GetPayloadLength;
    expectResult["GetPayloadData"] = &GetPayloadData;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_Update
TEST_F(CRTPPacketLazyTEST, Bind_Update)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    rtpPacketLazy.Reset();
    CheckRTPMember(&rtpPacketLazy);

    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_Update_Increase_Extension_Length
TEST_F(CRTPPacketLazyTEST, Bind_Update_Increase_Extension_Length)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock tmpMb;
    CCmMessageBlock *mb = nullptr;
    uint32_t rv = rtpPackPtr->Pack(tmpMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    mb = tmpMb.DuplicateChained();
    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(*mb));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb->CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
    mb->DestroyChained();
}

#define xxBind_Buffer_Update
TEST_F(CRTPPacketLazyTEST, Bind_Buffer_Update)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind((void *)mb.GetTopLevelReadPtr(), (uint32_t)mb.GetTopLevelLength()));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    rtpPacketLazy.Reset();
    CheckRTPMember(&rtpPacketLazy);

    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBind_Buffer_Update_Increase_Extension_Length
TEST_F(CRTPPacketLazyTEST, Bind_Buffer_Update_Increase_Extension_Length)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind((void *)mb.GetTopLevelReadPtr(), (uint32_t)mb.GetTopLevelLength()));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                28 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    //expectResult["GetExtensionElement"] = &increaseExtensionMap;
    //uint32_t GetExtensionLength = 10;
    //expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBindTwoByteExt_Marker_CSRC_Extension_SetPadding
TEST_F(CRTPPacketLazyTEST, BindTwoByteExt_Marker_CSRC_Extension_SetPadding)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                8*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    uint32_t GetExtensionLength = 8;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBindTwoByteExt_Update
TEST_F(CRTPPacketLazyTEST, BindTwoByteExt_Update)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                8*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    uint32_t GetExtensionLength = 8;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;

    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    rtpPacketLazy.Reset();
    CheckRTPMember(&rtpPacketLazy);

    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxBindTwoByteExt_Update_Increase_Extension_Length
TEST_F(CRTPPacketLazyTEST, BindTwoByteExt_Update_Increase_Extension_Length)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                44 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 11;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;

    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_Increase_RemoveExt_ModifySwapExdID
TEST_F(CRTPPacketLazyTEST, Bind_Increase_RemoveExt_ModifySwapExdID)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }
    increaseExtensionMap.clear();
    increaseExtensionMap[13] = tmpBuff2;
    increaseExtensionMap[8] = tmpBuff3;
    rtpPacketLazy.RemoveExtensionElement(1);
    rtpPacketLazy.ModifyExtensionID(14, 13);
    rtpPacketLazy.SwapExtensionID(8, 13);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                28 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 7;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBindTwoByteExt_Increase_RemoveExt_ModifySwapExdID
TEST_F(CRTPPacketLazyTEST, BindTwoByteExt_Increase_RemoveExt_ModifySwapExdID)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }
    increaseExtensionMap.clear();
    increaseExtensionMap[13] = tmpBuff2;
    increaseExtensionMap[8] = tmpBuff3;
    rtpPacketLazy.RemoveExtensionElement(1);
    rtpPacketLazy.ModifyExtensionID(14, 13);
    rtpPacketLazy.SwapExtensionID(8, 13);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                32 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 8;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;

    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_AddExt
TEST_F(CRTPPacketLazyTEST, Bind_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // exist test
    rv = rtpPacketLazy.AddExtensionElement(14, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    EXPECT_EQ((uint32_t)WRTP_ERR_REPEAT_EXTENSION_ELEMENT_ID, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_NoExt_AddExt
TEST_F(CRTPPacketLazyTEST, Bind_NoExt_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;

    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                12 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 3;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBindTwoByteExt_AddExt
TEST_F(CRTPPacketLazyTEST, BindTwoByteExt_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(12, 12);
    increaseExtensionMap[12] = tmpBuff4;

    rv = rtpPacketLazy.AddExtensionElement(12, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // exist test
    rv = rtpPacketLazy.AddExtensionElement(14, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    EXPECT_EQ((uint32_t)WRTP_ERR_REPEAT_EXTENSION_ELEMENT_ID, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                44 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 11;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;

    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_UpdateWholeExtension_Increase
TEST_F(CRTPPacketLazyTEST, Bind_UpdateWholeExtension_Increase)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // Prepare extension buffer
    vector<uint8_t> tmpBuff4(11, 11);
    rtpPackPtr->AddExtensionElement(11, (uint8_t *)&*tmpBuff4.begin(),
                                    (uint8_t)tmpBuff4.size());
    CCmMessageBlock mbIncreasedExt;
    rtpPackPtr->Pack(mbIncreasedExt);
    CRTPPacketLazy rtpPacketLazyIncreasedExt;
    rtpPacketLazyIncreasedExt.Bind(mbIncreasedExt);


    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));
    rv = rtpPacketLazy.UpdateExtension(rtpPacketLazyIncreasedExt.GetExtensionData(),
                                       rtpPacketLazyIncreasedExt.GetExtensionLength());

    uint8_t numMb = 0;
    CCmMessageBlock *currMb = &mb;
    while (currMb) {
        ++numMb;
        currMb = currMb->GetNext();
    }
    EXPECT_EQ(3, numMb);

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    increaseExtensionMap[11] = tmpBuff4;

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_UpdateWholeExtension_Decrease
TEST_F(CRTPPacketLazyTEST, Bind_UpdateWholeExtension_Decrease)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Prepare extension buffer
    CRTPPacket *rtpPackPtr2 = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr2->SetPadding(expect_padding);
    rtpPackPtr2->SetMarker();
    rtpPackPtr2->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    vector<uint8_t> tmpBuff4(11, 11);
    rtpPackPtr2->AddExtensionElement(11, (uint8_t *)&*tmpBuff4.begin(),
                                     (uint8_t)tmpBuff4.size());
    CCmMessageBlock mbIncreasedExt;
    rtpPackPtr2->Pack(mbIncreasedExt);
    CRTPPacketLazy rtpPacketLazyIncreasedExt;
    rtpPacketLazyIncreasedExt.Bind(mbIncreasedExt);
    delete rtpPackPtr2;

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));
    rv = rtpPacketLazy.UpdateExtension(rtpPacketLazyIncreasedExt.GetExtensionData(),
                                       rtpPacketLazyIncreasedExt.GetExtensionLength());

    uint8_t numMb = 0;
    CCmMessageBlock *currMb = &mb;
    while (currMb) {
        ++numMb;
        currMb = currMb->GetNext();
    }
    //EXPECT_EQ(3, numMb);

    RTPExtensionMap increaseExtensionMap;
    increaseExtensionMap[11] = tmpBuff4;

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                12 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 3;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_AddExt_DecreaseCSRC
TEST_F(CRTPPacketLazyTEST, Bind_AddExt_DecreaseCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // decrease csrc
    vector<uint32_t> csrcs;
    csrcs.push_back(400);
    csrcs.push_back(401);
    csrcs.push_back(402);
    csrcs.push_back(403);
    rv = rtpPacketLazy.UpdateCSRC(4, &*csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 4*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetCSRC"] = &csrcs;
    uint8_t GetCSRCCount = 4;
    expectResult["GetCSRCCount"] = &GetCSRCCount;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_AddExt_IncreaseCSRC
TEST_F(CRTPPacketLazyTEST, Bind_AddExt_IncreaseCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_IncreaseCSRC_IncreaseExt
TEST_F(CRTPPacketLazyTEST, Bind_IncreaseCSRC_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxBind_NoCSRC_IncreaseCSRC_IncreaseExt
TEST_F(CRTPPacketLazyTEST, Bind_NoCSRC_IncreaseCSRC_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    CRTPPacket rtpUnpack;
    CCmMessageBlock *combineMb = mb.CombineSingleBuffer();

    rv = rtpUnpack.Unpack(*combineMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    combineMb->DestroyChained();
}

#define xxRetain_Empty
TEST_F(CRTPPacketLazyTEST, Retain_Empty)
{
    CScopedTracer test_info;
    CCmMessageBlock emptyMb;
    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_INVALID_BUFFER, rtpPacketLazy.Retain(emptyMb));
    EXPECT_EQ(nullptr, rtpPacketLazy.GetRetainMb());
}

#define xxRetain_Update_Increase_Extension_Length
TEST_F(CRTPPacketLazyTEST, Retain_Update_Increase_Extension_Length)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock tmpMb;
    CCmMessageBlock *mb = nullptr;
    uint32_t rv = rtpPackPtr->Pack(tmpMb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    mb = tmpMb.DuplicateChained();
    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Retain(*mb));

    uint8_t GetPayloadType = 100;
    uint16_t GetSequenceNumber = 321;
    uint32_t GetTimestamp = 432;
    uint32_t GetSSRC = 543;
    vector<uint32_t> GetCSRC;
    for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
        GetCSRC.push_back(i + 200);
    }

    rtpPacketLazy.UpdatePayloadType(GetPayloadType);
    rtpPacketLazy.UpdateSequenceNumber(GetSequenceNumber);
    rtpPacketLazy.UpdateTimestamp(GetTimestamp);
    rtpPacketLazy.UpdateSSRC(GetSSRC);
    rtpPacketLazy.UpdateCSRC(expect_csrcNumber, &*GetCSRC.begin());
    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(10, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(11, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(15, 14);
    increaseExtensionMap[14] = tmpBuff3;
    for (RTPExtensionMap::iterator it = increaseExtensionMap.begin(); it != increaseExtensionMap.end(); ++it) {
        rtpPacketLazy.UpdateExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->second.size());
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetPayloadType"] = &GetPayloadType;
    expectResult["GetSequenceNumber"] = &GetSequenceNumber;
    expectResult["GetTimestamp"] = &GetTimestamp;
    expectResult["GetSSRC"] = &GetSSRC;
    expectResult["GetCSRC"] = &GetCSRC;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());
    CRTPPacket rtpUnpackRetain;
    rv = rtpUnpackRetain.Unpack(*rtpPacketLazy.GetRetainMb());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpackRetain, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
    mb->DestroyChained();
}

#define xxRetain_IncreaseCSRC_IncreaseExt
TEST_F(CRTPPacketLazyTEST, Retain_IncreaseCSRC_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Retain(mb));

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());
    CRTPPacket rtpUnpackRetain;
    rv = rtpUnpackRetain.Unpack(*rtpPacketLazy.GetRetainMb());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpackRetain, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxRetain_IncreaseCSRC_CombineBuff_IncreaseExt
TEST_F(CRTPPacketLazyTEST, Retain_IncreaseCSRC_CombineBuff_AddExt)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Retain(mb));

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());
    CRTPPacket rtpUnpackRetain;
    rv = rtpUnpackRetain.Unpack(*rtpPacketLazy.GetRetainMb());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpackRetain, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxRetain_Repeat
TEST_F(CRTPPacketLazyTEST, Retain_Repeat)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    CCmMessageBlock *mbPtr = mb.DuplicateChained();
    CCmMessageBlock *mbPtr2 = mb.DuplicateChained();
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Retain(*mbPtr));
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Retain(*mbPtr2));
    mbPtr->DestroyChained();
    mbPtr = nullptr;
    mbPtr2->DestroyChained();
    mbPtr2 = nullptr;

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    expectResult["GetExtensionElement"] = &increaseExtensionMap;
    uint32_t GetExtensionLength = 10;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    CheckRTPMember(&rtpPacketLazy, true, &expectResult, true, true);

    ASSERT_TRUE(nullptr != rtpPacketLazy.GetRetainMb());
    CRTPPacket rtpUnpackRetain;
    rv = rtpUnpackRetain.Unpack(*rtpPacketLazy.GetRetainMb());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckPackedMember(&rtpUnpackRetain, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxCalcPacketLength_Empty
TEST_F(CRTPPacketLazyTEST, CalcPacketLength_Empty)
{
    CScopedTracer test_info;
    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(0, rtpPacketLazy.CalcPacketLength());
}

#define xxCalcPacketLength_Mb
TEST_F(CRTPPacketLazyTEST, CalcPacketLength_Mb)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));
    EXPECT_EQ(rtpPackPtr->CalcPacketLength(), rtpPacketLazy.CalcPacketLength());

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxCalcPacketLength_Mb_Increase
TEST_F(CRTPPacketLazyTEST, CalcPacketLength_Mb_Increase)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->AddCSRC(888);
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    ASSERT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

    RTPExtensionMap increaseExtensionMap;
    vector<uint8_t> tmpBuff(1, 1);
    increaseExtensionMap[1] = tmpBuff;
    vector<uint8_t> tmpBuff2(8, 8);
    increaseExtensionMap[8] = tmpBuff2;
    vector<uint8_t> tmpBuff3(14, 14);
    increaseExtensionMap[14] = tmpBuff3;
    vector<uint8_t> tmpBuff4(11, 11);
    increaseExtensionMap[11] = tmpBuff4;

    // increase and split ext
    rv = rtpPacketLazy.AddExtensionElement(11, &*tmpBuff4.begin(), (uint8_t)tmpBuff4.size());
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // increase csrc
    rv = rtpPacketLazy.UpdateCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 15*4 + sizeof(RTPHeaderExtension) +
                                40 + expect_payloadLength + expect_padding;

    EXPECT_EQ(CalcPacketLength, rtpPacketLazy.CalcPacketLength());

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxCalcPacketLength_Buffer
TEST_F(CRTPPacketLazyTEST, CalcPacketLength_Buffer)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacketLazy rtpPacketLazy;
    EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind((void *)(mb.GetTopLevelReadPtr()), mb.GetTopLevelLength()));
    EXPECT_EQ(rtpPackPtr->CalcPacketLength(), rtpPacketLazy.CalcPacketLength());

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

