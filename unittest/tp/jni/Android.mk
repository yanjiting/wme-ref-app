LOCAL_PATH := $(call my-dir)
ROOT := ../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
UTIL_PATH := $(ROOT)/mediaengine/util
TP_PATH :=  $(ROOT)/mediaengine/tp

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := wtp util
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(GTEST_PATH)/include \
					$(GMOCK_PATH)/include \
					$(UTIL_PATH)/include \
					$(UTIL_PATH)/include/client \
					$(UTIL_PATH)/include/android \
					$(TP_PATH)/include \
					$(TP_PATH)/src \
                    $(ROOT)/api \
					$(ROOT)/vendor/tinyxml/include \
					$(ROOT)/vendor/openssl/ciscossl/include

LOCAL_MODULE := wtp-unittest

LOCAL_SHARED_LIBRARIES := util wtp
LOCAL_STATIC_LIBRARIES := gtest gmock
LOCAL_LDFLAGS := --coverage
LOCAL_CPPFLAGS += -fexceptions -Werror=format-security
LOCAL_LDLIBS := -llog 
LOCAL_CFLAGS := -DCM_DISABLE_EVENT_REPORT -DCM_USE_REACTOR_SELECT -DCM_PORT_CLIENT -DCM_ANDROID -DCM_LINUX -DCM_UNIX -DGTEST_HAS_TR1_TUPLE=1 -DGTEST_USE_OWN_TR1_TUPLE=1
LOCAL_CFLAGS += -DANDROID
LOCAL_SRC_FILES := \
	$(UTIL_PATH)/src/CmDebug.cpp \
    ../main.cpp \
    ../CmWSHandleRFC6455Test.cpp \
	../CmConnectionInterfaceTest.cpp	\
	../CmReactorBaseTest.cpp
include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
