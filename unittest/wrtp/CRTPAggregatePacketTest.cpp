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

class CRTPAggregatePacketTEST : public ::testing::Test
{
protected:
    typedef map<uint16_t, vector<uint8_t> > RTPExtensionMap;
    typedef map<string, void *> RTPParaMap;

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

        // For CRTPAggregatePacket Test
        expect_AggregatePacketLength = 3;
        for (uint8_t i = 0; i < 7; ++i) {
            // prepare test data
            CRTPPacket *rtpPackPtr = GetBasicRTPPacket(expect_sequence, expect_timestamp, expect_payloadType, expect_ssrc, *expect_refPayload);
            rtpPackPtr->SetPadding(expect_padding);
            rtpPackPtr->SetMarker();
            rtpPackPtr->SetCSRC(expect_csrcs.size(), &*expect_csrcs.begin());
            for (RTPExtensionMap::iterator it = expect_extensionMap.begin(); it != expect_extensionMap.end(); ++it) {
                rtpPackPtr->AddExtensionElement(it->first, &*(it->second.begin()), (uint8_t)it->first);
            }
            CCmMessageBlock mb;
            rtpPackPtr->Pack(mb);

            expect_AggregatePacketList.push_back(mb.DuplicateChained());
            delete rtpPackPtr;
            expect_PayloadMapping.push_back(111+i);
            expect_ActiveSteamCSIList.push_back(222+i);

            expect_AggregatePacketLength += 7+mb.GetTopLevelLength();
        }
    }

    virtual void TearDown()
    {
        expect_refPayload->DestroyChained();
        expect_refOneBytePayload->DestroyChained();
        for (uint8_t i = 0; i < 7; ++i) {
            expect_AggregatePacketList[i]->DestroyChained();
        }

    }

    void CheckCRTPAggregatePacketMember(CRTPAggregatePacket *aggregatePacketPtr,
                                        bool checkBasic = false, RTPParaMap *para = nullptr)
    {
        RTPParaMap expectResult;

        uint8_t GetVersion = WRTP_VERSION;
        uint32_t CalcPacketLength = 0;
        bool IsValid = false;
        vector<CCmMessageBlock *> GetAggregatePacketList;
        vector<uint8_t> GetPayloadMapping;
        vector<uint32_t> GetActiveSteamCSIList;
        bool HasAuthenticationFlag = false;
        uint8_t PayloadMappingFlag = 0;

        if (checkBasic) {
            CalcPacketLength = expect_AggregatePacketLength;
            IsValid = true;
            GetAggregatePacketList = expect_AggregatePacketList;
            GetPayloadMapping = expect_PayloadMapping;
            GetActiveSteamCSIList = expect_ActiveSteamCSIList;
            HasAuthenticationFlag = true;
            PayloadMappingFlag = 1;
        }

        expectResult["GetVersion"] = &GetVersion;
        expectResult["CalcPacketLength"] = &CalcPacketLength;
        expectResult["IsValid"] = &IsValid;
        expectResult["GetAggregatePacketList"] = &GetAggregatePacketList;
        expectResult["GetPayloadMapping"] = &GetPayloadMapping;
        expectResult["GetActiveSteamCSIList"] = &GetActiveSteamCSIList;
        expectResult["HasAuthenticationFlag"] = &HasAuthenticationFlag;
        expectResult["PayloadMappingFlag"] = &PayloadMappingFlag;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], aggregatePacketPtr->GetVersion());
        EXPECT_EQ(*(uint32_t *)expectResult["CalcPacketLength"], aggregatePacketPtr->CalcPacketLength());
        EXPECT_EQ(((vector<CCmMessageBlock *> *)expectResult["GetAggregatePacketList"])->size(), aggregatePacketPtr->GetAggregatePacketList().size());
        for (uint8_t i = 0; i < (uint8_t)(*(vector<CCmMessageBlock *> *)expectResult["GetAggregatePacketList"]).size();
                ++i) {
            memcmp((*(vector<CCmMessageBlock *> *)expectResult["GetAggregatePacketList"])[i]->GetTopLevelReadPtr(),
                   aggregatePacketPtr->GetAggregatePacketList()[i]->GetTopLevelReadPtr(),
                   (*(vector<CCmMessageBlock *> *)expectResult["GetAggregatePacketList"])[i]->GetTopLevelLength());
            EXPECT_EQ((*(vector<uint8_t> *)expectResult["GetPayloadMapping"])[i*(*(uint8_t *)expectResult["PayloadMappingFlag"])], aggregatePacketPtr->GetPayloadMapping()[i*(*(uint8_t *)expectResult["PayloadMappingFlag"])]);
            EXPECT_EQ((*(vector<uint32_t> *)expectResult["GetActiveSteamCSIList"])[i], aggregatePacketPtr->GetActiveSteamCSIList()[i]);
        }
        EXPECT_EQ((*(bool *)expectResult["HasAuthenticationFlag"]), aggregatePacketPtr->HasAuthenticationFlag());
        EXPECT_EQ(*(bool *)expectResult["IsValid"], aggregatePacketPtr->IsValid());
    }

    void CheckPackedCRTPAggregatePacketMember(CCmMessageBlock *rtpMb, RTPParaMap *para = nullptr)
    {
        RTPParaMap expectResult;

        uint8_t GetVersion = WRTP_VERSION;
        uint32_t CalcPacketLength = expect_AggregatePacketLength;
        vector<CCmMessageBlock *> GetAggregatePacketList = expect_AggregatePacketList;
        uint8_t NP = (uint8_t)GetAggregatePacketList.size();
        uint8_t HasAuthenticationFlag = 1;
        uint8_t PayloadMappingFlag = 1;
        vector<uint8_t> GetPayloadMapping = expect_PayloadMapping;
        vector<uint32_t> GetActiveSteamCSIList= expect_ActiveSteamCSIList;
        uint8_t AS = (uint8_t)expect_ActiveSteamCSIList.size();

        expectResult["GetVersion"] = &GetVersion;
        expectResult["CalcPacketLength"] = &CalcPacketLength;
        expectResult["GetAggregatePacketList"] = &GetAggregatePacketList;
        expectResult["NP"] = &NP;
        expectResult["HasAuthenticationFlag"] = &HasAuthenticationFlag;
        expectResult["PayloadMappingFlag"] = &PayloadMappingFlag;
        expectResult["GetPayloadMapping"] = &GetPayloadMapping;
        expectResult["GetActiveSteamCSIList"] = &GetActiveSteamCSIList;
        expectResult["AS"] = &AS;

        if (para) {
            for (RTPParaMap::iterator it = para->begin(); it != para->end(); ++it) {
                expectResult[it->first] = it->second;
            }
        }

        // Check packed value
        uint8_t *buff = (uint8_t *)rtpMb->GetTopLevelReadPtr();
        uint32_t buffLen = rtpMb->GetTopLevelLength();

        EXPECT_EQ(*(uint8_t *)expectResult["GetVersion"], (buff[0]&0xc0)>>6);
        EXPECT_EQ(*(uint32_t *)expectResult["CalcPacketLength"], buffLen);
        EXPECT_EQ(*(uint8_t *)expectResult["NP"], (buff[0]&0x38)>>3);
        EXPECT_EQ(*(uint8_t *)expectResult["HasAuthenticationFlag"], (buff[0]&0x04)>>2);
        EXPECT_EQ(*(uint8_t *)expectResult["PayloadMappingFlag"], (buff[0]&0x02)>>1);
        EXPECT_EQ(*(uint8_t *)expectResult["AS"], (buff[0]&0x01)<<2|((buff[1]&0xc0)>>6));
    }

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
            for (uint8_t i = 0; i < expect_csrcNumber; ++i) {
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
        if (hasExt) {
            expectResult["HasExtension"] = &expect_one;
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

    // For CRTPAggregatePacket Test
    uint32_t expect_AggregatePacketLength;
    vector<CCmMessageBlock *> expect_AggregatePacketList;
    vector<uint8_t> expect_PayloadMapping;
    vector<uint32_t> expect_ActiveSteamCSIList;

};

TEST_F(CRTPAggregatePacketTEST, Pack_NormalTest)
{
    CScopedTracer test_info;
    CRTPAggregatePacket rtpAggregatePacket;

    rtpAggregatePacket.SetAuthenticationFlag();
    for (uint8_t i = 0; i < 7; ++i) {
        rtpAggregatePacket.AddActiveStreamCSI(expect_ActiveSteamCSIList[i]);
        rtpAggregatePacket.AddAggregatePacket(*(expect_AggregatePacketList[i]));
        rtpAggregatePacket.AddPayloadMapping(expect_PayloadMapping[i]);
    }

    CheckCRTPAggregatePacketMember(&rtpAggregatePacket, true);

    CCmMessageBlock mb;
    uint32_t rv = rtpAggregatePacket.Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CheckPackedCRTPAggregatePacketMember(&mb);
}

TEST_F(CRTPAggregatePacketTEST, Pack_NotEnoughBufferTest)
{
    CScopedTracer test_info;
    CRTPAggregatePacket rtpAggregatePacket;

    rtpAggregatePacket.SetAuthenticationFlag();
    for (uint8_t i = 0; i < 7; ++i) {
        rtpAggregatePacket.AddActiveStreamCSI(expect_ActiveSteamCSIList[i]);
        rtpAggregatePacket.AddAggregatePacket(*(expect_AggregatePacketList[i]));
        rtpAggregatePacket.AddPayloadMapping(expect_PayloadMapping[i]);
    }

    CheckCRTPAggregatePacketMember(&rtpAggregatePacket, true);

    CCmMessageBlock mb(1);
    uint32_t rv = rtpAggregatePacket.Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_BUFFER_NOT_ENOUGH, rv);
}

