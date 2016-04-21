LOCAL_PATH := $(call my-dir)
LOCAL_PATH_FULL_PATH := $(LOCAL_PATH)

include $(LOCAL_PATH_FULL_PATH)/ClickCall.mk

ifeq ($(BUILD_ALL_WME_IN_REFAPP), 1)
LOCAL_PATH_FULL_PATH := $(realpath $(LOCAL_PATH_FULL_PATH))
include $(LOCAL_PATH_FULL_PATH)/wme.mk
endif

