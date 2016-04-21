LOCAL_PATH := $(call my-dir)
ROOT := ../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
SECUR_PATH := $(ROOT)/vendor/security
QOEM_PATH := $(ROOT)/vendor/qoem
WME_PATH := $(ROOT)/mediaengine/wme
SHARK_PATH := $(ROOT)/mediaengine/shark
DOLPHIN_PATH := $(ROOT)/mediaengine/dolphin
UTIL_PATH := $(ROOT)/mediaengine/util
TRANS_PATH := $(ROOT)/mediaengine/transmission
JSON_PATH := $(ROOT)/vendor/SuperEasyJSON

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := util wmeutil wsertp audioengine
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_MODULE := dolphin-unittest

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(ROOT)/api \
	$(UTIL_PATH)/include \
	$(UTIL_PATH)/include/android \
	$(TRANS_PATH)/include \
	$(DOLPHIN_PATH)/include \
	$(DOLPHIN_PATH)/src/AudioEngine/h \
	$(DOLPHIN_PATH)/src/AudioEngine//src/cpve/pp20aec/pp20aec \
	$(DOLPHIN_PATH)/src/AudioEngine//src/cpve/pp20cmod/pp20cmod \
	$(DOLPHIN_PATH)/src/AudioEngine//src/cpve/pp20util/pp20util \
	$(DOLPHIN_PATH)/src/AudioEngine/src/common \
	$(DOLPHIN_PATH)/src/AudioEngine/src/aqe \
	$(DOLPHIN_PATH)/src/AudioEngine/src/framework \
	$(DOLPHIN_PATH)/src/AudioEngine/src/datadump \
	$(DOLPHIN_PATH)/src/cochelea/include \
	$(SHARK_PATH)/include \
	$(SHARK_PATH)/src/h \
	$(SHARK_PATH)/src/common \
	$(WME_PATH)/src/h \
	$(WME_PATH)/src/common \
	$(WME_PATH)/src/client \
	$(WME_PATH)/src/client/android \
	$(SECUR_PATH)/include \
	$(GTEST_PATH)/include \
	$(GTEST_PATH)/../wbxmock/src \
	$(GTEST_PATH)/../wbxmock/src/wbxmock \
	$(GMOCK_PATH)/include \
    $(JSON_PATH) \
    $(QOEM_PATH)/include
 

### for aqe
#../aqe/AQE_Unit_Gtest.cpp
#../aqe/ParameterClass_UnitTest.cpp 
LOCAL_SRC_FILES += \
				   ../aqe/AQE_Unit_MyTest.cpp \
				   ../aqe/Modules_UnitTest.cpp

### for common
#../common/CWbxAJBJitterbufferTest.cpp
LOCAL_SRC_FILES += \
				   ../common/CwbxAeMediaBlockListTest.cpp \
				   ../common/CWBXAEMediaBlockTest.cpp \
				   ../common/CWbxAJBPolicyTest.cpp \
				   ../common/CWbxMemPoolTest.cpp \
				   ../common/CWbxAJBJitterbufferTest.cpp

### for jitterbuffer
#../jitterbuffer/CWbxAJBJitterbufferTest.cpp
#../jitterbuffer/CWbxAJBPolicyTest.cpp 
#LOCAL_SRC_FILES += \
#				   ../jitterbuffer/CWbxAeMediaBlockListTest.cpp \
#				   ../jitterbuffer/CWbxAeMediaBlockTest.cpp 

### for framework
#../framework/CWbxAePlaybackChannelTest.cpp
#../framework/CWbxAeRecordChannelTest.cpp 
LOCAL_SRC_FILES += \
	../framework/CMixerDestroyEventTest.cpp \
	../framework/CMixerStartEventTest.cpp \
	../framework/CMixerStopEventTest.cpp \
	../framework/CWbxAeAudioChannelTest.cpp \
	../framework/CWbxAeVoiceMixerTest.cpp \
	../framework/CWbxAudioEngineImplTest.cpp \
	../framework/CAudioDefaultSettingsTest.cpp \
        ../framework/CWBXAeWavAdaptorTest.cpp

LOCAL_SRC_FILES += $(JSON_PATH)/json.cpp
LOCAL_SRC_FILES += ../main.cpp

LOCAL_CFLAGS := -w -DUNIX -DWSE_WBX_UNIFIED_TRACE -DANDROID -DEMBEDDEDOS -DARM -DEMBEDDEDOS -DGTEST_USE_OWN_TR1_TUPLE=1 -DGTEST_HAS_TR1_TUPLE=1 -DNDEBUG=1
LOCAL_CFLAGS += -DGTEST -DCM_LINUX -DLINUX -DCM_DISABLE_EVENT_REPORT -DCM_ANDROID -D__USE_CISCO_SAFE_C__
LOCAL_CFLAGS += -D_U_TEST_
#LOCAL_CPPFLAGS += -frtti
LOCAL_LDLIBS += -ldl -llog
LOCAL_SHARED_LIBRARIES := util wmeutil wsertp audioengine 
LOCAL_STATIC_LIBRARIES += gtest gmock

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