TEST_F(CRTPAggregatePacketTEST, Unpack_NormalTest)
{
    CScopedTracer test_info;
    CRTPAggregatePacket rtpAggregatePacket;

    rtpAggregatePacket.SetAuthenticationFlag();
    for (uint8_t i = 0; i < 7; ++i) {
        rtpAggregatePacket.AddActiveStreamCSI(expect_ActiveSteamCSIList[i]);
        rtpAggregatePacket.AddAggregatePacket(*(expect_AggregatePacketList[i]));
        rtpAggregatePacket.AddPayloadMapping(expect_PayloadMapping[i]);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpAggregatePacket.Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);

    CRTPAggregatePacket rtpAggregatePacketUnpack;
    rv = rtpAggregatePacketUnpack.Unpack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    CheckCRTPAggregatePacketMember(&rtpAggregatePacketUnpack, true);
}

TEST_F(CRTPAggregatePacketTEST, Unpack_InvalidBufferTest)
{
    CScopedTracer test_info;
    CCmMessageBlock mb(1);
    CRTPAggregatePacket rtpAggregatePacket;

    EXPECT_EQ(WRTP_ERR_INVALID_BUFFER, rtpAggregatePacket.Unpack(mb));
    CheckCRTPAggregatePacketMember(&rtpAggregatePacket);
}

