#include "WbxAeDefine.h"
#include "WbxAeAJBPolicy.h"
#include "WseCommonTypes.h"
//#define DEBUGJITTER



#define PEAK_NUM 16
#define SPIKE_NUM_REQUIRED 6



class CWbxAJBPolicyWmeOrigin: public IWbxJitterPolicy
{
public:
	CWbxAJBPolicyWmeOrigin();
	~CWbxAJBPolicyWmeOrigin();
	// from IWbxJitterPolicy
	virtual int JitterPolicy(unsigned int ssrc, unsigned int samplesPerMS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, unsigned int& playbackTime);
	virtual int GetJitterInformation(WbxAEAJBStatistics& jitterStatistics);
	virtual int SetDelay(int ulMinDelay, int ulMaxDelay);
	virtual int Reset();
	virtual int GetAverageDelay();
	virtual int ResetJitterStatistics();

    
    virtual int GetDelay(int &nMinDelay, int &nMaxDelay) {
        nMinDelay = m_nMinDelay;
        nMinDelay = m_nMaxDelay;
        return 0;
    }
    
    
#ifdef DEBUGJITTER
	void SaveLogStringToFile(char* lpsz);
#endif

private:
  typedef enum
  {
    AJB_MODE_NORMAL = 0,
    AJB_MODE_SPIKE
  }AJB_MODE;
  
	int                     m_nFramesInPacket;

	int                     m_nSpikeNum;
	int					    m_nPeakHeight[PEAK_NUM];/*save the height of the latest PEAK_NUM peaks */
	unsigned int					m_dwLastPeakArrivedTime;
	unsigned int					m_dwMaxPeakPeriod;
	BOOL					m_bIsFirstPeak;

	int								m_nMaxJitter;
	int								m_nMinJitter;
	int								m_nAverageJitter;

	unsigned int					m_dwTotalReceivedCount;
	unsigned int					m_dwTotalLostCount;
	unsigned short					m_dwReceivedMaxSeq;

	unsigned short						m_wLastSequence;
	unsigned int					m_dwLastRTPTimeStamp;
	unsigned int					m_dwLastArrivedTime;
	unsigned int					m_dwLastEstPlayTime;
	unsigned int*					m_pLastFewRTPTimeStamp;
	unsigned int*					m_pLastFewArrivedTime;

	int                         m_nLastFewPnr;
	BOOL                        m_bIsFirstRound;

	BOOL						m_bIsFirstFrameArrived;

	unsigned short						m_wFirstSequence;
	unsigned int					m_dwFirstRTPTimeStamp;
	unsigned int					m_dwFirstArrivedTime;
	unsigned int					m_dwFirstEstPlayTime;

	int								m_nCurrentJitter;
	int								m_nPreviousJitter;
	int								m_nCurrentJitterVar;                          
	int								m_nPreviousJitterVar;

	int								m_nCurrentOff;
	int								m_nPreviousOff;

	AJB_MODE				m_AJBMode;

	int								m_nMinDelay;
	int								m_nMaxDelay;

	int								m_nPlayTimeBase;
	int								m_nJitConDecCon;
	int								m_nConDecMultipleCnt;

	int					m_nThreshold;

};

extern "C"
{
	int CreateIWbxJitterPolicyOrigin(IWbxJitterPolicy** ppIWbxJitterPolicy);

	int DestoryIWbxJitterPolicyOrigin(IWbxJitterPolicy* pIWbxJitterPolicy);

};
