#ifndef __FRAGMENTOP_H__
#define __FRAGMENTOP_H__
#include "wrtpmediaapi.h"

using namespace wrtp;

enum PayloadHeaderType {
    PHT_SINGLE      = 0,
    PHT_AGGREGATED  = 1,
    PHT_FU_START    = 2,
    PHT_FU_END      = 4,
    PHT_FU_MID      = 8,
};

///////////////////////////////////////////////////////
// Packet Format:
// Single:      PHT_SINGLE(1) | length(2) | data (x)
// Aggregated:  PHT_AGGREGATED (1) | count(2) | PHT_SINGLE(1) | length-1(2) | data-1(x) | PHT_SINGLE(1) | length-2(2) | data-2(y)
// FU_start:    PHT_FU_START(1) | length(2) | PHT_SINGLE(1) | length(2) | data(x)
// FU_end:      PHT_FU_END(1) | length(2) | data(x)
// FU_mid:      PHT_FU_MID(1) | length(2) | data(x)


#pragma pack(1)
struct RTPPayload {
    uint8_t type;
    union {
        uint16_t _length;  // length of of the packet: type + u + data
        uint16_t _count;   // NAL count contained in the aggregated packet
    } u;
    uint8_t data[1];
};
#pragma pack()

#define PAYLOAD_GET_LENGTH(p)       ( ntohs(((RTPPayload*)(p))->u._length) )
#define PAYLOAD_SET_LENGTH(p, v)    ( ((RTPPayload*)(p))->u._length = htons(v) )

#define PAYLOAD_GET_COUNT(p)        ( ntohs(((RTPPayload*)(p))->u._count) )
#define PAYLOAD_SET_COUNT(p, v)     ( ((RTPPayload*)(p))->u._count = htons(v) )

#define RTP_PAYLOAD_INIT(p, length)             \
    do {                                            \
        RTPPayload* packet = (RTPPayload*)p;        \
        packet->type = PHT_SINGLE;                  \
        PAYLOAD_SET_LENGTH(packet, length);         \
    } while (0)

#define RTP_PAYLOAD_HEADER_LEN 3

class CRTPPacketizationMock : public IPacketizationOperator
{
public:
    CRTPPacketizationMock() {}
    ~CRTPPacketizationMock() {}

    virtual RTPPayloadStructureType GetPayloadStructureType(const uint8_t *rawData, uint32_t length)
    {
        EXPECT_TRUE(rawData > 0);
        EXPECT_TRUE(length > 0);

        const RTPPayload *payload = reinterpret_cast<const RTPPayload *>(rawData);
        switch (payload->type) {
            case PHT_SINGLE: {
                return PAYLOAD_STRUCTURE_TYPE_SINGLE_NALU;
            }
            break;

            case PHT_AGGREGATED: {
                return PAYLOAD_STRUCTURE_TYPE_STAP_A;
            }
            break;

            case PHT_FU_START:
            case PHT_FU_END:
            case PHT_FU_MID: {
                return PAYLOAD_STRUCTURE_TYPE_FU_A;
            }
            break;

            default: {
                return PAYLOAD_STRUCTURE_TYPE_UNKNOWN;
            }
            break;
        }

        return PAYLOAD_STRUCTURE_TYPE_UNKNOWN;
    }

    // assume all the parameters are valid
    // Aggregated:  PHT_AGGREGATED (1) | count(2) | PHT_SINGLE(1) | length-1(2) | data-1(x) | PHT_SINGLE(1) | length-2(2) | data-2(y)
    virtual int32_t Aggregate(FragmentBuffer nals[], uint32_t count, uint8_t *outputBuffer, uint32_t &length)
    {
        EXPECT_TRUE(count > 1);
        EXPECT_TRUE(outputBuffer != NULL);
        EXPECT_GT(length, RTP_PAYLOAD_HEADER_LEN + count * RTP_PAYLOAD_HEADER_LEN);

        RTPPayload *aggPacket = reinterpret_cast<RTPPayload *>(outputBuffer);
        aggPacket->type     = PHT_AGGREGATED;
        PAYLOAD_SET_COUNT(aggPacket, count);

        uint8_t *todatap          = aggPacket->data;
        uint32_t remainingLength  = length - RTP_PAYLOAD_HEADER_LEN;
        for (uint32_t ii = 0; ii < count; ++ii) {
            FragmentBuffer &curNal  = nals[ii];
            uint32_t copyLength       = curNal.fragmentLength;
            EXPECT_GE(remainingLength, copyLength);

            memcpy(todatap, curNal.pBuffer, copyLength);
            todatap += copyLength;
            remainingLength -= copyLength;
        }

        // update the output length
        length = todatap - outputBuffer;

        return 0;
    }

