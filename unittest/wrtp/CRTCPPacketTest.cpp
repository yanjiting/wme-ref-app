#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rtcppacket.h"
#include "testutil.h"

using namespace wrtp;

#ifdef _WIN32
    typedef struct _stat f_stat_t;
    #define f_stat       _stat
    #define f_remove     remove
    #define f_rename     rename
#else
    typedef struct stat f_stat_t;
    #define f_stat      stat
    #define f_remove    remove
    #define f_rename    rename
#endif

CCmMessageBlock *GetFileData(const char *fileName)
{
    FILE *fp = fopen(fileName, "rb");
    if (nullptr == fp) {
        return nullptr;
    };
    fseek(fp, 0, SEEK_END);
    uint32_t fileSize = ftell(fp);
    if (fileSize < 4 && fileSize > 10*1024*1024) {
        return nullptr;
    }
    CCmMessageBlock *mb = new CCmMessageBlock(fileSize);
    fseek(fp, 0, SEEK_SET);
    uint32_t len = fread((char *)mb->GetTopLevelReadPtr(), 1, fileSize, fp);
    fclose(fp);
    mb->AdvanceTopLevelWritePtr(len);
    return mb;
}

TEST(CRTCPPacketTest, SetRTCPHeader)
{
    CScopedTracer test_info;
    CRTCPPacket packet;
    RTCPHeader h;
    h.flag = 0x85;
    h.type = RTCP_PT_SR;
    h.length = 16;
    packet.SetRTCPHeader(h);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetCount(), 5);
    EXPECT_EQ(packet.GetType(), RTCP_PT_SR);
    EXPECT_EQ(packet.GetLength(), h.length);

    EXPECT_EQ(packet.CalcEncodeSize(), RTCP_HEADER_SIZE);
}

TEST(CSRPacketTest, EncodeAndDecode)
{
    CScopedTracer test_info;
    // test encode
    CSRPacket packet;
    packet.ssrc = 0x12345678;
    packet.srInfo.ntph = 0x96;
    packet.srInfo.ntpl = 0x5566;
    packet.srInfo.ts = 0x7788;
    packet.srInfo.count = 0x123;
    packet.srInfo.bytes = 0x556677;

    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetCount(), 6);
    EXPECT_EQ(packet.GetType(), RTCP_PT_SR);

    // test decode
    CCmMessageBlock *mb1 = GetFileData("srpacket.dat");
    if (nullptr == mb1) {
        return;
    }
    CCmByteStreamNetwork is(*mb1);
    RTCPHeader h;
    EXPECT_EQ(DecodeRTCPHeader(is, h), TRUE);
    CSRPacket packet1;
    packet1.SetRTCPHeader(h);
    EXPECT_EQ(packet1.Decode(is, *mb1), TRUE);
    EXPECT_EQ(packet1.GetVersion(), 2);
    EXPECT_EQ(packet1.GetCount(), 6);
    EXPECT_EQ(packet1.GetType(), RTCP_PT_SR);
    EXPECT_EQ(packet1.ssrc, 0x12345678);
    EXPECT_EQ(packet1.srInfo.ntph, 0x96);
    EXPECT_EQ(packet1.srInfo.ntpl, 0x5566);
    EXPECT_EQ(packet1.srInfo.ts, 0x7788);
    EXPECT_EQ(packet1.srInfo.count, 0x123);
    EXPECT_EQ(packet1.srInfo.bytes, 0x556677);
    EXPECT_EQ(packet1.rrBlocks.size(), 6);
    EXPECT_EQ(packet1.rrBlocks[5].ssrc, 0x3579);
    EXPECT_EQ(packet1.rrBlocks[5].flost, 0x21);
    EXPECT_EQ(packet1.rrBlocks[5].plost, 0x4567);
    EXPECT_EQ(packet1.rrBlocks[5].xseq, 0x23456);
    EXPECT_EQ(packet1.rrBlocks[5].jitter, 0x17);
    EXPECT_EQ(packet1.rrBlocks[5].lsr, 0x9876);
    EXPECT_EQ(packet1.rrBlocks[5].dlsr, 0x76543);
    //mb1->DestroyChained();
    delete mb1;
}

