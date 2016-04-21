#include "gmock/gmock.h"
#include "gtest/gtest.h"
//#include "wbxmock/wbxmock.h"
#include "CmMessageBlock.h"

#include <map>
#include <vector>
#include <string>

using namespace std;

#include "retransmissionpacket.h"
#include "testutil.h"

using namespace wrtp;

class CRetransmissionPacketTEST : public ::testing::Test
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
    void CheckRTPMember(CRetransmissionPacket *rtpRetranPackPtr, bool checkBasic = false, RTPParaMap *para = nullptr,
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

        uint16_t GetOriginalSequenceNumber = 0;
        uint32_t GetOriginalPayloadLength = 0;
        uint8_t *GetOriginalPayloadData = nullptr;

        if (checkBasic) {
            IsValid = expect_true;
            GetPayloadType = expect_payloadType;
            GetSequenceNumber = expect_sequence + 10;
            GetTimestamp = expect_timestamp;
            GetSSRC = expect_ssrc;
            GetCSRC = expect_csrcs;
            GetExtensionElement = expect_extensionMap;
            CalcPacketLength = sizeof(RTPHeader)+expect_payloadLength + 2;
            GetPayloadLength = expect_payloadLength;
            GetPayloadData = expect_payload;

            GetOriginalSequenceNumber = expect_sequence;
            GetOriginalPayloadLength = expect_payloadLength;
            GetOriginalPayloadData = expect_payload;
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

        expectResult["GetOriginalSequenceNumber"] = &GetOriginalSequenceNumber;
        expectResult["GetOriginalPayloadLength"] = &GetOriginalPayloadLength;
        expectResult["GetOriginalPayloadData"] = &GetOriginalPayloadData;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        EXPECT_EQ(*(bool *)expectResult["IsValid"], rtpRetranPackPtr->IsValid());
        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], rtpRetranPackPtr->GetVersion());
        EXPECT_EQ(*(bool *)expectResult["HasPadding"], rtpRetranPackPtr->HasPadding());
        EXPECT_EQ(*(bool *)expectResult["HasExtension"], rtpRetranPackPtr->HasExtension());
        EXPECT_EQ(*(uint8_t *)expectResult["GetCSRCCount"], rtpRetranPackPtr->GetCSRCCount());
        EXPECT_EQ(*(bool *)expectResult["HasMarker"], rtpRetranPackPtr->HasMarker());
        EXPECT_EQ(*(uint8_t *)expectResult["GetPayloadType"], rtpRetranPackPtr->GetPayloadType());
        EXPECT_EQ(*(uint16_t *)expectResult["GetSequenceNumber"], rtpRetranPackPtr->GetSequenceNumber());
        EXPECT_EQ(*(uint32_t *)expectResult["GetTimestamp"], rtpRetranPackPtr->GetTimestamp());
        EXPECT_EQ(*(uint32_t *)expectResult["GetSSRC"], rtpRetranPackPtr->GetSSRC());
        EXPECT_EQ((*(RTPExtensionProfileID *)expectResult["GetExtensionProfileID"]).id, rtpRetranPackPtr->GetExtensionProfileID().id);
        EXPECT_EQ(*(uint32_t *)expectResult["CalcPacketLength"], rtpRetranPackPtr->CalcPacketLength());
        EXPECT_EQ(*(uint32_t *)expectResult["GetPayloadLength"], rtpRetranPackPtr->GetPayloadLength());
        EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetPayloadData"], rtpRetranPackPtr->GetPayloadData(), rtpRetranPackPtr->GetPayloadLength()));

        EXPECT_EQ(*(uint16_t *)expectResult["GetOriginalSequenceNumber"], rtpRetranPackPtr->GetOriginalSequenceNumber());
        EXPECT_EQ(*(uint32_t *)expectResult["GetOriginalPayloadLength"], rtpRetranPackPtr->GetOriginalPayloadLength());
        EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetOriginalPayloadData"], rtpRetranPackPtr->GetOriginalPayloadData(), rtpRetranPackPtr->GetOriginalPayloadLength()));

        // check invalid csrc index, no matter what CSRC number is, it should OK
        EXPECT_EQ((uint32_t)0, rtpRetranPackPtr->GetCSRC(100));
        EXPECT_EQ(WRTP_ERR_INVALID_CSRC_INDEX, rtpRetranPackPtr->UpdateCSRC(100, 999));

        if (hasCSRC) {
            for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
                EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetCSRC"])[i], rtpRetranPackPtr->GetCSRC(i));
            }
        } else {
            // if has no csrc, update invalid csrc should return error
            EXPECT_EQ(WRTP_ERR_INVALID_CSRC_INDEX, rtpRetranPackPtr->UpdateCSRC(8, 999));
        }

        uint8_t *elementData = (uint8_t *)888;
        uint8_t elementLength = 222;
        uint32_t rv = 0;
        // not exist test
        rv = rtpRetranPackPtr->GetExtensionElement(999, elementData, elementLength);
        EXPECT_EQ(nullptr, elementData);
        EXPECT_EQ(0, elementLength);

        if (hasExt) {
            EXPECT_EQ((uint32_t)WRTP_ERR_INVALID_EXTENSION_ELEMENT_ID, rv);
            for (RTPExtensionMap::iterator it = (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).begin();
                    it != (*(RTPExtensionMap *)expectResult["GetExtensionElement"]).end(); ++it) {
                rv = rtpRetranPackPtr->GetExtensionElement(it->first, elementData, elementLength);
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
        if (hasExt) {
            expectResult["HasExtension"] = &expect_one;
        }
        expectResult["GetCSRCCount"] = &expect_zero;
        if (hasCSRC) {
            expectResult["GetCSRCCount"] = &expect_csrcNumber;
        }
        expectResult["HasMarker"] = &expect_zero;
        expectResult["GetPayloadType"] = &expect_payloadType;
        uint16_t newSeq = expect_sequence + 10;
        expectResult["GetSequenceNumber"] = &newSeq;
        expectResult["GetTimestamp"] = &expect_timestamp;
        expectResult["GetSSRC"] = &expect_ssrc;
        expectResult["GetCSRC"] = &expect_csrcs;
        expectResult["GetExtensionProfileID"] = &expect_extensionId;
        expectResult["GetExtensionElement"] = &expect_extensionMap;
        expectResult["GetPayloadLength"] = &expect_payloadLength;

        expectResult["GetOriginalSequenceNumber"] = &expect_sequence;
        expectResult["GetOriginalPayloadLength"] = &expect_payloadLength;
        expectResult["GetOriginalPayloadData"] = &expect_payload;

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
            for (uint32_t i = 0; i < expect_csrcNumber; ++i) {
                EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetCSRC"])[i], ntohl(rtpHeader->csrcs[i]));
            }
        }

        if (hasExt) {
            RTPHeaderExtension *headerExt = (RTPHeaderExtension *)(rtpHeader->csrcs + rtpHeader->csrcNumber);
            EXPECT_EQ((*(RTPExtensionProfileID *)expectResult["GetExtensionProfileID"]).id, headerExt->extid.id);
            EXPECT_EQ(expect_extensionLength, ntohs(headerExt->length));

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

            uint8_t *payload = headerExt->extension + ntohs(headerExt->length)*4;
            uint16_t originalSeq = ntohs(*(uint16_t *)payload);
            EXPECT_EQ(*(uint16_t *)expectResult["GetOriginalSequenceNumber"], originalSeq);
            // check payload value
            EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetOriginalPayloadData"], payload+2, *(uint32_t *)expectResult["GetOriginalPayloadLength"]));
        } else {
            uint8_t *payload = (uint8_t *)(rtpHeader->csrcs + rtpHeader->csrcNumber);
            uint16_t originalSeq = ntohs(*(uint16_t *)payload);
            EXPECT_EQ(*(uint16_t *)expectResult["GetOriginalSequenceNumber"], originalSeq);
            // check payload value
            EXPECT_EQ(0, memcmp(*(uint8_t **)expectResult["GetOriginalPayloadData"], payload+2, *(uint32_t *)expectResult["GetOriginalPayloadLength"]));
        }
    }

    // new seq = original + 10
    CRetransmissionPacket *GetBasicRetranRTPPacket()
    {
        CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
        CCmMessageBlock mb;
        rtpPackPtr->Pack(mb);

        CRetransmissionPacket *rtpRetranPackPtr = new CRetransmissionPacket(mb);
        rtpRetranPackPtr->SetSequenceNumber(expect_sequence + 10);

        delete rtpPackPtr;
        rtpPackPtr = nullptr;
        return rtpRetranPackPtr;
    }

    CRetransmissionPacket *GetBasicRetranRTPPacketOneBytePayload()
    {
        CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refOneBytePayload);
        CCmMessageBlock mb;
        rtpPackPtr->Pack(mb);

        CRetransmissionPacket *rtpRetranPackPtr = new CRetransmissionPacket(mb);
        rtpRetranPackPtr->SetSequenceNumber(expect_sequence + 10);

        delete rtpPackPtr;
        rtpPackPtr = nullptr;
        return rtpRetranPackPtr;
    }

