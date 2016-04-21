//
//  ClickCall.h
//  MediaSessionTest
//
//  Created by Soya Li on 3/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#ifndef MediaSessionTest_ClickCall_h
#define MediaSessionTest_ClickCall_h

#include "MediaConnection4T.h"
#include "bearer_oauth.h"
#include "calliope_client.h"
#include "json/json.h"
#include "Calabash.h"
#include <sstream>
#include "StatsInfo.h"
#include "testconfig.h"
#include "externaltransport.h"
#include "rtpdump.h"

using namespace wme;

// Attention: Only allow to append new param, TA will use the enum number to set param
enum PARAM_TYPE_NUM {
    PARAM_AUDIO = 1,
    PARAM_VIDEO = 2,
    PARAM_SCREEN = 3,
    PARAM_QOS = 4,
};

class IUIRenderUpdateSink {
public:
    virtual void UpdateUI(uint32_t uiWidth, uint32_t uiHeight, void *data) = 0;
};


string newUUID();

typedef struct _tag_cpu_usage1{
    float total_cpu_usage;
    std::list<float> cores_cpu_usage;
} CPU_USAGE1;

typedef struct _tag_process_cpu_usage1{
    float total_process_cpu_usage;
    std::map<uint64_t,float> threads_cpu_usage;
} PROCESS_CPU_USAGE1;

class CScreenTrackViewAnalyzer;
class CScreenFileCapturer;
class Endpoint;
class EventSink
{
public:
    virtual ~EventSink() {};
    virtual void onEvent(Endpoint *origin, const string &type, const string &text = "", const string &extra = "") = 0;
};
class ExternalAudioRender :public IWmeExternalRenderer
{
public:
	ExternalAudioRender(WmeMediaExternalRenderTypeConfig nType);
	~ExternalAudioRender();
public:
	virtual WMERESULT IsPushModeSupport(bool& isSupport){
		isSupport = true;
		return 0;
	};
	virtual WMERESULT RenderMediaData(IWmeMediaPackage *pPackage){ return -1; };
	virtual WMERESULT RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength);
	virtual WMERESULT IsPullModeSupport(bool& isSupport){
		isSupport = false;
		return 0;
	};
	virtual WMERESULT RegisterRequestAnswerer(IWmeExternalRenderAnswerer* pAnswerer){ return -1; };
	virtual WMERESULT UnregisterRequestAnswerer(){ return -1; };
	virtual WmeMediaExternalRenderTypeConfig GetTypeExt() { return m_renderType; };
	virtual void SetTypeExt(WmeMediaExternalRenderTypeConfig eType) { m_renderType = eType; };
private:
	FILE* m_pFileSaveAudioData;
	WmeMediaExternalRenderTypeConfig m_renderType;
};
class Endpoint : public IWmeMediaConnectionSink, public CCmTimerWrapperIDSink
{
public:
	Endpoint(EventSink *sink, bool bHost);
    ~Endpoint();
    
    void SetUISink(IUIRenderUpdateSink *sink) { m_pUISink = sink; }

