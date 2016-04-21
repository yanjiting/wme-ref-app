/************************************************************************/
/* the media conference system								            */
/*                                                                      */
/* Copyright (C) WebEx Communications  Inc.                             */
/* All rights reserved                                 	                */
/* author                                                               */
/*          Ross Luo , RossL@hz.webex.com                               */
/* History                                                              */
/*          Nov 29th , 2007                                             */
/* for play sound from file or buffer									*/
/************************************************************************/



#include "MMSoundPlay.h"

#ifdef WIN32
//#include <process.h>
#include <tchar.h>
#include "Utility.h"
#endif

#include "MmAudioTrace.h"
#include "AudioCCDef.h"

void playFunc_( void *ch )
{
	CMMSoundPlay* ctrl = (CMMSoundPlay*)ch;

 	if ( ctrl)
		ctrl->PlayFunc();
}

CMMSoundPlay::CMMSoundPlay():
		m_timestamp(0),
		m_codec(0),
		m_frequency(0),
		m_sampleRate (0),
		m_frameSize (0),
		m_sequence (1),
		m_startPlayout (false),
		m_playbackChannel(NULL),
		m_currentPlaySoundID(0),
		m_pThread(NULL)
{
	m_status = STATUS_PLAYER_IDLE;

	for ( int i = 0 ; i < MAX_SOUND_NUM; i++)
	{
		m_bufferSize[i] = 0;
		m_databuffer[i] = NULL;
	}

	StartPlaythread();

	m_trackID = 0;
}

CMMSoundPlay::~CMMSoundPlay()
{
	Destroy();
	for ( int i = 0 ; i < MAX_SOUND_NUM; i++)
	{
		m_bufferSize[i] = 0;
		SAFE_DELETE_ARRAY(m_databuffer[i]);
	}	
	
}


int CMMSoundPlay::Initialize(IAudioMediaWMEChannel* channel)
{
	m_playbackChannel = channel;
	if (m_playbackChannel)
	{
		m_playbackChannel->CreateAdhocAudioTrack(m_trackID);
	}
	return true;
}

int CMMSoundPlay::SetDataInfo(int soundID,	void* databuffer, int bufferSize)
{
	if ( databuffer == NULL )
		return false;
	
	if ( bufferSize == 0)
		return false;

	if ( soundID <0 || soundID >= MAX_SOUND_NUM)
		return false;

	m_bufferSize[soundID] = bufferSize;
	SAFE_DELETE_ARRAY(m_databuffer[soundID]);
	if (m_bufferSize[soundID] >0 )
	{
		m_databuffer[soundID] = new BYTE[ m_bufferSize[soundID] ];
		//memcpy ( m_databuffer[soundID], databuffer, m_bufferSize[soundID] );
        memcpy_s(m_databuffer[soundID], m_bufferSize[soundID], databuffer, m_bufferSize[soundID]);
	}
	return true;
}

int CMMSoundPlay::SetDataFormat(
		BYTE codec, int frequency, int sampleRate, int frameSize )
{
	if ( frequency <= 0 || sampleRate <= 0 )
		return false;

	if ( frameSize <= 0 )
		return false;

	m_codec = codec;
	m_frequency = frequency;
	m_sampleRate = sampleRate;
	m_frameSize = frameSize;
	return true;
}

int CMMSoundPlay::Destroy()
{
	m_mutex.Lock();	
	m_status = STATUS_PLAYER_STOP;	
	m_mutex.UnLock();

	CCmThreadManager::SleepMs(m_frequency*2);

	StopPlayThread();
	
	//DWORD dwExitCode = 0;
	//while ( m_threadHandle != NULL && GetExitCodeThread (m_threadHandle, &dwExitCode) )
	//{
	//	AT_TRACE_INFO(0, _T("CMMSoundPlay::Destroy beep thread still working 0x%x, dwExitCode %d"),m_threadHandle, dwExitCode );
	//	if ( dwExitCode == STILL_ACTIVE  )
	//	{
	//		Sleep(100);
	//	}
	//	else
	//	{
	//		m_threadHandle = NULL;
	//	}		
	//}

	//AT_TRACE_INFO(0, _T("CMMSoundPlay::Destroy beep thread exit 0x%x dwExitCode %d"),m_threadHandle,dwExitCode );

	//m_mutex.Lock();	
	//m_threadHandle = NULL;
	//m_mutex.UnLock();


	return true;
}