public:
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

#define xxEmtpy_Package
TEST_F(CRetransmissionPacketTEST, Emtpy_Package)
{
    CScopedTracer test_info;
    CCmMessageBlock mb(2000);
    CRTPPacket rtpPacket;
    CRetransmissionPacket retranPacket;
    CRetransmissionPacket retranPacketInvalid(mb);

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, retranPacket.Pack(mb));
    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, retranPacket.Unpack(mb));
    // Check member status
    CheckRTPMember(&retranPacketInvalid);
}

#define xxPack_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRetransmissionPacketTEST, Pack_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRetransmissionPacket *rtpRetranPackPtr = GetBasicRetranRTPPacket();

    // Check member status
    CheckRTPMember(rtpRetranPackPtr, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpRetranPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb);

    delete rtpRetranPackPtr;
    rtpRetranPackPtr = nullptr;
}

#define xxPackOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc
TEST_F(CRetransmissionPacketTEST, PackOneBytePayload_NoPadding_NoExt_NoMarker_NoCsrc)
{
    CScopedTracer test_info;
    // prepare test data
    CRetransmissionPacket *rtpRetranPackPtr = GetBasicRetranRTPPacketOneBytePayload();

    uint32_t CalcPacketLength = sizeof(RTPHeader) + 2 + 1;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["GetPayloadLength"] = &expect_payloadOneByteLength;
    expectResult["GetPayloadData"] = &exptect_oneBytePayload;
    expectResult["GetOriginalPayloadLength"] = &expect_payloadOneByteLength;
    expectResult["GetOriginalPayloadData"] = &exptect_oneBytePayload;

    // Check member status
    CheckRTPMember(rtpRetranPackPtr, true, &expectResult);

    CCmMessageBlock mb;
    uint32_t rv = rtpRetranPackPtr->Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    CheckPackedMember(&mb, &expectResult);

    delete rtpRetranPackPtr;
    rtpRetranPackPtr = nullptr;
}