TEST(CRRPacketTest, EncodeAndDecode)
{
    CScopedTracer test_info;
    // test encode
    CRRPacket packet;
    packet.ssrc = 0x12345678;

    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);
    packet.rrBlocks.push_back(rrBlock);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetCount(), 6);
    EXPECT_EQ(packet.GetType(), RTCP_PT_RR);

    // test decode
    CCmMessageBlock *mb1 = GetFileData("rrpacket.dat");
    if (nullptr == mb1) {
        return;
    }
    CCmByteStreamNetwork is(*mb1);
    RTCPHeader h;
    EXPECT_EQ(DecodeRTCPHeader(is, h), TRUE);
    CRRPacket packet1;
    packet1.SetRTCPHeader(h);
    EXPECT_EQ(packet1.Decode(is, *mb1), TRUE);
    EXPECT_EQ(packet1.GetVersion(), 2);
    EXPECT_EQ(packet1.GetCount(), 6);
    EXPECT_EQ(packet1.GetType(), RTCP_PT_RR);
    EXPECT_EQ(packet1.ssrc, 0x12345678);
    EXPECT_EQ(packet1.rrBlocks.size(), 6);
    EXPECT_EQ(packet1.rrBlocks[5].ssrc, 0x3579);
    EXPECT_EQ(packet1.rrBlocks[5].flost, 0x21);
    EXPECT_EQ(packet1.rrBlocks[5].plost, 0x4567);
    EXPECT_EQ(packet1.rrBlocks[5].xseq, 0x23456);
    EXPECT_EQ(packet1.rrBlocks[5].jitter, 0x17);
    EXPECT_EQ(packet1.rrBlocks[5].lsr, 0x9876);
    EXPECT_EQ(packet1.rrBlocks[5].dlsr, 0x76543);
    //mb1->DestroyChained();
    delete mb1;
}

TEST(CSDESPacketTest, EncodeAndDecode)
{
    CScopedTracer test_info;
    // test encode
    CSDESPacket packet;

    SDESTrunk trunk;
    trunk.ssrc = 0x12345678;
    SDESItem item;
    item.type = 1;
    item.content = (uint8_t *)"cname_test";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);
    item.type = 2;
    item.content = (uint8_t *)"name_test";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);
    item.type = 3;
    item.content = (uint8_t *)"folkib@cisco.com";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);

    packet.sdesTrunks.push_back(trunk);
    packet.sdesTrunks.push_back(trunk);
    packet.sdesTrunks.push_back(trunk);
    packet.sdesTrunks.push_back(trunk);
    packet.sdesTrunks.push_back(trunk);
    packet.sdesTrunks.push_back(trunk);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetCount(), 6);
    EXPECT_EQ(packet.GetType(), RTCP_PT_SDES);

    // we haven't alloc memory for trunk items' content, so cleanup
    packet.sdesTrunks.clear();

    /*
        FILE* fp = fopen("sdespacket.dat", "wb");
        if(fp)
        {
            fwrite(mb.GetTopLevelReadPtr(), 1, mb.GetTopLevelLength(), fp);
            fclose(fp);
        }
    */
    // test decode
    CCmMessageBlock *mb1 = GetFileData("sdespacket.dat");
    if (nullptr == mb1) {
        return;
    }
    CCmByteStreamNetwork is(*mb1);
    RTCPHeader h;
    EXPECT_EQ(DecodeRTCPHeader(is, h), TRUE);
    CSDESPacket packet1;
    packet1.SetRTCPHeader(h);
    EXPECT_EQ(packet1.Decode(is, *mb1), TRUE);
    EXPECT_EQ(packet1.GetVersion(), 2);
    EXPECT_EQ(packet1.GetCount(), 6);
    EXPECT_EQ(packet1.GetType(), RTCP_PT_SDES);
    //mb1->DestroyChained();
    delete mb1;
}

