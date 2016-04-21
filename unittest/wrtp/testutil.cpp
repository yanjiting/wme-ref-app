#include "testutil.h"
#include "rtpsessionconfig.h"
#include "rtpheaderext.h"
//#include "CmTimerQueueOrderedList.h"

#include <random>
#include <chrono>

/*
static void DumpMemoryToHexString(const uint8_t* memory, uint32_t length, char* hexBuffer, uint32_t hexBufferLength)
{
    if (!memory || !hexBuffer || hexBufferLength < length*2 + 1) {
        return;
    }

    uint32_t remainLength = hexBufferLength;
    for (const uint8_t* ptr = memory; ptr < memory + length; ++ptr) {
        int n = snprintf(hexBuffer, remainLength, "%02X", *ptr);
        hexBuffer       += n;
        remainLength    -= n;
    }
}*/

CFrameUnitUniquePtr ConstructFrameUnit(uint32_t channelId, uint32_t ssrc, uint8_t payloadType,
                                       uint32_t clockRate, uint8_t timeoffsetExtId, uint32_t captureTimestamp,
                                       uint32_t sampleTimestamp,uint8_t priority, RTPSessionContextSharedPtr &sessionConfig, CRTPChannel *channel /*= nullptr*/)
{
    sessionConfig->GetOutboundConfig()->RegisterRTPExtensionId(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetExtId);
    CMMFrameManager* mgr = sessionConfig->GetFrameManager();
    CFrameUnitUniquePtr frame(mgr->GetFrame(sessionConfig));
    auto& metaInfo = frame->GetMediaMetaInfo();
    metaInfo.SetChannelId(channelId);
    frame->SetSSRC(ssrc);
    frame->SetCloclRate(clockRate);
    metaInfo.SetPayloadType(payloadType);
    metaInfo.SetCaptureTickMS(captureTimestamp);
    metaInfo.SetRTPTimestamp(sampleTimestamp);
    metaInfo.SetPriority(priority);

    return frame;
}

CFrameUnitUniquePtr CreateFrameUnit(const char *message, uint32_t channelId, uint32_t ssrc, uint8_t payloadType,
                                    uint32_t clockRate, uint8_t timeoffsetExtId, uint32_t captureTimestamp,
                                    uint32_t sampleTimestamp,uint8_t priority, RTPSessionContextSharedPtr &sessionConfig,
                                    IFecScheme *fecScheme /*= nullptr*/, uint32_t fecSSRC /*= 0*/, CRTPChannel *channel /*= nullptr*/)
{
    CFrameUnitUniquePtr frame = ConstructFrameUnit(channelId,
                                ssrc,
                                payloadType,
                                clockRate,
                                timeoffsetExtId,
                                captureTimestamp,
                                sampleTimestamp,
                                priority,
                                sessionConfig);

    auto &nal = frame->CreateNAL();
    if (!nal) {
        return nullptr;
    }

    uint32_t dataLen = strlen(message) + 1;
    auto &fragments = nal->CreateFragment(dataLen, 1, sessionConfig->GetFrameManager());
    if (fragments.empty()) {
        return nullptr;
    }

    int32_t ret = fragments[0]->CopyDataIn(reinterpret_cast<const uint8_t *>(message), dataLen);
    if (0 != ret) {
        return nullptr;
    }

    fragments[0]->SetMarker(1);

    return frame;
}

CRTPPacket *GetBasicRTPPacket(uint16_t sequenceNumber, uint32_t timestamp, uint8_t payloadType, uint32_t ssrc, CCmMessageBlock &mb, RTPExtensionProfileID extProfileID/* = RTP_ONE_BYTE_HEADER_EXTENSION*/)
{
    CRTPPacket *rtpPackPtr = new CRTPPacket(extProfileID);
    rtpPackPtr->SetSequenceNumber(sequenceNumber);
    rtpPackPtr->SetTimestamp(timestamp);
    rtpPackPtr->SetPayloadType(payloadType);
    rtpPackPtr->SetSSRC(ssrc);
    rtpPackPtr->SetPayload(mb);

    return rtpPackPtr;
}

