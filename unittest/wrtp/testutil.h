
#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include "mmframemgr.h"
#include "wrtpmediaapi.h"
#include "rtcphandler.h"

#include "CmUtilMisc.h"
#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CmThreadTask.h"
#include "CmEventQueueBase.h"
#include "CmTimerQueueOrderedList.h"


class CCmTimerQueueBase;

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::StrictMock;

using namespace wrtp;
using namespace std;

#define CODEC_TYPE_AUDIO_TEST   101
#define PAYLOAD_TYPE_AUDIO_TEST 101
#define CODEC_TYPE_VIDEO_TEST   98
#define PAYLOAD_TYPE_VIDEO_TEST 98

#ifndef CM_LITTLE_ENDIAN
    #define CM_LITTLE_ENDIAN
#endif
typedef struct {
#if defined (CM_LITTLE_ENDIAN)
    uint8_t csrcNumber: 4;
    uint8_t extension: 1;
    uint8_t padding: 1;
    uint8_t version: 2;
    uint8_t payloadType: 7;
    uint8_t marker: 1;
#else
    uint8_t version: 2;
    uint8_t padding: 1;
    uint8_t extension: 1;
    uint8_t csrcNumber: 4;
    uint8_t marker: 1;
    uint8_t payloadType: 7;
#endif
    uint16_t sequence;
    uint32_t timestamp;
    uint32_t ssrc;
    uint32_t csrcs[];
} UnittestRTPHeader;

//static void DumpMemoryToHexString(const uint8_t* memory, uint32_t length, char* hexBuffer, uint32_t hexBufferLength);

CSendFramePtr ConstructFrameUnit(uint32_t channelId, uint32_t ssrc, uint8_t payloadType,
                                       uint32_t clockRate, uint8_t timeoffsetExtId, uint32_t captureTimestamp,
                                       uint32_t sampleTimestamp,uint8_t priority, RTPSessionContextSharedPtr &sessionConfig, CRTPChannel *channel = NULL);

CSendFramePtr CreateFrameUnit(const char *message, uint32_t channelId, uint32_t ssrc, uint8_t payloadType,
                                    uint32_t clockRate, uint8_t timeoffsetExtId, uint32_t captureTimestamp,
                                    uint32_t sampleTimestamp,uint8_t priority, RTPSessionContextSharedPtr &sessionConfig,
                                    IFecScheme *fecScheme = NULL, uint32_t fecSSRC = 0, CRTPChannel *channel = NULL);
CRTPPacket *GetBasicRTPPacket(uint16_t sequenceNumber, uint32_t timestamp, uint8_t payloadType, uint32_t ssrc, CCmMessageBlock &mb, RTPExtensionProfileID extProfileID = RTP_ONE_BYTE_HEADER_EXTENSION);
CRTPPacket *GetBasicNopayloadRTPPacket(uint16_t sequenceNumber, uint32_t timestamp, uint8_t payloadType, uint32_t ssrc, RTPExtensionProfileID extProfileID = RTP_ONE_BYTE_HEADER_EXTENSION);

void InitBuffer(uint8_t *buffer, uint32_t length);
WRTPMediaData *CreateVoIPData(uint32_t timestamp, uint32_t sampleTimestamp, uint32_t dataLen, uint8_t marker = 1, CodecType codecType = CODEC_TYPE_AUDIO_TEST);
WRTPMediaData *CreateVideoData(uint32_t timestamp, uint32_t sampleTimestamp, uint32_t dataLen, uint8_t DID, uint8_t marker = 1, CodecType codecType = CODEC_TYPE_VIDEO_TEST);
void DestroyMediaData(WRTPMediaData *md);


#define WRTP_TEST_WAIT_UNTIL(INTERVAL, TIMES, PREDICTION)           \
    do {                                                                \
        for (int ii = 0; ii <= (TIMES); ++ii) {                         \
            if (PREDICTION) { break; }                                  \
            ::SleepMs(INTERVAL);                                        \
        }                                                               \
        if (!(PREDICTION)) {                                            \
            WRTP_WARNTRACE("WRTP_TEST_WAIT_UNTIL: prediction fails!!"); \
        }                                                               \
        EXPECT_TRUE(PREDICTION);                                        \
    } while (0)

class CScopedTracer
{
public:
    CScopedTracer(string label = ""): m_label(label)
    {
        if ("" == label) {
            const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();


            m_label += test_info->test_case_name();
            m_label += ".";
            m_label += test_info->name();
        }
        WRTP_INFOTRACE("Entering "<< (m_label.empty() ? "..." : m_label));
    }

    ~CScopedTracer()
    {
        WRTP_INFOTRACE("Leaving  " << (m_label.empty() ? "..." : m_label));
    }

private:
    string m_label;
};

class CFakeEventQueue : public CCmEventQueueBase
{
public:
    CFakeEventQueue() {}
    virtual ~CFakeEventQueue() {}

    virtual CmResult PostEvent(ICmEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);

};

class CFakeThread : public CCmThreadTask
{
public:
    CFakeThread() { m_Stop = false;}
    virtual ~CFakeThread() {}

    //virtual void OnThreadInit();
    virtual CmResult Stop(CCmTimeValue *aTimeout = NULL);
    virtual void OnThreadRun();
    virtual ICmEventQueue *GetEventQueue();

protected:
    CFakeEventQueue m_fpEventQueue;
    bool m_Stop;
};

CmResult CreateFakeThread(const char *name, ACmThread *&aThread, TFlag aFlag, BOOL bWithTimerQueue, TType aType);

//INTERVAL could be used to simulate the timer interval when scheduled
#define WRTP_TEST_TRIGGER_ON_TIMER(INTERVAL, TIMES)                                 \
    do {                                                                                \
        ACmThread *thread = ::GetThread(TT_MAIN);                                       \
        if (!thread) {                                                                  \
            WRTP_ERRTRACE("WRTP_TEST_TRIGGER_ON_TIMER: no TT_MAIN thread");             \
            break;                                                                      \
        }                                                                               \
        CCmTimerQueueBase *timer_queue = (CCmTimerQueueBase *)thread->GetTimerQueue();  \
        if (!timer_queue) {                                                             \
            WRTP_ERRTRACE("WRTP_TEST_TRIGGER_ON_TIMER: no timer_queue");                \
            break;                                                                      \
        }                                                                               \
        for (int ii = 0; ii < (TIMES); ++ii) {                                          \
            ::SleepMs(INTERVAL);                                                        \
            timer_queue->CheckExpire();                                                 \
        }                                                                               \
    } while (0)



/** Generate a security random binary string, which can be used as KEY
 * Parameters:
 *  @Param[in] buffer: buffer to store the random string
 *  @Param[in] length: the length of the random string
 * Return value:
 *   WRTP_ERR_NOERR for success, others for failure
 */
 int32_t WRTPGetCryptoRandom(uint8_t *buffer, uint32_t length);

#endif // ~__TEST_UTIL_H__
