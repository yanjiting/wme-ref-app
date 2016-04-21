LOCAL_PATH := $(call my-dir)
ROOT := ../../..
DIST := $(ROOT)/distribution/android/armv7

GTEST_PATH := $(ROOT)/vendor/gtest/gtest-1.7.0
GMOCK_PATH := $(ROOT)/vendor/gtest/gmock-1.7.0
BOOST_PATH := $(ROOT)/vendor/boost
UTIL_PATH := $(ROOT)/mediaengine/util
WQOS_PATH := $(ROOT)/mediaengine/wqos
TRANS_PATH :=  $(ROOT)/mediaengine/transmission
MARI_PATH := $(ROOT)/vendor/mari

ifeq ($(NDK_DEBUG), 1)
configuration_name=debug
else
configuration_name=release
endif

include $(ROOT)/build/android/common.mk

LIBPATH := $(DIST)/${configuration_name}
MODULES := srtp util wqos wrtp
$(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(mod),$(LIBPATH)/lib$(mod).so)))

LIBPATH := $(ROOT)/vendor/gtest/bld/android/obj/local/armeabi-v7a
MODULES := gtest gmock
$(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(mod),$(LIBPATH)/lib$(mod).a)))


include $(CLEAR_VARS)
LOCAL_MODULE := wrtp-unittest
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(GTEST_PATH)/include \
					$(GMOCK_PATH)/include \
					$(ROOT)/vendor/gtest/wbxmock/src \
					$(BOOST_PATH)/.. \
					$(MARI_PATH)/code/include \
					$(UTIL_PATH)/include \
					$(UTIL_PATH)/include/client \
					$(UTIL_PATH)/include/android \
					$(UTIL_PATH)/src \
    				$(WQOS_PATH)/include \
					$(WQOS_PATH)/src \
					$(MARI_PATH)/code/include/mari/ \
					$(MARI_PATH)/code/include/mari/rsfec \
					$(MARI_PATH)/code/include/mari/wqos \
					$(TRANS_PATH)/include \
					$(TRANS_PATH)/src \
					$(ROOT)/vendor/libsrtp/include \
					$(ROOT)/vendor/libsrtp/crypto/include \
					$(ROOT)/api
        
LOCAL_SHARED_LIBRARIES := srtp util wqos wrtp 
LOCAL_STATIC_LIBRARIES := gtest gmock
LOCAL_LDLIBS := -llog 
LOCAL_CFLAGS := -w -DCM_ANDROID -DCM_LINUX -DCM_UNIX -DCM_PORT_CLIENT -DCM_DISABLE_TRACE -O2 -DCM_DISABLE_EVENT_REPORT -DCM_SUPPORT_T120_UTIL 
LOCAL_CFLAGS += -DANDROID -DGTEST_HAS_TR1_TUPLE=1 -DGTEST_USE_OWN_TR1_TUPLE=1
LOCAL_SRC_FILES := \
 	../ActiveBufferTest.cpp \
	../AVSyncAPITest.cpp \
	../CFBPacketTest.cpp \
	../CFECEncoderTest.cpp \
	../CFECLevelTest.cpp \
	../CInformQoSAPITEST.cpp \
	../CMMFrameManagerTest.cpp \
	../CPSFBPacketTest.cpp \
	../CRetransmissionPacketTest.cpp \
	../CRTCPHandlerTest.cpp \
	../CRTCPPacketTest.cpp \
	../CRTPAggregatePacketTest.cpp \
	../CRTPChannelTest.cpp \
	../CRTPFBPacketTest.cpp \
	../CRTPPacketLazyTest.cpp \
	../CRTPPacketTest.cpp \
	../CRTPSessionClientAPITest.cpp \
	../CRTPSessionClientTest.cpp \
	../CRTPStreamTest.cpp \
	../CRTPUtilsTest.cpp \
	../CSecurityStrategyWrapperClientTest.cpp \
	../CSendWindowTest.cpp \
	../CSeqCountorTest.cpp \
	../CSequenceDBTest.cpp \
	../CVideoStreamTest.cpp \
	../CWmeCodecPayloadTypeAPITest.cpp \
	../FrameNALFragmentTest.cpp \
	../main.cpp \
	../MediaStatisticsTest.cpp \
	../NewTraceInterfaceTest.cpp \
	../PktStatTest.cpp \
	../PSFBCallFlowTest.cpp \
	../RTPMediaAPITest.cpp \
	../RTPPacketizationTest.cpp \
	../RTPSessionConfigTest.cpp \
	../RTPTimeTest.cpp \
	../RTPWMEAPITest.cpp \
	../SecurityPolicyAPITest.cpp \
	../SendControlAPITestForAS.cpp \
	../SendControlTest.cpp \
	../SendingControlTest.cpp \
	../SesscionConfigObserverTest.cpp \
	../SpeedMeasureTest.cpp \
	../testutil.cpp \
	../TMMBRTest.cpp \
	../WMEInterfaceMock.cpp \
	../SimulcastCallflowTest.cpp \
	../SimulcastTest.cpp \
	../CRTPThreadTest.cpp \
	../NetworkIndicatorTest.cpp \
	../RTPHeaderExtensionVideoTest.cpp \
	../RTPStreamManagementTest.cpp \
	../CRTPLossRatioStatsTest.cpp \
	../VideoRecvBufferTest/RTPVideoRecvFrameManagerTest.cpp \
	../VideoRecvBufferTest/RTPVideoRecvFrameTest.cpp \
	../VideoRecvBufferTest/VideoRecvFrameInfoTest.cpp \
	../VideoRecvBufferTest/VideoRecvFramePacketTest.cpp

include $(ROOT)/build/android/extra.mk
include $(BUILD_SHARED_LIBRARY)