    virtual uint32_t GetContainedNALCount(const uint8_t *rawData, uint32_t length)
    {
        RTPPayload *packet = reinterpret_cast<RTPPayload *>(const_cast<uint8_t *>(rawData));
        switch (packet->type) {
            case PHT_SINGLE: {
                return 1;
            }
            break;

            case PHT_AGGREGATED: {
                return PAYLOAD_GET_COUNT(packet);
            }
            break;

            default: {
                return 0;
            }
            break;
        }

        return 0;
    }

    // Aggregated:  PHT_AGGREGATED (1) | count(2) | PHT_SINGLE(1) | length-1(2) | data-1(x) | PHT_SINGLE(1) | length-2(2) | data-2(y)
    virtual int32_t Deaggregate(const uint8_t *rawData, uint32_t length, FragmentBuffer nals[], uint32_t count)
    {
        EXPECT_TRUE(rawData != NULL);
        RTPPayload *aggPacket = reinterpret_cast<RTPPayload *>(const_cast<uint8_t *>(rawData));
        EXPECT_EQ(count, PAYLOAD_GET_COUNT(aggPacket));

        uint8_t *datap = aggPacket->data;
        for (uint32_t ii = 0; ii < count; ++ii) {
            EXPECT_LT(datap, rawData + length);

            RTPPayload *curNal          = reinterpret_cast<RTPPayload *>(datap);
            FragmentBuffer &curBuffer   = nals[ii];
            uint16_t copyLength           = PAYLOAD_GET_LENGTH(curNal);

            EXPECT_LE(copyLength, curBuffer.bufferLength);

            memcpy(curBuffer.pBuffer, datap, copyLength);
            curBuffer.fragmentLength = copyLength;

            datap += copyLength;
        }

        EXPECT_EQ(length, datap - rawData);

        return 0;
    }

    virtual uint32_t GetFragmentCount(uint32_t totalLength, uint32_t maxFuSize)
    {
        uint32_t count = 0;
        int32_t remainLength = totalLength;
        while (remainLength > 0) {
            count += 1;

            remainLength -= (maxFuSize - RTP_PAYLOAD_HEADER_LEN);
        }

        return count;
    }

    // FU_start:    PHT_FU_START(1) | length(2) | PHT_SINGLE(1) | length(2) | data(x)
    // FU_end:      PHT_FU_END(1) | length(2) | data(x)
    // FU_mid:      PHT_FU_MID(1) | length(2) | data(x)
    virtual int32_t Fragment(RTP_IN uint8_t *rawData, uint32_t totalLength, RTP_INOUT FragmentBuffer fragments[], uint32_t count)
    {
        EXPECT_GT(count, 0);

        uint32_t fuDataLength = (totalLength % count == 0) ? (totalLength / count) : ((totalLength + count) / count);
        uint8_t *fromdatap = rawData;

        uint32_t remainLength = totalLength;
        for (uint32_t ii = 0; ii < count; ++ii) {
            RTPPayload *packet = reinterpret_cast<RTPPayload *>(fragments[ii].pBuffer);
            packet->type = (ii == 0) ? PHT_FU_START : ((ii == count-1) ? PHT_FU_END : PHT_FU_MID);

            uint32_t copyLength = (remainLength > fuDataLength) ? fuDataLength : remainLength;
            memcpy(packet->data, fromdatap, copyLength);

            PAYLOAD_SET_LENGTH(packet, copyLength + RTP_PAYLOAD_HEADER_LEN);
            fragments[ii].fragmentLength = copyLength + RTP_PAYLOAD_HEADER_LEN;

            fromdatap += copyLength;
            remainLength -= copyLength;
        }
        EXPECT_EQ(0, remainLength);


        return 0;
    }
    virtual bool IsStartFu(RTP_IN uint8_t *fragment, uint32_t length)
    {
        return !!(fragment[0] & PHT_FU_START);
    }
    virtual bool IsEndFu(RTP_IN uint8_t *fragment, uint32_t length)
    {
        return !!(fragment[0] & PHT_FU_END);
    }

    // FU_start:    PHT_FU_START(1) | length(2) | PHT_SINGLE(1) | length(2) | data(x)
    // FU_end:      PHT_FU_END(1) | length(2) | data(x)
    // FU_mid:      PHT_FU_MID(1) | length(2) | data(x)
    virtual int32_t Defragment(RTP_IN FragmentBuffer fragments[], uint32_t count, RTP_INOUT uint8_t *outputBuffer, RTP_INOUT uint32_t &length)
    {
        EXPECT_TRUE(count > 0);

        uint8_t *todatap = outputBuffer;

        for (uint32_t ii = 0; ii < count; ++ii) {
            FragmentBuffer &curBuffer = fragments[ii];
            uint8_t *fromdatap = curBuffer.pBuffer + RTP_PAYLOAD_HEADER_LEN;
            uint32_t copyLength = fragments[ii].fragmentLength - RTP_PAYLOAD_HEADER_LEN;
            memcpy(todatap, fromdatap, copyLength);

            todatap += copyLength;
        }

        length = todatap - outputBuffer;

        return 0;
    }

};


#endif
