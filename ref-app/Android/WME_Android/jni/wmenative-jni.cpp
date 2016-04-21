#include "wmenative-jni.h"

#include <jni.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

/// for log
#include <android/log.h>
#define TAG "WMENATIVE_JNI"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

/// for wme package
#ifdef ENABLE_WME
#include "DemoClient.h"

class DemoClientSink;

/// global client and sink
static DemoClient *gClient = NULL;
static DemoClientSink *gSink = NULL;

#define VIDEO_IN    0
#define AUDIO_IN    1
#define AUDIO_OUT   2
#define DEV_NUM     3

/// global devices and capbilities
DemoClient::DevicePropertyList gAVDevices[DEV_NUM]; // video in/audio in/audio out
DemoClient::AudioMediaCapabilityList gAudioCapabilities;
DemoClient::VideoMediaCapabilityList gVideoCapabilities;
vector<DemoClient::DeviceCapabilityList> gVideoCaptureParamVec(2); //front&back two cameras
	
static JavaVM* s_jvm = NULL;
static jclass s_statisticClass = NULL;

/// for client sink of network
class DemoClientSink : public backUISink
{
public:
    explicit DemoClientSink()
    {
#ifdef CUCUMBER_ENABLE
    m_pOutputAudioFileName = 0;
    m_pAudioRefFileName = 0;
    m_nAudioCheck = -1;
#endif
    }
    
	virtual void networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)
    {
		LOGI("DemoClientSink::networkDisconnectFromClient called");
#ifdef CUCUMBER_ENABLE
		LOGI("gClient->CheckAudioOutputFile called");
        //m_nAudioCheck = gClient->CheckAudioOutputFile( m_pOutputAudioFileName,  "/sdcard/fem1_1_16000_16_abc.pcm");
#endif
    }
    
	virtual void connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType)
    {
        // TODO: notify ui by JNI
    }
    
	virtual void OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent)
    {
        // nop for android
    }
    virtual void OnVolumeChange(WmeEventDataVolumeChanged &changeEvent)
    {
        // nop for android
    }

    virtual void OnEndOfStream(DEMO_MEDIA_TYPE mtype) 
    {
        LOGI("DemoClientSink::OnEndOfStream called");
#ifdef CUCUMBER_ENABLE
        gClient->DisConnect(mtype);
#endif
        //TODO: callback to java, to disconnect TP
    }
public:
#ifdef CUCUMBER_ENABLE
    void SetAudioRefFileName(const char* pchFileName)
    {
    	if(!m_pAudioRefFileName)
    		m_pAudioRefFileName = new char[256];

    	strcpy(m_pAudioRefFileName, pchFileName);

    }
    void SetAudioOutputFileName(const char* pchFileName)
    {
    	if(!m_pOutputAudioFileName)
    		m_pOutputAudioFileName = new char[256];

    	strcpy(m_pOutputAudioFileName, pchFileName);// strlen(pchFileName);
    }
    int GetAudioCheckResult()
    {
    	return m_nAudioCheck;
    }
#endif
private:
#ifdef CUCUMBER_ENABLE
    char*	m_pOutputAudioFileName;
    char*	m_pAudioRefFileName;
    int 	m_nAudioCheck;
#endif
};

