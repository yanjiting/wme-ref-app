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

class CRTPPacketTEST : public ::testing::Test
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
    void CheckRTPMember(CRTPPacket *rtpPackPtr, bool checkBasic = false, RTPParaMap *para = nullptr,
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
            EXPECT_EQ((uint32_t)WRTP_ERR_NO_EXTENSION, rv);
        }
    }

    void CheckPackedMember(CCmMessageBlock *rtpMb, RTPParaMap *para = nullptr,
                           bool hasCSRC = false, bool hasExt = false)
    {
        RTPParaMap expectResult;

        expectResult["GetVersion"] = &expect_version;
        expectResult["HasPadding"] = &expect_zero;
        expectResult["HasExtension"] = &expect_zero;
        uint16_t GetExtensionLength = 0;
        if (hasExt) {
            expectResult["HasExtension"] = &expect_one;
            GetExtensionLength = expect_extensionLength;
        }
        expectResult["GetCSRCCount"] = &expect_zero;
        if (hasCSRC) {
            expectResult["GetCSRCCount"] = &expect_csrcNumber;
        }
        expectResult["HasMarker"] = &expect_zero;
        expectResult["GetPayloadType"] = &expect_payloadType;
        expectResult["GetSequenceNumber"] = &expect_sequence;
        expectResult["GetTimestamp"] = &expect_timestamp;
        expectResult["GetSSRC"] = &expect_ssrc;
        expectResult["GetCSRC"] = &expect_csrcs;
        expectResult["GetExtensionProfileID"] = &expect_extensionId;
        expectResult["GetExtensionElement"] = &expect_extensionMap;
        expectResult["GetPayloadLength"] = &expect_payloadLength;
        expectResult["GetPayloadData"] = &expect_payload;
        expectResult["GetExtensionLength"] = &GetExtensionLength;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        // Check packed value
        uint8_t *buff = (uint8_t *)rtpMb->GetTopLevelReadPtr();
        UnittestRTPHeader *rtpHeader = (UnittestRTPHeader *)buff;

        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], rtpHeader->version);
        EXPECT_EQ(*(uint8_t *)expectResult["HasPadding"], rtpHeader->padding);
        EXPECT_EQ(*(uint8_t *)expectResult["HasExtension"], rtpHeader->extension);
        EXPECT_EQ(*(uint8_t *)expectResult["GetCSRCCount"], rtpHeader->csrcNumber);
        EXPECT_EQ(*(uint8_t *)expectResult["HasMarker"], rtpHeader->marker);
        EXPECT_EQ(*(uint8_t *)expectResult["GetPayloadType"], rtpHeader->payloadType);
        EXPECT_EQ(*(uint16_t *)expectResult["GetSequenceNumber"], ntohs(rtpHeader->sequence));
        EXPECT_EQ(*(uint32_t *)expectResult["GetTimestamp"], ntohl(rtpHeader->timestamp));
        EXPECT_EQ(*(uint32_t *)expectResult["GetSSRC"], ntohl(rtpHeader->ssrc));

        if (hasCSRC) {
            for (uint32_t i = 0; i < *(uint8_t *)expectResult["GetCSRCCount"]; ++i) {
                EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetCSRC"])[i], ntohl(rtpHeader->csrcs[i]));
            }
        }

        if (hasExt) {
            RTPHeaderExtension *headerExt = (RTPHeaderExtension *)(rtpHeader->csrcs + rtpHeader->csrcNumber);
            EXPECT_EQ((*(RTPExtensionProfileID *)expectResult["GetExtensionProfileID"]).id, headerExt->extid.id);
            EXPECT_EQ(*(uint16_t *)expectResult["GetExtensionLength"], ntohs(headerExt->length));

            uint8_t *extensionBegin = headerExt->extension;
            uint8_t *extensionEnd = headerExt->extension + expect_extensionLength*4;
            uint16_t extensionNum = 0;
            if (RTP_ONE_BYTE_HEADER_EXTENSION.id == headerExt->extid.id) {
                while (extensionBegin < extensionEnd) {
                    uint8_t elementID = (extensionBegin[0]&0xf0)>>4;
                    uint8_t elementLen = (extensionBegin[0]&0x0f) + 1;

                    if (0 == elementID) {
                        extensionBegin++;
                    } else if (15 == elementID) {
                        break;
                    } else {
                        RTPExtensionMap::iterator it =
                            (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).find(elementID);
                        EXPECT_EQ(true, (it != (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).end()));
                        EXPECT_EQ(it->second.size(), elementLen);
                        EXPECT_EQ(0, memcmp(&*(it->second.begin()), extensionBegin + 1, it->second.size()));
                        extensionBegin += elementLen + 1;
                        ++extensionNum;
                    }
                }
            } else if (RTP_TWO_BYTE_HEADER_EXTENSION.id == headerExt->extid.id) {
                while (extensionBegin < extensionEnd) {
                    uint8_t elementID = extensionBegin[0];
                    uint8_t elementLen = extensionBegin[1];

                    if (0 == elementID) {
                        extensionBegin++;
                    } else if (15 == elementID) {
                        break;
                    } else {
                        RTPExtensionMap::iterator it =
                            (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).find(elementID);
                        EXPECT_EQ(true, (it != (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).end()));
                        EXPECT_EQ(it->second.size(), elementLen);
                        EXPECT_EQ(0, memcmp(&*(it->second.begin()), extensionBegin + 2, it->second.size()));
                        extensionBegin += elementLen + 2;
                        ++extensionNum;
                    }
                }
            } else {
                // Do nothing
            }

            EXPECT_EQ((*(RTPExtensionMap *)expectResult["GetExtensionElement"]).size(), extensionNum);

            // check payload value
            EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetPayloadData"], headerExt->extension + ntohs(headerExt->length)*4, *(uint32_t *)expectResult["GetPayloadLength"]));
        } else {
            // check payload value
            EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetPayloadData"], rtpHeader->csrcs + rtpHeader->csrcNumber, *(uint32_t *)expectResult["GetPayloadLength"]));
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

TEST_F(CRTPPacketTEST, Pack_InavidPackageTest)
{
    CScopedTracer test_info;
    CCmMessageBlock mb(2000);
    CRTPPacket rtpPacket;

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, rtpPacket.Pack(mb));
    CheckRTPMember(&rtpPacket);
}

