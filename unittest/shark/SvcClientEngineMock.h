#ifndef _SVCCLENTENGINE_MOCK_H_
#define _SVCCLENTENGINE_MOCK_H_

#include "gmock/gmock.h"
#include "IWseCodec.h"
#include "WseEngine.h"
#include "jlbaseimp.h"
#include "wrtpclientapi.h"

using namespace testing;
using namespace shark;
class MockIWseEncoderSink: public IWseEncoderSink
{
public:

	//virtual void OnFrameEncoded(CWseEncoderOutput& output, bool lastFlag) = 0;
	MOCK_METHOD3(OnEncodeParamChanged, long(unsigned long ulWidth, unsigned long ulHeight, float fFps));
	MOCK_METHOD1(OnInputFrameRateChanged, long(float fFps));
	MOCK_METHOD1(OnEncoderOutputLayerInfo, long(WseEncoderOutputLayerInfo info));
    MOCK_METHOD1(OnEncoderIDRFrame, long(int nIDRDataLen));
};

class MockIWseVideoDecoderSink: public IWseVideoDecoderSink
{
public:
	//virtual void OnFrameEncoded(CWseEncoderOutput& output, bool lastFlag) = 0;
	MOCK_METHOD2(OnDecoded, void(IWseVideoSample* sample, bool bDecodeFailed));
	MOCK_METHOD3(OnDecodedAndRendered,void(unsigned long time_stamp, int width, int height));
	MOCK_METHOD2(OnMessage, long(WseCodecMessageType type,void *pParam));
	MOCK_METHOD0(OnKeyFrameLost, long());
	MOCK_METHOD1(OnLTRrecoveryRequest, long(void* sLTR_Recover_Request));
	MOCK_METHOD1(OnLTRMarking, long(void* sLTR_Marking_Feedback));
    MOCK_METHOD0(OnEc, long());
};

class MockIWseDataSink: public IWseDataSink
{
public:
	MOCK_METHOD1(OnOutputData, long(CWseEncoderOutput& output));
};

class MockIWseTransportSink: public IWseTransportSink
{
public:
	MOCK_METHOD5(SendData, long(WseVideoType type,unsigned long time_stamp,unsigned char *data, int len,WseFrameFlag dwFlag));
	MOCK_METHOD3(SendRtpPacket, long(unsigned long dwType, unsigned char* data, int len));
	MOCK_METHOD3(SendRtcpPacket, long(unsigned char* data, int len, bool bReliable));
};

class MockIWseVideoSourceChannelNotifySink: public IWseVideoSourceChannelNotifySink
{
public:
	MOCK_METHOD1(OnKeyFrameRequested, long(unsigned long dwSourceID));
	MOCK_METHOD3(OnSourceParameterChanged, long(unsigned long dwSrc, unsigned long dwType, LONG_PTR dwValue));
	MOCK_METHOD1(OnSourceBitrateStat, long(int  nBitrate)); 
	MOCK_METHOD2(OnNotifySourceBitrateInfo, long(unsigned long dwMinBw, unsigned long dwMaxBw));

	MOCK_METHOD1(OnCaptureParameterChanged, long(WseCameraCapability CapParam));
};

class MockIWseEncryptSink: public IWseEncryptSink
{
public:
	MOCK_METHOD4(Encrypt, long(unsigned char* data, int len, unsigned char** ppOut, int& iOut));
	MOCK_METHOD4(Decrypt, long(unsigned char* data, int len, unsigned char** ppOut, int& iOutLen));
	MOCK_METHOD1(FreeBlock, long(unsigned char* pOut));
};

class MockIWseEngineStatusSink: public IWseEngineStatusSink
{
public:
	MOCK_METHOD3(SystemIndication, long(int iCpuRate,int iMemRate,int NetworkState));
	MOCK_METHOD1(CaptureFpsIndication, long(int iFps));
};