TEST(CBYEPacketTest, EncodeAndDecode)
{
    CScopedTracer test_info;
    // test encode
    CBYEPacket packet;

    packet.ssrcs.push_back(0x12345678);
    packet.ssrcs.push_back(0x23456789);
    packet.ssrcs.push_back(0x34567890);
    packet.ssrcs.push_back(0x45678901);
    packet.ssrcs.push_back(0x56789012);
    packet.ssrcs.push_back(0x67890123);
    packet.reason = const_cast<char *>("I'm sad, so I leave");
    packet.reasonLength = strlen(packet.reason);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetCount(), 6);
    EXPECT_EQ(packet.GetType(), RTCP_PT_BYE);

    // we haven't alloc memory for BYE reason, so cleanup
    packet.reason = nullptr;
    packet.reasonLength = 0;

    /*
        FILE* fp = fopen("byepacket.dat", "wb");
        if(fp)
        {
            fwrite(mb.GetTopLevelReadPtr(), 1, mb.GetTopLevelLength(), fp);
            fclose(fp);
        }
    */
    // test decode
    CCmMessageBlock *mb1 = GetFileData("byepacket.dat");
    if (nullptr == mb1) {
        return;
    }
    CCmByteStreamNetwork is(*mb1);
    RTCPHeader h;
    EXPECT_EQ(DecodeRTCPHeader(is, h), TRUE);
    CBYEPacket packet1;
    packet1.SetRTCPHeader(h);
    EXPECT_EQ(packet1.Decode(is, *mb1), TRUE);
    EXPECT_EQ(packet1.GetVersion(), 2);
    EXPECT_EQ(packet1.GetCount(), 6);
    EXPECT_EQ(packet1.GetType(), RTCP_PT_BYE);
    //mb1->DestroyChained();
    delete mb1;
}

TEST(CCompoundPacket, Encode)
{
    CScopedTracer test_info;
    // test encode
    CCompoundPacket packet;

    CSRPacket packet01;
    packet01.ssrc = 0x12345678;
    packet01.srInfo.ntph = 0x96;
    packet01.srInfo.ntpl = 0x5566;
    packet01.srInfo.ts = 0x7788;
    packet01.srInfo.count = 0x123;
    packet01.srInfo.bytes = 0x556677;

    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);

    CSDESPacket packet02;
    SDESTrunk trunk;
    trunk.ssrc = 0x12345678;
    SDESItem item;
    item.type = 1;
    item.content = (uint8_t *)"cname_test";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);
    packet02.sdesTrunks.push_back(trunk);

    CBYEPacket packet03;
    packet03.ssrcs.push_back(0x12345678);
    packet03.ssrcs.push_back(0x23456789);
    packet03.ssrcs.push_back(0x34567890);
    packet03.ssrcs.push_back(0x45678901);
    packet03.ssrcs.push_back(0x56789012);
    packet03.ssrcs.push_back(0x67890123);
    packet03.reason = const_cast<char *>("I'm sad, so I leave");
    packet03.reasonLength = strlen(packet03.reason);

    packet.AddRtcpPacket(&packet01);
    packet.AddRtcpPacket(&packet02);
    packet.AddRtcpPacket(&packet03);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    // we haven't alloc memory for trunk items' content, so cleanup
    packet02.sdesTrunks.clear();
    packet03.reason = nullptr;
    packet03.reasonLength = 0;
    packet.packets.clear();
    /*
        FILE* fp = fopen("compundpacket.dat", "wb");
        if(fp)
        {
            fwrite(mb.GetTopLevelReadPtr(), 1, mb.GetTopLevelLength(), fp);
            fclose(fp);
        }
    */
    // test decode
    CCmMessageBlock *mb1 = GetFileData("compoundpacket.dat");
    if (nullptr == mb1) {
        return;
    }
    CCmByteStreamNetwork is(*mb1);
    RTCPHeader header;
    bool is_compound_packet = false;
    CCompoundPacket compound_packet;
    CRTCPPacket *packet1 = nullptr;
    do {
        packet1 = nullptr;
        uint32_t remain_length = mb1->GetChainedLength();
        if (!DecodeRTCPHeader(is, header)) {
            EXPECT_TRUE(false);
            break;
        }
        if (header.flag>>6 != 2) {
            // invalid packet
            EXPECT_TRUE(false);
            break;
        }
        uint32_t packet_length = (header.length + 1)<<2;
        if (packet_length > remain_length) {
            // invalid packet
            EXPECT_TRUE(false);
            break;
        }
        if (!is_compound_packet) {
            is_compound_packet = packet_length < remain_length;
        }
        switch (header.type) {
            case RTCP_PT_SR:
                packet1 = new CSRPacket();
                break;
            case RTCP_PT_RR:
                packet1 = new CRRPacket();
                break;
            case RTCP_PT_SDES:
                packet1 = new CSDESPacket();
                break;
            case RTCP_PT_BYE:
                packet1 = new CBYEPacket();
                break;
            case RTCP_PT_APP:
                packet1 = new CAPPPacket();
                break;
            case RTCP_PT_XR:
                packet1 = new CXRPacket();
                break;
            default:
                break;
        }
        if (nullptr == packet1) {
            EXPECT_TRUE(false);
            break;
        }
        packet1->AddReference();
        packet1->SetRTCPHeader(header);
        if (!packet1->Decode(is, *mb1)) {
            packet1->ReleaseReference();
            EXPECT_TRUE(false);
            break;
        }
        compound_packet.AddRtcpPacket(packet1);
    } while (is_compound_packet && mb1->GetChainedLength() > 0);

    EXPECT_EQ(compound_packet.packets.size(), 3);
    EXPECT_EQ(compound_packet.packets[0]->GetType(), RTCP_PT_SR);
    EXPECT_EQ(compound_packet.packets[1]->GetType(), RTCP_PT_SDES);
    EXPECT_EQ(compound_packet.packets[2]->GetType(), RTCP_PT_BYE);

    //mb1->DestroyChained();
    delete mb1;
}