#endif // ENABLE_WME


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void * reserved)
{
	LOGI("wmenative-jni onload");
	s_jvm = jvm;
	JNIEnv* env = NULL;

	if(jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
  		return -1;
  	
  // get java statistic class
	if(NULL != s_statisticClass)
	{
		env->DeleteGlobalRef(s_statisticClass);
		s_statisticClass = NULL;
	}
	jclass statClass = env->FindClass("com/cisco/wmeAndroid/StatisticData");
	s_statisticClass = (jclass)env->NewGlobalRef(statClass);
  	
  	
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
	LOGI("wmenative-jni onunload");
	JNIEnv* env = NULL;
	if(jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
  		return;
	if(NULL != s_statisticClass)
	{
		env->DeleteGlobalRef(s_statisticClass);
		s_statisticClass = NULL;
	}
}

// java env interface
bool AttachToJavaThread(JNIEnv **env)
{
	int result = JNI_OK;

	if(NULL == s_jvm)
		return false;

	LOGI("call AttachToJavaThread start");

	result = s_jvm->GetEnv((void **)env, JNI_VERSION_1_2);
	if(result == JNI_EVERSION)
	{
	}
	else if(result == JNI_EDETACHED)
	{
#if 1
		JavaVMAttachArgs args;
		args.version = JNI_VERSION_1_6;
		args.name = NULL;
		args.group = NULL;
		result = s_jvm->AttachCurrentThread(env, &args);
#else
		result = s_jvm->AttachCurrentThread(env, NULL);
#endif
		if(!result)
		{
			LOGI("call AttachToJavaThread end, need attach, env = %d", env);
			return true;
		}
	}
	else if(result == JNI_OK)
	{
	}
	else
	{
	}
	LOGI("call AttachToJavaThread end, not attach, env = %d, result = %d",*env,result);


	return false;
}

void DetachFromJavaThread()
{
	if(NULL == s_jvm)
		return;

	int result = s_jvm->DetachCurrentThread();
	if(result)
	{
		LOGI("call DetachCurrentThread failed, result = %d", result);
	}
}


///
/// JNI Implementation
///

DEFINE_WMEJNI(jboolean, Init)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
    if (gClient) {
        LOGI("Init, init again");
        return true;
    }

    do {
        gSink = new DemoClientSink();
        if (!gSink) {
            LOGE("Init, fail to new gSink");
            break;
        }

        gClient = new DemoClient(gSink);
        if (!gClient) {
            LOGE("Init, fail to new gClient");
            break;
        }

        if (gClient->Init() != WME_S_OK) {
            LOGE("Init, gClient->Init failed");
            break;
        }   
    
        // for device
        if(gClient->GetDeviceList(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, gAVDevices[VIDEO_IN]) != WME_S_OK) {
            LOGE("Init, fail to get Video In devices(camera)");
            break;
        }
#if 0
        if(gClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, gAVDevices[AUDIO_IN]) != WME_S_OK) {
            LOGE("Init, fail to get Audio In devices(mic)");
            break;
        }
        if(gClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, gAVDevices[AUDIO_OUT]) != WME_S_OK) {
            LOGE("Init, fail to get Audio out devices(speaker)");
            break;
        }
#endif

        // for capabilities
        if(gClient->GetMediaCapabilities(DEMO_MEDIA_VIDEO, &gVideoCapabilities) != WME_S_OK) {
            LOGE("Init, fail to get Video Capabilities");
            break;
        }
        if(gClient->GetMediaCapabilities(DEMO_MEDIA_AUDIO, &gAudioCapabilities) != WME_S_OK) {
            LOGE("Init, fail to get Audio Capabilities");
            break;
        }

        LOGI("Init, success, camera num=%d, video cap num=%d, audio cap num=%d", 
                gAVDevices[VIDEO_IN].size(), gVideoCapabilities.size(), gAudioCapabilities.size());
        return true;
    } while(0);
#endif
    return false;
}

DEFINE_WMEJNI(void, Uninit)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
    if (gClient) {
		for (int i=0;i<gVideoCaptureParamVec.size();i++)
		{
			gClient->ClearDeviceCapabilities(DEV_TYPE_CAMERA, gVideoCaptureParamVec[i]);
		}

        gClient->UnInit();
    }

    SAFE_DELETE(gClient);
    SAFE_DELETE(gSink);

    for (int k=0; k < DEV_NUM; k++) {
        DemoClient::DevicePropertyList::iterator iter;
        for (iter=gAVDevices[k].begin(); iter != gAVDevices[k].end(); iter++) {
            if (iter->dev)
                iter->dev->Release();
        }
        gAVDevices[k].clear();
    }
    gVideoCapabilities.clear();
    gAudioCapabilities.clear();
#endif
}

DEFINE_WMEJNI(void, HeartBeat)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
    DemoClient::HeartBeat();
#endif
}

DEFINE_WMEJNI(void, InitTPThread)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
    DemoClient::InitTPThread();
#endif
}

DEFINE_WMEJNI(jobjectArray, GetMediaDevices)(JNIEnv* env, jclass clazz, jint mtype, jint dtype)
{
    jobjectArray result = NULL;

#ifdef ENABLE_WME
    if (!gClient)
        return NULL;

    int index = -1;
    if (mtype == DEMO_MEDIA_VIDEO) {
        index = VIDEO_IN;
    }else if (mtype == DEMO_MEDIA_AUDIO && dtype == DEV_TYPE_MIC) {
        index = AUDIO_IN;
    }else if (mtype == DEMO_MEDIA_AUDIO && dtype == DEV_TYPE_SPEAKER) {
        index = AUDIO_OUT;
    }else {
        LOGE("GetMediaDevices, invalid media type=%d, or dtype=%d", mtype, dtype);
        return NULL;
    }

    int size = gAVDevices[index].size();
	jstring emptyString = env->NewStringUTF("");
    result = env->NewObjectArray(size, env->FindClass("java/lang/String"), emptyString);
	env->DeleteLocalRef(emptyString);
    if (result == NULL) {
        LOGE("GetMediaDevices, fail to NewObjectArray with size=%d", size);
        return NULL;
    }

	jstring string;
    for (int k=0; k < size; k++) {
        char devstr[260] = "Unknown Device";
        DeviceProperty *dp = &(gAVDevices[index][k]);
        snprintf(devstr, 260, "%s", dp->dev_name);
		string = env->NewStringUTF(devstr);
        env->SetObjectArrayElement(result, k, string);
		env->DeleteLocalRef(string);
    }
#endif

    return result;
}

