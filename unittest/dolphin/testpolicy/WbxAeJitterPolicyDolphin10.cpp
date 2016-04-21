#include "WbxAeJitterPolicyDolphin10.h"
#include "WbxAeAJBInterface.h"
#include "WbxAeTrace.h"
#ifdef WIN32
#else
#include "safe_mem_lib.h"
#endif


#define AVERAGE_JITTER_PARAMETER		0.95f

#define ALPHA_PARAMETER		0.25f


#define ADJUST_K			2

#define POLICY_INFO_TRACE(level, str) AE_INFO_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE(level, str) AE_WARNING_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE(level, str) AE_ERROR_TRACE(level, "[AJB policy]:"<<str)


#define POLICY_INFO_TRACE_THIS(level, str) AE_INFO_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE_THIS(level, str) AE_WARNING_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE_THIS(level, str) AE_ERROR_TRACE_THIS(level, "[AJB policy]:"<<str)


CWbxAJBPolicyDolphin10::CWbxAJBPolicyDolphin10()
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
	m_nCurrentJitter = 0;
	m_nPreviousJitter = 0;

	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	m_AJBMode = AJB_MODE_NORMAL;

	m_nPlayTimeBase = m_nMinDelay = WBXAE_AJB_DEFAULT_MIN_DELAY;
	m_nMaxDelay = WBXAE_AJB_DEFAULT_MAX_DELAY;
	m_nJitConDecCon = 0;

	m_nThreshold1 = WBXAE_AJB_DEFAULT_MIN_DELAY * 4;
	m_nThreshold2 = WBXAE_AJB_DEFAULT_MIN_DELAY * 2;
	m_dwVar = 0;
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::CWbxAJBPolicyDolphin10 end!");
}
CWbxAJBPolicyDolphin10::~CWbxAJBPolicyDolphin10()
{
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::~CWbxAJBPolicyDolphin10 end!");

}
	// from IWbxJitterPolicy
