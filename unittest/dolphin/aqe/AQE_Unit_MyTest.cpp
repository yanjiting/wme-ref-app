// WbxAQE_test.cpp : Defines the entry point for the console application.
//

#ifndef GTEST


#include "WbxAeAQE.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "MediaProcess_AQE_test.h"
#include "CReceiver_AQE_test.h"
#include "test_set.h"





unsigned long gsMultiMediaTimer_1 = NULL;
unsigned long gsMultiMediaTimer_2 = NULL;


void PASCAL Thread_func_1 (UINT wTimerID, UINT msg,
						   DWORD dwUser, DWORD dw1, DWORD dw2)
{
	int sampNum = PLAYBACK_SAMPLE_NUM;
	WBXAE_SAMPLE_RATE sampRate = PLAYBACK_SAMPLE_RATE;
	CWBXAE_AQE* pAQE = (CWBXAE_AQE*)dwUser;
	if (NULL != pAQE)
	{
		CWbxAeMediaBlock* pDataBlock = NULL;
		pDataBlock = new CWbxAeMediaBlock(MAX_MEDIA_BLOCK_SIZE);
		if (NULL != pDataBlock)
		{
			pDataBlock->AddRef();
			if (pDataBlock->GetWriteSpace() >= sizeof(short)*sampNum)
			{
				if (pPlaybackFile && !feof(pPlaybackFile))
				{
					fread(pDataBlock->GetWritePtr(),sizeof(short),sampNum,pPlaybackFile);
					pDataBlock->AdvanceWritePtr(sizeof(short)*sampNum);

					pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
					pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
					pDataBlock->GetWaveFormat()->nChannels = 1;
					pDataBlock->GetWaveFormat()->nSamplesPerSec = sampRate;
					pDataBlock->GetWaveFormat()->nAvgBytesPerSec = sampRate*sizeof(short);
					pDataBlock->GetWaveFormat()->nBlockAlign =  sizeof(short);
					pDataBlock->GetWaveFormat()->wBitsPerSample = sizeof(short)*8;

					pDataBlock->GetProperty()->dwSize = sizeof(WBXAEAudioProperty);
					pDataBlock->GetProperty()->dwEnerge = 0;
					pDataBlock->GetProperty()->bZero = FALSE;
					pDataBlock->GetProperty()->fSNR = 10;
					pDataBlock->GetProperty()->bSaturated = FALSE;
					pDataBlock->GetProperty()->fNoiseEnergy = 0;
					pDataBlock->GetProperty()->fResidualEchoEnergy = 0;
					pDataBlock->GetProperty()->bVAD = FALSE;

	//				pAQE->OnPlaybackData(pDataBlock);
				}
			}
			pDataBlock->Release();
		}	
	}
}

void PASCAL Thread_func_2 (UINT wTimerID, UINT msg,
						   DWORD dwUser, DWORD dw1, DWORD dw2)
{
	int sampNum = CAPTURE_SAMPLE_NUM;
	WBXAE_SAMPLE_RATE sampRate = CAPTURE_SAMPLE_RATE;
	CWBXAE_AQE* pAQE = (CWBXAE_AQE*)dwUser;
	if (NULL != pAQE)
	{
		CWbxAeMediaBlock* pDataBlock = new CWbxAeMediaBlock(MAX_MEDIA_BLOCK_SIZE);
		if (NULL != pDataBlock)
		{
			pDataBlock->AddRef();
			if (pDataBlock->GetWriteSpace() >= sizeof(short)*sampNum)
			{
				if (pCaptureFile && !feof(pCaptureFile))
				{
					fread(pDataBlock->GetWritePtr(),sizeof(short),sampNum,pCaptureFile);
					pDataBlock->AdvanceWritePtr(sizeof(short)*sampNum);

					pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
					pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
					pDataBlock->GetWaveFormat()->nChannels = 1;
					pDataBlock->GetWaveFormat()->nSamplesPerSec = sampRate;
					pDataBlock->GetWaveFormat()->nAvgBytesPerSec = sampRate*sizeof(short);
					pDataBlock->GetWaveFormat()->nBlockAlign =  sizeof(short);
					pDataBlock->GetWaveFormat()->wBitsPerSample = sizeof(short)*8;

					pDataBlock->GetProperty()->dwSize = sizeof(WBXAEAudioProperty);
					pDataBlock->GetProperty()->dwEnerge = 0;
					pDataBlock->GetProperty()->bZero = FALSE;
					pDataBlock->GetProperty()->fSNR = 10;
					pDataBlock->GetProperty()->bSaturated = FALSE;
					pDataBlock->GetProperty()->fNoiseEnergy = 0;
					pDataBlock->GetProperty()->fResidualEchoEnergy = 0;
					pDataBlock->GetProperty()->bVAD = FALSE;

					pAQE->OnCaptureData(pDataBlock);
				}	
			}
			pDataBlock->Release();
		}
	}
}