DEFINE_WMEJNI(jobjectArray, GetMediaCapabilities)(JNIEnv* env, jclass clazz, jint mtype)
{
    jobjectArray result = NULL;

#ifdef ENABLE_WME
    if (!gClient)
        return NULL;

    int size = 3;
    if (mtype == DEMO_MEDIA_VIDEO) {
        size = gVideoCapabilities.size();
    }else if (mtype == DEMO_MEDIA_AUDIO) {
        size = gAudioCapabilities.size();
    }else {
        LOGE("GetMediaCapabilities, invalid media type=%d", mtype);
        return NULL;
    }

	jstring emptyString = env->NewStringUTF("");
    result = env->NewObjectArray(size, env->FindClass("java/lang/String"), emptyString);
	env->DeleteLocalRef(emptyString);
    if (result == NULL) {
        LOGE("GetMediaCapabilities, fail to NewObjectArray with size=%d", size);
        return NULL;
    }

	jstring string;
    for (int k=0; k < size; k++) {
        char mcstr[260] = "Unknown Capability";
        if (mtype == DEMO_MEDIA_VIDEO) {
            WmeVideoMediaCapability *pVMC = &(gVideoCapabilities[k]);
            snprintf(mcstr, 260, "%s, %dx%d@%1.ffps", 
                kWmeVideoCodecTag[pVMC->eCodecType-WmeCodecType_AVC],
                (int)pVMC->width, (int)pVMC->height, 
                (float)pVMC->frame_layer[pVMC->frame_layer_number-1] * 1.0 / 100);
        }else if (mtype == DEMO_MEDIA_AUDIO){
            WmeAudioMediaCapability *pAMC = &(gAudioCapabilities[k]);
            snprintf(mcstr, 260, "%s, sample freq: %dKHZ, bitrate: %dKbps", 
                pAMC->stdname, pAMC->clockrate/1000, pAMC->rate/1000);
        }
		string = env->NewStringUTF(mcstr);
        env->SetObjectArrayElement(result, k, string);
		env->DeleteLocalRef(string);
    }
#endif

    return result;
}

DEFINE_WMEJNI(jobjectArray, GetCaptureParamList)(JNIEnv* env, jclass clazz, jint dtype, jint dev_index)
{
    jobjectArray result = NULL;

#ifdef ENABLE_WME
    if (!gClient)
        return NULL;

	if (dtype == DEV_TYPE_CAMERA)
	{
		if (dev_index >= 0 && dev_index < gAVDevices[VIDEO_IN].size()) {
			if (gVideoCaptureParamVec[dev_index].empty() == true)
			{
				DeviceProperty *pdp = &(gAVDevices[VIDEO_IN].at(dev_index));
				gClient->GetDeviceCapabilities((DEMO_DEV_TYPE)dtype, pdp->dev, gVideoCaptureParamVec[dev_index]);
				LOGI("first get capture device capabilities, dev_index=%d, num=%d", dev_index, gVideoCaptureParamVec[dev_index].size());
			}
			DemoClient::DeviceCapabilityList * pList = &gVideoCaptureParamVec[dev_index];
			int siz = pList->size();

			jstring emptyString = env->NewStringUTF("");
			result = env->NewObjectArray(siz/*siz_t*/, env->FindClass("java/lang/String"), emptyString);
			env->DeleteLocalRef(emptyString);
			if (result == NULL) {
				LOGE("GetCaptureParamList, fail to NewObjectArray with size=%d", siz);
				return NULL;
			}

			char mcstr[64] = "Unknown";
			jstring string;
			for (int i=0;i<siz;i++)
			{
				WmeCameraCapability* pCCap = (WmeCameraCapability*)(pList->at(i)).pCapalibity;
				snprintf(mcstr, 64, "%dx%d@%dfps,cs%d",pCCap->width,pCCap->height,(int)pCCap->MaxFPS,(int)pCCap->type);
				string = env->NewStringUTF(mcstr);
				env->SetObjectArrayElement(result, i, string);
				env->DeleteLocalRef(string);
			}

			LOGI("GetCaptureParamList for camera OK, siz=%d, last sentense: %s",siz, mcstr);
        }
	}
	else {
		return NULL;	//not support
	}

#endif

    return result;
}

DEFINE_WMEJNI(void, SetCaptureParam)(JNIEnv* env, jclass clazz, jint mtype, jint dtype, jint dev_index, jint param_index)
{
#ifdef ENABLE_WME
	if (!gClient)
        return;

	if (dtype == DEV_TYPE_CAMERA)
	{
		if (dev_index >= 0 && dev_index < gAVDevices[VIDEO_IN].size()) {
			DemoClient::DeviceCapabilityList * pList = &gVideoCaptureParamVec[dev_index];
			if (param_index < pList->size())
			{
				long ret = 0;
				WmeDeviceCapability* pDCap = &pList->at(param_index);
				ret = gClient->SetCameraCapability((DEMO_TRACK_TYPE)mtype, pDCap);
				WmeCameraCapability* pCCap = (WmeCameraCapability*)pDCap->pCapalibity;
				if (ret == 0)
				{
					LOGI("SetCaptureParam for camera OK,track=%d,%dx%d,fps%d,cs%d",mtype,pCCap->width,pCCap->height,(int)pCCap->MaxFPS,(int)pCCap->type);
				}
				else{
					LOGE("SetCaptureParam for camera FAIL,track=%d,%dx%d,fps%d,cs%d",mtype,pCCap->width,pCCap->height,(int)pCCap->MaxFPS,(int)pCCap->type);
				}
			}
		}
	}

#endif
}

