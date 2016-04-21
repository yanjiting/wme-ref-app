LOCAL_PATH := $(call my-dir)
ROOT := ../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
WME_PATH := $(ROOT)/mediaengine/wme
SHARK_PATH := $(ROOT)/mediaengine/shark
DOLPHIN_PATH := $(ROOT)/mediaengine/dolphin
UTIL_PATH := $(ROOT)/mediaengine/util
TRANS_PATH := $(ROOT)/mediaengine/transmission
APPSHARE_PATH := $(ROOT)/mediaengine/appshare

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := wmeclient wmeutil wsertp wseclient audioengine wrtp wqos srtp tp util
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_MODULE := wme-unittest

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(ROOT)/api \
	$(UTIL_PATH)/include \
	$(UTIL_PATH)/include/client \
	$(UTIL_PATH)/include/android \
	$(TRANS_PATH)/include \
	$(DOLPHIN_PATH)/include \
	$(DOLPHIN_PATH)/src/AudioEngine/h \
	$(SHARK_PATH)/include \
	$(SHARK_PATH)/src/h \
	$(SHARK_PATH)/src/common \
	$(APPSHARE_PATH)/include \
	$(WME_PATH)/src/h \
	$(WME_PATH)/src/common \
	$(WME_PATH)/src/client \
	$(WME_PATH)/src/client/android \
	$(GTEST_PATH)/include \
	$(GMOCK_PATH)/include

LOCAL_SRC_FILES += \
	../WmeH264PacketizationTest.cpp \
	../CWmeLocalAudioTrackTest.cpp \
	../CWmeLocalVideoTrackTest.cpp \
	../CWmeVideoCodecEnumeratorTest.cpp \
	../CWmeMediaSessionTest.cpp \
	../CWmeMediaSyncBoxTest.cpp \
	../CWmeRemoteAudioTrackTest.cpp \
	../CWmeVideoPreviewTrackTest.cpp \
	../CWmelAudioDeviceTest.cpp \
	../CWmeAudioVolumeControllerTest.cpp \
	../CWmeCodecEnumeratorTest.cpp \
	../CWmeInterfaceTest.cpp \
	../CWmelAudioDeviceTest.cpp \
	../CWmeLocalAudioExternalTrackTest.cpp \
    ../CWmeLocalVideoExternalTrackTest.cpp \
    ../CWmeMediaDeviceNotifierTest.cpp \
    ../CWmeMediaInfoTest.cpp \
    ../CWmeRemoteVideoTrackTest.cpp \
    ../CWmeVideoCaptureDeviceTest.cpp \
	../CWmeScreenShareLocalCompositeTrackTest.cpp \
	../CWmeScreenShareRemoteTrackTest.cpp \
	../CWmeScreenTrackTestBase.cpp \
	../CWmeNetworkIndicatorTest.cpp \
    ../WhitneyMediaSDK_GTest.cpp

LOCAL_CFLAGS := -w -DUNIX -DWSE_WBX_UNIFIED_TRACE -DANDROID -DEMBEDDEDOS -DARM -DEMBEDDEDOS -DGTEST_USE_OWN_TR1_TUPLE=1 -DGTEST_HAS_TR1_TUPLE=1 -DNDEBUG=1
LOCAL_CPPFLAGS += -fexceptions #-frtti
LOCAL_LDLIBS += -ldl -llog
LOCAL_SHARED_LIBRARIES := wmeclient wsertp wseclient audioengine wrtp util wmeutil
LOCAL_STATIC_LIBRARIES += gtest gmock

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
