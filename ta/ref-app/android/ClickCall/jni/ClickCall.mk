LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../../../../../

include $(CLEAR_VARS)

ifeq ($(NDK_DEBUG), 1)
configuration_name := debug
else
configuration_name := release
endif
NDK_APP_DST_DIR := ${ROOT}/distribution/android/armv7/${configuration_name}

LOCAL_MODULE    := clickcall

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../json-cpp/src/lib_json/ \
                    $(LOCAL_PATH)/../../../../json-cpp/include \
                    $(LOCAL_PATH)/../../../src \
                    $(LOCAL_PATH)/../../../../../api \
                    $(LOCAL_PATH)/../../../../../mediaengine/tp/include \
                    $(LOCAL_PATH)/../../../../../mediaengine/util/include \
                    $(LOCAL_PATH)/../../../../../mediaengine/util/include/client \
                    $(LOCAL_PATH)/../../../../../mediaengine/util/include/android

LOCAL_SRC_FILES := mainx.cpp \
                   $(LOCAL_PATH)/../../../../json-cpp/src/lib_json/json_reader.cpp \
                   $(LOCAL_PATH)/../../../../json-cpp/src/lib_json/json_value.cpp \
                   $(LOCAL_PATH)/../../../../json-cpp/src/lib_json/json_writer.cpp \
                   $(LOCAL_PATH)/../../../src/wsclient.cpp \
                   $(LOCAL_PATH)/../../../src/bearer_oauth.cpp \
                   $(LOCAL_PATH)/../../../src/stringtoargcargv.cpp \
                   $(LOCAL_PATH)/../../../src/calliope_client.cpp

LOCAL_CFLAGS += -DANDROID -DARM -DLINUX -DCM_ANDROID -g
LOCAL_CPPFLAGS += -std=c++11 -frtti
LOCAL_LDFLAGS += -llog

LOCAL_LDLIBS := -L${NDK_APP_DST_DIR} -L${ROOT}vendor/openssl/ciscossl/libs/android
LOCAL_LDLIBS += -lwmeclient -lwtp -lutil -llog -lsdp -lcrypto

ifeq ($(HAVE_GCOV_FLAG),1)
LOCAL_LDFLAGS += -lgcov
LOCAL_CFLAGS += -DENABLED_GCOV_FLAG
endif

include $(BUILD_SHARED_LIBRARY)