TEST(CXRBlockLRLETest, EncodeAndDecode)
{
    CScopedTracer test_info;
    CXRBlockLRLE lrle;
    lrle.SetThinning(7);
    lrle.ssrc = 0x12345678;
    lrle.beginSeq = 0x1234;
    lrle.endSeq = 0x5678;
    lrle.chunks.push_back(0x1111);
    lrle.chunks.push_back(0x2222);
    lrle.chunks.push_back(0x3333);
    lrle.chunks.push_back(0x4444);
    lrle.chunks.push_back(0x5555);
    lrle.chunks.push_back(0x6666);
    lrle.chunks.push_back(0x7777);

    uint32_t encodeSize = lrle.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(lrle.Encode(os, mb), TRUE);


    // test decode
    CCmByteStreamNetwork is(mb);
    CXRBlock *block = DecodeXRBlock(is, mb);
    EXPECT_NE(block, (void *)nullptr);
    if (nullptr == block) {
        return;
    }
    EXPECT_EQ(block->GetType(), (uint8_t)XR_BLOCK_LOSS_RLE);
    CXRBlockLRLE *lrle1 = (CXRBlockLRLE *)block;
    EXPECT_EQ(lrle1->GetThinning(), 7);
    EXPECT_EQ(lrle1->ssrc, 0x12345678);
    EXPECT_EQ(lrle1->beginSeq, 0x1234);
    EXPECT_EQ(lrle1->endSeq, 0x5678);
    EXPECT_EQ(lrle1->chunks.size(), 7);
    EXPECT_EQ(lrle1->chunks[6], 0x7777);
    block->ReleaseReference();
}

TEST(CXRPacketTest, EncodeAndDecode)
{
    CScopedTracer test_info;
    // test encode
    CXRPacket packet;
    packet.ssrc = 0x12345678;

    CXRBlockLRLE lrle1;
    lrle1.SetThinning(7);
    lrle1.ssrc = 0x12345678;
    lrle1.beginSeq = 0x1234;
    lrle1.endSeq = 0x5678;
    lrle1.chunks.push_back(0x1111);
    lrle1.chunks.push_back(0x2222);
    lrle1.chunks.push_back(0x3333);
    lrle1.chunks.push_back(0x4444);
    lrle1.chunks.push_back(0x5555);
    lrle1.chunks.push_back(0x6666);
    lrle1.chunks.push_back(0x7777);

    CXRBlockLRLE lrle2;
    lrle2.SetThinning(6);
    lrle2.ssrc = 0x12345678;
    lrle2.beginSeq = 0x2345;
    lrle2.endSeq = 0x6789;
    lrle2.chunks.push_back(0x1111);
    lrle2.chunks.push_back(0x2222);
    lrle2.chunks.push_back(0x3333);
    lrle2.chunks.push_back(0x4444);
    lrle2.chunks.push_back(0x5555);
    lrle2.chunks.push_back(0x6666);
    lrle2.chunks.push_back(0x8888);

    packet.xrBlocks.push_back(&lrle1);
    packet.xrBlocks.push_back(&lrle2);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);
    EXPECT_EQ(packet.GetVersion(), 2);
    EXPECT_EQ(packet.GetType(), RTCP_PT_XR);

    // we haven't alloc memory for XR blocks, so cleanup
    packet.xrBlocks.clear();

    /*
        FILE* fp = fopen("sdespacket.dat", "wb");
        if(fp)
        {
            fwrite(mb.GetTopLevelReadPtr(), 1, mb.GetTopLevelLength(), fp);
            fclose(fp);
        }
    */
    // test decode
    CCmByteStreamNetwork is(mb);
    CRTCPPacket *p1 = DecodeRTCPPacket(is, mb);
    EXPECT_NE(p1, (void *)nullptr);
    if (nullptr == p1) {
        return;
    }
    EXPECT_EQ(p1->GetType(), RTCP_PT_XR);
    CXRPacket *packet1 = (CXRPacket *)p1;
    EXPECT_EQ(packet1->ssrc, 0x12345678);
    EXPECT_EQ(packet1->xrBlocks.size(), 2);

    CXRBlockLRLE *lrle3 = (CXRBlockLRLE *)packet1->xrBlocks[1];
    EXPECT_EQ(lrle3->GetType(), 1);
    EXPECT_EQ(lrle3->GetThinning(), 6);
    EXPECT_EQ(lrle3->ssrc, 0x12345678);
    EXPECT_EQ(lrle3->beginSeq, 0x2345);
    EXPECT_EQ(lrle3->endSeq, 0x6789);
    EXPECT_EQ(lrle3->chunks.size(), 7);
    EXPECT_EQ(lrle3->chunks[6], 0x8888);

    p1->ReleaseReference();
}

