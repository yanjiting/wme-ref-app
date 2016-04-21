#include <stdlib.h>
#include <stdio.h>
#include <jni.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifndef DEFINE_UNITTESTJNI
#define DEFINE_UNITTESTJNI(r, f) EXTERNC JNIEXPORT r JNICALL Java_com_cisco_wme_unittest_UnitTestNative_##f
#endif

extern int tp_unittest_main(const char* xmlPath);
extern int util_unittest_main(const char* xmlPath, const char* filter);
extern int wqos_unittest_main(const char* xmlPath);
extern int wrtp_unittest_main(const char* xmlPath);

extern int dolphin_unittest_main(const char* xmlPath);
extern int shark_unittest_main(const char* xmlPath);
extern int wme_unittest_main(const char* xmlPath);
extern int appshare_unittest_main(const char* xmlPath);
DEFINE_UNITTESTJNI(jint, GotoUnittest)(JNIEnv* env, jclass clazz, jstring name, jstring path)
{
    const char *pName= env->GetStringUTFChars(name, NULL);
    if (pName == NULL) {
        return -1;
    }

    const char *pPath= env->GetStringUTFChars(path, NULL);
    if (pPath == NULL) {
        return -1;
    }

    char xmlPath[1024] = "";
    bool bforce = false;
    if (strncasecmp(pName, "all", 3) == 0) {
        bforce = true;
    }else {
        sprintf(xmlPath, "%s", pPath);
    }

    if (bforce || strncasecmp(pName, "wtp", 3) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-tp-gtest.xml", pPath);
        tp_unittest_main(xmlPath);
    }
    
    if (bforce || strncasecmp(pName, "util", 2) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-util-gtest.xml", pPath);
        util_unittest_main(xmlPath, NULL);
    }

    if (bforce || strncasecmp(pName, "wqos", 2) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-wqos-gtest.xml", pPath);
        wqos_unittest_main(xmlPath);
    }

    if (bforce || strncasecmp(pName, "wrtp", 2) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-wrtp-gtest.xml", pPath);
        wrtp_unittest_main(xmlPath);
    }

    if(bforce || strncasecmp(pName, "dolphin", 3) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-dolphin-gtest.xml", pPath);
        dolphin_unittest_main(xmlPath);
    }

    if(bforce || strncasecmp(pName, "shark", 3) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-shark-gtest.xml", pPath);
        shark_unittest_main(xmlPath);
    }

    if(bforce || strncasecmp(pName, "wme", 3) == 0) {
        if (bforce)
            sprintf(xmlPath, "%s/all-wme-gtest.xml", pPath);
        wme_unittest_main(xmlPath);
    }

    if(bforce || strncasecmp(pName, "appshare", 3) == 0) {
        if (bforce)
            sprintf(xmlPath, "--gtest_output %s/all-appshare-gtest.xml", pPath);
        appshare_unittest_main(xmlPath);
    }

    env->ReleaseStringUTFChars(name, pName);
    env->ReleaseStringUTFChars(path, pPath);

	return 0;
}