    void preview();
    void startCall();
    void answerReceived(const char *sdp);
    void acceptCall(const char *sdp, bool bUpdate = false);
    std::string stopCall();
    void resizeRender();
	void getStatistics(WmeAudioConnectionStatistics &audio, WmeVideoConnectionStatistics &video);
    void getFeatureToggleStatistics(string &stats);
    void getCpuStatistics(WmeCpuUsage& cpuStats);
    bool getTrackStatistics(unsigned index, WmeVideoStatistics &video);
    void getNetworkIndex(WmeNetworkDirection d, WmeNetworkIndex &index);
    void getUplinkNetworkStatus(WmeNetworkStatus& status){
        status = m_uplinkStatus;
    }
	void getScreenStatistics(WmeScreenConnectionStatistics &screen);
    void mute(bool bmute, bool video);
    void mute(bool bmute,WmeSessionType eWmeSessionType, bool bLocal = true);
    void changeOrientation(WmeCameraOrientation eOri);
    unsigned int getVoiceLevel(bool bSpk);
    void resume(bool resume);
    void holdCall(bool hold);
	long startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart);

	void setParam(PARAM_TYPE_NUM paramType, const char *param);
    void setGlobalParam(const char *param);
    void setAudioParam(const char *param);
    void setVideoParam(const char *param);
    void setScreenParam(const char *param);
    void setQosParam(const char *param);
    void setBandwidthMaually(unsigned int uBandwidthBps);
    
    std::string getParameters(PARAM_TYPE_NUM paramType);

    
    virtual void OnMediaReady(unsigned long mid,
                              WmeDirection direction,
                              WmeSessionType mediaType,
                              IWmeMediaTrackBase *pTrack);
    
    virtual void OnSDPReady(WmeSdpType sdpType, const char *sdp);
    virtual void OnParsedMLine(WmeSessionType mediaType, unsigned long mid);
    virtual void OnAvailableMediaChanged(unsigned long mid, int nCount);
    
    virtual void OnMediaBlocked(unsigned long mid, unsigned long vid, bool blocked);
    virtual void OnDecodeSizeChanged(unsigned long mid, unsigned long vid, uint32_t uWidth, uint32_t uHeight) {}
    virtual void OnEncodeSizeChanged(unsigned long mid, uint32_t uWidth, uint32_t uHeight) {}
    virtual void OnRenderSizeChanged(unsigned long mid, unsigned long vid, WmeDirection direction, WmeVideoSizeInfo *pSizeInfo);
    virtual void OnMediaError(unsigned long mid, unsigned long vid, WMERESULT errorCode);
    virtual void OnSessionStatus(unsigned long mid, WmeSessionType mediaType, WmeSessionStatus status);
    virtual void OnMediaStatus(unsigned long mid, unsigned long vid, WmeMediaStatus status, bool hasCSI, uint32_t csi);
    virtual void OnCSIsChanged(unsigned long mid, unsigned int vid, const unsigned int* oldCSIArray, unsigned int oldCSICount, const unsigned int* newCSIArray, unsigned int newCSICount);
    virtual void OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc);
    virtual void OnAudioTimeStampChanged(uint32_t csi, uint32_t timestamp);
    virtual void OnError(int errorCode);
    
    virtual void OnTimer(CCmTimerWrapperID* aId);

    void OnSDPReceived(const string &type, const string &text);

    void setCamera(IWmeMediaDevice *pDevice);
    WMERESULT setCamera (const char * pCurrCamera);
    void startSVS();
    bool setAudioPlaybackDevice (const char * pDevName);
    static std::map <std::string, IWmeMediaDevice *> getCameraList();
    
    //WMEIID_IWmeMediaEventObserver>> vagouz>>comments: user care this callback ?
    //virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
    //virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
    
    //screen sharing source
	void addScreenSouce(IWmeScreenSource *pScreenSource);
	void removeScreenSouce(IWmeScreenSource *pScreenSource);
	std::map <std::string, IWmeScreenSource *> & getScreenSourceList();
    void updateScreenCaptureMacFps();
    void updateScreenFileCaptureFileName();
	//
	CScreenTrackViewAnalyzer * getCScreenTrackViewAnalyzer();
    //IWmeScreenSourceEnumeratorEventObserver
    std::vector<IWmeMediaTrackBase*> m_pVideoTrackCallee;
	IWmeMediaTrackBase *m_pVideoTrackCaller, *m_pAudioTrackCaller, *m_pAudioTrackCallee;
	IWmeMediaTrackBase *m_pScreenTrackSharer, *m_pScreenTrackViewer;
	IWmeMediaConnection *m_pMediaConn;
    
    //sync status check
    bool checkSyncStatus(const char* strResult, int32_t iRate);
    
    void ShowScreenSharePreview(void *winScreenPreview);
    
    int getCSICount();
    unsigned int getVideoCSI(WmeDirection direction);
    
    void subscribeVideo(WmeVideoQualityType type, bool newTrack, WmeSubscribePolicy policy, unsigned int csi);
    void unsubscribeVideo(unsigned int vid);
    
    // Calabash testing
    Calabash * getCalabash();
    
	void GetCpuUsage(WmeCpuUsage &cpuUsage);
    void GetMemoryUsage(WmeMemoryUsage &memoryUsage);
    void GetNetworkMetrics(WmeAggregateNetworkMetricStats &stStat);
    
    WmeMediaStatus GetMediaStatus(WmeSessionType mediaType);
    
    long GetAudioDeviceEnum(WmeDeviceInOutType type,std::vector<string>& inputdevice,std::vector<string>& outputdevice);

    void SendEvent(const string &key, const string &value);
    
    void SetAutoSwitchDefaultMic(bool bFlag);
    void SetAutoSwitchDefaultSpeaker(bool bFlag);
    
protected:
	void AddScreenTrackViewAnalyzerAsExternalRender(IWmeMediaTrackBase *lpScreenTrackViewer);
    void checkDirection(WmeSdpParsedInfo *arraySdp, int num);
    
    void StartExternalTrans();
    void ConnectExternalTrans();
    void DisconnectExternalTrans();
    
public:
	void *m_winCaller, *m_winScreenViewer, *m_winScreenPreview;
    std::vector<void*> m_winCallee;
	string m_localSdp;

	bool m_audioFileEnded;
	bool m_videoFileEnded;
	bool m_bHost;

