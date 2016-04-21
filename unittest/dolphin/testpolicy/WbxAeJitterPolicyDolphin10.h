#include "WbxAeDefine.h"
#include "WbxAeAJBPolicy.h"




class CWbxAJBPolicyDolphin10: public IWbxJitterPolicy
{
public:
	CWbxAJBPolicyDolphin10();
	~CWbxAJBPolicyDolphin10();
	// from IWbxJitterPolicy
	virtual int JitterPolicy(unsigned int ssrc, unsigned int samplesPerMS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, unsigned int& playbackTime);
	virtual int GetJitterInformation(WbxAEAJBStatistics& jitterStatistics);
	virtual int SetDelay(int ulMinDelay, int ulMaxDelay);
	virtual int Reset();
	virtual int GetAverageDelay();
    virtual int GetDelay(int &nMinDelay, int &nMaxDelay) {
        nMinDelay = m_nMinDelay;
        nMinDelay = m_nMaxDelay;
        return 0;
    }
private:
  typedef enum
  {
    AJB_MODE_NORMAL = 0,
    AJB_MODE_SPIKE
  }AJB_MODE;
  
private:
	int								m_nMaxJitter;
	int								m_nMinJitter;
	int								m_nAverageJitter;

	unsigned int					m_dwTotalReceivedCount;
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

	int								m_nCurrentJitter;
	int								m_nPreviousJitter;


	int								m_nCurrentOff;
	int								m_nPreviousOff;

	AJB_MODE				m_AJBMode;
	
	int								m_nMinDelay;
	int								m_nMaxDelay;

	int								m_nPlayTimeBase;
	int								m_nJitConDecCon;

	int					m_nThreshold1;
	int					m_nThreshold2;
	int					m_dwVar;
};

extern "C"
{
	int CreateIWbxJitterPolicyDolphin10(IWbxJitterPolicy** ppIWbxJitterPolicy);

	int DestoryIWbxJitterPolicyDolphin10(IWbxJitterPolicy* pIWbxJitterPolicy);

};