#define xxPack_Marker_CSRC_Extension_SetPadding
TEST_F(CRetransmissionPacketTEST, Pack_Marker_CSRC_Extension_SetPadding)
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
    delete rtpPackPtr;
    rtpPackPtr = nullptr;

    CRetransmissionPacket *rtpRetranPackPtr = new CRetransmissionPacket(mb);
    rtpRetranPackPtr->SetSequenceNumber(expect_sequence + 10);

    uint32_t CalcPacketLength = sizeof(RTPHeader) + expect_csrcNumber*4 + sizeof(RTPHeaderExtension) +
                                expect_extensionLength*4 + expect_payloadLength + expect_padding + 2;
    RTPParaMap expectResult;
    expectResult["CalcPacketLength"] = &CalcPacketLength;
    expectResult["HasMarker"] = &expect_true;
    expectResult["HasPadding"] = &expect_true;

    // Check member status
    CheckRTPMember(rtpRetranPackPtr, true, &expectResult, true, true);

    CCmMessageBlock mb2;
    rv = rtpRetranPackPtr->Pack(mb2);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    // Check packed value
    expectResult["HasMarker"] = &expect_one;
    expectResult["HasPadding"] = &expect_one;
    CheckPackedMember(&mb2, &expectResult, true, true);

    delete rtpRetranPackPtr;
    rtpRetranPackPtr = nullptr;
}