TEST_F(CRTPPacketTEST, Pack_BufferNotEnoughTest)
{
    CScopedTracer test_info;
    CCmMessageBlock mb(2);
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    EXPECT_EQ(WRTP_ERR_BUFFER_NOT_ENOUGH, rtpPackPtr->Pack(mb));

    delete rtpPackPtr;
}

TEST_F(CRTPPacketTEST, Pack_Packet_OnebyteExtBoundary)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    uint16_t elementId = 14;
    uint8_t elementDataBuff[16];
    for (unsigned i = 0; i < sizeof(elementDataBuff); ++i) {
        elementDataBuff[i] = i;
    }
    uint8_t elementDataBuffLength = sizeof(elementDataBuff);
    rtpPackPtr->AddExtensionElement(elementId, elementDataBuff, elementDataBuffLength);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacket unpack;
    uint8_t *elementDataBuffTest = nullptr;
    uint8_t elementDataBuffLengthTest = 0;
    unpack.Unpack(mb);
    rv = unpack.GetExtensionElement(elementId, elementDataBuffTest, elementDataBuffLengthTest);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    ASSERT_EQ(sizeof(elementDataBuff), elementDataBuffLengthTest);
    EXPECT_EQ(0, memcmp(elementDataBuff, elementDataBuffTest, elementDataBuffLengthTest));

    delete rtpPackPtr;
}

TEST_F(CRTPPacketTEST, Pack_Packet_TwobyteExtBoundary)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    uint16_t elementId = 14;
    uint8_t elementDataBuff[255];
    for (unsigned i = 0; i < sizeof(elementDataBuff); ++i) {
        elementDataBuff[i] = i;
    }
    uint8_t elementDataBuffLength = sizeof(elementDataBuff);
    rtpPackPtr->AddExtensionElement(elementId, elementDataBuff, elementDataBuffLength);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacket unpack;
    uint8_t *elementDataBuffTest = nullptr;
    uint8_t elementDataBuffLengthTest = 0;
    unpack.Unpack(mb);
    rv = unpack.GetExtensionElement(elementId, elementDataBuffTest, elementDataBuffLengthTest);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    ASSERT_EQ(sizeof(elementDataBuff), elementDataBuffLengthTest);
    EXPECT_EQ(0, memcmp(elementDataBuff, elementDataBuffTest, elementDataBuffLengthTest));

    delete rtpPackPtr;
}


