LOCAL_PATH := $(call my-dir)

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

CLICKCALL_WME_PATH :=  $(LOCAL_PATH)/../../../../../
CLICKCALL_WME_PATH := $(realpath $(CLICKCALL_WME_PATH))
CLICKCALL_WME_BIN := $(CLICKCALL_WME_PATH)/distribution/android/armv7/${configuration_name}
#$(info "Current Building using NDK_DEBUG=$(NDK_DEBUG)")
ifeq ($(BUILD_ALL_IN_ONE_MK), 1)

include $(CLICKCALL_WME_PATH)/mediaengine/util/src/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/tp/src/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/wqos/src/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/transmission/ciscosrtp/bld/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/transmission/src/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/wmeutil/bld/client/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/appshare/bld/client/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/dolphin/src/cochelea/cwcochlea_bld/Android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/dolphin/bld/client/Android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/shark/bld/client/android/wsertp_arm/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/shark/src/wsevp/build/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/shark/bld/client/android/wseclient_arm/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediaengine/wme/bld/client/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/vendor/libsdp/code/bld/android/jni/Android.mk
include $(CLICKCALL_WME_PATH)/mediasession/bld/android/jni/Android.mk

else #BUILD_ALL_IN_ONE_MK


include $(CLEAR_VARS)
LOCAL_MODULE := appshare
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libappshare.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audioengine
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libaudioengine.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mediasession
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libmediasession.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := sdp
LOCAL_SRC_FILES := $(CLICKCALL_WME_PATH)/vendor/libsdp/build/android/armv7/${configuration_name}/libsdp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := srtp
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libsrtp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wtp
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwtp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := util
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wmeclient
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwmeclient.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wmeutil
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwmeutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wqos
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwqos.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wrtp
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwrtp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wseclient
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwseclient.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := wsertp
LOCAL_SRC_FILES := $(CLICKCALL_WME_BIN)/libwsertp.so
include $(PREBUILT_SHARED_LIBRARY)

endif #BUILD_ALL_IN_ONE_MK


include $(CLEAR_VARS)
LOCAL_MODULE := openh264
LOCAL_SRC_FILES := $(CLICKCALL_WME_PATH)/vendor/openh264/libs/android/libopenh264.so
include $(PREBUILT_SHARED_LIBRARY)
