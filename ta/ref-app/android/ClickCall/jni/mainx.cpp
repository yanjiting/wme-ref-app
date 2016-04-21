#ifndef _GCOV_JNI_H_
#define _GCOV_JNI_H_

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <jni.h>

#include <string>
#include <android/log.h>
#include <cstdlib>

#include "wsclient.h"
#include "json/json.h"
#include "WmeInterface.h"
using namespace wme;
#include <sstream>
#include "testconfig.h"
#include "calliope_client.h"

TestConfig TestConfig::testConfig;

#ifndef TAG
#define TAG "ClickCall"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifndef DEFINE_CLICKCALLJNI
#define DEFINE_CLICKCALLJNI(r, f) EXTERNC JNIEXPORT r JNICALL Java_com_wx2_clickcall_NativeClickCall_##f
#endif

#ifndef DEFINE_WSCLIENTJNI
#define DEFINE_WSCLIENTJNI(r, f) EXTERNC JNIEXPORT r JNICALL Java_com_wx2_support_WSClient_##f
#endif

#ifndef DEFINE_CALLIOPEJNI
#define DEFINE_CALLIOPEJNI(r, f) EXTERNC JNIEXPORT r JNICALL Java_com_wx2_support_CalliopeClient_##f
#endif

extern "C" void __gcov_flush();

static void init_gcov()
{
#if defined(ENABLED_GCOV_FLAG)
    setenv("GCOV_PREFIX", "/sdcard/wme_gcov/", 1);
    setenv("GCOV_PREFIX_STRIP", "6", 1);
#endif
}

static void uninit_gcov()
{
#if defined(ENABLED_GCOV_FLAG)
    //__gcov_flush();
    exit(0);
#endif
}

DEFINE_CLICKCALLJNI(void, doGCov)(JNIEnv* env, jclass clazz, jint action) 
{
    LOGI("doGCov, action=%d", action);
    if (action == 0) {
        init_gcov();
    }else {
        uninit_gcov();
    }
}

DEFINE_CLICKCALLJNI(void, SetEnv)(JNIEnv* env, jclass clazz, jstring name, jstring value)
{
    const char* szName = env->GetStringUTFChars(name, NULL);
    const char* szValue = env->GetStringUTFChars(value, NULL);
    if(szName == NULL || szValue == NULL) {
        return ;
    }

    LOGV("DEFINE_CLICKCALLJNI::SetEnv, szName=%s, szValue=%s",szName,szValue);
    setenv(szName,szValue,true);
}

JavaVM *global_vm = NULL;
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("ClickCall, JNI_OnLoad");
    global_vm = vm;

    return JNI_VERSION_1_6;
}

class CWSClientSink : public ISignalHook {
public:
    CWSClientSink(JNIEnv* env, jobject pSink) {
        m_pWsClient = new CWsClient(this);
        m_pSink = env->NewGlobalRef(pSink);
        m_pEnv = env;
    }

    void detach(JNIEnv* env) {
        m_pEnv = NULL;
        if(m_pSink) {
            env->DeleteGlobalRef(m_pSink);
            m_pSink = NULL;
        }

        if(m_pWsClient) {
            delete m_pWsClient;
            m_pWsClient = NULL;
        }
    }

    virtual ~CWSClientSink() {
    }

    virtual void OnConnected() {
        jclass listenerCls = m_pEnv->GetObjectClass(m_pSink);
        jmethodID method = m_pEnv->GetMethodID(listenerCls, "onOpen", "()V");

        m_pEnv->CallVoidMethod(m_pSink, method);
    }

    virtual void OnStartCall(int nCount) {
        jclass listenerCls = m_pEnv->GetObjectClass(m_pSink);
        jmethodID method = m_pEnv->GetMethodID(listenerCls, "onStartCall", "(I)V");

        m_pEnv->CallVoidMethod(m_pSink, method, (jint)nCount);
    }

    virtual void OnSDPReceived(const std::string &type, const std::string &text) {
        jclass listenerCls = m_pEnv->GetObjectClass(m_pSink);
        jmethodID method = m_pEnv->GetMethodID(listenerCls, "onMessage", "(Ljava/lang/String;Ljava/lang/String;)V");

        jstring sType = m_pEnv->NewStringUTF(type.c_str());
        jstring sText = m_pEnv->NewStringUTF(text.c_str());

        m_pEnv->CallVoidMethod(m_pSink, method, sType, sText);
    }

