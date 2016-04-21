#ifndef DEMO_PARAMETERS
#define DEMO_PARAMETERS

#define DEMO_MAX_TRACE_LEN		1024
#define PATH_MAX_CHAR_SIZE		1024

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <limits>

// wme headers
#include "WmeInterface.h"
#include "WmeEngine.h"
#include "WmeError.h"
#include "WmeEvent.h"
#include "WmeSession.h"
#include "WmeDataDump.h"

// util header
#include "CmUtil.h"
#include "CmMutex.h"
#include "CmMessageBlock.h"
#include "CmThread.h"
#include "CmThreadManager.h"


using namespace std;
using namespace wme;


#pragma warning(disable : 4002)


#define SAFE_DELETE(p) \
	if(p) {\
	delete p;\
	p = NULL;}

#define SAFE_RELEASE(p) \
	if(p) {\
	p->Release();\
	p = NULL;}

#define SAFE_INCREASE_REF(p) \
	if(p) {\
	p->IncreaseReference();}

#define SAFE_DECREASE_REF(p) \
	if(p) {\
	p->DecreaseReference();\
	p = NULL;}


typedef struct receiverResolution
{
	int width;
	int height;
}receiverResolution;

typedef enum
{
	DEMO_MEDIA_NONE		= 0,
	DEMO_MEDIA_AUDIO,
	DEMO_MEDIA_VIDEO,
	DEMO_MEDIA_DESKSHARE, // whsu
}DEMO_MEDIA_TYPE;

typedef enum
{
    DEMO_LOCAL_TRACK,
    DEMO_PREVIEW_TRACK,
    DEMO_REMOTE_TRACK,
    DEMO_LOCAL_DESKTOP_SHARE_TRACK, //whsu
    DEMO_REMOTE_DESKTOP_SHARE_TRACK,
}DEMO_TRACK_TYPE;

typedef enum
{
	DEV_TYPE_CAMERA,
	DEV_TYPE_MIC,
	DEV_TYPE_SPEAKER,
}DEMO_DEV_TYPE;


typedef enum{
	AUDIO_OUT_VOICE = 0,
	AUDIO_OUT_SPEAKER = 1,
}DEMO_AUDIO_OUT_TYPE;

typedef enum {
    DEV_POSITION_UNKNOWN = 0,
    DEV_CAMERA_FRONT = 1,
    DEV_CAMERA_BACK = 2,
    DEV_SPEAKER_BUILTIN = 3,
    DEV_SPEAKER_NON_BUILTIN = 4,
}DEMO_DEV_POSITION;

typedef enum {
    SOURCE_MIC = 0,
    SOURCE_CAMERA = 0,
    SOURCE_FILE = 1,
}DEMO_MEDIA_SOURCE;

typedef enum {
    TARGET_FILE = 0,
    TARGET_SCREEN = 1,
    TARGET_SPEAKER = 1,
}DEMO_MEDIA_TARGET;

#define MAX_DEV_NAME_LEN 260
typedef struct
{
	int dev_index;
	DEMO_DEV_TYPE dev_type;
	char unique_name[MAX_DEV_NAME_LEN];
	int	unique_name_len;
	char dev_name[MAX_DEV_NAME_LEN];
    int dev_name_len;
	IWmeMediaDevice *dev;
	int is_default_dev;
    DEMO_DEV_POSITION position;
}DeviceProperty;


///
/// codec name for audio
const char kWmeAudioCodecTag[][32]=
{
    "Unknown",
	"G711_ULAW",	///< audio codec G.711 uLaw
	"G711_ALAW",		///< audio codec G.711 aLaw
	"ILBC",			///< audio codec iLBC
	"OPUS",			///< audio codec Opus
	"G722",          ///< audio codec G.722
	"CNG",			///< audio codec CNG
};

///
/// codec name for video
const char kWmeVideoCodecTag[][32] = {
    "AVC",          ///< video codec AVC
    "SVC",			///< video codec SVC
    "HEVC",			///< video codec HEVC
    "VP8"			///< video codec VP8
};

///
/// for network socket port
#define DEMO_P2P_AUDIO_DATA_PORT			3456
#define DEMO_P2P_AUDIO_CRTL_PORT			3457

#define DEMO_P2P_VIDEO_DATA_PORT			4566
#define DEMO_P2P_VIDEO_CRTL_PORT			4567


#define DEMO_P2P_SCREEN_VIDEO_DATA_PORT			4576
#define DEMO_P2P_SCREEN_VIDEO_CRTL_PORT			4577

///
/// for video default max parameters
#define MAX_VIDEO_WIDTH				1280
#define MAX_VIDEO_HEIGHT			720
#define MAX_VIDEO_FPS				30
#define MAX_VIDEO_SPACIAL_LAYER     4


///
/// assert macro
#ifndef return_if_fail
#define _LOGP(p) CM_WARNING_TRACE("[ASSERT_WARN] file="<<__FILE__<<", func="<<__FUNCTION__<<", line="<<__LINE__<<", <"<<#p<<"> is false");
#define return_if_fail(p) do{if(!(p)) {_LOGP(p); return;}}while(0)
#define returnv_if_fail(p, v)   do{if(!(p)) {_LOGP(p); return (v);}} while(0)
#endif


///
/// default label for a/v track
const uint32_t kAudioLocalTrack_Label   = 10001;
const uint32_t kAudioRemoteTrack_Label  = 10002;
const uint32_t kVideoLocalTrack_Label   = 20001;
const uint32_t kVideoRemoteTrack_Label  = 20002;
const uint32_t kVideoPreviewTrack_Label = 20003;
const uint32_t kLocalScreenShareTrack_Label = 20004; // whsu
const uint32_t kRemoteScreenShareTrack_Label = 20005; // whsu


#ifdef WIN32
#define WM_MESSAGE_STATISTICS_WINDOW_CLOSE (WM_USER+400)
#define WM_MESSAGE_CLOSE_VIDEO_WINDOW (WM_USER+401)
#define WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE (WM_USER+402)
#define WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE (WM_USER+403)
#endif

#endif // DEMO_PARAMETERS
