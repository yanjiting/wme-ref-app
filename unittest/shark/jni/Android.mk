LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
SHARK_PATH := $(ROOT)/mediaengine/shark
UTIL_PATH := $(ROOT)/mediaengine/util
TRANS_PATH := $(ROOT)/mediaengine/transmission
H264_PATH := $(ROOT)/vendor/openh264

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := ../../../distribution/android/armv7/${configuration_name}
MODULES := wsertp wseclient wmeutil util
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := ../../../vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_MODULE := shark-unittest

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(H264_PATH)/api \
    $(ROOT)/api \
	$(SHARK_PATH)/include \
	$(SHARK_PATH)/src/h \
	$(SHARK_PATH)/src/common \
	$(SHARK_PATH)/src/jlcore \
	$(SHARK_PATH)/src/client/h \
	$(SHARK_PATH)/src/client/SvcClientEngine \
	$(SHARK_PATH)/src/client/SvcClientEngine/android \
        $(SHARK_PATH)/src/wsevp/interface \
	$(UTIL_PATH)/include \
	$(UTIL_PATH)/include/client \
	$(UTIL_PATH)/include/android \
	$(TRANS_PATH)/include \
	$(GTEST_PATH)/include \
	$(GTEST_PATH)/../wbxmock/src \
	$(GMOCK_PATH)/include

LOCAL_SRC_FILES += \
				   ../CWseVideoProcessingTest.cpp \
				   ../CWseEncodeParamGeneratorTest.cpp \
				   ../CWseEncodeControlTest.cpp \
				   ../CWseEngineImpTest.cpp \
				   ../CWseH264SvcDecoderTest.cpp \
				   ../CWseH264SvcEncoderTest.cpp \
				   ../CWseVideoEncoderTest.cpp \
				   ../CWseVideoListenChannelTest.cpp \
				   ../CWseVideoMediaInfoTest.cpp \
				   ../CWseVideoSourceChannelTest.cpp \
				   ../CWseVideoSampleTest.cpp \
				   ../WseRtpApPacketTest.cpp \
				   ../WseRtpFuPacketTest.cpp \
				   ../WseClientUtilTest.cpp \
				   ../CWseUtil.cpp \
				   ../main.cpp

LOCAL_CFLAGS := -w -DUNIX -DWSE_WBX_UNIFIED_TRACE -DANDROID -DEMBEDDEDOS -DARM -DEMBEDDEDOS 
LOCAL_CFLAGS += -DGTEST_USE_OWN_TR1_TUPLE=1 -DGTEST_HAS_TR1_TUPLE=1 -DNDEBUG=1 -D_U_TEST_ 
#LOCAL_CPPFLAGS += -frtti
LOCAL_LDLIBS += -ldl -llog
LOCAL_SHARED_LIBRARIES := wsertp wseclient wmeutil util
LOCAL_STATIC_LIBRARIES += gtest gmock

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