TEST(TMMBRN, EncodeAndDecode_1)
{
    CScopedTracer test_info;
    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = 11111;
    item.maxTBR = 2*1024*1024 + 424365;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    CCmByteStreamNetwork is(mb);
    CRTCPPacket *p1 = DecodeRTCPPacket(is, mb);
    ASSERT_NE(p1, (void *)nullptr);
    ASSERT_EQ(p1->GetType(), RTCP_PT_RTPFB);
    ASSERT_EQ(p1->GetLength(), 4);

    CRTPFBPacket *p2 = (CRTPFBPacket *)(p1);
    ASSERT_NE(p2, (void *)nullptr);
    ASSERT_EQ(p2->m_ssrc, 123456);
    ASSERT_EQ(p2->m_ssrcSrc, 0);
    ASSERT_EQ(p2->GetFeedbackMessageType(), (uint8_t)RTCP_RTPFB_TMMBR);
    ASSERT_EQ(p2->m_fciLength, 8);

    EXPECT_TRUE(!!DecodeTMMBRFCI(*p2, items));
    EXPECT_EQ(items[0].ssrc, item.ssrc);
    EXPECT_TRUE(items[0].maxTBR <= item.maxTBR);
    EXPECT_EQ(items[0].oh, item.oh);

    p1->ReleaseReference();
}

TEST(TMMBRN, EncodeAndDecode_n)
{
    CScopedTracer test_info;
    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item1;
    item1.ssrc = 11111;
    item1.maxTBR = 2*1024*1024 + 424365;
    item1.oh = 28;
    items.push_back(item1);
    TMMBRTuple item2;
    item2.ssrc = 22222;
    item2.maxTBR = 1024*1024*1024 + 424365;
    item2.oh = 28;
    items.push_back(item2);
    TMMBRTuple item3;
    item3.ssrc = 33333;
    item3.maxTBR = 99365;
    item3.oh = 28;
    items.push_back(item3);
    uint8_t itemCount = 3;
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, itemCount*8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    CCmByteStreamNetwork is(mb);
    CRTCPPacket *p1 = DecodeRTCPPacket(is, mb);
    ASSERT_NE(p1, (void *)nullptr);
    ASSERT_EQ(p1->GetType(), RTCP_PT_RTPFB);
    ASSERT_EQ(p1->GetLength(), 2 + 2 * itemCount);

    CRTPFBPacket *p2 = (CRTPFBPacket *)(p1);
    ASSERT_NE(p2, (void *)nullptr);
    ASSERT_EQ(p2->m_ssrc, 123456);
    ASSERT_EQ(p2->m_ssrcSrc, 0);
    ASSERT_EQ(p2->GetFeedbackMessageType(), (uint8_t)RTCP_RTPFB_TMMBR);
    ASSERT_EQ(p2->m_fciLength, itemCount*8);

    EXPECT_TRUE(!!DecodeTMMBRFCI(*p2, items));
    EXPECT_EQ(items[0].ssrc, item1.ssrc);
    EXPECT_TRUE(items[0].maxTBR <= item1.maxTBR);
    EXPECT_EQ(items[0].oh, item1.oh);
    EXPECT_EQ(items[1].ssrc, item2.ssrc);
    EXPECT_TRUE(items[1].maxTBR <= item2.maxTBR);
    EXPECT_EQ(items[1].oh, item2.oh);
    EXPECT_EQ(items[2].ssrc, item3.ssrc);
    EXPECT_TRUE(items[2].maxTBR == item3.maxTBR);
    EXPECT_EQ(items[2].oh, item3.oh);

    p1->ReleaseReference();
}