class MockIRTPChannel: public wrtp::IRTPChannel
{
public:
	MOCK_METHOD0(IncreaseReference,UINT32());
	MOCK_METHOD0(DecreaseReference,UINT32());
	MOCK_METHOD1(SetChannelId, void(UINT32 channelId));
	MOCK_METHOD0(GetChannelId,UINT32());
	MOCK_METHOD1(SetPayloadType, void(UINT8 type));
	MOCK_METHOD0(GetPayloadType,UINT8());
	MOCK_METHOD1(SetFECPayloadType, void(UINT8 type));
	MOCK_METHOD0(GetFECPayloadType, UINT8());
	MOCK_METHOD1(SetFecType, void(wrtp::WRTPFecType type));
	MOCK_METHOD0(GetFecType,wrtp::WRTPFecType());
	MOCK_METHOD1(SetFragmentOperator, void(RTP_IN wrtp::IFragmentOperator* pFragmentOp));	// source channel and listen channel call
	MOCK_METHOD0(GetFragmentOperator,wrtp::IFragmentOperator*());
	MOCK_METHOD1(SetPacketizationOperator, void(wrtp::IPacketizationOperator* packetizationOp));
	MOCK_METHOD0(GetPacketizationOperator, wrtp::IPacketizationOperator*());
	MOCK_METHOD1(SendMediaData, INT32(RTP_IN wrtp::WRTPMediaData* pData)); // source channel call
	MOCK_METHOD1(SetMediaDataRecvSink, void(RTP_IN wrtp::IMediaDataRecvSink* pSink)); // listen channel call
//	MOCK_METHOD1(RecvRTPPacket, INT32(RTP_IN CCmMessageBlock& rtpPacket));
	INT32 RecvRTPPacket(RTP_IN CCmMessageBlock& rtpPacket) {return 0;}
	MOCK_METHOD1(ReceiveRTPPacket, INT32(wme::IWmeMediaPackage* rtpPackage));
    MOCK_METHOD2(ReceiveRTPPacket, INT32(void *pData, UINT32 len));
	MOCK_METHOD0(Close,void());
    MOCK_METHOD1(SetMediaContentCipher, INT32(IRTPMediaContentCipher* cipher));
	MOCK_METHOD3(SetOption, INT32(wrtp::WRTPOption opt, const void* buf, UINT32 len));
	MOCK_METHOD3(GetOption, INT32(wrtp::WRTPOption opt, void* buf, UINT32* len));
    MOCK_METHOD1(GetActiveStatistics, INT32(wrtp::StreamInStats &stStat));
    MOCK_METHOD2(GetLocalStatistics, INT32(wrtp::StreamOutStats *stStat, UINT32 &len));
    MOCK_METHOD1(UpdateCSI, INT32(UINT32));
    MOCK_METHOD2(AddExternalRenderer, INT32(wme::IWmeExternalRenderer *pExternalRender, bool bExternalOnly));
    MOCK_METHOD0(RemoveExternalRenderer, INT32());
};

class MockIWseVideoDeliverer : public IWseVideoDeliverer
{
public:
	virtual unsigned long JLCALLTYPE AddRef() {return 0;};
	virtual unsigned long JLCALLTYPE Release() {return 0;};
	virtual JLRESULT JLCALLTYPE QueryInterface(REFJLIID iid,void **ppvObject) {return 0;};

	MOCK_METHOD1(DeliverImage, WSERESULT(IWseVideoSample* pSample));
    WSERESULT DeliverImage(VideoRawDataPack* pVideoPack)
    {
        return WSE_S_OK;
    }
#ifdef WIN32
	MOCK_METHOD3(RenderWithDC, WSERESULT(HDC& dc, RECT* pRect, BOOL bRedraw));
#endif
};

class MockIWseVideoListenChannelNotifySink : public IWseVideoListenChannelNotifySink
{
public:
	//these function callback to client on main thread
	MOCK_METHOD1(OnKeyFrameLost, long (unsigned long iSrcID));
	MOCK_METHOD2(OnRenderBlocked, long (unsigned long dwSrcID, bool bBlocked));
#ifdef WIN32
	MOCK_METHOD1(RequestDC, long (unsigned long dwSrc));
#endif

	MOCK_METHOD4(OnDecodedFrame, long (WseVideoFormat videotype,unsigned char *data,unsigned long dwLen,bool bOutput));
    MOCK_METHOD1(OnRenderIndication, long(unsigned long dwSrcID));
	//for QA TEST
	//dwTimeStamp is the RTP timestamp field
	//this function will be call back to client on listen pump thread(if channel work in Ansync mode).
#ifdef ENABLE_QA_TEST
	MOCK_METHOD2(OnListenRenderTimeStampChange, long (unsigned long iSrcID, unsigned long dwTimeStamp));
#endif
	//end
	MOCK_METHOD3(OnListenParameterChanged, long (unsigned long dwSrc, unsigned long dwType, unsigned long dwValue));
	MOCK_METHOD2(OnMediaDataInputInterrupt, long (unsigned long dwSrc, bool bInterruptionStart) );
    MOCK_METHOD1(OnDecoderControlStatus, void (bool bDecoderControlInProgress));
};

#endif

