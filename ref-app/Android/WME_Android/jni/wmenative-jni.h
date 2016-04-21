#ifndef _WMENATIVE_JNI_H_
#define _WMENATIVE_JNI_H_

#include <jni.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void * reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved);

#ifndef DEFINE_WMEJNI
#define DEFINE_WMEJNI(r, f) EXTERNC JNIEXPORT r JNICALL Java_com_cisco_webex_wme_WmeNative_##f
#endif

void javaSetVideoCaptureResolution_STAT(int width, int height); 
void javaSetVideoCaptureFPS_STAT(int fps);
void javaSetVideoEncodeResolution_STAT(int width, int height);
void javaSetVideoEncodeFPS_STAT(int fps);
void javaSetVideoDecodeResolution_STAT(int width, int height);
void javaSetVideoDecodeFPS_STAT(int fps);

//
// Init and Uninit, once is enougth
DEFINE_WMEJNI(jboolean, Init)(JNIEnv* env, jclass clazz);
DEFINE_WMEJNI(void, Uninit)(JNIEnv* env, jclass clazz);
DEFINE_WMEJNI(void, HeartBeat)(JNIEnv* env, jclass clazz);
DEFINE_WMEJNI(void, InitTPThread)(JNIEnv* env, jclass clazz);

DEFINE_WMEJNI(jobjectArray, GetMediaDevices)(JNIEnv* env, jclass clazz, jint mtype, jint dtype);
DEFINE_WMEJNI(jobjectArray, GetMediaCapabilities)(JNIEnv* env, jclass clazz, jint mtype);

DEFINE_WMEJNI(jobjectArray, GetCaptureParamList)(JNIEnv* env, jclass clazz, jint dtype, jint dev_index);
DEFINE_WMEJNI(void, SetCaptureParam)(JNIEnv* env, jclass clazz, jint mtype, jint dtype, jint dev_index, jint param_index);

//
// Create/Destroy Media Client
DEFINE_WMEJNI(jint, CreateMediaClient)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(jint, CreateMediaClientEx)(JNIEnv* env, jclass clazz, jint mtype, jint ttype);
DEFINE_WMEJNI(void, DeleteMediaClient)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(void, DeleteMediaClientEx)(JNIEnv* env, jclass clazz, jint mtype, jint ttype);

//
// Set media device/capability/codec/session options
DEFINE_WMEJNI(void, SetMediaDevice)(JNIEnv* env, jclass clazz, jint ttype, jint dtype, jint index);
DEFINE_WMEJNI(void, SetMediaCapability)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint index);
DEFINE_WMEJNI(void, SetMediaCodec)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint ctype);
DEFINE_WMEJNI(void, SetSessionCodec)(JNIEnv* env, jclass clazz, jint mtype, jint ttype, jint ctype);
DEFINE_WMEJNI(void, SetVideoQuality)(JNIEnv* env, jclass clazz, jint ttype, jint quality);

//
// For audio volume of audio devices(mic/speaker)
DEFINE_WMEJNI(jint, GetAudioVolume)(JNIEnv* env, jclass clazz, jint dtype);
DEFINE_WMEJNI(void, SetAudioVolume)(JNIEnv* env, jclass clazz, jint dtype, jint volume);
DEFINE_WMEJNI(void, MuteAudio)(JNIEnv* env, jclass clazz, jint dtype);
DEFINE_WMEJNI(void, UnMuteAudio)(JNIEnv* env, jclass clazz, jint dtype);
DEFINE_WMEJNI(jboolean, IsAudioMute)(JNIEnv* env, jclass clazz, jint dtype);
DEFINE_WMEJNI(void, SetAudioOutType)(JNIEnv* env, jclass clazz, jint atype);