DEFINE_WMEJNI(jint, CreateMediaClient)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
	jint res = 0;
    if (gClient) {
        res = gClient->CreateMediaClient((DEMO_MEDIA_TYPE)mtype);
		if (res == 0)
		{
			LOGI("CreateMediaClient OK, mtype=%d",mtype);
		}
		else
		{
			LOGE("CreateMediaClient fail, mtype=%d",mtype);
		}
		
		return res;
    }
	return -1;
#endif
}

DEFINE_WMEJNI(jint, CreateMediaClientEx)(JNIEnv* env, jclass clazz, jint mtype, jint ttype)
{
#ifdef ENABLE_WME
    if (!gClient)
        return -1;
    jint res = gClient->CreateMediaClient((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype);
    return res;
#endif
}

DEFINE_WMEJNI(void, DeleteMediaClient)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->DeleteMediaClient((DEMO_MEDIA_TYPE)mtype);
    }
#endif
}

DEFINE_WMEJNI(void, DeleteMediaClientEx)(JNIEnv* env, jclass clazz, jint mtype, jint ttype)
{
#ifdef ENABLE_WME
    if (!gClient)
        return;
    gClient->DeleteMediaClient((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype);
#endif
}

DEFINE_WMEJNI(void, SetMediaDevice)(JNIEnv* env, jclass clazz, jint ttype, jint dtype, jint index)
{
#ifdef ENABLE_WME
    if (!gClient)
        return;

    if (dtype == DEV_TYPE_CAMERA) {
        if (index >= 0 && index < gAVDevices[VIDEO_IN].size()) {
            DeviceProperty *pdp = &(gAVDevices[VIDEO_IN].at(index));
            gClient->SetCamera((DEMO_TRACK_TYPE)ttype, pdp->dev);
        }
    }else if (dtype == DEV_TYPE_MIC) {
        if (index >= 0 && index < gAVDevices[AUDIO_IN].size()) {
            DeviceProperty *pdp = &(gAVDevices[AUDIO_IN].at(index));
            gClient->SetMic(pdp->dev);
        }
    }else if (dtype == DEV_TYPE_SPEAKER) {
        if (index >= 0 && index < gAVDevices[AUDIO_OUT].size()) {
            DeviceProperty *pdp = &(gAVDevices[AUDIO_OUT].at(index));
            gClient->SetSpeaker(pdp->dev);
        }
    }
#endif
}

DEFINE_WMEJNI(void, SetMediaCapability)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint index)
{
#ifdef ENABLE_WME
    if(!gClient)
        return;
    
    WmeMediaBaseCapability *cap = NULL;
    WmeCodecType codec = WmeCodecType_Unknown;

    if (mtype == DEMO_MEDIA_AUDIO) {
        if (ttype == DEMO_LOCAL_TRACK && index >= 0 && index < gAudioCapabilities.size())
        {
            cap = &(gAudioCapabilities.at(index));
        }
        else if (ttype == DEMO_REMOTE_TRACK)
        {
            codec = WmeCodecType_OPUS;
        }
    }else if (mtype == DEMO_MEDIA_VIDEO) {
        /*if (ttype == DEMO_LOCAL_TRACK && index >= 0 && index < gVideoCapabilities.size())
        {
            cap = &(gVideoCapabilities.at(index));
        }
        else if (ttype == DEMO_REMOTE_TRACK)
        {
            codec = WmeCodecType_SVC;
        }*/

		if (index >= 0 && index < gVideoCapabilities.size())
		{
			cap = &(gVideoCapabilities.at(index));
			
			if (cap)
			{
				WmeVideoMediaCapability* cap_debug = (WmeVideoMediaCapability*)cap;
				LOGI("JNI::SetMediaCapability, codectype=%d, w=%d, h=%d",cap_debug->eCodecType,cap_debug->width,cap_debug->height);
			}
		}
    }

    if (cap) {
		gClient->SetMediaCodec((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, cap->eCodecType);
        gClient->SetMediaCapability((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, cap);
        gClient->SetMediaSession((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, cap->eCodecType);
    }else if (codec != WmeCodecType_Unknown) {
        gClient->SetMediaSession((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, codec);
    }
#endif
}

DEFINE_WMEJNI(void, SetMediaCodec)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint ctype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->SetMediaCodec((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, (wme::WmeCodecType)ctype);
    }
#endif
}

DEFINE_WMEJNI(void, SetSessionCodec)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint ctype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->SetMediaSession((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype, (wme::WmeCodecType)ctype);
    }
#endif
}

DEFINE_WMEJNI(void, SetVideoQuality)(JNIEnv* env, jclass clazz, jint ttype, jint quality)
{
#ifdef ENABLE_WME
	if (gClient) {
		LOGI("JNI::SetVideoQuality");
		gClient->SetVideoQuality((DEMO_TRACK_TYPE)ttype, (wme::WmeVideoQualityType)quality);
	}
#endif
}

DEFINE_WMEJNI(jint, GetAudioVolume)(JNIEnv* env, jclass clazz, jint dtype)
{
    int nVolume = 0;
#ifdef ENABLE_WME
    if (gClient) {
        gClient->GetAudioVolume((DEMO_DEV_TYPE)dtype, nVolume);
    }
#endif
    return nVolume;
}

DEFINE_WMEJNI(void, SetAudioVolume)(JNIEnv* env, jclass clazz, jint dtype, jint volume)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->SetAudioVolume((DEMO_DEV_TYPE)dtype, volume);
    }
#endif
}