    CWsClient* Get() {
        return m_pWsClient;
    }

private:
    CWsClient *m_pWsClient;
    jobject m_pSink;
    JNIEnv *m_pEnv;
};

DEFINE_WSCLIENTJNI(jlong, connect)(JNIEnv* env, jclass clazz, jstring sURI, jobject pSink)
{
    LOGI("wsclient::connect");
    if(sURI == NULL || pSink == NULL)
        return 0;

    const char *szURL = NULL;
    if(sURI)
        szURL = env->GetStringUTFChars(sURI, NULL);

    CWSClientSink *pWsClient = new CWSClientSink(env, pSink);
    pWsClient->Get()->Open(szURL);

    if(sURI && szURL)
        env->ReleaseStringUTFChars(sURI, szURL);

    return (jlong)pWsClient;
}

DEFINE_WSCLIENTJNI(jint, send)(JNIEnv* env, jclass clazz, jlong nativeRef, jstring type, jstring msg)
{
    LOGI("wsclient::send");
    const char *szMsg = NULL, *szType = NULL;
    if(!msg || !type || !nativeRef)
        return -1;

    szType = env->GetStringUTFChars(type, NULL);
    szMsg = env->GetStringUTFChars(msg, NULL);

    CM_ASSERTE_RETURN(szType != NULL && szMsg != NULL, -2);

    CWSClientSink* pWsClient = (CWSClientSink*)nativeRef;
    pWsClient->Get()->SendTo(szType, szMsg);

    env->ReleaseStringUTFChars(msg, szMsg);
    env->ReleaseStringUTFChars(type, szType);

    return 0;
}

DEFINE_WSCLIENTJNI(jint, close)(JNIEnv* env, jclass clazz, jlong nativeRef)
{
    LOGI("wsclient::close");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CWSClientSink* pWsClient = (CWSClientSink*)nativeRef;
    pWsClient->detach(env);
    delete pWsClient;

    return 0;
}

/////////////
class CCalliopeClientSink : public IOrpheusClientSink {
public:
    CCalliopeClientSink(JNIEnv* env, jobject pSink, const char *szTrackingID) {
        m_pCalliopeClient = new COrpheusClient(this, szTrackingID);
        m_pSink = env->NewGlobalRef(pSink);
        m_pEnv = env;
    }

    void detach(JNIEnv* env) {
        m_pEnv = NULL;
        if(m_pSink) {
            env->DeleteGlobalRef(m_pSink);
            m_pSink = NULL;
        }

        if(m_pCalliopeClient) {
            delete m_pCalliopeClient;
            m_pCalliopeClient = NULL;
        }
    }

    virtual ~CCalliopeClientSink() {
    }

    virtual void OnVenue(const string &venueUrl, COrpheusClient* pOrigin)
    {
        jclass listenerCls = m_pEnv->GetObjectClass(m_pSink);
        jmethodID method = m_pEnv->GetMethodID(listenerCls, "onVenue", "(Ljava/lang/String;)V");

        jstring sVenueUrl = m_pEnv->NewStringUTF(venueUrl.c_str());
        m_pEnv->CallVoidMethod(m_pSink, method, sVenueUrl);
    }

    virtual void OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin)
    {
        jclass listenerCls = m_pEnv->GetObjectClass(m_pSink);
        jmethodID method = m_pEnv->GetMethodID(listenerCls, "onConfluence", "(Ljava/lang/String;Ljava/lang/String;)V");

        jstring sSDP = m_pEnv->NewStringUTF(parameters.c_str());
        jstring sUrl = m_pEnv->NewStringUTF(url.c_str());
        m_pEnv->CallVoidMethod(m_pSink, method, sSDP, sUrl);
    }

    COrpheusClient* Get() {
        return m_pCalliopeClient;
    }

private:
    COrpheusClient *m_pCalliopeClient;
    jobject m_pSink;
    JNIEnv *m_pEnv;
};

DEFINE_CALLIOPEJNI(jint, destroy)(JNIEnv* env, jclass clazz, jlong nativeRef)
{
    LOGI("calliopeclient::destory");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    pCalliopeClient->detach(env);
    delete pCalliopeClient;

    return 0;
}

