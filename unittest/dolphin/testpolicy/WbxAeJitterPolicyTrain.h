#include "WbxAeDefine.h"



typedef struct tagWbxAEAJBStatisticsTrain
{
	unsigned int receivedNums;
	unsigned int lostNums;
	int maxJitter;
	int minJitter;
	int averageJitter;
	double lostRate;
	unsigned int dropedDataMs;
  
}WbxAEAJBStatisticsTrain, *pWbxAEAJBStatisticsTrain;



class IWbxJitterPolicyTrain
{
public:
	virtual ~ IWbxJitterPolicyTrain() {};
	virtual int JitterPolicy(unsigned int ssrc, unsigned int samplesPer10MS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp,  bool bFirstRTP, unsigned int& playbackTime) = 0;
	virtual int GetJitterInformation(WbxAEAJBStatisticsTrain& jitterStatistics) = 0;
	virtual int SetDelay(int nMinDelay, int nMaxDelay) = 0;
	virtual int Reset() = 0;
	virtual int GetAverageDelay() = 0;
};


class CWbxAJBPolicyTrain: public IWbxJitterPolicyTrain
{
public:
	CWbxAJBPolicyTrain();
	~CWbxAJBPolicyTrain();
	// from IWbxJitterPolicy
	 int JitterPolicy(unsigned int ssrc, unsigned int samplesPerMS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, bool bFirtRTP , unsigned int& playbackTime);
	 int GetJitterInformation(WbxAEAJBStatisticsTrain& jitterStatistics);
	 int SetDelay(int ulMinDelay, int ulMaxDelay);
	 int Reset();
	 int GetAverageDelay();


private:
	enum DIRECTION { DIRECTION_UP, DIRECTION_DOWN } ;
  typedef enum
  {
    AJB_MODE_NORMAL = 0,
    AJB_MODE_SPIKE
  }AJB_MODE;
  
	int								m_nMaxJitter;
	int								m_nMinJitter;
	int								m_nAverageJitter;

	unsigned int	m_dwTotalReceivedCount;
	unsigned int					m_dwTotalLostCount;
	unsigned int					m_dwReceivedMaxSeq;

	unsigned short						m_wLastSequence;
	unsigned int					m_dwLastRTPTimeStamp;
	unsigned int					m_dwLastArrivedTime;
	unsigned int					m_dwLastEstPlayTime;

	bool						m_bIsFirstFrameArrived;

	unsigned short						m_wFirstSequence;
  unsigned int					m_dwFirstRTPTimeStamp;
	unsigned int					m_dwFirstArrivedTime;
	unsigned int					m_dwFirstEstPlayTime;

	int								m_nCurrentAVGJitter;
	int								m_nPreviousJitter;
	int								m_nPreviousMAXJitter;
	DIRECTION						m_iJitterDirection;


	int								m_nCurrentOff;
	int								m_nPreviousOff;

	AJB_MODE				m_AJBMode;
	
	int								m_nMinDelay;
	int								m_nMaxDelay;

	int								m_nPlayTimeBase;
	int								m_nJitConDecCon;
	int								m_nConDecMultipleCnt;

	int					m_nThreshold1;
	int					m_nThreshold2;
	int					m_dwVar;
};

extern "C"
{
	int CreateIWbxJitterPolicyTrain(IWbxJitterPolicyTrain** ppIWbxJitterPolicy);

	int DestoryIWbxJitterPolicyTrain(IWbxJitterPolicyTrain* pIWbxJitterPolicy);

};