DEFINE_WMEJNI(jboolean, IsAudioMute)(JNIEnv* env, jclass clazz, jint dtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        return gClient->IsAudioMute((DEMO_DEV_TYPE)dtype);
    }
#endif
    return false;
}

DEFINE_WMEJNI(void, MuteAudio)(JNIEnv* env, jclass clazz, jint dtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->MuteAudio((DEMO_DEV_TYPE)dtype, true);
    }
#endif
}

DEFINE_WMEJNI(void, UnMuteAudio)(JNIEnv* env, jclass clazz, jint dtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->MuteAudio((DEMO_DEV_TYPE)dtype, false);
    }
#endif
}

DEFINE_WMEJNI(void, SetAudioOutType)(JNIEnv* env, jclass clazz, jint atype)
{
#ifdef ENABLE_WME
	if (gClient) {
		gClient->SetAudioOutType((DEMO_AUDIO_OUT_TYPE)atype);
	}
#endif
}

DEFINE_WMEJNI(void, SetRenderView)(JNIEnv* env, jclass clazz, jint ttype, jobject surface)
{
#ifdef ENABLE_WME
    if (gClient) {
        if (surface != NULL)
            gClient->SetRenderView((DEMO_TRACK_TYPE)ttype, (void *)surface);
        else
            gClient->StopRenderView((DEMO_TRACK_TYPE)ttype);
    }
#endif
}

DEFINE_WMEJNI(void, SetRenderAspectRatioSameWithSource)(JNIEnv* env, jclass clazz, jint ttype, jboolean keepsame)
{
#ifdef ENABLE_WME
	if (gClient) {
			gClient->SetRenderAspectRatioSameWithSource((DEMO_TRACK_TYPE)ttype, keepsame);
	}
#endif
}

DEFINE_WMEJNI(void, SetRenderMode)(JNIEnv* env, jclass clazz, jint ttype, jint mode)
{
#ifdef ENABLE_WME
	if (gClient) {
			gClient->SetRenderMode((DEMO_TRACK_TYPE)ttype, (WmeTrackRenderScalingModeType)mode);
	}
#endif
}

DEFINE_WMEJNI(void, StartMediaSending)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->StartMediaSending((DEMO_MEDIA_TYPE)mtype);
    }
#endif
}

DEFINE_WMEJNI(void, StopMediaSending)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->StopMediaSending((DEMO_MEDIA_TYPE)mtype);
    }
#endif
}

DEFINE_WMEJNI(void, InitHost)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->InitHost((DEMO_MEDIA_TYPE)mtype);
    }
#endif
}

DEFINE_WMEJNI(void, ConnectRemote)(JNIEnv* env, jclass clazz, jint mtype, jstring ipaddr)
{
#ifdef ENABLE_WME
    if (gClient) {
        const char *pIPAddr = env->GetStringUTFChars(ipaddr, NULL);
        gClient->ConnectRemote((DEMO_MEDIA_TYPE)mtype, (char *)pIPAddr);
        env->ReleaseStringUTFChars(ipaddr, pIPAddr);
    }
#endif
}

DEFINE_WMEJNI(void, InitHostIce)(JNIEnv* env, jclass clazz, jint mtype, jstring myname, 
    jstring jingleip, jint jingleport, jstring stunip, jint stunport)
{
#ifdef ENABLE_WME
    if (gClient) {
        const char *pMyName= env->GetStringUTFChars(myname, NULL);
        const char *pJingleIP = env->GetStringUTFChars(jingleip, NULL);
        const char *pStunIP = env->GetStringUTFChars(stunip, NULL);

        gClient->InitHost((DEMO_MEDIA_TYPE)mtype, pMyName, pJingleIP, jingleport, pStunIP, stunport);

        env->ReleaseStringUTFChars(myname, pMyName);
        env->ReleaseStringUTFChars(jingleip, pJingleIP);
        env->ReleaseStringUTFChars(stunip, pStunIP);
    }
#endif
}

