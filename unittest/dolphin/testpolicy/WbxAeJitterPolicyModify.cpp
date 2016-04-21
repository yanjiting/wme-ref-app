#include "WbxAeJitterPolicyModify.h"
#include "WbxAeAJBInterface.h"
#include "WbxAeTrace.h"

#include "safe_mem_lib.h"

#ifdef DEBUGJITTER
#include <time.h>
#endif


#define AVERAGE_JITTER_PARAMETER		0.975f

#define ALPHA_PARAMETER		0.25f


#define ADJUST_K			3

#define JITTER_ABNORMAL_THRESHOLD 2000
#define POLICY_INFO_TRACE(level, str) AE_INFO_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE(level, str) AE_WARNING_TRACE(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE(level, str) AE_ERROR_TRACE(level, "[AJB policy]:"<<str)


#define POLICY_INFO_TRACE_THIS(level, str) AE_INFO_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_WARNING_TRACE_THIS(level, str) AE_WARNING_TRACE_THIS(level, "[AJB policy]:"<<str)
#define POLICY_ERROR_TRACE_THIS(level, str) AE_ERROR_TRACE_THIS(level, "[AJB policy]:"<<str)


#define WBX_POLICY_INFO_TRACE_THIS(str)    WBXAE_INFO_TRACE_THIS("[AJB.policy]:"<<str)
#define WBX_POLICY_WARNING_TRACE_THIS(str) WBXAE_WARNING_TRACE_THIS("[AJB.policy]:"<<str)
#define WBX_POLICY_ERROR_TRACE_THIS(str)   WBXAE_ERROR_TRACE_THIS("[AJB.policy]:"<<str)
#define WBX_POLICY_DETAIL_TRACE_THIS(str)  WBXAE_DETAIL_TRACE_THIS("[AJB.policy]:"<<str)
#define WBX_POLICY_DEBUG_TRACE_THIS(str)   WBXAE_DEBUG_TRACE_THIS("[AJB.policy]:"<<str)
#define WBX_POLICY_ALL_TRACE_THIS(str)     WBXAE_ALL_TRACE_THIS("[AJB.policy]:"<<str)



#define JITTER_DEC_COUNT 2


#ifdef DEBUGJITTER
static FILE*  s_FileSaveTrace=NULL;
#endif

CWbxAJBPolicyWmeOrigin::CWbxAJBPolicyWmeOrigin()
{
	WBX_POLICY_DEBUG_TRACE_THIS("CWbxAJBPolicyWmeOrigin::CWbxAJBPolicyWmeOrigin(),begin.");
	m_nFramesInPacket = 2;

	m_nSpikeNum = 0;
	m_dwLastPeakArrivedTime = 0;
	m_dwMaxPeakPeriod = 1000;
	m_bIsFirstPeak = FALSE;

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
	m_pLastFewRTPTimeStamp = NULL;
	m_pLastFewArrivedTime = NULL;
	m_nLastFewPnr = 0;


	m_bIsFirstFrameArrived = FALSE;
	m_bIsFirstRound = FALSE;
	m_wFirstSequence = 0;
	m_dwFirstRTPTimeStamp = 0;
	m_dwFirstArrivedTime = 0;
	m_dwFirstEstPlayTime = 0;
	m_nCurrentJitter = 0;
	m_nPreviousJitter = 0;
	m_nCurrentJitterVar = 0;
	m_nPreviousJitterVar = 0;

	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	m_AJBMode = AJB_MODE_NORMAL;

	m_nThreshold = WBXAE_AJB_DEFAULT_MIN_DELAY * 25 ;

	m_nPlayTimeBase = m_nMinDelay = WBXAE_AJB_DEFAULT_MIN_DELAY;
	m_nMaxDelay = WBXAE_AJB_DEFAULT_MAX_DELAY;
	m_nJitConDecCon = 0;
	m_nConDecMultipleCnt = 0;
	cisco_memset_s(m_nPeakHeight, sizeof(int)*PEAK_NUM, 0);
	if(!(m_pLastFewRTPTimeStamp = new unsigned int [m_nFramesInPacket]))
	{
		WBX_POLICY_ERROR_TRACE_THIS("CWbxAJBPolicyWmeOrigin::CWbxAJBPolicyWmeOrigin() , m_pLastFewRTPTimeStamp is NULL!m_nFramesInPacket:"<<m_nFramesInPacket);
		return;
	}
	else
	{
		cisco_memset_s(m_pLastFewRTPTimeStamp, sizeof(unsigned int)*m_nFramesInPacket, 0);
	}

	if(!(m_pLastFewArrivedTime = new unsigned  [m_nFramesInPacket]))
	{
		WBX_POLICY_ERROR_TRACE_THIS( "CWbxAJBPolicyWmeOrigin::CWbxAJBPolicyWmeOrigin() , m_pLastFewArrivedTime is NULL!");
		return;
	}
	else
	{
		cisco_memset_s(m_pLastFewArrivedTime, sizeof(unsigned int)*m_nFramesInPacket, 0);
	}

	WBX_POLICY_DEBUG_TRACE_THIS("CWbxAJBPolicyWmeOrigin::CWbxAJBPolicyWmeOrigin() end!");
#ifdef DEBUGJITTER
	SaveLogStringToFile("CWbxAJBPolicyWmeOrigin() begin!");
#endif
}