int CMMSoundPlay::StartPlay(int soundID)
{
	AT_TRACE_INFO(100, _T("CMMSoundPlay::StartPlay m_status %d"),m_status );
	if ( m_status == STATUS_PLAYER_IDLE )
	{
		m_currentPlaySoundID = soundID ;

		if ( m_playbackChannel  )
		{
			m_startPlayout = false;

			//DWORD dwExitCode = 0;
			//if ( m_threadHandle != NULL && GetExitCodeThread (m_threadHandle, &dwExitCode) )
			//{
			//	AT_TRACE_INFO(0, _T("CMMSoundPlay::StartPlay beep thread still working 0x%x"),m_threadHandle );

			//	if ( dwExitCode == STILL_ACTIVE )
			//	{
			//		AT_TRACE_INFO(0, _T("CMMSoundPlay::StartPlay beep thread Error m_threadHandle 0x%x, dwExitCode %d"),m_threadHandle,dwExitCode );
			//		return FALSE;
			//	}
			//}

			m_mutex.Lock();
			//m_threadHandle = NULL;
			m_status = STATUS_PLAYER_PRE_PLAYING;
			AT_TRACE_INFO(100, _T("CMMSoundPlay::StartPlay m_status %d"),m_status );
			m_mutex.UnLock();

			if ( !m_startPlayout )
				m_startPlayout = true;
			
			//if ( m_startPlayout)
			//{			
			//	m_threadHandle = (HANDLE)_beginthread(playFunc_,0,(void*)this);
			//	if ( m_threadHandle == 0 )
			//	{
			//		AT_TRACE_INFO(0, _T("CMMSoundPlay::StartPlay create thread error") );
			//	}
			//}

			BOOL bStart = FALSE;
			if ( m_pThread && m_startPlayout)
			{
				ICmEventQueue* temp = m_pThread->GetEventQueue();
				if (temp != NULL)
				{
					CPlayBeepEvent* event = new CPlayBeepEvent(this);
					if (event == NULL)
					{
						CM_ERROR_TRACE_THIS("CMMSoundPlay::StartPlay, new CPlayBeepEvent failed");					
					}

					AT_TRACE_INFO(0, _T("CMMSoundPlay::StartPlay Post play event to thread"));
					CmResult ret = temp->PostEvent(event);
					if (ret != CM_OK)
					{
						event = NULL;
						CM_ERROR_TRACE_THIS("CMMSoundPlay::StartPlay, PostEvent() failed");
					}
					else
					{
						bStart = TRUE;
					}


				}

			}


			if ( !bStart )
			{
				m_mutex.Lock();
				m_status = STATUS_PLAYER_IDLE;
				AT_TRACE_INFO(100, _T("CMMSoundPlay::StartPlay m_status %d"),m_status );
				m_mutex.UnLock();
			}
		}

	}

	return true;
}

int CMMSoundPlay::StopPlay()
{

	m_mutex.Lock();	
	m_status = STATUS_PLAYER_STOP;
	AT_TRACE_INFO(100, _T("CMMSoundPlay::StopPlay m_status %d"),m_status );
	m_mutex.UnLock();

	return true;
}

int CMMSoundPlay::SetReadyToPlay()
{
	m_mutex.Lock();
	m_status = STATUS_PLAYER_IDLE;	
	AT_TRACE_INFO(100, _T("CMMSoundPlay::SetReadyToPlay m_status %d"),m_status );
	m_mutex.UnLock();

	return true;
}

BOOL CMMSoundPlay::IsPlaying()
{
	m_mutex.Lock();
	BOOL result = ( m_status == STATUS_PLAYER_PLAYING || m_status == STATUS_PLAYER_PRE_PLAYING );
	m_mutex.UnLock();
	return result;
}

RTPFrame* CMMSoundPlay::BuildRTP(int frameSize, void* framedata,
								 unsigned char playloadtype, WORD sequence, DWORD timestamp, DWORD SSID)
{
	RTPFrame* rtpdata = new RTPFrame(frameSize +12 );
	if ( rtpdata == NULL)
		return NULL;

	rtpdata->SetSyncSource(SSID);
	rtpdata->SetPayloadType(playloadtype);
	rtpdata->SetSequenceNumber(sequence);
	rtpdata->SetTimestamp(timestamp);

	//memset( rtpdata->GetBuffer(), 0x80, 1);
#if !defined(MACOS)
    memset_s(rtpdata->GetBuffer(), 1, 0x80);
#else
    cs_memset_s( rtpdata->GetBuffer(), 1, 0x80);
#endif
	//memcpy ( rtpdata->GetBuffer() + 12, framedata, frameSize );
    memcpy_s(rtpdata->GetBuffer() + 12, frameSize, framedata, frameSize);

	return rtpdata;
}