int CWbxAJBPolicyDolphin10::JitterPolicy(unsigned int ssrc, unsigned int samplesPer10MS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, unsigned int& playbackTime)
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
		m_nCurrentJitter = 0;
		m_nPreviousJitter = 0;
		POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::JitterPolicy, Rec ssrc:" << ssrc 
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
			POLICY_ERROR_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::JitterPolicy, timer reserved! time diff:" << int(receivedTime - m_dwLastArrivedTime));
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

		//implement formula Ji=Ji-1+(D(i,i-1)-Ji-1)/16
		m_nCurrentJitter =(int) ((15.0*m_nPreviousJitter+abs(twoFrameDifference))/16.0);

		//implement formula off=Ri-tsi*ri,ri is constant
		m_nCurrentOff = (int)(receivedTime - m_dwFirstArrivedTime)
			- (int)(rtpTimestamp - m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 ;
		int instantOff = m_nCurrentOff;

		if (abs(instantOff) >=800) // if it's abnormal, it need reset
		{
			
			POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::JitterPolicy, Rec ssrc:"<<ssrc
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
			m_nPreviousJitter /= 2;
			playbackTime = m_dwLastEstPlayTime = receivedTime + m_nPlayTimeBase;
			
			return WBXAE_SUCCESS;
		}
		if (m_nMinJitter > m_nCurrentJitter)
		{
			m_nMinJitter = m_nCurrentJitter;
		}

		if (m_nMaxJitter < m_nCurrentJitter)
		{
			m_nMaxJitter = m_nCurrentJitter;
		}

		m_nAverageJitter =(int)( m_nAverageJitter * AVERAGE_JITTER_PARAMETER + (1 - AVERAGE_JITTER_PARAMETER) * m_nCurrentJitter);
		
		//detect SPIKE
		if(AJB_MODE_NORMAL == m_AJBMode)
		{
			if (abs(m_nCurrentJitter) > m_nThreshold1 )
			{
				m_dwVar = 0;
				m_AJBMode = AJB_MODE_SPIKE;
			}
		}
		else
		{
			m_dwVar =(m_dwVar / 2 +m_nCurrentJitter / 2);
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
			m_nCurrentOff = m_nPreviousOff + m_nCurrentJitter;
			POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::JitterPolicy, change to spike mode!");
		}



		int adjustTime = m_nCurrentOff+ ADJUST_K*m_nCurrentJitter;
		if (adjustTime > m_nMaxDelay)
		{
			adjustTime = m_nMaxDelay;
		}



		unsigned int duration = 0;

		duration  = (rtpTimestamp-m_dwFirstRTPTimeStamp)* 10 / samplesPer10MS  ;

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
		
		if (tmpBase - m_nPlayTimeBase < 0 )
		{
			if (m_nJitConDecCon++ >= 50)
			{
				m_nPlayTimeBase	-= 1;
				m_nJitConDecCon = 0;
			}
			else
			{

			}
		}
		else
		{
			m_nJitConDecCon = 0;
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
// 				AJBPOLICY_WARNING_TRACE("CWbxAJBPolicyDolphin10::JitterPolicy, *** playbackTime: " << playbackTime
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


		POLICY_INFO_TRACE_THIS(30, "CWbxAJBPolicyDolphin10::JitterPolicy, Rec ssrc:" <<ssrc 
			<<"seq: " << rtpSequce
			<<", timestamp:" <<rtpTimestamp <<"( "<<(rtpTimestamp -  m_dwLastRTPTimeStamp) /samplesPer10MS * 10<<" ms)"
			<<", RL (" << receivedTime - m_dwLastArrivedTime
			<<"ms), ellips " << (int)(rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10
			<<"ms, RF:" << receivedTime - m_dwFirstArrivedTime
			<<" ms, RF -ellips : "<< (int)( receivedTime - m_dwFirstArrivedTime - (rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 )
			<<" ms, RTC:" << receivedTime
			<<", PTC:" << playbackTime
			<<", delayed " <<(int) (playbackTime - receivedTime)
			<<"to play, Base:" <<m_nPlayTimeBase
			<<", TFDifference:" << twoFrameDifference
			<<", adjust:" << adjustTime
			<<", instantOff:" << instantOff
			<<", CurJitter:" <<m_nCurrentJitter
			<<", MinDelay:" << m_nMinDelay
			<<", CurOff:" << m_nCurrentOff
			<<", PreOff:" <<m_nPreviousOff
			<<",MinJitter:" <<m_nMinJitter
			<<",MaxJitter:" <<m_nMaxJitter
			<<",AveJitter:" << m_nAverageJitter
			<<", SamplesPer10MS:" <<samplesPer10MS);

		m_dwLastRTPTimeStamp  = rtpTimestamp;
		m_wLastSequence  = (WORD)rtpSequce;
		m_dwLastArrivedTime = receivedTime;

		m_nPreviousOff = m_nCurrentOff;
		m_nPreviousJitter = m_nCurrentJitter;
		

	}
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyDolphin10::GetJitterInformation(WbxAEAJBStatistics& jitterStatistics)
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

int CWbxAJBPolicyDolphin10::Reset()
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
	m_nCurrentJitter = 0;
	m_nPreviousJitter *= 0.75;


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

	POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::Reset, after reset m_nPlayTimeBase:" <<m_nPlayTimeBase);
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyDolphin10::GetAverageDelay()
{
	return m_nPlayTimeBase;
}


int CWbxAJBPolicyDolphin10::SetDelay(int nMinDelay, int nMaxDelay)
{
	if ( nMinDelay > WBXAE_AJB_MAX_DELAY ||nMaxDelay > WBXAE_AJB_MAX_DELAY )
	{
		return WBXAE_AJB_ERROR_SET_DELAY_FAILED;
	}
	m_nPlayTimeBase = m_nMinDelay = nMinDelay;
	m_nMaxDelay = nMaxDelay;
	POLICY_INFO_TRACE_THIS(0, "CWbxAJBPolicyDolphin10::SetDelay, m_nMinDelay:" << m_nMinDelay
		<<", m_nMaxDelay:" << m_nMaxDelay
		<<", m_nPlayTimeBase:" <<m_nPlayTimeBase);
	return WBXAE_SUCCESS;
}


extern "C"
{
	int CreateIWbxJitterPolicyDolphin10(IWbxJitterPolicy** ppIWbxJitterPolicy)
	{
		int result = WBXAE_AJB_ERROR_BASE;
		if (ppIWbxJitterPolicy)
		{
			CWbxAJBPolicyDolphin10 *pWbxAJBPolicy = new CWbxAJBPolicyDolphin10();
			if( pWbxAJBPolicy )
			{
				*ppIWbxJitterPolicy = pWbxAJBPolicy;
				result = WBXAE_SUCCESS;
			}
		}
		return result;
	}

	int DestoryIWbxJitterPolicyDolphin10(IWbxJitterPolicy* pIWbxJitterPolicy)
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