TEST_F(CRTPAggregatePacketTEST, Unpack_InvalidPacketTest)
{
    CScopedTracer test_info;
    CRTPAggregatePacket rtpAggregatePacket;

    rtpAggregatePacket.SetAuthenticationFlag();
    for (uint8_t i = 0; i < 7; ++i) {
        rtpAggregatePacket.AddActiveStreamCSI(expect_ActiveSteamCSIList[i]);
        rtpAggregatePacket.AddAggregatePacket(*(expect_AggregatePacketList[i]));
        rtpAggregatePacket.AddPayloadMapping(expect_PayloadMapping[i]);
    }

    CCmMessageBlock mb;
    uint32_t rv = rtpAggregatePacket.Pack(mb);
    ASSERT_EQ((uint32_t)WRTP_ERR_NOERR, rv);
    mb.RetreatTopLevelWritePtr(30);

    EXPECT_EQ(WRTP_ERR_INVALID_PACKAGE, rtpAggregatePacket.Unpack(mb));
}

TEST_F(CRTPAggregatePacketTEST, Unpack_InvalidVersionTest)
{
    CScopedTracer test_info;
    CRTPAggregatePacket rtpAggregatePacket;

    CCmMessageBlock mb(1000);
    mb.AdvanceTopLevelWritePtr(500);
    memset((void *)mb.GetTopLevelReadPtr(), 0 , 1000);

    EXPECT_EQ(WRTP_ERR_INVALID_VERSION, rtpAggregatePacket.Unpack(mb));
}

