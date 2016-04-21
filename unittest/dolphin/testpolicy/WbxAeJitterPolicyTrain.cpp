#include "WbxAeJitterPolicyTrain.h"
#include "WbxAeAJBInterface.h"
#include "WbxAeTrace.h"
#ifdef WIN32
#else
#include "safe_mem_lib.h"
#endif


#define AVERAGE_JITTER_PARAMETER		0.95f

#define ALPHA_PARAMETER		0.25f

//Ross, MAX_HANDLE_JITTER is the jitter can handle max jitter, some case may generate max jitter
// 1. TCP network broken, and failover, the first boudle packets' jitter maybe bigger than it
// 2. really bad network, this may hard happen
// Whenever jitter or Off is bigger than this value, we will not used it to decide jitter buffer delay.
#define MAX_HANDLE_JITTER   3500

// MIN HANDLE JITTER, for big jitter issue, some packet is less than this value, should not update jitter
// otherwise, jitter will be AVG to be small, make AVG jitter change too quickly
#define MIN_HANDLE_JITTER   20


#define ADJUST_K			3

#define JITTER_ABNORMAL_THRESHOLD 2000
#define POLICY_INFO_TRACE(level, str) AE_INFO_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE(level, str) AE_WARNING_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE(level, str) AE_ERROR_TRACE(level, "[AJB policy]:"<<str)


#define POLICY_INFO_TRACE_THIS(level, str) AE_INFO_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE_THIS(level, str) AE_WARNING_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE_THIS(level, str) AE_ERROR_TRACE_THIS(level, "[AJB policy]:"<<str)


CWbxAJBPolicyTrain::CWbxAJBPolicyTrain()
{
	m_nMaxJitter = 0;
	m_nMinJitter = 0;
	m_nAverageJitter = 0;

	m_dwTotalReceivedCount = 0;
	m_dwTotalLostCount = 0;
	m_dwReceivedMaxSeq = 0;

	m_wLastSequence = 0;
	m_dwLastRTPTimeStamp = 0;
	m_dwLastArrivedTime = 0;
	m_dwLastEstPlayTime = 0;


	m_bIsFirstFrameArrived = FALSE;
	m_wFirstSequence = 0;
	m_dwFirstRTPTimeStamp = 0;
	m_dwFirstArrivedTime = 0;
	m_dwFirstEstPlayTime = 0;
	m_nCurrentAVGJitter = 0;
	m_nPreviousJitter = 0;
	m_nPreviousMAXJitter = MIN_HANDLE_JITTER;

	m_iJitterDirection = DIRECTION_UP;

	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	m_AJBMode = AJB_MODE_NORMAL;

	m_nPlayTimeBase = m_nMinDelay = WBXAE_AJB_DEFAULT_MIN_DELAY * 2;
	m_nMaxDelay = WBXAE_AJB_DEFAULT_MAX_DELAY;
	m_nJitConDecCon = 0;
	m_nConDecMultipleCnt = 0;

	m_nThreshold1 = WBXAE_AJB_DEFAULT_MIN_DELAY * 25;
	m_nThreshold2 = WBXAE_AJB_DEFAULT_MIN_DELAY * 2;
	m_dwVar = 0;
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyTrain::CWbxAJBPolicyTrain end!");
}
CWbxAJBPolicyTrain::~CWbxAJBPolicyTrain()
{
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyTrain::~CWbxAJBPolicyTrain end!");

}
	// from IWbxJitterPolicy