BOOL StartSimulateSendData(CWBXAE_AQE* pAQE)
{
	gsMultiMediaTimer_1= timeSetEvent(PLAYBACK_INTERVAL,1, 
		(LPTIMECALLBACK)Thread_func_1, 
		(DWORD_PTR)pAQE, 
		TIME_PERIODIC);

	gsMultiMediaTimer_2= timeSetEvent(CAPTURE_INTERVAL,1, 
		(LPTIMECALLBACK)Thread_func_2, 
		(DWORD_PTR)pAQE, 
		TIME_PERIODIC);

	return TRUE;
}

BOOL StopSimulateSendData()
{
	timeKillEvent(gsMultiMediaTimer_1);
	timeKillEvent(gsMultiMediaTimer_2);
	return TRUE;
}


int My_AQE_Test()
{


	CWBXAE_AQE*	pAQE = new CWBXAE_AQE();
	BOOL result = TRUE;
	srand( (unsigned)time( NULL ) );

	ICaptureReceiver* pCaptureReceiver = new CaptureReceiver();
	pAQE->SetSinkCaptureData(pCaptureReceiver);

	IPlaybackReceiver* pPlaybackReceiver = new PlaybackReceiver();
	pAQE->SetSinkPlaybackData(pPlaybackReceiver);


	result = pAQE->Initial();

// 	result = pAQE->LoadAllBasicModules();
// 	result = pAQE->InitialBasicModulesDefault();
// 	result = pAQE->LinkModulesInAQE();

	int intervalTime = 10;
	pAQE->SetModulePara(AQE, INTERVALTIME, AQE_MODULE, &intervalTime);
	WBXAE_SAMPLE_RATE NetInSampRate = AE_SAMPLE_RATE_48K;
	pAQE->SetModulePara(AQE, NET_IN_SAMPRATE, AQE_MODULE, &NetInSampRate);
	WBXAE_SAMPLE_RATE MicInSampRate = AE_SAMPLE_RATE_48K;
	pAQE->SetModulePara(AQE, MIC_IN_SAMPRATE, AQE_MODULE, &MicInSampRate);
	WBXAE_SAMPLE_RATE NetOutSampRate = AE_SAMPLE_RATE_24K;
	pAQE->SetModulePara(AQE, NET_OUT_SAMPRATE, AQE_MODULE, &NetOutSampRate);
	WBXAE_SAMPLE_RATE SpkOutSampRate = AE_SAMPLE_RATE_48K;
	pAQE->SetModulePara(AQE, SPK_OUT_SAMPRATE, AQE_MODULE, &SpkOutSampRate);
	WBXAE_SAMPLE_RATE NetDAGCsampRate = NetInSampRate;
	pAQE->SetModulePara(DAGC, SAMPLERATE, POST_PROC_BEFORE_AEC, &NetDAGCsampRate);
	WBXAE_SAMPLE_RATE AAGCsampRate = MicInSampRate;
	pAQE->SetModulePara(AAGC, SAMPLERATE, PRE_PROC_BEFORE_AEC, &AAGCsampRate);
	WBXAE_SAMPLE_RATE AECsampRate = AE_SAMPLE_RATE_48K;
	pAQE->SetModulePara(AEC, SAMPLERATE, IN_AEC, &AECsampRate);
	WBXAE_SAMPLE_RATE NRsampRate = AE_SAMPLE_RATE_8K;
	pAQE->SetModulePara(NR, SAMPLERATE, PRE_PROC_AFTER_AEC, &NRsampRate);
	WBXAE_SAMPLE_RATE MicDAGCsampRate = AE_SAMPLE_RATE_24K;
	pAQE->SetModulePara(DAGC, SAMPLERATE, PRE_PROC_AFTER_AEC, &MicDAGCsampRate);



	BOOL nrStatus = FALSE;
	pAQE->SetModulePara(NR, STATUS, PRE_PROC_AFTER_AEC, &nrStatus);
// 	BOOL aecStatus = FALSE;
// 	pAQE->SetModulePara(AEC, STATUS, IN_AEC, &aecStatus);
	BOOL netindagcStatus = FALSE;
	pAQE->SetModulePara(DAGC, STATUS, POST_PROC_BEFORE_AEC, &netindagcStatus);
// 	BOOL netoutdagcStatus = FALSE;
// 	pAQE->SetModulePara(DAGC, STATUS, PRE_PROC_AFTER_AEC, &netoutdagcStatus);
// 	BOOL aagcStatus = FALSE;
// 	pAQE->SetModulePara(AAGC, STATUS, PRE_PROC_BEFORE_AEC, &aagcStatus);

// 	BOOL netinSAStatus = FALSE;
// 	pAQE->SetModulePara(SA, STATUS, POST_PROC_BEFORE_AEC, &netinSAStatus);
// 	BOOL micinSAStatus = FALSE;
// 	pAQE->SetModulePara(SA, STATUS, PRE_PROC_BEFORE_AEC, &micinSAStatus);
// 
// 	BOOL netoutSAStatus = FALSE;
// 	pAQE->SetModulePara(SA, STATUS, PRE_PROC_AFTER_AEC, &netoutSAStatus);
// 	BOOL spkoutSAStatus = FALSE;
// 	pAQE->SetModulePara(SA, STATUS, POST_PROC_AFTER_AEC, &spkoutSAStatus);




	int level = 10;
	pAQE->SetModulePara(DAGC, LEVEL, POST_PROC_BEFORE_AEC, &level);

	
	//SA_NetIn
	IWbxMediaProcess* POST_SA_NetIn__input	= new MediaProcess_AQE("POST_SA_NetIn__input");
	pAQE->SetSinkMediaProcess(POST_SA_NetIn__input, SA, POST_PROC_BEFORE_AEC, TRUE);
	IWbxMediaProcess* POST_SA_NetIn__output	= new MediaProcess_AQE("POST_SA_NetIn__output");
	pAQE->SetSinkMediaProcess(POST_SA_NetIn__output, SA, POST_PROC_BEFORE_AEC, FALSE);
	//DAGC_NetIn
	IWbxMediaProcess* POST_DAGC_NetIn__input	= new MediaProcess_AQE("POST_DAGC_NetIn__input");
	pAQE->SetSinkMediaProcess(POST_DAGC_NetIn__input, DAGC, POST_PROC_BEFORE_AEC, TRUE);
	IWbxMediaProcess* POST_DAGC_NetIn__output	= new MediaProcess_AQE("POST_DAGC_NetIn__output");
	pAQE->SetSinkMediaProcess(POST_DAGC_NetIn__output, DAGC, POST_PROC_BEFORE_AEC, FALSE);
	//SA_SpkOut
	IWbxMediaProcess* POST_SA_SpkOut__input	= new MediaProcess_AQE("POST_SA_SpkOut__input");
	pAQE->SetSinkMediaProcess(POST_SA_SpkOut__input, SA, POST_PROC_AFTER_AEC, TRUE);
	IWbxMediaProcess* POST_SA_SpkOut__output	= new MediaProcess_AQE("POST_SA_SpkOut__output");
	pAQE->SetSinkMediaProcess(POST_SA_SpkOut__output, SA, POST_PROC_AFTER_AEC, FALSE);


	//SA_MicIn
	IWbxMediaProcess* PRE_SA_MicIn__input	= new MediaProcess_AQE("PRE_SA_MicIn__input");
	pAQE->SetSinkMediaProcess(PRE_SA_MicIn__input, SA, PRE_PROC_BEFORE_AEC, TRUE);
	IWbxMediaProcess* PRE_SA_MicIn__output	= new MediaProcess_AQE("PRE_SA_MicIn__output");
	pAQE->SetSinkMediaProcess(PRE_SA_MicIn__output, SA, PRE_PROC_BEFORE_AEC, FALSE);
	//AAGC
	IWbxMediaProcess* PRE_AAGC__input	= new MediaProcess_AQE("PRE_AAGC__input");
	pAQE->SetSinkMediaProcess(PRE_AAGC__input, AAGC, PRE_PROC_BEFORE_AEC, TRUE);
	IWbxMediaProcess* PRE_AAGC__output	= new MediaProcess_AQE("PRE_AAGC__output");
	pAQE->SetSinkMediaProcess(PRE_AAGC__output, AAGC, PRE_PROC_BEFORE_AEC, FALSE);
	//AEC
	IWbxMediaProcess* PRE_AEC__input	= new MediaProcess_AQE("PRE_AEC__input");
	pAQE->SetSinkMediaProcess(PRE_AEC__input, AEC, IN_AEC, TRUE);
	IWbxMediaProcess* PRE_AEC__output	= new MediaProcess_AQE("PRE_AEC__output");
	pAQE->SetSinkMediaProcess(PRE_AEC__output, AEC, IN_AEC, FALSE);
	//NR
	IWbxMediaProcess* PRE_NR__input	= new MediaProcess_AQE("PRE_NR__input");
	pAQE->SetSinkMediaProcess(PRE_NR__input, NR, PRE_PROC_AFTER_AEC, TRUE);
	IWbxMediaProcess* PRE_NR__output	= new MediaProcess_AQE("PRE_NR__output");
	pAQE->SetSinkMediaProcess(PRE_NR__output, NR, PRE_PROC_AFTER_AEC, FALSE);
	//DAGC_NetOut
	IWbxMediaProcess* PRE_DAGC_NetOut__input	= new MediaProcess_AQE("PRE_DAGC_NetOut__input");
	pAQE->SetSinkMediaProcess(PRE_DAGC_NetOut__input, DAGC, PRE_PROC_AFTER_AEC, TRUE);
	IWbxMediaProcess* PRE_DAGC_NetOut__output	= new MediaProcess_AQE("PRE_DAGC_NetOut__output");
	pAQE->SetSinkMediaProcess(PRE_DAGC_NetOut__output, DAGC, PRE_PROC_AFTER_AEC, FALSE);
	//SA_NetOut
	IWbxMediaProcess* PRE_SA_NetOut__input	= new MediaProcess_AQE("PRE_SA_NetOut__input");
	pAQE->SetSinkMediaProcess(PRE_SA_NetOut__input, SA, PRE_PROC_AFTER_AEC, TRUE);
	IWbxMediaProcess* PRE_SA_NetOut__output	= new MediaProcess_AQE("PRE_SA_NetOut__output");
	pAQE->SetSinkMediaProcess(PRE_SA_NetOut__output, SA, PRE_PROC_AFTER_AEC, FALSE);
	

	

	StartSimulateSendData(pAQE);

 	result = pAQE->StartWork();

	for (int i=0; i< RUN_TIME_S; i++)
	{
		int delay = rand()%100;
		Sleep(1000);

// 		int level = i*3+10;
// 		pAQE->SetModulePara(DAGC, LEVEL, POST_PROC_BEFORE_AEC, &level);
	}


	StopSimulateSendData();



	result = pAQE->StopWork();
	


	pAQE->RemoveSinkCaptureData(pCaptureReceiver);
	pAQE->RemoveSinkPlaybackData(pPlaybackReceiver);
	pAQE->RemoveMediaProcessAll();

	//SA_NetIn
	delete POST_SA_NetIn__input;
	delete POST_SA_NetIn__output;
	//DAGC_NetIn
	delete POST_DAGC_NetIn__input;	
	delete POST_DAGC_NetIn__output;
	//SA_SpkOut
	delete POST_SA_SpkOut__input;	
	delete POST_SA_SpkOut__output;	
	//SA_MicIn
	delete PRE_SA_MicIn__input;	
	delete PRE_SA_MicIn__output;	
	//AAGC
	delete PRE_AAGC__input;
	delete PRE_AAGC__output;	
	//AEC
	delete PRE_AEC__input;	
	delete PRE_AEC__output;
	//NR
	delete PRE_NR__input;	
	delete PRE_NR__output;	
	//DAGC_NetOut
	delete PRE_DAGC_NetOut__input;
	delete PRE_DAGC_NetOut__output;	
	//SA_NetOut
	delete PRE_SA_NetOut__input;	
	delete PRE_SA_NetOut__output;	
	

	result = pAQE->Unload();
// 	result = pAQE->UnloadAllBasicModules();

	if (NULL != pAQE)
	{
		delete pAQE;
		pAQE = NULL;
	}
	return 0;
}



#endif