CRTPPacket *GetBasicNopayloadRTPPacket(uint16_t sequenceNumber, uint32_t timestamp, uint8_t payloadType, uint32_t ssrc, RTPExtensionProfileID extProfileID/* = RTP_ONE_BYTE_HEADER_EXTENSION*/)
{
    CRTPPacket *rtpPackPtr = new CRTPPacket(extProfileID);
    rtpPackPtr->SetSequenceNumber(sequenceNumber);
    rtpPackPtr->SetTimestamp(timestamp);
    rtpPackPtr->SetPayloadType(payloadType);
    rtpPackPtr->SetSSRC(ssrc);
    CCmMessageBlock mb(10);
    rtpPackPtr->SetPayload(mb);
    return rtpPackPtr;
}


void InitBuffer(uint8_t *buffer, uint32_t length)
{
    for (uint32_t ii = 0; ii < length; ++ii) {
        buffer[ii] = 'a' + (ii % 26);
    }

    return;
}

WRTPMediaData *CreateVoIPData(uint32_t timestamp, uint32_t sampleTimestamp, uint32_t dataLen, uint8_t marker/* = 1*/, CodecType codecType/* = CODEC_TYPE_AUDIO_TEST*/)
{
    WRTPMediaDataVoIP *md = new WRTPMediaDataVoIP();
    md->type            = MEDIA_DATA_VOIP;
    md->timestamp       = timestamp;
    md->codecType       = codecType;
    md->sampleTimestamp = sampleTimestamp;
    md->marker          = marker;
    md->size            = dataLen;
    md->data            = new uint8_t[dataLen];

    InitBuffer(md->data, md->size);

    return md;
}

WRTPMediaData *CreateVideoData(uint32_t timestamp, uint32_t sampleTimestamp, uint32_t dataLen, uint8_t DID, uint8_t marker/* = 1*/, CodecType codecType/* = CODEC_TYPE_VIDEO_TEST*/)
{
    WRTPMediaDataVideo *md = new WRTPMediaDataVideo();
    md->type            = MEDIA_DATA_VIDEO;
    md->timestamp       = timestamp;
    md->codecType       = codecType;
    md->sampleTimestamp = sampleTimestamp;
    md->marker          = marker;
    md->size            = dataLen;
    md->data            = new uint8_t[dataLen];
    md->DID             = DID;
    md->maxDID          = DID;

    InitBuffer(md->data, md->size);

    return md;
}

void DestroyMediaData(WRTPMediaData *md)
{
    if (md) {
        if (md->data) {
            delete [](md->data);
            md->data = nullptr;
        }

        delete md;
    }
}

CmResult CFakeEventQueue::PostEvent(ICmEvent *aEvent, EPriority aPri)
{
    CM_INFO_TRACE_THIS("CFakeEventQueue::PostEvent");

    aEvent->OnEventFire();
    aEvent->OnDestorySelf();
    return CM_OK;
}

CmResult CFakeThread::Stop(CCmTimeValue *aTimeout)
{
    CM_INFO_TRACE_THIS("CFakeThread::Stop");
    m_Stop = true;
    return CM_OK;
}

void CFakeThread::OnThreadRun()
{
    CM_INFO_TRACE_THIS("CFakeThread::OnThreadRun, enter");
    while (!m_Stop) {::SleepMs(10);}
    CM_INFO_TRACE_THIS("CFakeThread::OnThreadRun leave");
}

ICmEventQueue *CFakeThread::GetEventQueue()
{
    return &m_fpEventQueue;
}


CmResult CreateFakeThread(const char *name, ACmThread *&aThread, TFlag aFlag, BOOL bWithTimerQueue, TType aType)
{
    ACmThread *pThread = nullptr;
    pThread = new CFakeThread();

    pThread->Create(name , aType, aFlag, TRUE);

    aThread = pThread;
    return CM_OK;
}

int32_t WRTPGetCryptoRandom(uint8_t *buffer, uint32_t length)
{
    std::mt19937_64 randGen(std::chrono::system_clock::now().time_since_epoch().count());
    for (uint32_t ii = 0; ii < length; ) {
        uint64_t randVal = randGen();
        auto         ptr = reinterpret_cast<uint8_t*>(&randVal);
        for (auto jj = 0; ii < length && jj < 8; ++ii, ++jj) {
            buffer[ii] = ptr[jj];
        }
    }
    return 0;
}