int CMMSoundPlay::PlayFunc()
{
	BOOL readyToPlay = TRUE;

	if ( m_frequency == 0 || 	
		m_sampleRate == 0 || 
		/*m_bufferSize == 0 ||*/ m_frameSize <=0 )
	{
		AT_TRACE_INFO(0, _T("CMMSoundPlay::PlayFunc() not initialize error"));
		readyToPlay = FALSE;
	}

	//update status, begin to play 
	m_mutex.Lock();	
	if ( m_status != STATUS_PLAYER_PRE_PLAYING )
	{
		readyToPlay = FALSE;
	}
	else
	{
		m_status = STATUS_PLAYER_PLAYING;
		AT_TRACE_INFO(100, _T("CMMSoundPlay::PlayFunc m_status %d"),m_status );
	}
	m_mutex.UnLock();

	AT_TRACE_INFO(0, _T("CMMSoundPlay::PlayFunc begin to play beep, readyToPlay %d"),readyToPlay );


	if ( readyToPlay )
	{	
		int frameNum = m_bufferSize[m_currentPlaySoundID]/m_frameSize;
		
		DWORD timediff = m_frequency*m_sampleRate;		
		static DWORD SSID = 0x1234 ;//+ AT_Rand()%100;

		char* nullVoice = new char[m_frameSize+1];
		//memset(nullVoice, 0 , m_frameSize);
#if !defined(MACOS)
        memset_s(nullVoice, m_frameSize, 0);
#else
        cs_memset_s(nullVoice, m_frameSize , 0);
#endif

		for ( int i = 0 ; i < frameNum ; i++)
		{
			CCmThreadManager::SleepMs(m_frequency);
			if ( m_status != STATUS_PLAYER_PLAYING )
			{
				AT_TRACE_INFO(0, _T("CMMSoundPlay::PlayFunc Exit Beep by Force") );
				break;
			}

			m_sequence ++;
			m_timestamp += timediff;
			void* databuffer = NULL;
			if ( i < frameNum )
				databuffer = (void*)(m_databuffer[m_currentPlaySoundID] + i* m_frameSize);
			else
				databuffer = nullVoice;

			
			
			RTPFrame* rtpdata = BuildRTP(m_frameSize,  databuffer,
				m_codec, m_sequence, m_timestamp,SSID);

			if ( rtpdata != NULL)
			{
				if ( m_playbackChannel )
					m_playbackChannel->RecvRTPData(m_trackID,(const char*)rtpdata->GetBuffer(), m_frameSize + 12);

				SAFE_DELETE(rtpdata);
			}

			if ( m_status != STATUS_PLAYER_PLAYING )
			{
				AT_TRACE_INFO(0, _T("CMMSoundPlay::PlayFunc Exit Beep by Force") );
				break;
			}

		}

		SAFE_DELETE_ARRAY(nullVoice);

	}

	//update status to idle
	m_mutex.Lock();	
	if ( m_status == STATUS_PLAYER_PLAYING )
	{
		AT_TRACE_INFO(100, _T("CMMSoundPlay::PlayFunc m_status %d"),m_status );
		m_status = STATUS_PLAYER_IDLE;
	}
	m_mutex.UnLock();

//	m_threadHandle = NULL;
	AT_TRACE_INFO(0, _T("CMMSoundPlay::PlayFunc play beep Exit") );
//	_endthread();

	return true;
}

BOOL CMMSoundPlay::StartPlaythread()
{
	CM_INFO_TRACE_THIS("CAudioWizardImpl::StartPlayThread()");
	if (m_pThread == NULL)
	{
#ifdef TRAIN_WME
		CreateUserTaskThread("audio_cc_soundplay", m_pThread);
#else
		CCmThreadManager::Instance()->CreateUserTaskThread(m_pThread, CCmThreadManager::TF_DETACHED, TRUE);
#endif
		if (m_pThread == NULL)
		{
			CM_ERROR_TRACE_THIS("CAudioWizardImpl::StartPlaythread(), Create thread failed");
			return FALSE;
		}
	}

	return TRUE;
}
BOOL CMMSoundPlay::StopPlayThread()
{
	CM_INFO_TRACE_THIS("CAudioWizardImpl::StopPlayThread()");
	CmResult rv = CM_OK;
	if(m_pThread)
	{
		rv = m_pThread->Stop();
		m_pThread->Join();
		if(rv != CM_OK)
			m_pThread->Terminate();
		else
			rv = m_pThread->Destory(0);

		m_pThread = NULL;
	}

	return TRUE;

}

CPlayBeepEvent::CPlayBeepEvent(CMMSoundPlay* play)
{
	m_pPlay = play;
}

CPlayBeepEvent::~CPlayBeepEvent()
{
}

CmResult CPlayBeepEvent::OnEventFire()
{
	if ( m_pPlay )
		m_pPlay->PlayFunc();
	return CM_OK;
}