DEFINE_CALLIOPEJNI(jlong, create)(JNIEnv* env, jclass clazz, jstring trackingID, jobject callback)
{
    LOGI("calliopeclient::create");

    const char* szTrackingID = env->GetStringUTFChars(trackingID, NULL);

    LOGI("calliopeclient::create, %s", szTrackingID);
    CCalliopeClientSink* pCalliopeClient = new CCalliopeClientSink(env, callback, szTrackingID);

    if(szTrackingID) {
        env->ReleaseStringUTFChars(trackingID, szTrackingID);
    }
    return (jlong)pCalliopeClient;
}

DEFINE_CALLIOPEJNI(void, setUserPasswd)(JNIEnv* env, jclass clazz, jstring uid, jstring pwd)
{
    LOGI("calliopeclient::setUserPasswd");
    const char* szUsername = env->GetStringUTFChars(uid, NULL);
    const char* szPassword = env->GetStringUTFChars(pwd, NULL);

    if(szUsername != NULL) {
        TestConfig::i().m_sOAuthUID = szUsername;
        env->ReleaseStringUTFChars(uid, szUsername);
    }

    if(szPassword != NULL) {
        TestConfig::i().m_sOAuthPasswd = szPassword;
        env->ReleaseStringUTFChars(pwd, szPassword);
    }
}

DEFINE_CALLIOPEJNI(void, setLinus)(JNIEnv* env, jclass clazz, jstring linusUrl, jboolean bLocalLinus)
{
    LOGI("calliopeclient::setLinus");
    if(linusUrl != NULL) {
        const char* szLinusUrl = env->GetStringUTFChars(linusUrl, NULL);
        if(szLinusUrl) {
            TestConfig::i().m_sLinusUrl = szLinusUrl;
            env->ReleaseStringUTFChars(linusUrl, szLinusUrl);
        }
    }else {
        TestConfig::i().m_sLinusUrl = "";
    }
    TestConfig::i().m_bLocalLinus = (bool)bLocalLinus;
}

DEFINE_CALLIOPEJNI(jint, createVenue)(JNIEnv* env, jclass clazz, jlong nativeRef)
{
    LOGI("calliopeclient::createVenue");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    pCalliopeClient->Get()->CreateVenue();

    return 0;
}

DEFINE_CALLIOPEJNI(jint, requestFloor)(JNIEnv* env, jclass clazz, jlong nativeRef)
{
    LOGI("calliopeclient::requestFloor");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    pCalliopeClient->Get()->RequestFloor();

    return 0;
}

DEFINE_CALLIOPEJNI(jint, releaseFloor)(JNIEnv* env, jclass clazz, jlong nativeRef)
{
    LOGI("calliopeclient::releaseFloor");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    pCalliopeClient->Get()->ReleaseFloor();

    return 0;
}

DEFINE_CALLIOPEJNI(jint, createConfluence)(JNIEnv* env, jclass clazz, jlong nativeRef, jstring sVenue, jstring sSdp, jstring uuid)
{
    LOGI("calliopeclient::createConfluence");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    const char* szVenue = env->GetStringUTFChars(sVenue, NULL);
    const char* szSDP = env->GetStringUTFChars(sSdp, NULL);
    const char* szUUID = env->GetStringUTFChars(uuid, NULL);

    if(TestConfig::i().m_bLocalLinus) {
        pCalliopeClient->Get()->CreateLinusConfluence(TestConfig::i().m_sLinusUrl, szVenue, szSDP, szUUID);
    }else {
        pCalliopeClient->Get()->CreateFlow(szVenue, szSDP, szUUID);
    }

    if(szVenue) {
        env->ReleaseStringUTFChars(sVenue, szVenue);
    }
    if(szSDP) {
        env->ReleaseStringUTFChars(sSdp, szSDP);
    }
    if(szUUID) {
        env->ReleaseStringUTFChars(uuid, szUUID);
    }

    return 0;
}

DEFINE_CALLIOPEJNI(jint, doDelete)(JNIEnv* env, jclass clazz, jlong nativeRef, jstring url, jint type) {
    LOGI("calliopeclient::doDelete");
    CM_ASSERTE_RETURN(nativeRef != 0, -1);

    CCalliopeClientSink* pCalliopeClient = (CCalliopeClientSink*)nativeRef;
    const char* szUrl = env->GetStringUTFChars(url, NULL);

    if(type == 0) {
        pCalliopeClient->Get()->DeleteVenue();
    }else if(type == 1) {
        pCalliopeClient->Get()->DeleteConfluence(true);
    }
    if(szUrl) {
        env->ReleaseStringUTFChars(url, szUrl);
    }
}

#endif
