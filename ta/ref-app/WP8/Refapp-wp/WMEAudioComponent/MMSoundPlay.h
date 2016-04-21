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

#ifndef __MM_AUDIO_PLAY_SOUND_CTRL__
#define __MM_AUDIO_PLAY_SOUND_CTRL__


#include "IAudioStreamWMEChannel.h"
#include "RTPFrame.h"


#define MAX_SOUND_NUM 5

// #define STATUS_IDLE 1
// #define	STATUS_PLAYING 2
// #define STATUS_STOP 3

class CMMSoundPlay
{
public:
	enum 
	{
		STATUS_PLAYER_IDLE,
		STATUS_PLAYER_PRE_PLAYING,
		STATUS_PLAYER_PLAYING,
		STATUS_PLAYER_STOP
	};

	CMMSoundPlay();
	~CMMSoundPlay();


	int Initialize(IAudioMediaWMEChannel* channel);
	int Destroy();

	int SetDataInfo(int soundID,	void* m_databuffer, int bufferSize);
	int SetDataFormat (BYTE codec, int frequency, int sampleRate, int frameSize );

	int StartPlay(int soundID);	
	int StopPlay();

	BOOL IsPlaying();

	int PlayFunc();
	int SetReadyToPlay();
private:
	RTPFrame* BuildRTP(int frameSize, void* framedata,
		unsigned char playloadtype, WORD sequence, DWORD timestamp, DWORD SSID);

	BOOL StartPlaythread();
	BOOL StopPlayThread();


private:
	unsigned char* m_databuffer[MAX_SOUND_NUM];
	int m_bufferSize[MAX_SOUND_NUM];

	int m_currentPlaySoundID;

	IAudioMediaWMEChannel* m_playbackChannel;
	BOOL m_startPlayout;
	CCmMutexThreadRecursive m_mutex;

	//audio data info
	unsigned char m_codec;
	int m_frequency;
	int m_sampleRate;
	int m_frameSize;

	//GIPS playback sequence and sequence need to be increased, else will no playback
	WORD m_sequence;
	DWORD m_timestamp;

	int m_trackID;


	unsigned short m_status;
	ACmThread* m_pThread;
};

class CPlayBeepEvent : public ICmEvent
{
public:
	CPlayBeepEvent(CMMSoundPlay* play);
	virtual ~CPlayBeepEvent();

public:
	virtual CmResult OnEventFire();
private:
	CMMSoundPlay* m_pPlay;
};

#endif