CWbxAJBPolicyWmeOrigin::~CWbxAJBPolicyWmeOrigin()
{
	WBX_POLICY_DEBUG_TRACE_THIS("CWbxAJBPolicyWmeOrigin::~CWbxAJBPolicyWmeOrigin(),begin.");
	if (m_pLastFewRTPTimeStamp)
	{
		delete [] m_pLastFewRTPTimeStamp;
		m_pLastFewRTPTimeStamp = NULL;
	}
	if (m_pLastFewArrivedTime)
	{
		delete [] m_pLastFewArrivedTime;
		m_pLastFewArrivedTime = NULL;
	}
	WBX_POLICY_DEBUG_TRACE_THIS( "CWbxAJBPolicyWmeOrigin::~CWbxAJBPolicyWmeOrigin() end!");
	
#ifdef DEBUGJITTER
	if(NULL != s_FileSaveTrace)
	{
		fclose(s_FileSaveTrace);
		s_FileSaveTrace = NULL;
	}
#endif
}

int CWbxAJBPolicyWmeOrigin::ResetJitterStatistics()
{
	m_nCurrentJitter = 0;
	m_nPreviousJitter = 0;
	m_nCurrentJitterVar = 0;
	m_nPreviousJitterVar = 0;
	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	WBX_POLICY_ALL_TRACE_THIS("CWbxAJBPolicyWmeOrigin,JitterStatistics() Reset!");
	return WBXAE_SUCCESS;

}
	// from IWbxJitterPolicy