//
// Set render for local/preview/remote video track
DEFINE_WMEJNI(void, SetRenderView)(JNIEnv* env, jclass clazz, jint ttype, jobject surface);
DEFINE_WMEJNI(void, SetRenderAspectRatioSameWithSource)(JNIEnv* env, jclass clazz, jint ttype, jboolean keepsame);
DEFINE_WMEJNI(void, SetRenderMode)(JNIEnv* env, jclass clazz, jint ttype, jint mode);

//
// Start or Stop Session for Sending
DEFINE_WMEJNI(void, StartMediaSending)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(void, StopMediaSending)(JNIEnv* env, jclass clazz, jint mtype);

//
// Open network as role of server(host listening) or client (conect with remote) directly or by ice
DEFINE_WMEJNI(void, InitHost)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(void, ConnectRemote)(JNIEnv* env, jclass clazz, jint mtype, jstring ipaddr);
DEFINE_WMEJNI(void, InitHostIce)(JNIEnv* env, jclass clazz, jint mtype, jstring myname, 
    jstring jingleip, jint jingleport, jstring stunip, jint stunport);
DEFINE_WMEJNI(void, ConnectRemoteIce)(JNIEnv* env, jclass clazz, jint mtype, jstring myname, jstring hostname,
    jstring jingleip, jint jingleport, jstring stunip, jint stunport);
DEFINE_WMEJNI(void, DisConnect)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(void, DisableSendingFilterFeedback)(JNIEnv* env, jclass clazz);

//
// Start/Stop media track
DEFINE_WMEJNI(jint, StartMediaTrack)(JNIEnv* env, jclass clazz, jint mtype, jint ttype);
DEFINE_WMEJNI(jint, StopMediaTrack)(JNIEnv* env, jclass clazz, jint mtype, jint ttype);

//
// For Statistics
DEFINE_WMEJNI(jint, GetTrackStatistics)(JNIEnv* env, jclass clazz, jint mtype, jint ttype);
DEFINE_WMEJNI(jint, GetSessionStatistics)(JNIEnv* env, jclass clazz, jint mtype);
DEFINE_WMEJNI(jint, SetDumpEnabled)(JNIEnv* env, jclass clazz, jint dflag);

//
// For external track
DEFINE_WMEJNI(jint, SetVideoInputFile)(JNIEnv* env, jclass clazz, jstring fname, jint width, jint height, jint fps, jint color);
DEFINE_WMEJNI(void, SetVideoSource)(JNIEnv* env, jclass clazz, jint stype);

DEFINE_WMEJNI(jint, SetVideoOutputFile)(JNIEnv* env, jclass clazz, jstring fname);
DEFINE_WMEJNI(void, SetVideoTarget)(JNIEnv* env, jclass clazz, jint stype);

DEFINE_WMEJNI(jint, SetAudioInputFile)(JNIEnv* env, jclass clazz, jstring fname, jint channels, jint samplerate, jint bitspersample);
DEFINE_WMEJNI(void, SetAudioSource)(JNIEnv* env, jclass clazz, jint stype);

DEFINE_WMEJNI(jint, SetAudioOutputFile)(JNIEnv* env, jclass clazz, jstring fname);	////output wave format is fixed
DEFINE_WMEJNI(void, SetAudioTarget)(JNIEnv* env, jclass clazz, jint stype);

DEFINE_WMEJNI(void, SetQoSMaxLossRatio)(JNIEnv* env, jclass clazz, jfloat maxLossRatio);
DEFINE_WMEJNI(void, SetQoSMinBandwidth)(JNIEnv* env, jclass clazz, jint minBandwidth);
DEFINE_WMEJNI(void, SetInitialBandwidth)(JNIEnv* env, jclass clazz, jint initBandwidth);
DEFINE_WMEJNI(void, EnableQoS)(JNIEnv* env, jclass clazz, jboolean enable);

DEFINE_WMEJNI(float, CheckAudioOutputFile)(JNIEnv* env, jclass clazz, jstring strAudioPathName);
DEFINE_WMEJNI(void, AddVideoFileRenderSink)(JNIEnv* env, jclass clazz);

#endif // _WMENATIVE_JNI_H_