protected:
	EventSink *m_pSink;
    IUIRenderUpdateSink* m_pUISink;

	//string m_sScreenSourceId;
	std::map <std::string, IWmeMediaDevice *> m_cameraList;
    IWmeMediaTrackBase* FindTrackByVid(uint8_t vid);
    IWmeMediaTrackBase* FindTrackByCsi(unsigned int csi, bool bAvailable, uint8_t &itvid);
    struct csi_vid{
        unsigned int csi;
        uint8_t vid;
        uint8_t o_vid;
        bool bHide;
    };
    unsigned int m_possibleActiveCsi;
    std::vector<struct csi_vid> m_csivids;
    void ShowVideo(IWmeMediaTrackBase* pTrack, unsigned int vid, bool bShow);
    void HideDuplication(unsigned int csi);

	std::string GetDebugOption(WmeSessionType mediaType);
    std::string GetParam(WmeSessionType mediaType);
    std::string GetGlobalParam();

    //screen sharing source
	std::vector <IWmeScreenSource*> m_vctToShareScreenSource;
    
    unsigned long m_audioMid;
    unsigned long m_videoMid;
    unsigned long m_shareMid;
    
    CCmTimerWrapperID m_svsTimer, m_svsDelayAddTimer;
    IWmeExternalInputter* m_pSvsInputPin;
    IWmeExternalInputter* m_pSvsInputPin2;
    IWmeMediaTrackBase *m_pSvsLocalAudio2, *m_pSvsLocalVideo2;
    
    CCmTimerWrapperID m_svsAudioTimer;
    IWmeExternalInputter* m_pSvsAudioInputPin;
    IWmeExternalInputter* m_pSvsAudioInputPin2;
    CCmTimerWrapperID m_svsLocalStats;
    
    WmeNetworkStatus m_uplinkStatus;

    // Calabash testing
    Calabash * mCalabash;
    
    WmeDirection m_audioRemoteDirection;
    WmeDirection m_videoRemoteDirection;
    CScreenTrackViewAnalyzer   *m_pScreenTrackViewAnalyzer;
    CScreenFileCapturer * m_pScreenFileCapturer;
    unsigned int m_trackIndex;

    
    map<string, IWmeMediaDevice *> m_AudioDeviceList;
    
    std::vector<string> m_vectorCaptureDevice;
    std::vector<string> m_vectorPlayDevice;
    
    ExternalTransport m_externalAudio;
    ExternalTransport m_externalVideo;
	//
	ExternalAudioRender *m_externalAuidoRender[4];
    
    ClickCall::CRtpDumper *m_pVideoRtpDumper;
    ClickCall::CRtpDumper *m_pAudioRtpDumper;
};

//CTaIWmeStunTraceSink class for TA
class CTaIWmeStunTraceSink : public IWmeStunTraceSink
{
public:
    static CTaIWmeStunTraceSink *GetInstance();
    virtual WMERESULT OnResult(WmeStunTraceResult reason, const char* szDetail);
    int getStunTraceRet() {return stunTraceRet;};
    std::string getJsonRet() {return jsonRet;}
    
private:
    int stunTraceRet;
    std::string jsonRet;
    CTaIWmeStunTraceSink();
    CTaIWmeStunTraceSink(CTaIWmeStunTraceSink const&);
    void operator=(CTaIWmeStunTraceSink const&);
};

//CTaIWmeTraceServerSink class for TA
class CTaIWmeTraceServerSink : public IWmeTraceServerSink
{
public:
    static CTaIWmeTraceServerSink *GetInstance();
    virtual WMERESULT OnTraceServerResult(WmeStunTraceResult reason, const char* szDetail);
    int getTraceServerRet() {return traceRet;};
    std::string getJsonRet() {return jsonRet;}
    
private:
    int traceRet;
    std::string jsonRet;
    CTaIWmeTraceServerSink();
	CTaIWmeTraceServerSink(CTaIWmeTraceServerSink const&);
    void operator=(CTaIWmeTraceServerSink const&);
};

class IAppSink;

#ifdef __cplusplus
extern "C"
{
#endif
	void doStart(int argc, char **argv, void *winRemote, void *winLocal, void *winScreenViewer);		
	void doStop();
	void stringToArgcArgv(const std::string& str, int* argc, char*** argv);
	bool cmdOptionExists(char** begin, char** end, const std::string& option);
	char* getCmdOption(char ** begin, char ** end, const std::string & option);
	void InitCmdArguments(int argc, char **argv);
	void InitApp(int argc, char **argv);
    void doStartCall(void *winRemote, void *winLocal, void *winScreenViewer, IAppSink *pApp = NULL);

#ifdef __cplusplus
}
#endif

#endif