TEST_F(CRTPPacketTEST, Unpack_InavidPackageTest)
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
    mb.RetreatTopLevelWritePtr(mb.GetTopLevelLength()-20);

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, rtpPackPtr->Unpack(mb));

    delete rtpPackPtr;
}

TEST_F(CRTPPacketTEST, Unpack_InavidPaddingTest)
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
    uint8_t *padLen = (uint8_t *)mb.GetTopLevelWritePtr() - 1;
    // Set invalid larger padding length
    *padLen = 255;
    mb.RetreatTopLevelWritePtr(mb.GetTopLevelLength()-20);

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, rtpPackPtr->Unpack(mb));

    delete rtpPackPtr;
}

TEST_F(CRTPPacketTEST, Unpack_InavidBufferTest)
{
    CScopedTracer test_info;
    CCmMessageBlock mb(2000);
    CRTPPacket rtpPacket;

    EXPECT_EQ(WRTP_ERR_INVALID_BUFFER, rtpPacket.Unpack(mb));
    CheckRTPMember(&rtpPacket);
}

#define xxPack_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRTPPacketTEST, Pack_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    // Check member status
    CheckRTPMember(rtpPackPtr, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // also test buff not enough and enough mb, because we can't resize buff, so return error
    CCmMessageBlock invalidMb(2);
    invalidMb.AdvanceTopLevelWritePtr(2);
    rv = rtpPackPtr->Pack(invalidMb);
    EXPECT_EQ((uint32_t)WRTP_ERR_BUFFER_NOT_ENOUGH, rv);
    CCmMessageBlock enoughMb(2000);
    enoughMb.AdvanceTopLevelWritePtr(2000);
    rv = rtpPackPtr->Pack(enoughMb);
    EXPECT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPackOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRTPPacketTEST, PackOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refOneBytePayload);
    rtpPackPtr->SetPayload(*expect_refOneBytePayload);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 1;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["GetPayloadLength"] = &expect_payloadOneByteLength;
    expectResult["GetPayloadData"] = &exptect_oneBytePayload;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, &expectResult);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_NoPadding_NoExt_NoMarker_SetCSRC
