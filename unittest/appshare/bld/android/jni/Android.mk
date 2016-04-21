LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../../../../..
DIST := ../../../../../distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
SHARK_PATH := $(ROOT)/mediaengine/shark
UTIL_PATH := $(ROOT)/mediaengine/util
APPSHARE_PATH := $(ROOT)/mediaengine/appshare
APPSHARE_TEST_SRC_PATH := ../../../src

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := wmeutil appshare
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_MODULE := appshare-unittest

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(SHARK_PATH)/include \
	$(SHARK_PATH)/src/h \
	$(SHARK_PATH)/src/common \
	$(APPSHARE_PATH)/include \
	$(APPSHARE_PATH)/src \
	$(APPSHARE_PATH)/src/android \
	$(GTEST_PATH)/include \
	$(GMOCK_PATH)/include \
    $(ROOT)/api \
    $(UTIL_PATH)/include \
    $(UTIL_PATH)/include/android \
	$(ROOT)/vendor/SuperEasyJSON

LOCAL_SRC_FILES += \
				   $(APPSHARE_TEST_SRC_PATH)/share_util_unittest.cpp \
				   $(APPSHARE_TEST_SRC_PATH)/share_common_frame_unittest.cpp \
				   $(APPSHARE_TEST_SRC_PATH)/share_capture_engine_unittest.cpp \
				   $(APPSHARE_TEST_SRC_PATH)/appshare_unittest.cpp

LOCAL_CFLAGS := -w -DUNIX -DWSE_WBX_UNIFIED_TRACE -DANDROID -DEMBEDDEDOS -DARM -DEMBEDDEDOS -DGTEST_USE_OWN_TR1_TUPLE=1 -DGTEST_HAS_TR1_TUPLE=1 -DNDEBUG=1
LOCAL_CPPFLAGS += -fexceptions #-frtti
LOCAL_LDLIBS += -ldl -llog
LOCAL_SHARED_LIBRARIES := wmeutil appshare
LOCAL_STATIC_LIBRARIES += gtest gmock

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
