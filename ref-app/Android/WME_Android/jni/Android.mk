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
	../../../common/src/PESQ/src/dsp.cpp \
	../../../common/src/PESQ/src/pesqdsp.cpp \
	../../../common/src/PESQ/src/pesqmain.cpp \
	../../../common/src/PESQ/src/pesqio.cpp \
	../../../common/src/PESQ/src/pesqmod.cpp \
    ../../../common/src/FileCaptureEngineImp.cpp \
    ../../../common/src/FileMediaConnection.cpp \
	../../../common/src/MiscMutex.cpp \
	../../../common/src/MediaManager.cpp \
	../../../common/src/IMediaSettings.cpp \
	../../../common/src/IMediaClient.cpp \
	../../../common/src/MediaTransport.cpp \
	../../../common/src/AudioSender.cpp \
	../../../common/src/AudioReceiver.cpp \
	../../../common/src/VideoSender.cpp \
	../../../common/src/VideoReceiver.cpp \
	../../../common/src/VideoPreview.cpp \
	../../../common/src/DeskShareReceiver.cpp \
	../../../common/src/DeskShareSender.cpp \
	../../../common/src/DemoClient.cpp \
	../../../common/src/MyMediaConnection.cpp \
	../../../common/src/WMEMediaFileRenderSink.cpp \
	../../../common/src/IcerClient/CandidateParse.cpp  \
	../../../common/src/IcerClient/IcerClient.cpp  \
	../../../common/src/IcerClient/SimpleClientSession.cpp \
	../../../common/src/tinyxml/tinystr.cpp \
	../../../common/src/tinyxml/tinyxml.cpp \
	../../../common/src/tinyxml/tinyxmlerror.cpp \
	../../../common/src/tinyxml/tinyxmlparser.cpp


LOCAL_C_INCLUDES := \
	. \
	../../../common/src/PESQ/inc/ \
	../../../../vendor/zbar/include/ \
	../../../../vendor/zbar/include/zbar/ \
	../../../common/include/ \
    	../../../common/src/ \
	../../../common/src/tinyxml/ \
	../../../common/src/IcerClient/

######## CWME SVN DEPENDENCIES #######

LOCAL_C_INCLUDES += \
    ../../../../api/ \
    ../../../../mediaengine/util/include/ \
    ../../../../mediaengine/util/include/android \
    ../../../../mediaengine/util/include/client \
    ../../../../mediaengine/wqos/include/ \
    ../../../../mediaengine/transmission/include \
    ../../../../vendor/security/include

ifeq ($(NDK_DEBUG), 1)
LOCAL_LDLIBS += -L../../../../distribution/android/armv7/debug -lwtp -lutil -lwmeclient
else
LOCAL_LDLIBS += -L../../../../distribution/android/armv7/release -lwtp -lutil -lwmeclient
endif


######## CWME SVN DEPENDENCIES (end) #######

LOCAL_LDLIBS += -ldl -llog 

LOCAL_CFLAGS := -DANDROID -DHAVE_NEON -DARM -DLINUX -DENABLE_WME -DCUCUMBER_ENABLE -fexceptions
LOCAL_CPPFLAGS += -frtti

include $(BUILD_SHARED_LIBRARY)
