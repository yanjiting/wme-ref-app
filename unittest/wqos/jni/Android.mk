LOCAL_PATH := $(call my-dir)
ROOT := ../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
BOOST_PATH := $(ROOT)/vendor/boost
UTIL_PATH := $(ROOT)/mediaengine/util
WQOS_PATH :=  $(ROOT)/mediaengine/wqos
MARI_PATH := $(ROOT)/vendor/mari

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := util wqos
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(GTEST_PATH)/include \
					$(GMOCK_PATH)/include \
					$(BOOST_PATH)/.. \
					$(MARI_PATH)/code/include \
					$(UTIL_PATH)/include \
					$(UTIL_PATH)/include/client \
					$(UTIL_PATH)/include/android \
    				$(WQOS_PATH)/include \
                    $(ROOT)/api \
    				$(WQOS_PATH)/src
    
LOCAL_MODULE := wqos-unittest
LOCAL_SHARED_LIBRARIES := util wqos 
LOCAL_STATIC_LIBRARIES := gtest gmock
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -w -DCM_ANDROID -DCM_LINUX -DCM_UNIX -O2 -DCM_SUPPORT_T120_UTIL -DGTEST_HAS_TR1_TUPLE=1 -DGTEST_USE_OWN_TR1_TUPLE=1
LOCAL_CFLAGS += -DANDROID
LOCAL_SRC_FILES := \
 	../main.cpp \
	../QoSBandwidthControllerTest.cpp\
	../BandwidthAllocatorTest.cpp

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