DEFINE_WMEJNI(void, ConnectRemoteIce)(JNIEnv* env, jclass clazz, jint mtype, jstring myname, jstring hostname,
    jstring jingleip, jint jingleport, jstring stunip, jint stunport)
{
#ifdef ENABLE_WME
    if (gClient)
    {
        const char *pMyName= env->GetStringUTFChars(myname, NULL);
        const char *pHostName= env->GetStringUTFChars(hostname, NULL);
        const char *pJingleIP = env->GetStringUTFChars(jingleip, NULL);
        const char *pStunIP = env->GetStringUTFChars(stunip, NULL);

        gClient->ConnectRemote((DEMO_MEDIA_TYPE)mtype, pMyName, pHostName, pJingleIP, jingleport, pStunIP, stunport);

        env->ReleaseStringUTFChars(myname, pMyName);
        env->ReleaseStringUTFChars(myname, pHostName);
        env->ReleaseStringUTFChars(jingleip, pJingleIP);
        env->ReleaseStringUTFChars(stunip, pStunIP);
    }
#endif
}

DEFINE_WMEJNI(void, ConnectFile)(JNIEnv* env, jclass clazz, jint mtype, jstring filename,
	jstring sourceip, jint sourceport, jstring destinationip, jint destinationport)
{
#ifdef ENABLE_WME
    if (gClient)
    {
        const char *pFileName = env->GetStringUTFChars(filename, NULL);
        const char *pSourceIP = env->GetStringUTFChars(sourceip, NULL);
        const char *pDestinationIP = env->GetStringUTFChars(destinationip, NULL);

        gClient->ConnectFile((DEMO_MEDIA_TYPE)mtype, pFileName, pSourceIP, sourceport, pDestinationIP, destinationport);

        env->ReleaseStringUTFChars(filename, pFileName);
        env->ReleaseStringUTFChars(sourceip, pSourceIP);
        env->ReleaseStringUTFChars(destinationip, pDestinationIP);
    }
#endif
}

DEFINE_WMEJNI(void, DisConnect)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
    if (gClient) {
        gClient->DisConnect((DEMO_MEDIA_TYPE)mtype);
    }
#endif
}