TEST(TMMBRN, Decode_Abnormal)
{
    CScopedTracer test_info;
    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = 11111;
    item.maxTBR = 2*1024*1024 + 424365;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    packet.m_fciLength -= 4;

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    CCmByteStreamNetwork is(mb);
    CRTCPPacket *p1 = DecodeRTCPPacket(is, mb);
    ASSERT_NE(p1, (void *)nullptr);
    ASSERT_EQ(p1->GetType(), RTCP_PT_RTPFB);
    ASSERT_EQ(p1->GetLength(), 3);

    CRTPFBPacket *p2 = (CRTPFBPacket *)(p1);
    ASSERT_NE(p2, (void *)nullptr);
    ASSERT_EQ(p2->m_ssrc, 123456);
    ASSERT_EQ(p2->m_ssrcSrc, 0);
    ASSERT_EQ(p2->GetFeedbackMessageType(), (uint8_t)RTCP_RTPFB_TMMBR);
    ASSERT_EQ(p2->m_fciLength, 4);

    items.clear();
    EXPECT_FALSE(!!DecodeTMMBRFCI(*p2, items));
    EXPECT_TRUE(items.empty());

    p1->ReleaseReference();
}

TEST(TMMBRN, DecodeTMMBRFCI_Normal)
{
    CScopedTracer test_info;
    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = 11111;
    item.maxTBR = 2*1024*1024 + 424365;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();
    packet.SetLength(4);
    EXPECT_TRUE(!!DecodeTMMBRFCI(packet, items));
    EXPECT_EQ(items[0].ssrc, item.ssrc);
    EXPECT_TRUE(items[0].maxTBR <= item.maxTBR);
    EXPECT_EQ(items[0].oh, item.oh);
}

TEST(TMMBRN, DecodeTMMBRFCI_small_maxTBR)
{
    CScopedTracer test_info;
    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = 11111;
    item.maxTBR = 43653;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();
    packet.SetLength(4);
    EXPECT_TRUE(!!DecodeTMMBRFCI(packet, items));
    EXPECT_EQ(items[0].ssrc, item.ssrc);
    EXPECT_EQ(items[0].maxTBR, item.maxTBR);
    EXPECT_EQ(items[0].oh, item.oh);
}

TEST(MARI, EncodeMARIFCI)
{
    CScopedTracer test_info;
    uint32_t ssrc = 2348123;
    uint32_t ssrcSrc = 92349345;
    float lossRatio = 0.13f;
    uint16_t qdelay = 35;
    uint32_t timestamp = 98238213;
    uint64_t receiveRate = 384234;
    CPSFBPacket packetMARI;
    BOOL ret = EncodePsfbMARIPacket(ssrc, ssrcSrc, lossRatio, qdelay, timestamp, receiveRate, packetMARI);
    ASSERT_TRUE(ret == TRUE);
    EXPECT_EQ(16, packetMARI.m_fciLength);
}

TEST(MARI, EncodeDecodeMARIFCI)
{
    CScopedTracer test_info;
    uint32_t ssrc = 2348123;
    uint32_t ssrcSrc = 92349345;
    float lossRatio = 0.13f;
    uint16_t qdelay = 35;
    uint32_t timestamp = 98238213;
    uint32_t receiveRate = 384234;
    CPSFBPacket packetMARI;
    BOOL ret = EncodePsfbMARIPacket(ssrc, ssrcSrc, lossRatio, qdelay, timestamp, receiveRate, packetMARI);
    ASSERT_TRUE(ret == TRUE);
    EXPECT_EQ(16, packetMARI.m_fciLength);

    // decode
    float lossRatio1 = 0.0f;
    uint16_t qdelay1 = 0;
    uint32_t timestamp1 = 0;
    uint64_t receiveRate1 = 0;
    ret = DecodePsfbMARIPacket(packetMARI, lossRatio1, qdelay1, timestamp1, receiveRate1);
    ASSERT_TRUE(ret == TRUE);
    EXPECT_EQ(uint8_t(lossRatio*256), uint8_t(lossRatio1*256));
    EXPECT_EQ(qdelay, qdelay1);
    EXPECT_EQ(timestamp, timestamp1);
    EXPECT_EQ(receiveRate, receiveRate1);
}