TEST_F(CRTPPacketTEST, Pack_NoPadding_NoExt_NoMarker_SetCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, nullptr, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_NoPadding_NoExt_NoMarker_AddCSRC
TEST_F(CRTPPacketTEST, Pack_NoPadding_NoExt_NoMarker_AddCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    for (uint32_t i = 0; i < expect_csrcs.size(); ++i) {
        rtpPackPtr->AddCSRC(expect_csrcs[i]);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, nullptr, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_NoPadding_NoExt_NoMarker_UpdateCSRC
TEST_F(CRTPPacketTEST, Pack_NoPadding_NoExt_NoMarker_UpdateCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    for (uint32_t i = 0; i < expect_csrcs.size(); ++i) {
        rtpPackPtr->AddCSRC(expect_csrcs[i]);
    }

    vector<uint32_t> update_csrcs;
    for (uint32_t i = 0; i < expect_csrcs.size(); ++i) {
        update_csrcs.push_back((i + 2000));
    }

    for (uint32_t i = 0; i < expect_csrcs.size(); ++i) {
        rtpPackPtr->UpdateCSRC(i, update_csrcs[i]);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["GetCSRC"] = &update_csrcs;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, &expectResult, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_NoPadding_NoMarker_CSRC_SetExtension
TEST_F(CRTPPacketTEST, Pack_NoPadding_NoMarker_CSRC_SetExtension)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 +
                                sizeof(RTPHeaderExtension) + expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_NoPadding_Marker_CSRC_Extension
TEST_F(CRTPPacketTEST, Pack_NoPadding_Marker_CSRC_Extension)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) + expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxPack_Marker_CSRC_Extension_SetPadding
TEST_F(CRTPPacketTEST, Pack_Marker_CSRC_Extension_SetPadding)
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

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_Marker_CSRC_Extension_SetPadding
TEST_F(CRTPPacketTEST, Unpack_Marker_CSRC_Extension_SetPadding)
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;

    // Check member status
    CheckRTPMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_NoPadding_Marker_CSRC_Extension
TEST_F(CRTPPacketTEST, Unpack_NoPadding_Marker_CSRC_Extension)
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;

    // Check member status
    CheckRTPMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_NoPadding_NoMarker_CSRC_Extension
TEST_F(CRTPPacketTEST, Unpack_NoPadding_NoMarker_CSRC_Extension)
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;

    // Check member status
    CheckRTPMember(&rtpUnpack, true, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_NoPadding_NoMarker_NoExt_CSRC
TEST_F(CRTPPacketTEST, Unpack_NoPadding_NoMarker_NoExt_CSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + expect_payloadLength;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;

    // Check member status
    CheckRTPMember(&rtpUnpack, true, &expectResult, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_NoPadding_NoMarker_NoExt_NoCSRC
TEST_F(CRTPPacketTEST, Unpack_NoPadding_NoMarker_NoExt_NoCSRC)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check member status
    CheckRTPMember(&rtpUnpack, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_Pack
TEST_F(CRTPPacketTEST, Unpack_Pack)
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // TODO: Change some element and then pack
    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);
    CCmMessageBlock mbUnpackPack;
    rv = rtpUnpack.Pack(mbUnpackPack);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_Pack_Nopayload
TEST_F(CRTPPacketTEST, Unpack_Pack_Nopayload)
{
    CScopedTracer test_info;
    // prepare test data
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // TODO: Change some element and then pack
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

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);
    CCmMessageBlock mbUnpackPack;
    rv = rtpUnpack.Pack(mbUnpackPack);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_Pack_TwoByteExt
TEST_F(CRTPPacketTEST, Unpack_Pack_TwoByteExt)
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
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // TODO: Change some element and then pack
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

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);
    CCmMessageBlock mbUnpackPack;
    rv = rtpUnpack.Pack(mbUnpackPack);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_Pack_TwoByteExt_OneBytePayload
TEST_F(CRTPPacketTEST, Unpack_Pack_TwoByteExt_OneBytePayload)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refOneBytePayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    // reset payload to detect whether has memory leak
    rtpPackPtr->SetPayload(*expect_refOneBytePayload);

    rtpPackPtr->SetPadding(expect_padding);
    rtpPackPtr->SetMarker();
    rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
    for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
        rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    // TODO: Change some element and then pack
    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                8*4 + 1 + expect_padding;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;
    uint32_t GetExtensionLength = 8;
    expectResult["GetExtensionLength"] = &GetExtensionLength;
    RTPExtensionProfileID GetExtensionProfileID;
    GetExtensionProfileID.id = RTP_TWO_BYTE_HEADER_EXTENSION.id;
    expectResult["GetExtensionProfileID"] = &GetExtensionProfileID;
    expectResult["GetPayloadLength"] = &expect_payloadOneByteLength;
    expectResult["GetPayloadData"] = &exptect_oneBytePayload;

    // Check member status
    CheckRTPMember(rtpPackPtr, true, &expectResult, true, true);
    CCmMessageBlock mbUnpackPack;
    rv = rtpUnpack.Pack(mbUnpackPack);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb, &expectResult, true, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

#define xxUnpack_SetPayload_Dont_DeleteMb
TEST_F(CRTPPacketTEST, Unpack_SetPayload_Dont_DeleteMb)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refOneBytePayload, RTP_TWO_BYTE_HEADER_EXTENSION);
    // reset payload to detect whether has memory error
    CCmMessageBlock tmpMb(1, (const char *)exptect_oneBytePayload, 0, 1);
    rtpPackPtr->SetPayload(tmpMb);
    ASSERT_NE((uint8_t *)tmpMb.GetTopLevelReadPtr(), rtpPackPtr->GetPayloadData());

    CCmMessageBlock packetMb;
    ASSERT_EQ(0, rtpPackPtr->Pack(packetMb));
    CCmMessageBlock tmpPacketMb(packetMb.GetTopLevelLength(), packetMb.GetTopLevelReadPtr(), 0, packetMb.GetTopLevelLength());
    CRTPPacket unpackPacket;
    ASSERT_EQ(0, unpackPacket.Unpack(tmpPacketMb));
    ASSERT_NE((uint8_t *)tmpPacketMb.GetTopLevelReadPtr()+RTP_FIX_HEADER_LENGTH, unpackPacket.GetPayloadData());

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

TEST_F(CRTPPacketTEST, CopyConstructNormal)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    CRTPPacket test(*rtpPackPtr);
    // Check member status
    CheckRTPMember(&test, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

TEST_F(CRTPPacketTEST, UnpackCopyConstructNormal)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
    CCmMessageBlock mb;
    uint32_t rv = rtpPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CRTPPacket rtpUnpack;
    rv = rtpUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPPacket test(rtpUnpack);
    // Check member status
    CheckRTPMember(&test, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

TEST_F(CRTPPacketTEST, CopyConstructInvalid)
{
    CScopedTracer test_info;
    CRTPPacket test;
    CRTPPacket test2(test);

    EXPECT_FALSE(test2.IsValid());
}

TEST_F(CRTPPacketTEST, AssignConstructNormal)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    CRTPPacket test = *rtpPackPtr;
    // Check member status
    CheckRTPMember(&test, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}

TEST_F(CRTPPacketTEST, AssignConstructInvalid)
{
    CScopedTracer test_info;
    CRTPPacket test;
    CRTPPacket test2(test);

    EXPECT_FALSE(test2.IsValid());
}

TEST_F(CRTPPacketTEST, AssignConstructSelf)
{
    CScopedTracer test_info;
    // prepare test data
    CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);

    CRTPPacket test = *rtpPackPtr;
    test = test;
    // Check member status
    CheckRTPMember(&test, true);

    delete rtpPackPtr;
    rtpPackPtr = nullptr;
}


TEST_F(CRTPPacketTEST, SequenceUtilTest)
{
    uint32_t a, b;
    a = b = 0;
    EXPECT_TRUE(isSequenceEQ(a, b));
    EXPECT_FALSE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_TRUE(isSequenceGE(a, b));
    EXPECT_FALSE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b),0);

    a = b = (0 - 1);
    EXPECT_TRUE(isSequenceEQ(a, b));
    EXPECT_FALSE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_TRUE(isSequenceGE(a, b));
    EXPECT_FALSE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b), 0);

    a = b = (uint32_t)((0 - 1) / 2);
    EXPECT_TRUE(isSequenceEQ(a, b));
    EXPECT_FALSE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_TRUE(isSequenceGE(a, b));
    EXPECT_FALSE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b), 0);


    a = 1;
    b = 2;
    EXPECT_FALSE(isSequenceEQ(a, b));
    EXPECT_TRUE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_FALSE(isSequenceGE(a, b));
    EXPECT_TRUE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b), -1);

    a = (0 - 1);
    b = 1;
    EXPECT_FALSE(isSequenceEQ(a, b));
    EXPECT_TRUE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_FALSE(isSequenceGE(a, b));
    EXPECT_TRUE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b), -2);

    a = (0 - 2);
    b = (0 - 1);
    EXPECT_FALSE(isSequenceEQ(a, b));
    EXPECT_TRUE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_FALSE(isSequenceGE(a, b));
    EXPECT_TRUE(isSequenceNE(a, b));

    a = (uint32_t)((0 - 1) / 2) - 1 ;
    b = (uint32_t)((0 - 1) / 2) + 1;
    EXPECT_FALSE(isSequenceEQ(a, b));
    EXPECT_TRUE(isSequenceLT(a, b));
    EXPECT_FALSE(isSequenceGT(a, b));
    EXPECT_TRUE(isSequenceLE(a, b));
    EXPECT_FALSE(isSequenceGE(a, b));
    EXPECT_TRUE(isSequenceNE(a, b));
    EXPECT_EQ(sequenceDiff<int32_t>(a, b), -2);

    uint16_t m = 65534;
    uint16_t n = 2;
    EXPECT_EQ(5, sequenceDiff<uint16_t>(n, m) + 1);
    EXPECT_EQ(4294901765, sequenceDiff<uint32_t>(n, m) + 1);

    m = 65535;
    n = 0;
    EXPECT_EQ(2, sequenceDiff<uint16_t>(n, m) + 1);
    EXPECT_EQ(4294901762, sequenceDiff<uint32_t>(n, m) + 1);
}