DEFINE_WMEJNI(jint, StartMediaTrack)(JNIEnv* env, jclass clazz, jint mtype, jint ttype)
{
#ifdef ENABLE_WME
	long res = 0;
    if (gClient) {
        res = gClient->StartMediaTrack((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype);
		if (res == 0)
		{
			LOGI("StartMediaTrack OK, mtype=%d, ttype=%d",mtype,ttype);
		}
		else
		{
			LOGE("StartMediaTrack fail mtype=%d, ttype=%d",mtype,ttype);
		}
		
		return res;
    }
	return -1;
#endif
}

DEFINE_WMEJNI(jint, StopMediaTrack)(JNIEnv* env, jclass clazz, jint mtype, jint ttype)
{
#ifdef ENABLE_WME
	long res = 0;
    if (gClient) {
        res = gClient->StopMediaTrack((DEMO_MEDIA_TYPE)mtype, (DEMO_TRACK_TYPE)ttype);
		if (res == 0)
		{
			LOGI("StopMediaTrack OK, mtype=%d, ttype=%d",mtype,ttype);
		}
		else
		{
			LOGE("StopMediaTrack fail mtype=%d, ttype=%d",mtype,ttype);
		}

		return res;
    }
	return -1;
#endif
}

DEFINE_WMEJNI(jint, GetTrackStatistics)(JNIEnv* env, jclass clazz, jint mtype, jint ttype)
{
#ifdef ENABLE_WME
	if (!gClient) { return -1; }

	long lret = -1;
	switch(mtype) {
	case DEMO_MEDIA_VIDEO:{
		WmeVideoStatistics tstat;
		memset(&tstat, 0, sizeof(tstat));
		gClient->GetVideoStatistics((DEMO_TRACK_TYPE)ttype, tstat);
		
		if (ttype == DEMO_LOCAL_TRACK)
		{
			jclass cls = env->FindClass("com/cisco/wmeAndroid/StatisticData");
			jfieldID fid = env->GetStaticFieldID(cls, "videoEncodeMaxWidth", "I");
			env->SetStaticIntField(cls, fid, tstat.uWidth);
			fid = env->GetStaticFieldID(cls, "videoEncodeMaxHeight", "I");
			env->SetStaticIntField(cls, fid, tstat.uHeight);
			fid = env->GetStaticFieldID(cls, "videoEncodeMaxFPS", "I");
			env->SetStaticIntField(cls, fid, (jint)tstat.fFrameRate);
		}
		else if (ttype == DEMO_REMOTE_TRACK)
		{
			jclass cls = env->FindClass("com/cisco/wmeAndroid/StatisticData");
			jfieldID fid = env->GetStaticFieldID(cls, "videoDecodeWidth", "I");
			env->SetStaticIntField(cls, fid, tstat.uWidth);
			fid = env->GetStaticFieldID(cls, "videoDecodeHeight", "I");
			env->SetStaticIntField(cls, fid, tstat.uHeight);
			fid = env->GetStaticFieldID(cls, "videoDecodeFPS", "I");
			env->SetStaticIntField(cls, fid, (jint)tstat.fFrameRate);
		}
		
		CM_INFO_TRACE("GetTrackStatistics [video], ttype="<<ttype<<",width="<<tstat.uWidth<<", height="<<tstat.uHeight<<", framerate="<<tstat.fFrameRate
				<<", bitrate="<<tstat.fBitRate<<", average bitrate="<<tstat.fAvgBitRate);
		break;
	}
	case DEMO_MEDIA_AUDIO: 
		break;
	};
#endif
	return 0;
}

DEFINE_WMEJNI(jint, GetSessionStatistics)(JNIEnv* env, jclass clazz, jint mtype)
{
#ifdef ENABLE_WME
	if (!gClient) { return -1; }

	WmeSessionStatistics sstat;
	memset(&sstat, 0, sizeof(sstat));
	switch(mtype) {
	case DEMO_MEDIA_VIDEO:
		gClient->GetVideoStatistics(sstat);
		break;
	case DEMO_MEDIA_AUDIO: 
		gClient->GetAudioStatistics(sstat);
		break;
	};

	WmeNetworkStatistics &instat = sstat.stInNetworkStat;
	WmeNetworkStatistics &outstat = sstat.stOutNetworkStat;
	CM_INFO_TRACE("GetSessionStatistics [in], loss ratio="<<instat.fLossRatio<<", jitter="<<instat.uJitter<<", rtt="<<instat.uRoundTripTime
			<<", bytes="<<instat.uBytes<<", packets="<<instat.uPackets<<", bytesRate="<<instat.uBitRate);
	CM_INFO_TRACE("GetSessionStatistics [out], loss ratio="<<outstat.fLossRatio<<", jitter="<<outstat.uJitter<<", rtt="<<outstat.uRoundTripTime
			<<", bytes="<<outstat.uBytes<<", packets="<<outstat.uPackets<<", bytesRate="<<outstat.uBitRate);
#endif
	return 0;
}

DEFINE_WMEJNI(jint, SetDumpEnabled)(JNIEnv* env, jclass clazz, jint dflag)
{
#ifdef ENABLE_WME
	if (!gClient) { return -1; }
    gClient->SetDumpDataEnabled(dflag);
#endif
    return 0;
}

DEFINE_WMEJNI(jint, SetVideoInputFile)(JNIEnv* env, jclass clazz, jstring fname, jint width, jint height, jint fps, jint color)
{
    int iret = 0;
#ifdef ENABLE_WME
	if (!gClient) { return -1; }
    const char *pName= env->GetStringUTFChars(fname, NULL);
    WmeVideoRawFormat fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.iWidth = width;
    fmt.iHeight = height;
    fmt.eRawType = (WmeVideoRawType)color;
    fmt.fFrameRate = fps;
    iret = gClient->SetVideoInputFile(pName, &fmt);
    env->ReleaseStringUTFChars(fname, pName);
#endif
    return iret;
}

DEFINE_WMEJNI(void, SetVideoSource)(JNIEnv* env, jclass clazz, jint stype)
{
#ifdef ENABLE_WME
	if (!gClient) { return; }
    gClient->SetVideoSource(stype);
#endif
}

DEFINE_WMEJNI(jint, SetVideoOutputFile)(JNIEnv* env, jclass clazz, jstring fname)
{
      int iret = 0;
#ifdef ENABLE_WME
    if (!gClient) { return -1; }
    const char *pName= env->GetStringUTFChars(fname, NULL);
    iret = gClient->SetVideoOutputFile(pName);
    env->ReleaseStringUTFChars(fname, pName);
#endif
    return iret;  
}

DEFINE_WMEJNI(void, SetVideoTarget)(JNIEnv* env, jclass clazz, jint stype)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetVideoTarget(stype);
#endif
}

DEFINE_WMEJNI(jint, SetAudioInputFile)(JNIEnv* env, jclass clazz, jstring fname, jint channels, jint samplerate, jint bitspersample)
{
#ifdef ENABLE_WME
    if (!gClient) { return -1; }
    const char *pName= env->GetStringUTFChars(fname, NULL);
    WmeAudioRawFormat fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.iChannels = channels;
    fmt.iSampleRate = samplerate;
    fmt.iBitsPerSample = bitspersample;
    fmt.eRawType = WmePCM;


    // WBXWAVEFORMAT fmt;
    // memset(&fmt, 0, sizeof(fmt));
    // fmt.nChannels = channels;
    // fmt.nSamplesPerSec = samplerate;
    // fmt.wBitsPerSample = bitspersample;
    // fmt.wFormatTag = 1;
    // fmt.nAvgBytesPerSec = fmt.nChannels * fmt.nSamplesPerSec * (fmt.wBitsPerSample / 8);
    // fmt.nBlockAlign = (fmt.wBitsPerSample / 8) * fmt.nChannels;
    // fmt.dwSize = sizeof(WBXWAVEFORMAT);

    gClient->SetAudioInputFile(pName, &fmt);


#ifdef CUCUMBER_ENABLE
    if(!gSink)
    	return -1;
    gSink->SetAudioRefFileName(pName);
#endif
	env->ReleaseStringUTFChars(fname, pName);
#endif
}

