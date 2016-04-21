# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE	:= arm

LOCAL_MODULE    := wmenative-jni

LOCAL_SRC_FILES := \
	wmenative-jni.cpp

LOCAL_SRC_FILES += \
    src/FileCaptureEngineImp.cpp \
    src/FileMediaConnection.cpp \
	src/MiscMutex.cpp \
	src/MediaManager.cpp \
	src/IMediaSettings.cpp \
	src/IMediaClient.cpp \
	src/MediaTransport.cpp \
	src/AudioSender.cpp \
	src/AudioReceiver.cpp \
	src/VideoSender.cpp \
	src/VideoReceiver.cpp \
	src/VideoPreview.cpp \
	src/DeskShareReceiver.cpp \
	src/DeskShareSender.cpp \
	src/DemoClient.cpp \
	src/MyMediaConnection.cpp \
	src/IcerClient/CandidateParse.cpp  \
	src/IcerClient/IcerClient.cpp  \
	src/IcerClient/SimpleClientSession.cpp \
	src/tinyxml/tinystr.cpp \
	src/tinyxml/tinyxml.cpp \
	src/tinyxml/tinyxmlerror.cpp \
	src/tinyxml/tinyxmlparser.cpp
	

LOCAL_C_INCLUDES := \
	. \
	include/ \
	src/ \
	src/tinyxml/ \
	src/IcerClient/

######## CWME SVN DEPENDENCIES #######

LOCAL_C_INCLUDES += \
   	../../include/common \
 	../../include/tp \
	../../include/wqos \
	../../include/wrtp \
	../../include/wme \
	../../include/security

ifeq ($(NDK_DEBUG), 1)
LOCAL_LDLIBS += -L../../libs/debug
else
LOCAL_LDLIBS += -L../../libs/release
endif

LOCAL_LDLIBS +=  -lwtp -lutil -lwmeclient

######## CWME SVN DEPENDENCIES (end) #######
LOCAL_LDLIBS += -ldl -llog 

LOCAL_CFLAGS := -DANDROID -DHAVE_NEON -DARM -DLINUX -DENABLE_WME
LOCAL_CPPFLAGS += -frtti

include $(BUILD_SHARED_LIBRARY)