int CWbxAJBPolicyWmeOrigin::JitterPolicy(unsigned int ssrc, unsigned int samplesPer10MS, unsigned int receivedTime, unsigned short rtpSequce,  unsigned int rtpTimestamp, unsigned int& playbackTime)
{
#ifdef DEBUGJITTER
	char charLog[100];
#endif

	if ( samplesPer10MS <= 0 || samplesPer10MS > 480)
	{
		return WBXAE_AJB_ERROR_INVALIDE_PARAMS;
	}

	if (m_bIsFirstFrameArrived == FALSE)
	{
		m_bIsFirstFrameArrived = TRUE;
		m_bIsFirstRound = FALSE;
		m_nLastFewPnr = 0;

		m_pLastFewRTPTimeStamp[m_nLastFewPnr] = m_dwLastRTPTimeStamp = m_dwFirstRTPTimeStamp = (unsigned int)rtpTimestamp;
		m_dwReceivedMaxSeq = m_wLastSequence = m_wFirstSequence =  rtpSequce;
		m_pLastFewArrivedTime[m_nLastFewPnr] = m_dwLastArrivedTime = m_dwFirstArrivedTime = receivedTime;
		playbackTime = m_dwLastEstPlayTime = m_dwFirstEstPlayTime = receivedTime + m_nPlayTimeBase;
		m_dwTotalReceivedCount = 1;
		m_nCurrentOff = 0;
		m_nPreviousOff = 0;
		m_nMaxJitter = 0;
		m_nMinJitter = 0;
		m_nAverageJitter = 0;

		m_dwTotalReceivedCount = 0;
		m_dwTotalLostCount = 0;
		m_dwReceivedMaxSeq = 0;
//		m_nCurrentJitter = 0;
//		m_nPreviousJitter = 0;
		
		if(++m_nLastFewPnr == m_nFramesInPacket)
		{
			m_bIsFirstRound = TRUE;
			m_nLastFewPnr = 0;
		}
		
		WBX_POLICY_DEBUG_TRACE_THIS( "CWbxAJBPolicyWmeOrigin::JitterPolicy(), Rec ssrc:" << ssrc 
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
		int twoPktDifference = twoFrameDifference;
		
		if(m_bIsFirstRound == FALSE)
		{				
			m_pLastFewRTPTimeStamp[m_nLastFewPnr] = rtpTimestamp;
			m_pLastFewArrivedTime[m_nLastFewPnr] = receivedTime;

			if(m_nLastFewPnr++ == m_nFramesInPacket-1)
			{
				m_bIsFirstRound = TRUE;
				m_nLastFewPnr = 0;
			}
		}
		else
		{
			twoPktDifference =(int) (receivedTime - m_pLastFewArrivedTime[m_nLastFewPnr]) 
				-(int)(rtpTimestamp-m_pLastFewRTPTimeStamp[m_nLastFewPnr]) / (int)samplesPer10MS * 10;
			

			
			WBX_POLICY_DETAIL_TRACE_THIS("CWbxAJBPolicyWmeOrigin::JitterPolicy(), pkt IAT("<<(int)(receivedTime - m_pLastFewArrivedTime[m_nLastFewPnr])<<" ms),"
									<<"pkt timestamp diff("<<(int)(rtpTimestamp-m_pLastFewRTPTimeStamp[m_nLastFewPnr]) / (int)samplesPer10MS * 10<<" ms),"			
									<<"twoPktDifference("<<(int)twoPktDifference<<" ms)"				
										);

			m_pLastFewRTPTimeStamp[m_nLastFewPnr] = rtpTimestamp;
			m_pLastFewArrivedTime[m_nLastFewPnr] = receivedTime;

			if (int(receivedTime - m_dwLastArrivedTime) < 0 || int(receivedTime - m_pLastFewArrivedTime[m_nLastFewPnr]) < 0)
		    {
				WBX_POLICY_ERROR_TRACE_THIS( "CWbxAJBPolicyWmeOrigin::JitterPolicy(), timer reserved! time diff:" << int(receivedTime - m_dwLastArrivedTime)
											<<"Packet diff:" << int(receivedTime - m_dwLastArrivedTime));
			}

			if(m_nLastFewPnr++ == m_nFramesInPacket - 1)
			{
				m_nLastFewPnr = 0;
			}
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
		m_nCurrentJitter =(int) ((15.0*m_nPreviousJitter+abs(twoPktDifference))/16.0);

		//implement formula off=Ri-tsi*ri,ri is constant
		m_nCurrentOff = (int)(receivedTime - m_dwFirstArrivedTime)
			- (int)(rtpTimestamp - m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 ;
		int instantOff = m_nCurrentOff;

		if (abs(instantOff) >=JITTER_ABNORMAL_THRESHOLD) // if it's abnormal, it need reset
		{
			
			POLICY_WARNING_TRACE_THIS(0, "CWbxAJBPolicyWmeOrigin::JitterPolicy, Rec ssrc:"<<ssrc
				<<",  instantOff is " <<instantOff <<", so doing reset!"
				<<", rtpSequce:" <<rtpSequce
				<<", m_wLastSequence:" <<m_wLastSequence
				<<",rtpTimestamp:  " <<rtpTimestamp
				<<", m_dwLastRTPTimeStamp:" <<m_dwLastRTPTimeStamp
				<<", receivedTime - m_dwFirstArrivedTime:" <<receivedTime - m_dwFirstArrivedTime
				<<", rtpTimestamp - m_dwFirstRTPTimeStamp:" <<(int)(rtpTimestamp - m_dwFirstRTPTimeStamp)
          <<", receivedTime - m_pLastFewArrivedTime[m_nLastFewPnr]:" <<(int)(receivedTime - m_pLastFewArrivedTime[m_nLastFewPnr])
				<<", rtpTimestamp - m_pLastFewRTPTimeStamp[m_nLastFewPnr]:" <<(int)(rtpTimestamp - m_pLastFewRTPTimeStamp[m_nLastFewPnr])/samplesPer10MS * 10
				<<", samplesPer10MS:" <<samplesPer10MS);
			
			Reset();

			//reset jitter related statistics
			m_bIsFirstFrameArrived = TRUE;
			m_pLastFewRTPTimeStamp[m_nLastFewPnr] = m_dwLastRTPTimeStamp = m_dwFirstRTPTimeStamp = rtpTimestamp;
			m_pLastFewArrivedTime[m_nLastFewPnr] = m_dwLastArrivedTime = m_dwFirstArrivedTime = receivedTime;
			m_dwReceivedMaxSeq = m_wLastSequence = m_wFirstSequence = (unsigned short)rtpSequce;
			playbackTime = m_dwLastEstPlayTime = m_dwFirstEstPlayTime = receivedTime + m_nPlayTimeBase;
			m_dwTotalReceivedCount = 1;
			
			m_nCurrentJitter = 0;
			m_nPreviousJitter = 0;
			m_nCurrentJitterVar = 0;
			m_nPreviousJitterVar = 0;
			m_nCurrentOff = 0 ;
			m_nPreviousOff = 0;

			if(m_nLastFewPnr++ == m_nFramesInPacket - 1)
			{
				m_bIsFirstRound = TRUE;
				m_nLastFewPnr = 0;
			}
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
		
		
		//detect SPIKE_MODE
		if(AJB_MODE_NORMAL == m_AJBMode)
		{
			//whether change to spike mode

			if ((abs(twoPktDifference) >= m_nPlayTimeBase+m_nThreshold) && (abs(twoPktDifference) >= 2*m_nPlayTimeBase))
			{
				// a spike detected
				if(!m_bIsFirstPeak)
				{	
					//this is the first spike
					m_bIsFirstPeak = TRUE;
					m_nSpikeNum = 0;
					cisco_memset_s(m_nPeakHeight, sizeof(int)*PEAK_NUM , 0);
					m_dwLastPeakArrivedTime = receivedTime;
					WBX_POLICY_ALL_TRACE_THIS( "CWbxAJBPolicy::JitterPolicy(),warning!first spike,not change to spike mode yet!");				
				}
				else
				{	
					//some spikes have been detected before
					m_dwLastPeakArrivedTime = receivedTime;
					m_nPeakHeight[m_nSpikeNum] = abs(twoPktDifference);
					m_nSpikeNum++;
						
					if(m_nSpikeNum == SPIKE_NUM_REQUIRED-1)
					{
						m_AJBMode = AJB_MODE_SPIKE;
						WBX_POLICY_WARNING_TRACE_THIS("CWbxAJBPolicy::JitterPolicy(),warning!" << m_nSpikeNum+1
															<< " spikes,change to spike mode!");
					}
					else
					{
						WBX_POLICY_ALL_TRACE_THIS( "CWbxAJBPolicy::JitterPolicy(),warning!" << m_nSpikeNum+1
							                            << " spikes,not change to spike mode yet!");
					}

					if(PEAK_NUM == m_nSpikeNum)
					{
						m_nSpikeNum = 0;
					}			
				}
			}
			else
			{
				if(receivedTime-m_dwLastPeakArrivedTime > m_dwMaxPeakPeriod && m_bIsFirstPeak)
				{
					//too int time elapsed since last peak,reset spike related data
					m_bIsFirstPeak = FALSE;
					cisco_memset_s(m_nPeakHeight, sizeof(int)*PEAK_NUM, 0);
					WBX_POLICY_ALL_TRACE_THIS( "CWbxAJBPolicy::JitterPolicy(),warning!(" <<receivedTime-m_dwLastPeakArrivedTime
													<<" ms) elapsed since last peak! too int time elapsed since last peak,reset spike related data.");
				}
			}
		}
		else
		{
			//whether keep in spike mode

			if ((abs(twoPktDifference) >= m_nPlayTimeBase+m_nThreshold) && (abs(twoPktDifference) >= 2*m_nPlayTimeBase))
			{
				//a spike detected		
				m_nPeakHeight[m_nSpikeNum] = abs(twoPktDifference);
				m_nSpikeNum++;
				if(PEAK_NUM == m_nSpikeNum)
				{
					m_nSpikeNum = 0;
				}
				WBX_POLICY_ALL_TRACE_THIS("CWbxAJBPolicyWmeOrigin::JitterPolicy,a spike detected,now in spike mode!");
				m_dwLastPeakArrivedTime = receivedTime;
			}
			else
			{
				if(receivedTime-m_dwLastPeakArrivedTime > 2*m_dwMaxPeakPeriod && m_bIsFirstPeak)
				{
					//too int time elapse since last peak,log out the spike mode
					m_bIsFirstPeak = FALSE;
					m_AJBMode = AJB_MODE_NORMAL;
					cisco_memset_s(m_nPeakHeight, sizeof(int)*PEAK_NUM, 0);
					WBX_POLICY_WARNING_TRACE_THIS("CWbxAJBPolicyWmeOrigin::JitterPolicy,(" <<receivedTime-m_dwLastPeakArrivedTime
														<<" ms) elapsed since last peak!Logout spike mode.");
				}
			}
		}

		m_nCurrentOff = (int)( ALPHA_PARAMETER*m_nCurrentOff+(1-ALPHA_PARAMETER)*m_nPreviousOff);

		int adjustTime = 0;
		if(AJB_MODE_NORMAL == m_AJBMode)
		{
			adjustTime = m_nCurrentOff+ ADJUST_K*m_nCurrentJitter;
		}
		else
		{
			int i = 0;
			int temPeakJitter = 0;
			for(;i<m_nSpikeNum;i++)
			{
				if( m_nPeakHeight[i]>temPeakJitter )
					temPeakJitter = m_nPeakHeight[i];
			}
			adjustTime = m_nCurrentOff+ ADJUST_K*temPeakJitter;
#ifdef DEBUGJITTER
			sprintf_s(charLog,_countof(charLog),"PeakMode£¬adjustTime:%d,temPeakJitter:%d¡£\n",adjustTime,temPeakJitter);
			SaveLogStringToFile(charLog);			
#endif
		}


		if (adjustTime > m_nMaxDelay)
		{
			adjustTime = m_nMaxDelay;
		}



		// if it has disorder, for example, first packet is N, and the sencond packet is N-1,
		// if duration is unsigned long, the duration will be a bigger value
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
#ifdef DEBUGJITTER
		sprintf_s(charLog,_countof(charLog),"adjustTime:%d,instantOff:%d,tmpBase:%d£¬m_nPlayTimeBase:%d,m_nCurrentJitter:%d.",adjustTime,instantOff,tmpBase,m_nPlayTimeBase,m_nCurrentJitter);
		SaveLogStringToFile(charLog);			
#endif
		
		playbackTime += 20;


		if (tmpBase - m_nPlayTimeBase < 0 )
		{
			if (m_nJitConDecCon++ >= 30)// Speed to reduce delay, if every 30 packet, the jitter becomes smaller. then reduce it.
			{
				m_nConDecMultipleCnt ++;
				if (m_nConDecMultipleCnt <= 5)
				{
					m_nPlayTimeBase	-= 20; // it needs 3 seconds to reduce 100ms delay.
				}
				else if (m_nConDecMultipleCnt <= 10)
				{
					m_nPlayTimeBase	-= 40; // it needs 6 seconds to reduce 300ms delay(100 + 40 * 5 = 300ms)
				}
        else if(m_nConDecMultipleCnt <= 20)
        {
          m_nPlayTimeBase	-= 50;  // it needs 12 seconds  to reduce 900ms delay( 300 + 50 * 10 = 800ms)
        }
				else
				{
					m_nPlayTimeBase	-= 60;

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


		if (m_nPlayTimeBase < m_nMinDelay)
		{
			m_nPlayTimeBase = m_nMinDelay;
		}

		if (m_nPlayTimeBase > m_nMaxDelay)
		{
			m_nPlayTimeBase = m_nMaxDelay;
		}



		WBX_POLICY_DETAIL_TRACE_THIS("CWbxAJBPolicyWmeOrigin::JitterPolicy(), Rec ssrc:" <<ssrc
			<<"seq: " << rtpSequce
			<<", timestamp:" <<rtpTimestamp <<"( "<<(rtpTimestamp -  m_dwLastRTPTimeStamp) /samplesPer10MS * 10<<" ms)"
			<<", RL (" << receivedTime - m_dwLastArrivedTime
			<<"ms), rtp-TS-ellips " << (int)(rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10
			<<"ms, RF:" << receivedTime - m_dwFirstArrivedTime
			<<" ms, RF -ellips : "<< (int)( receivedTime - m_dwFirstArrivedTime - (rtpTimestamp-m_dwFirstRTPTimeStamp) / (int)samplesPer10MS * 10 )
			<<" ms, RTC:" << receivedTime
			<<", PTC:" << playbackTime
			<<", delayed " <<(int) (playbackTime - receivedTime)
			<<" to play, m_nPlayTimeBase:" <<m_nPlayTimeBase
      <<", duration:" <<duration
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
		m_wLastSequence  = (unsigned short)rtpSequce;
		m_dwLastArrivedTime = receivedTime;

		m_nPreviousOff = m_nCurrentOff;
		m_nPreviousJitter = m_nCurrentJitter;
		m_nPreviousJitterVar = m_nCurrentJitterVar;
    
    if (m_dwTotalReceivedCount % 50 == 0 || (receivedTime - m_dwFirstArrivedTime) >= 1000)
    {
      int  nReceivedTimeDiff =(int)(receivedTime -m_dwFirstArrivedTime);
      int nRtpSeqDiff = (int)(rtpSequce - m_wFirstSequence);
      m_dwFirstArrivedTime = receivedTime;
      m_dwFirstRTPTimeStamp = rtpTimestamp;
      m_dwFirstEstPlayTime = playbackTime;
      m_wFirstSequence = rtpSequce;
      WBX_POLICY_DEBUG_TRACE_THIS("CWbxAJBPolicyWmeOrigin::JitterPolicy(), received 50 packets, m_dwFirstArrivedTime change to " << m_dwFirstArrivedTime
        <<", m_dwFirstRTPTimeStamp change to " << m_dwFirstRTPTimeStamp
        <<", m_wFirstSequence:" << m_wFirstSequence
        <<", m_dwFirstEstPlayTime change to " << m_dwFirstEstPlayTime
        <<", m_dwTotalReceivedCount:" << m_dwTotalReceivedCount
        <<", nReceivedTimeDiff:" << nReceivedTimeDiff
        <<", nRtpSeqDiff:"<< nRtpSeqDiff);
    }
		
		

	}
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyWmeOrigin::GetJitterInformation(WbxAEAJBStatistics& jitterStatistics)
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

int CWbxAJBPolicyWmeOrigin::Reset()
{
//	m_nSpikeNum = 0;
//	m_dwLastPeakArrivedTime = 0;
//	m_bIsFirstPeak = FALSE;

	m_nMaxJitter = 0;
	m_nMinJitter = 0;
	m_nAverageJitter = 0;

	m_dwTotalReceivedCount = 0;
	m_dwTotalLostCount = 0;
	m_dwReceivedMaxSeq = 0;

	m_wLastSequence = 0;
	m_dwLastRTPTimeStamp = 0;
	m_nLastFewPnr = 0;
	m_bIsFirstRound = FALSE;

	m_bIsFirstFrameArrived = FALSE;
	m_wFirstSequence = 0;
	m_dwFirstRTPTimeStamp = 0;
	m_dwFirstArrivedTime = 0;
	m_dwFirstEstPlayTime = 0;
//	m_nCurrentJitter = 0;
//	m_nPreviousJitter *= 0.75;
//	m_nCurrentJitterVar = 0;
//	m_nPreviousJitterVar *= 0.75;

	m_nCurrentOff = 0 ;
	m_nPreviousOff = 0;
	m_AJBMode = AJB_MODE_NORMAL;
//	if (m_nPlayTimeBase* 0.85 > m_nMinDelay)
//	{
//		m_nPlayTimeBase =  m_nPlayTimeBase * 0.85;
//	}
//	else
//	{
//		m_nPlayTimeBase = m_nMinDelay;
//	}
	m_nJitConDecCon = 0;
	
//	cisco_memset_s(m_nPeakHeight, sizeof(int)*PEAK_NUM, 0);
	cisco_memset_s(m_pLastFewRTPTimeStamp, sizeof(unsigned int)*m_nFramesInPacket, 0);
	cisco_memset_s(m_pLastFewArrivedTime, sizeof(unsigned int)*m_nFramesInPacket, 0);

	WBX_POLICY_INFO_TRACE_THIS("CWbxAJBPolicyWmeOrigin::Reset(), after reset m_nPlayTimeBase:" <<m_nPlayTimeBase);
	return WBXAE_SUCCESS;
}

int CWbxAJBPolicyWmeOrigin::GetAverageDelay()
{
	return m_nPlayTimeBase;
}


int CWbxAJBPolicyWmeOrigin::SetDelay(int nMinDelay, int nMaxDelay)
{
	if ( nMinDelay > WBXAE_AJB_MAX_DELAY ||nMaxDelay > WBXAE_AJB_MAX_DELAY )
	{
		return WBXAE_AJB_ERROR_SET_DELAY_FAILED;
	}
	m_nPlayTimeBase = m_nMinDelay = nMinDelay;
	m_nMaxDelay = nMaxDelay;
	return WBXAE_SUCCESS;
}
#ifdef DEBUGJITTER
void CWbxAJBPolicyWmeOrigin::SaveLogStringToFile(char* lpsz)
{

	if(NULL == s_FileSaveTrace)
	{
		//		CString strModule;
		//		GetModulePathName(strModule);
		//		CString strFileName = strModule +;
		fopen_s(&s_FileSaveTrace,"JitterPolicy.txt","w+b");
	}
	if(NULL != s_FileSaveTrace)
	{
		time_t timeVal;	
		time( &timeVal );
		struct tm* tmVar;
		struct tm mytm = {0};
		localtime_s(&mytm, &timeVal);
		tmVar = &mytm;		
		int tm_mon = 0;
		int tm_mday = 0;
		int tm_year = 0;
		int tm_hour = 0;
		int tm_min = 0;
		int tm_sec = 0;
		if (tmVar != NULL)
		{
			tm_mon = tmVar->tm_mon;
			tm_mday  = tmVar->tm_mday;
			tm_year  = tmVar->tm_year;
			tm_hour  = tmVar->tm_hour;
			tm_min  = tmVar->tm_min;
			tm_sec  = tmVar->tm_sec;
		}
		fprintf_s(s_FileSaveTrace,
			"[%02d:%02d:%02d]:    %s \n", 
			tm_hour,
			tm_min,
			tm_sec,
			lpsz);
	}

}
#endif

extern "C"
{
	int CreateIWbxJitterPolicyOrigin(IWbxJitterPolicy** ppIWbxJitterPolicy)
	{
		int result = WBXAE_AJB_ERROR_BASE;
		if (ppIWbxJitterPolicy)
		{
			CWbxAJBPolicyWmeOrigin *pWbxAJBPolicy = new CWbxAJBPolicyWmeOrigin();
			if( pWbxAJBPolicy )
			{
				*ppIWbxJitterPolicy = pWbxAJBPolicy;
				result = WBXAE_SUCCESS;
			}
		}
		return result;
	}

	int DestoryIWbxJitterPolicyOrigin(IWbxJitterPolicy* pIWbxJitterPolicy)
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
