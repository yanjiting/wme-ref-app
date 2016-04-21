LOCAL_PATH := $(call my-dir)
CURRENT_DIR := $(LOCAL_PATH)
include $(CURRENT_DIR)/../../../../build/android/common.mk

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

ifdef BUILD_ALL_IN_ONE_MK

ifeq ($(BUILD_ALL_IN_ONE_MK), 1)
WMEDIR := $(CURRENT_DIR)/../../../../mediaengine

# for building wme modules of util, srtp/wrtp, wqos, wtp
MODULES := util/src wqos/src transmission/ciscosrtp/bld/android transmission/src tp/src
$(foreach mod,$(MODULES),$(eval $(call include_module,$(WMEDIR)/$(mod)/jni/Android.mk)))

# for wme modules of wmeutil, audioengine, appshare
MODULES := wmeutil/bld/client/android dolphin/src/cochelea/cwcochlea_bld/Android dolphin/bld/client/Android appshare/bld/client/android
$(foreach mod,$(MODULES),$(eval $(call include_module,$(WMEDIR)/$(mod)/jni/Android.mk)))

# for wme modules of wsevp, wsertp, wseclient, wmeclient
MODULES := shark/src/wsevp/build/android shark/bld/client/android/wsertp_arm shark/bld/client/android/wseclient_arm wme/bld/client/android
$(foreach mod,$(MODULES),$(eval $(call include_module,$(WMEDIR)/$(mod)/jni/Android.mk)))

# for gtest/gmock
$(eval $(call include_module,$(WMEDIR)/../vendor/gtest/bld/android/jni/Android.mk))

# for unittest modules of util, wtp, ...
MODULES := util tp wqos wrtp shark dolphin wme
UT_PATH := $(CURRENT_DIR)/../../../../unittest
$(foreach mod,$(MODULES),$(eval $(call include_module,$(UT_PATH)/$(mod)/jni/Android.mk)))

# for unittest module of appshare
UT_PATH := $(CURRENT_DIR)/../../../../unittest
$(eval $(call include_module,$(UT_PATH)/appshare/bld/android/jni/Android.mk))
endif

else

ROOT := ../../../..
DIST := $(ROOT)/distribution/android/armv7

LIBPATH := $(DIST)/${configuration_name}
MODULES := wmeclient wmeutil wsertp wseclient audioengine wrtp wqos srtp wtp util appshare
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/unittest
MODULES := util wqos wrtp dolphin shark wme 
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod)-unittest,$(LIBPATH)/$(mod)/libs/armeabi-v7a/lib$(mod)-unittest.so)))
MODULES := tp 
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,wtp-unittest,$(LIBPATH)/$(mod)/libs/armeabi-v7a/libwtp-unittest.so)))
MODULES := appshare
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod)-unittest,$(LIBPATH)/$(mod)/bld/android/libs/armeabi-v7a/lib$(mod)-unittest.so)))

endif



#=====================================================



LOCAL_PATH := $(CURRENT_DIR)
ROOT := $(LOCAL_PATH)/../../../..

include $(CLEAR_VARS)
LOCAL_MODULE := all-unittest

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := .
LOCAL_SRC_FILES += all-unittest.cpp

LOCAL_CFLAGS := -w -DUNIX -DWSE_WBX_UNIFIED_TRACE -DANDROID -DEMBEDDEDOS -DARM -DEMBEDDEDOS 
LOCAL_CFLAGS += -DGTEST_USE_OWN_TR1_TUPLE=1 -DGTEST_HAS_TR1_TUPLE=1 -DNDEBUG=1
LOCAL_CPPFLAGS += -frtti
LOCAL_LDLIBS += -ldl -llog
LOCAL_SHARED_LIBRARIES := util wqos srtp wrtp wtp wmeutil wsertp wseclient audioengine appshare wmeclient
LOCAL_SHARED_LIBRARIES += wtp-unittest util-unittest wqos-unittest wrtp-unittest dolphin-unittest shark-unittest wme-unittest appshare-unittest 

include $(BUILD_SHARED_LIBRARY)