int CWbxAJBPolicyTrain::JitterPolicy(unsigned int ssrc, unsigned int samplesPer10MS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, bool bFirstRTP , unsigned int& playbackTime)
{
	if ( samplesPer10MS <= 0 || samplesPer10MS > 480)
	{
		return WBXAE_AJB_ERROR_INVALIDE_PARAMS;
	}

	if (m_bIsFirstFrameArrived == FALSE)
	{
		m_bIsFirstFrameArrived = TRUE;
		m_dwLastRTPTimeStamp = m_dwFirstRTPTimeStamp = (unsigned int)rtpTimestamp;
		m_dwReceivedMaxSeq = m_wLastSequence = m_wFirstSequence = (WORD) rtpSequce;
		m_dwLastArrivedTime = m_dwFirstArrivedTime = receivedTime;
		playbackTime = m_dwLastEstPlayTime = m_dwFirstEstPlayTime = receivedTime + m_nPlayTimeBase;
		m_dwTotalReceivedCount = 1;
		m_nCurrentOff = 0 ;
		m_nPreviousOff = 0;
		m_nMaxJitter = 0;
		m_nMinJitter = 0;
		m_nAverageJitter = 0;

		m_dwTotalReceivedCount = 0;
		m_dwTotalLostCount = 0;
		m_dwReceivedMaxSeq = 0;
		m_nCurrentAVGJitter = 0;
		m_nPreviousJitter = 0;
		m_nPreviousMAXJitter = MIN_HANDLE_JITTER;
		POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyTrain::JitterPolicy, Rec ssrc:" << ssrc 
			<<" first packet!, rtpSequce:" <<rtpSequce
			<<" rtpTimestamp:" <<rtpTimestamp
			<<", receivedTime:" <<receivedTime
			<<", playbackTime:" <<playbackTime);
	}
	else
	{
		m_dwTotalReceivedCount++;

		//implement formula D(i,i-1) = (Ri-Ri-1) -(Si-Si-1);
		int twoFrameDifference =(int) (receivedTime - m_dwLastArrivedTime) 
			-(int)(rtpTimestamp-m_dwLastRTPTimeStamp) / (int)samplesPer10MS * 10;
		if (int(receivedTime - m_dwLastArrivedTime) < 0 )
		{
			POLICY_ERROR_TRACE_THIS(0, "CWbxAJBPolicyTrain::JitterPolicy, timer reserved! time diff:" << int(receivedTime - m_dwLastArrivedTime));
		}

		int frameJump = rtpSequce - m_dwReceivedMaxSeq;
		if (frameJump >= 1)
		{
			m_dwReceivedMaxSeq = rtpSequce;
		}
		if (frameJump > 1)
		{
			m_dwTotalLostCount += frameJump - 1;

		}

		if (frameJump < 0 && m_dwTotalLostCount > 0)
		{
			m_dwTotalLostCount = m_dwTotalLostCount - 1;
		}

		DIRECTION iJitterDirection = m_iJitterDirection;

		//keep not change if same value twoFrameDifference==m_nPreviousJitter 
		if ( twoFrameDifference > m_nPreviousJitter )
			iJitterDirection = DIRECTION_UP;
		else if ( twoFrameDifference < m_nPreviousJitter )
			iJitterDirection = DIRECTION_DOWN;

		if ( iJitterDirection == DIRECTION_DOWN && m_iJitterDirection == DIRECTION_UP //only get a period's biggest jitter for AVG
			&& m_nPreviousJitter> MIN_HANDLE_JITTER      //drop litter jitter, to avoid jitter chang too quickly
			&& m_nPreviousJitter < MAX_HANDLE_JITTER     //drop larg jitter, to avoid network failover case issue
			)		
		{
			m_nPreviousMAXJitter = m_nPreviousJitter; 

			//implement formula Ji=Ji-1+(D(i,i-1)-Ji-1)/16
			m_nCurrentAVGJitter =(int) ( 15.0*m_nCurrentAVGJitter+abs(m_nPreviousMAXJitter))/16.0;
		}

		if ( m_nPreviousJitter > MAX_HANDLE_JITTER )
		{
			POLICY_WARNING_TRACE_THIS(1, "CWbxAJBPolicyTrain::JitterPolicy, cannot be handle m_nPreviousJitter=" << m_nPreviousJitter);
		}

		m_iJitterDirection = iJitterDirection;
		m_nPreviousJitter = twoFrameDifference;



		//implement formula off=Ri-tsi*ri,ri is constant
		if ( !bFirstRTP )
		{
			int TmpOff = (int)(receivedTime - m_dwFirstArrivedTime)
				- (int)(rtpTimestamp - m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 ;

			if ( TmpOff < MAX_HANDLE_JITTER )
				m_nCurrentOff = TmpOff;
			else
				POLICY_WARNING_TRACE_THIS(1, "CWbxAJBPolicyTrain::JitterPolicy, cannot be handle CurrentOff=" << TmpOff);

		}
		else
		{
			//for speaker start flag, just reset first RTP information
			m_dwFirstArrivedTime = receivedTime;
			m_dwFirstRTPTimeStamp = (unsigned int)rtpTimestamp;
		}

		int instantOff = m_nCurrentOff;

/*		if (abs(instantOff) >=JITTER_ABNORMAL_THRESHOLD) // if it's abnormal, it need reset
		{
			
			POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyTrain::JitterPolicy, Rec ssrc:"<<ssrc
				<<",  instantOff is " <<instantOff <<", so doing reset!"
				<<", rtpSequce:" <<rtpSequce
				<<", m_wLastSequence:" <<m_wLastSequence
				<<",rtpTimestamp:  " <<rtpTimestamp
				<<", m_dwLastRTPTimeStamp:" <<m_dwLastRTPTimeStamp
				<<", receivedTime - m_dwFirstArrivedTime:" <<receivedTime - m_dwFirstArrivedTime
				<<", rtpTimestamp - m_dwFirstRTPTimeStamp:" <<rtpTimestamp - m_dwFirstRTPTimeStamp
				<<", samplesPer10MS:" <<samplesPer10MS);
			Reset();
			m_dwLastRTPTimeStamp  = rtpTimestamp;
			m_wLastSequence  = (WORD)rtpSequce;
			m_dwLastArrivedTime = receivedTime;

			m_bIsFirstFrameArrived = TRUE;
			m_dwLastRTPTimeStamp = m_dwFirstRTPTimeStamp = rtpTimestamp;
			m_dwReceivedMaxSeq = m_wLastSequence = m_wFirstSequence = (WORD)rtpSequce;
			m_dwLastArrivedTime = m_dwFirstArrivedTime = receivedTime;
			playbackTime = m_dwLastEstPlayTime = m_dwFirstEstPlayTime = receivedTime + m_nPlayTimeBase;
			m_dwTotalReceivedCount = 1;


			m_nPreviousOff /=2;
			m_nCurrentAVGJitter /= 2;
			playbackTime = m_dwLastEstPlayTime = receivedTime + m_nPlayTimeBase;
			
			return WBXAE_SUCCESS;
		}*/

		if (m_nMinJitter > m_nCurrentAVGJitter)
		{
			m_nMinJitter = m_nCurrentAVGJitter;
		}

		if (m_nMaxJitter < m_nCurrentAVGJitter)
		{
			m_nMaxJitter = m_nCurrentAVGJitter;
		}

		m_nAverageJitter =(int)( m_nAverageJitter * AVERAGE_JITTER_PARAMETER + (1 - AVERAGE_JITTER_PARAMETER) * m_nCurrentAVGJitter);
		
		//detect SPIKE
		if(AJB_MODE_NORMAL == m_AJBMode)
		{
			if (abs(m_nCurrentAVGJitter) > m_nThreshold1 + m_nPlayTimeBase)
			{
				m_dwVar = 0;
				m_AJBMode = AJB_MODE_SPIKE;
			}
		}
		else
		{
			m_dwVar =(m_dwVar / 2 +m_nCurrentAVGJitter / 2);
			if (abs(m_dwVar) < m_nThreshold2 )
			{
				m_AJBMode = AJB_MODE_NORMAL;
			}
		}


		//Modify m_nOffiCurrent;
		if(AJB_MODE_NORMAL == m_AJBMode)
		{
			m_nCurrentOff = (int)( ALPHA_PARAMETER*m_nCurrentOff+(1-ALPHA_PARAMETER)*m_nPreviousOff);

		}
		else
		{
			m_nCurrentOff = m_nPreviousOff + m_nCurrentAVGJitter;
			POLICY_WARNING_TRACE_THIS(1, "CWbxAJBPolicyTrain::JitterPolicy, change to spike mode!, m_nPreviousOff:"  << m_nPreviousOff 
				<<", m_nCurrentAVGJitter:" <<m_nCurrentAVGJitter
				<<", m_nCurrentOff:" <<m_nCurrentOff);
		}



		int adjustTime = m_nCurrentOff+ ADJUST_K*m_nCurrentAVGJitter;
		if (adjustTime > m_nMaxDelay)
		{
			adjustTime = m_nMaxDelay;
		}



		// if it has disorder, for example, first packet is N, and the sencond packet is N-1,
		// if duration is unsigned int, the duration will be a bigger value
		int duration = 0;

		duration  = (int)(rtpTimestamp-m_dwFirstRTPTimeStamp)* 10 / (int)samplesPer10MS  ;

		int tmpBase = 0;
		if (adjustTime < 0 && instantOff < 0)
		{
			playbackTime = (unsigned int )(duration + m_dwFirstArrivedTime + m_nPlayTimeBase) ;
			tmpBase = (int)(0.975 * m_nPlayTimeBase + 0.025 * adjustTime);
			
		}
		else if (adjustTime < m_nPlayTimeBase * 1.25)
		{
			playbackTime = (unsigned int )(duration + m_dwFirstArrivedTime + m_nPlayTimeBase);
			tmpBase = (int)(0.998 * m_nPlayTimeBase + 0.002 * adjustTime);
		}
		else
		{
			playbackTime = (unsigned int )(duration + m_dwFirstArrivedTime + adjustTime);
			tmpBase = (int)(0.9875 * m_nPlayTimeBase + 0.0125 * adjustTime);
		}
		
//		playbackTime += 20;


		if (tmpBase - m_nPlayTimeBase < 0 )
		{
			if (m_nJitConDecCon++ >= 25) // change to 500ms to fast reduce delay.
			{
				m_nConDecMultipleCnt ++;
				if (m_nConDecMultipleCnt <= 5)
				{
					m_nPlayTimeBase	-= m_nConDecMultipleCnt;
				}
				else if (m_nConDecMultipleCnt <= 10)
				{
					m_nPlayTimeBase	-= 5;
				}
				else
				{
					m_nPlayTimeBase	-= 20;

				}

				m_nJitConDecCon = 0;
			}
			else
			{

			}
		}
		else
		{
			m_nJitConDecCon = 0;
			m_nConDecMultipleCnt = 0;
			m_nPlayTimeBase = tmpBase;
		}
// 		if (adjustTime < 0)
// 		{
// 			// it means that send side spead is higher than receive side, need play fast
// 			playbackTime = duration // duration
// 				+ m_dwFirstArrivedTime // first arrive time
// 				+ m_nPlayTimeBase + adjustTime;  // adjust base
// 			m_nPlayTimeBase = 0.9975 * m_nPlayTimeBase + 0.0025 * adjustTime;
// 
// 		}
// 		else if	(adjustTime <= m_nPlayTimeBase  + 60)
// 		{
// 			playbackTime = duration // duration
// 				+ m_dwFirstArrivedTime // first arrive time
// 				+ m_nPlayTimeBase;  // adjust base
// 				m_nPlayTimeBase = 0.995 * m_nPlayTimeBase + 0.005 * adjustTime;
// 			
// 		}
// 		else
// 
// 		{
// 			playbackTime = receivedTime  // frame duration
// 				+ 0.85 * adjustTime + 0.15 * m_nPlayTimeBase; // first arrive time
// 			m_nPlayTimeBase = + 0.85 * adjustTime + 0.15 * m_nPlayTimeBase;
// 		}

		if (m_nPlayTimeBase < m_nMinDelay)
		{
			m_nPlayTimeBase = m_nMinDelay;
		}

		if (m_nPlayTimeBase > m_nMaxDelay)
		{
			m_nPlayTimeBase = m_nMaxDelay;
		}

// 		if (playbackTime - receivedTime < m_nMinDelay && )
// 		{
// 			if (1)
// 			{
// 				AJBPOLICY_WARNING_TRACE("CWbxAJBPolicyTrain::JitterPolicy, *** playbackTime: " << playbackTime
// 					<<", receivedTime:" <<receivedTime
// 					<<", playbackTime - receivedTime:" <<playbackTime - receivedTime
// 					<<", m_nMinDelay:" << m_nMinDelay
// 					<<", duration:" <<duration
// 					<<", m_dwFirstArrivedTime:" <<m_dwFirstArrivedTime
// 					<<", adjustTime:" <<adjustTime);
// 			}
// 			playbackTime = receivedTime + m_nMinDelay;
// 		}
		// debug trace here!


		POLICY_INFO_TRACE_THIS(1, "CWbxAJBPolicyTrain::JitterPolicy, Rec ssrc:" <<ssrc 
			<<"seq: " << rtpSequce
			<<", timestamp:" <<rtpTimestamp <<"( "<<((int)(rtpTimestamp -  m_dwLastRTPTimeStamp)) /(int)samplesPer10MS * 10<<" ms)"
			<<", bFirst:" << bFirstRTP
			<<", RL (" << receivedTime - m_dwLastArrivedTime
			<<"ms), ellips " << (int)(rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10
			<<"ms, RF:" << receivedTime - m_dwFirstArrivedTime
			<<" ms, RF -ellips : "<< (int)( receivedTime - m_dwFirstArrivedTime - (int)(rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 )
			<<" ms, RTC:" << receivedTime
			<<", PTC:" << playbackTime
			<<", delayed " <<(int) (playbackTime - receivedTime)
			<<"to play, Base:" <<m_nPlayTimeBase
			<<", TFDifference:" << twoFrameDifference
			<<", m_nPreviousMAXJitter:" << m_nPreviousMAXJitter
			<<", adjust:" << adjustTime
			<<", InstantOffNew:" << instantOff
			<<", CurOff:" << m_nCurrentOff
			<<", CurJitter:" <<m_nCurrentAVGJitter
			<<", PreOff:" <<m_nPreviousOff
			<<", MinDelay:" << m_nMinDelay
			<<",MinJitter:" <<m_nMinJitter
			<<",MaxJitter:" <<m_nMaxJitter
			<<",AveJitter:" << m_nAverageJitter
			<<", SamplesPer10MS:" <<samplesPer10MS
			<<", timestamp diff(ul):" <<rtpTimestamp <<"( "<<(rtpTimestamp -  m_dwLastRTPTimeStamp) /samplesPer10MS * 10<<" ms)"
			);

		m_dwLastRTPTimeStamp  = rtpTimestamp;
		m_wLastSequence  = (WORD)rtpSequce;
		m_dwLastArrivedTime = receivedTime;

		m_nPreviousOff = m_nCurrentOff;
		

	}
	return WBXAE_SUCCESS;
}


int CWbxAJBPolicyTrain::GetJitterInformation(WbxAEAJBStatisticsTrain& jitterStatistics)
{
	jitterStatistics.averageJitter = m_nAverageJitter;
	jitterStatistics.maxJitter = m_nMaxJitter;
	jitterStatistics.minJitter = m_nMinJitter;
	jitterStatistics.lostNums = m_dwTotalLostCount;
	jitterStatistics.receivedNums = m_dwTotalLostCount;
	jitterStatistics.lostRate  = 0;
	if (m_dwTotalReceivedCount > 0 && m_dwTotalLostCount > 0)
	{
		jitterStatistics.lostRate  = (1.0 * m_dwTotalLostCount) / (m_dwTotalReceivedCount + m_dwTotalLostCount);
	}
	m_nMaxJitter = 0;
	m_nMinJitter = 0;
	m_nAverageJitter = 0;
	m_dwTotalReceivedCount = 0;
	m_dwTotalLostCount = 0;
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyTrain::Reset()
{
	m_nMaxJitter = 0;
	m_nMinJitter = 0;
	m_nAverageJitter = 0;

	m_dwTotalReceivedCount = 0;
	m_dwTotalLostCount = 0;
	m_dwReceivedMaxSeq = 0;

	m_wLastSequence = 0;
	m_dwLastRTPTimeStamp = 0;

	m_bIsFirstFrameArrived = FALSE;
	m_wFirstSequence = 0;
	m_dwFirstRTPTimeStamp = 0;
	m_dwFirstArrivedTime = 0;
	m_dwFirstEstPlayTime = 0;
	m_nCurrentAVGJitter = 0;


	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	m_AJBMode = AJB_MODE_NORMAL;
	if (m_nPlayTimeBase* 0.85 > m_nMinDelay)
	{
		m_nPlayTimeBase =  m_nPlayTimeBase * 0.85;
	}
	else
	{
		m_nPlayTimeBase = m_nMinDelay;
	}
	m_dwVar = 0;
	m_nJitConDecCon = 0;

	POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyTrain::Reset, after reset m_nPlayTimeBase:" <<m_nPlayTimeBase);
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyTrain::GetAverageDelay()
{
	return m_nPlayTimeBase;
}


int CWbxAJBPolicyTrain::SetDelay(int nMinDelay, int nMaxDelay)
{
	if ( nMinDelay > WBXAE_AJB_MAX_DELAY ||nMaxDelay > WBXAE_AJB_MAX_DELAY )
	{
		return WBXAE_AJB_ERROR_SET_DELAY_FAILED;
	}
	m_nPlayTimeBase = m_nMinDelay = nMinDelay;
	m_nMaxDelay = nMaxDelay;
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyTrain::SetDelay, m_nMinDelay:" << m_nMinDelay
		<<", m_nMaxDelay:" << m_nMaxDelay
		<<", m_nPlayTimeBase:" <<m_nPlayTimeBase);
	return WBXAE_SUCCESS;
}


extern "C"
{
	int CreateIWbxJitterPolicyTrain(IWbxJitterPolicyTrain** ppIWbxJitterPolicy)
	{
		int result = WBXAE_AJB_ERROR_BASE;
		if (ppIWbxJitterPolicy)
		{
			CWbxAJBPolicyTrain *pWbxAJBPolicy = new CWbxAJBPolicyTrain();
			if( pWbxAJBPolicy )
			{
				*ppIWbxJitterPolicy = pWbxAJBPolicy;
				result = WBXAE_SUCCESS;
			}
		}
		return result;
	}

	int DestoryIWbxJitterPolicyTrain(IWbxJitterPolicyTrain* pIWbxJitterPolicy)
	{
		int result = WBXAE_AJB_ERROR_BASE;
		if (pIWbxJitterPolicy)
		{
			delete pIWbxJitterPolicy;
			pIWbxJitterPolicy = NULL;
			result = WBXAE_SUCCESS;
		}
		return result;
	}

};