DEFINE_WMEJNI(void, SetAudioSource)(JNIEnv* env, jclass clazz, jint stype)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetAudioSource(stype);
#endif
}

DEFINE_WMEJNI(jint, SetAudioOutputFile)(JNIEnv* env, jclass clazz, jstring fname)
{
#ifdef ENABLE_WME
    if (!gClient) { return -1; }
    const char *pName= env->GetStringUTFChars(fname, NULL);

    gClient->SetAudioOutputFile(pName);
#ifdef CUCUMBER_ENABLE
    if(!gSink)
    	return -1;
    gSink->SetAudioOutputFileName(pName);
#endif
    env->ReleaseStringUTFChars(fname, pName);
#endif
}

DEFINE_WMEJNI(void, SetAudioTarget)(JNIEnv* env, jclass clazz, jint stype)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetAudioTarget(stype);
#endif
}

DEFINE_WMEJNI(void, DisableSendingFilterFeedback)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
	if (!gClient) { return; }
    gClient->DisableSendingFilterFeedback();
#endif
}

DEFINE_WMEJNI(void, SetQoSMaxLossRatio)(JNIEnv* env, jclass clazz, jfloat maxLossRatio)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetQoSMaxLossRatio(maxLossRatio);
#endif
}

DEFINE_WMEJNI(void, SetQoSMinBandwidth)(JNIEnv* env, jclass clazz, jint minBandwidth)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetQoSMinBandwidth(minBandwidth);
#endif
}

DEFINE_WMEJNI(void, SetInitialBandwidth)(JNIEnv* env, jclass clazz, jint initBandwidth)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->SetInitialBandwidth(initBandwidth);
#endif
}

DEFINE_WMEJNI(void, EnableQoS)(JNIEnv* env, jclass clazz, jboolean enable)
{
#ifdef ENABLE_WME
    if (!gClient) { return; }
    gClient->EnableQoS(enable);
#endif
}

DEFINE_WMEJNI(float, CheckAudioOutputFile)(JNIEnv* env, jclass clazz, jstring strAudioPathName)
{
    float nRet = -1.0;
#ifdef ENABLE_WME
#ifdef CUCUMBER_ENABLE
	//if (!gSink) { return false; }
    const char *pName= env->GetStringUTFChars(strAudioPathName, NULL);

    CM_INFO_TRACE("CheckAudioOutputFile, pName = "<< pName);
    nRet = gClient->CheckAudioOutputFile( "/sdcard/audio_output.pcm", const_cast<char*>(pName));
    CM_INFO_TRACE("CheckAudioOutputFile, nRet = "<< nRet);
    env->ReleaseStringUTFChars(strAudioPathName, pName);
#endif
#endif
    return nRet;
}

DEFINE_WMEJNI(void, AddVideoFileRenderSink)(JNIEnv* env, jclass clazz)
{
#ifdef ENABLE_WME
#ifdef CUCUMBER_ENABLE
	if (!gClient) { return; }
    gClient->AddVideoFileRenderSink();
#endif
#endif
}


void javaSetVideoCaptureResolution_STAT(int width, int height)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoCaptureResolution_jni","(II)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, width, height);
			LOGI("javaSetVideoCaptureResolution_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

void javaSetVideoCaptureFPS_STAT(int fps)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoCaptureFPS_jni","(I)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, fps);
			LOGI("javaSetVideoCaptureFPS_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

void javaSetVideoEncodeResolution_STAT(int width, int height)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoEncodeMaxResolution_jni","(II)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, width, height);
			LOGI("javaSetVideoEncodeResolution_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

void javaSetVideoEncodeFPS_STAT(int fps)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoEncodeMaxFPS_jni","(I)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, fps);
			LOGI("javaSetVideoEncodeFPS_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

void javaSetVideoDecodeResolution_STAT(int width, int height)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoDecodeResolution_jni","(II)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, width, height);
			LOGI("javaSetVideoDecodeResolution_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

void javaSetVideoDecodeFPS_STAT(int fps)
{
	JNIEnv* env = NULL;
	int flag = 0;
	bool bAttach = AttachToJavaThread(&env);
	if(NULL != env)
	{
		if (s_statisticClass != NULL)
		{
			jmethodID getMethod = env->GetStaticMethodID(s_statisticClass,"setVideoDecodeFPS_jni","(I)V");
			env->CallStaticVoidMethod(s_statisticClass, getMethod, fps);
			LOGI("javaSetVideoDecodeFPS_STAT");
		}
	}
	if(true == bAttach)
	{
		DetachFromJavaThread();
	}
}

