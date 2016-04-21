#ifdef GTEST

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WbxAeAQE.h"

#include "MediaProcess_AQE_test.h"
#include "CReceiver_AQE_test.h"
#include "test_set.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;


#define  INTETVAL_TIME (30)
#define  FRAME_LEN    (48*INTETVAL_TIME)
#define  DELETE_POINTER(x)	  if (NULL != x){delete x; x =NULL;}
	

CWBXAE_AQE*	pAQE;
CaptureReceiver* pCaptureReceiver;
PlaybackReceiver* pPlaybackReceiver;
IWbxMediaProcess* POST_SA_NetIn__input;
IWbxMediaProcess* POST_SA_NetIn__output;
IWbxMediaProcess* POST_DAGC_NetIn__input;
IWbxMediaProcess* POST_DAGC_NetIn__output;
IWbxMediaProcess* POST_SA_SpkOut__input;
IWbxMediaProcess* POST_SA_SpkOut__output;
IWbxMediaProcess* PRE_SA_MicIn__input;
IWbxMediaProcess* PRE_SA_MicIn__output;
IWbxMediaProcess* PRE_AAGC__input;
IWbxMediaProcess* PRE_AAGC__output;
IWbxMediaProcess* PRE_AEC__input;
IWbxMediaProcess* PRE_AEC__output;
IWbxMediaProcess* PRE_NR__input;
IWbxMediaProcess* PRE_NR__output;
IWbxMediaProcess* PRE_DAGC_NetOut__input;
IWbxMediaProcess* PRE_DAGC_NetOut__output;
IWbxMediaProcess* PRE_SA_NetOut__input;
IWbxMediaProcess* PRE_SA_NetOut__output;

class AQETest : public testing::Test
{
public:
protected:
	static void SetUpTestCase()
	{
		pAQE = new CWBXAE_AQE();
		pCaptureReceiver = new CaptureReceiver();
		pPlaybackReceiver = new PlaybackReceiver();	
		POST_SA_NetIn__input	= new MediaProcess_AQE("POST_SA_NetIn__input");
		POST_SA_NetIn__output	= new MediaProcess_AQE("POST_SA_NetIn__output");
		POST_DAGC_NetIn__input	= new MediaProcess_AQE("POST_DAGC_NetIn__input");
		POST_DAGC_NetIn__output	= new MediaProcess_AQE("POST_DAGC_NetIn__output");
		POST_SA_SpkOut__input	= new MediaProcess_AQE("POST_SA_SpkOut__input");
		POST_SA_SpkOut__output	= new MediaProcess_AQE("POST_SA_SpkOut__output");
		PRE_SA_MicIn__input	= new MediaProcess_AQE("PRE_SA_MicIn__input");
		PRE_SA_MicIn__output	= new MediaProcess_AQE("PRE_SA_MicIn__output");
		PRE_AAGC__input	= new MediaProcess_AQE("PRE_AAGC__input");
		PRE_AAGC__output	= new MediaProcess_AQE("PRE_AAGC__output");
		PRE_AEC__input	= new MediaProcess_AQE("PRE_AEC__input");
		PRE_AEC__output	= new MediaProcess_AQE("PRE_AEC__output");
		PRE_NR__input	= new MediaProcess_AQE("PRE_NR__input");
		PRE_NR__output	= new MediaProcess_AQE("PRE_NR__output");
		PRE_DAGC_NetOut__input	= new MediaProcess_AQE("PRE_DAGC_NetOut__input");
		PRE_DAGC_NetOut__output	= new MediaProcess_AQE("PRE_DAGC_NetOut__output");
		PRE_SA_NetOut__input	= new MediaProcess_AQE("PRE_SA_NetOut__input");
		PRE_SA_NetOut__output	= new MediaProcess_AQE("PRE_SA_NetOut__output");
	}

	static void TearDownTestCase()
	{
// 		DELETE_POINTER(pAQE);
// 		DELETE_POINTER(pCaptureReceiver);
// 		DELETE_POINTER(pPlaybackReceiver);
// 
// 		DELETE_POINTER(POST_SA_NetIn__input);
// 		DELETE_POINTER(POST_SA_NetIn__output);
// 		DELETE_POINTER(POST_DAGC_NetIn__input);
// 		DELETE_POINTER(POST_DAGC_NetIn__output);
// 		DELETE_POINTER(POST_SA_SpkOut__input);
// 		DELETE_POINTER(POST_SA_SpkOut__output);
// 		DELETE_POINTER(PRE_SA_MicIn__input);
// 		DELETE_POINTER(PRE_SA_MicIn__output);
// 		DELETE_POINTER(PRE_AAGC__input);
// 		DELETE_POINTER(PRE_AAGC__output);
// 		DELETE_POINTER(PRE_AEC__input);
// 		DELETE_POINTER(PRE_AEC__output);
// 		DELETE_POINTER(PRE_NR__input);
// 		DELETE_POINTER(PRE_NR__output);
// 		DELETE_POINTER(PRE_DAGC_NetOut__input);
// 		DELETE_POINTER(PRE_DAGC_NetOut__output);
// 		DELETE_POINTER(PRE_SA_NetOut__input);
// 		DELETE_POINTER(PRE_SA_NetOut__output);
	}


private:
	
};




TEST_F(AQETest,LoadAllBasicModules)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->LoadAllBasicModules());
}

TEST_F(AQETest,LoadOneModule)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->LoadOneModule(AEC,IN_AEC));
}

TEST_F(AQETest,InitialBasicModulesDefault)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->InitialBasicModulesDefault());
}

TEST_F(AQETest,InitialOneModule)
{
	ModuleType moduOptType = AEC;
	PlaceType placeType = IN_AEC;
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->InitialOneModule(moduOptType,placeType));
}

TEST_F(AQETest,SetModulePara_GetModulePara)
{
	//AQE
	{	
		int intervalTime = 20;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AQE, INTERVALTIME, AQE_MODULE, &intervalTime));
		WBXAE_SAMPLE_RATE NetInSampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AQE, NET_IN_SAMPRATE, AQE_MODULE, &NetInSampRate));
		WBXAE_SAMPLE_RATE MicInSampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AQE, MIC_IN_SAMPRATE, AQE_MODULE, &MicInSampRate));
		WBXAE_SAMPLE_RATE NetOutSampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AQE, NET_OUT_SAMPRATE, AQE_MODULE, &NetOutSampRate));
		WBXAE_SAMPLE_RATE SpkOutSampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AQE, SPK_OUT_SAMPRATE, AQE_MODULE, &SpkOutSampRate));
		
		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AQE, INTERVALTIME, AQE_MODULE, pValue));
		if (NULL != pValue)
		{
			int intervalTime_get = *(int*)pValue;
			EXPECT_EQ(intervalTime,intervalTime_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AQE, NET_IN_SAMPRATE, AQE_MODULE, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE NetInSampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(NetInSampRate,NetInSampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AQE, MIC_IN_SAMPRATE, AQE_MODULE, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE MicInSampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(MicInSampRate,MicInSampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AQE, NET_OUT_SAMPRATE, AQE_MODULE, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE NetOutSampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(NetOutSampRate,NetOutSampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AQE, SPK_OUT_SAMPRATE, AQE_MODULE, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE SpkOutSampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(SpkOutSampRate,SpkOutSampRate_get);
		}
	}

	//AEC 
	{	
		PlaceType place = IN_AEC;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, PLACE, IN_AEC, &place));
		int complexity = 0;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, COMPLEXITY, IN_AEC, &complexity));
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, STATUS, IN_AEC, &status));
		int level = 1;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, LEVEL, IN_AEC, &level));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, SAMPLERATE, IN_AEC, &sampRate));
		int micDelay = 100;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, MICDELAY, IN_AEC, &micDelay));
		int spkDelay = 200;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AEC, SPKDELAY, IN_AEC, &spkDelay));
        float srcEchoLev = -10;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(AEC, AEC_SRC_ECHO_LEVEL, IN_AEC, &srcEchoLev));
		float resEchoLev = -30;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(AEC, AEC_RES_ECHO_LEVEL, IN_AEC, &resEchoLev));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, PLACE, IN_AEC, pValue));
		if (NULL != pValue)
		{
			PlaceType value_get = *(PlaceType*)pValue;
			EXPECT_EQ(place,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, COMPLEXITY, IN_AEC, pValue));
		if (NULL != pValue)
		{
			int value_get = *(int*)pValue;
			EXPECT_EQ(complexity,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, STATUS, IN_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, LEVEL, IN_AEC, pValue));
		if (NULL != pValue)
		{
			int level_get = *(int*)pValue;
			EXPECT_EQ(level,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, SAMPLERATE, IN_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, MICDELAY, IN_AEC, pValue));
		if (NULL != pValue)
		{
			int micDelay_get = *(int*)pValue;
			EXPECT_EQ(micDelay,micDelay_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AEC, SPKDELAY, IN_AEC, pValue));
		if (NULL != pValue)
		{
			int spkDelay_get = *(int*)pValue;
			EXPECT_EQ(spkDelay,spkDelay_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(AEC, AEC_SRC_ECHO_LEVEL, IN_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(srcEchoLev,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(AEC, AEC_RES_ECHO_LEVEL, IN_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(resEchoLev,value_get);
		}
	}
     //NR
	{	
		int complexity = 0;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(NR, COMPLEXITY, PRE_PROC_AFTER_AEC, &complexity));
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(NR, STATUS, PRE_PROC_AFTER_AEC, &status));
		int level = 1;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(NR, LEVEL, PRE_PROC_AFTER_AEC, &level));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_8K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(NR, SAMPLERATE, PRE_PROC_AFTER_AEC, &sampRate));
		float srcNoiseLev = -15;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(NR, NR_SRC_NOISE_LEVEL, PRE_PROC_AFTER_AEC, &srcNoiseLev));
		float resNoiseLev = -35;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(NR, NR_RES_NOISE_LEVEL, PRE_PROC_AFTER_AEC, &resNoiseLev));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(NR, COMPLEXITY, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			int value_get = *(int*)pValue;
			EXPECT_EQ(complexity,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(NR, STATUS, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(NR, LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			int level_get = *(int*)pValue;
			EXPECT_EQ(level,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(NR, SAMPLERATE, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}		
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(NR, NR_SRC_NOISE_LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(srcNoiseLev,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(NR, NR_RES_NOISE_LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(resNoiseLev,value_get);
		}
	}

	//DAGC_Mic
	{	
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, STATUS, PRE_PROC_AFTER_AEC, &status));
		int level = 189;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, LEVEL, PRE_PROC_AFTER_AEC, &level));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, SAMPLERATE, PRE_PROC_AFTER_AEC, &sampRate));
		float inputLevel = -25;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(DAGC, DAGC_INPUT_LEVEL, PRE_PROC_AFTER_AEC, &inputLevel));
		float outputLevel = -15;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(DAGC, DAGC_OUTPUT_LEVEL, PRE_PROC_AFTER_AEC, &outputLevel));
		float curGain = 3.5;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(DAGC, DAGC_GAIN, PRE_PROC_AFTER_AEC, &curGain));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, STATUS, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			int level_get = *(int*)pValue;
			EXPECT_EQ(level,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, SAMPLERATE, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(DAGC, DAGC_INPUT_LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			float level_get = *(float*)pValue;
			EXPECT_EQ(inputLevel,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(DAGC, DAGC_OUTPUT_LEVEL, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			float level_get = *(float*)pValue;
			EXPECT_EQ(outputLevel,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(DAGC, DAGC_GAIN, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			float level_get = *(float*)pValue;
			EXPECT_EQ(curGain,level_get);
		}
	}

	//DAGC_Spk
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, STATUS, POST_PROC_BEFORE_AEC, &status));
		int level = 189;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, LEVEL, POST_PROC_BEFORE_AEC, &level));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(DAGC, SAMPLERATE, POST_PROC_BEFORE_AEC, &sampRate));
		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, STATUS, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, LEVEL, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			int level_get = *(int*)pValue;
			EXPECT_EQ(level,level_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(DAGC, SAMPLERATE, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
	}

	//AAGC
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AAGC, STATUS, PRE_PROC_BEFORE_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(AAGC, SAMPLERATE, PRE_PROC_BEFORE_AEC, &sampRate));
		int AnaAGCRevVal = 160;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(AAGC, ANA_AGC_REV_VAL, PRE_PROC_BEFORE_AEC, &AnaAGCRevVal));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AAGC, STATUS, PRE_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(AAGC, SAMPLERATE, PRE_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(AAGC, ANA_AGC_REV_VAL, PRE_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			int value_get = *(int*)pValue;
			EXPECT_EQ(AnaAGCRevVal,value_get);
		}
	}

	//SA_NetIn
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, STATUS, POST_PROC_BEFORE_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, SAMPLERATE, POST_PROC_BEFORE_AEC, &sampRate));
		
		float snr = 20;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, SNR, POST_PROC_BEFORE_AEC, &snr));
		float noiseLevel = -20;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, NOISE_LEVEL, POST_PROC_BEFORE_AEC, &noiseLevel));
		float audioLevel = -10;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, AUDIO_LEVEL, POST_PROC_BEFORE_AEC, &audioLevel));
		BOOL vadFig = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, VAD_FIG, POST_PROC_BEFORE_AEC, &vadFig));
		BOOL zeroFig = FALSE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, ZERO_FIG, POST_PROC_BEFORE_AEC, &zeroFig));
		BOOL saturateFig = FALSE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModuleInfo(SA, SATURATE_FIG, POST_PROC_BEFORE_AEC, &saturateFig));
			
		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, STATUS, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, SAMPLERATE, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, SNR, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(snr,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, NOISE_LEVEL, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(noiseLevel,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, AUDIO_LEVEL, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			float value_get = *(float*)pValue;
			EXPECT_EQ(audioLevel,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, VAD_FIG, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL value_get = *(BOOL*)pValue;
			EXPECT_EQ(vadFig,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, ZERO_FIG, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL value_get = *(BOOL*)pValue;
			EXPECT_EQ(zeroFig,value_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModuleInfo(SA, SATURATE_FIG, POST_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL value_get = *(BOOL*)pValue;
			EXPECT_EQ(saturateFig,value_get);
		}
	}

	//SA_MicIn
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, STATUS, PRE_PROC_BEFORE_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, SAMPLERATE, PRE_PROC_BEFORE_AEC, &sampRate));
		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, STATUS, PRE_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, SAMPLERATE, PRE_PROC_BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
	}

	//SA_NetOut
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, STATUS, PRE_PROC_AFTER_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, SAMPLERATE, PRE_PROC_AFTER_AEC, &sampRate));
		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, STATUS, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, SAMPLERATE, PRE_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
	}

	//SA_SpkOut
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, STATUS, POST_PROC_AFTER_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(SA, SAMPLERATE, POST_PROC_AFTER_AEC, &sampRate));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, STATUS, POST_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(SA, SAMPLERATE, POST_PROC_AFTER_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}
	}

	//Prepare AEC data
	{
		BOOL status = TRUE;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(PREPAEC, STATUS, BEFORE_AEC, &status));
		WBXAE_SAMPLE_RATE sampRate = AE_SAMPLE_RATE_48K;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetModulePara(PREPAEC, SAMPLERATE, BEFORE_AEC, &sampRate));

		void* pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(PREPAEC, STATUS, BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			BOOL status_get = *(BOOL*)pValue;
			EXPECT_EQ(status,status_get);
		}
		pValue = NULL;
		EXPECT_EQ(WBXAE_SUCCESS,pAQE->GetModulePara(PREPAEC, SAMPLERATE, BEFORE_AEC, pValue));
		if (NULL != pValue)
		{
			WBXAE_SAMPLE_RATE sampRate_get = *(WBXAE_SAMPLE_RATE*)pValue;
			EXPECT_EQ(sampRate,sampRate_get);
		}

	}




}

TEST_F(AQETest,LinkModulesInAQE)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->LinkModulesInAQE());
}


TEST_F(AQETest,SetSinkInfoCollector)
{
	IInfoCollector* pCollector = (IInfoCollector*)66;
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkInfoCollector(pCollector));
}


TEST_F(AQETest,SetSinkMediaProcess)
{
	
	//SA_NetIn
	POST_SA_NetIn__input	= new MediaProcess_AQE("POST_SA_NetIn__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_SA_NetIn__input, SA, POST_PROC_BEFORE_AEC, TRUE));
	POST_SA_NetIn__output	= new MediaProcess_AQE("POST_SA_NetIn__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_SA_NetIn__output, SA, POST_PROC_BEFORE_AEC, FALSE));
	//DAGC_NetIn
	POST_DAGC_NetIn__input	= new MediaProcess_AQE("POST_DAGC_NetIn__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_DAGC_NetIn__input, DAGC, POST_PROC_BEFORE_AEC, TRUE));
	POST_DAGC_NetIn__output	= new MediaProcess_AQE("POST_DAGC_NetIn__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_DAGC_NetIn__output, DAGC, POST_PROC_BEFORE_AEC, FALSE));
	//SA_SpkOut
	POST_SA_SpkOut__input	= new MediaProcess_AQE("POST_SA_SpkOut__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_SA_SpkOut__input, SA, POST_PROC_AFTER_AEC, TRUE));
	POST_SA_SpkOut__output	= new MediaProcess_AQE("POST_SA_SpkOut__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(POST_SA_SpkOut__output, SA, POST_PROC_AFTER_AEC, FALSE));

	//SA_MicIn
	PRE_SA_MicIn__input	= new MediaProcess_AQE("PRE_SA_MicIn__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_SA_MicIn__input, SA, PRE_PROC_BEFORE_AEC, TRUE));
	PRE_SA_MicIn__output	= new MediaProcess_AQE("PRE_SA_MicIn__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_SA_MicIn__output, SA, PRE_PROC_BEFORE_AEC, FALSE));
	//AAGC
	PRE_AAGC__input	= new MediaProcess_AQE("PRE_AAGC__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_AAGC__input, AAGC, PRE_PROC_BEFORE_AEC, TRUE));
	PRE_AAGC__output	= new MediaProcess_AQE("PRE_AAGC__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_AAGC__output, AAGC, PRE_PROC_BEFORE_AEC, FALSE));
	//AEC
	PRE_AEC__input	= new MediaProcess_AQE("PRE_AEC__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_AEC__input, AEC, IN_AEC, TRUE));
	PRE_AEC__output	= new MediaProcess_AQE("PRE_AEC__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_AEC__output, AEC, IN_AEC, FALSE));
	//NR
	PRE_NR__input	= new MediaProcess_AQE("PRE_NR__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_NR__input, NR, PRE_PROC_AFTER_AEC, TRUE));
	PRE_NR__output	= new MediaProcess_AQE("PRE_NR__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_NR__output, NR, PRE_PROC_AFTER_AEC, FALSE));
	//DAGC_NetOut
	PRE_DAGC_NetOut__input	= new MediaProcess_AQE("PRE_DAGC_NetOut__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_DAGC_NetOut__input, DAGC, PRE_PROC_AFTER_AEC, TRUE));
	PRE_DAGC_NetOut__output	= new MediaProcess_AQE("PRE_DAGC_NetOut__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_DAGC_NetOut__output, DAGC, PRE_PROC_AFTER_AEC, FALSE));
	//SA_NetOut
	PRE_SA_NetOut__input	= new MediaProcess_AQE("PRE_SA_NetOut__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_SA_NetOut__input, SA, PRE_PROC_AFTER_AEC, TRUE));
	PRE_SA_NetOut__output	= new MediaProcess_AQE("PRE_SA_NetOut__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(PRE_SA_NetOut__output, SA, PRE_PROC_AFTER_AEC, FALSE));
}



TEST_F(AQETest,StartWork)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->StartWork());
}



TEST_F(AQETest,SetSinkCaptureData)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkCaptureData(pCaptureReceiver));
}

TEST_F(AQETest,SetSinkPlaybackData)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkPlaybackData(pPlaybackReceiver));
}

TEST_F(AQETest,OnSendData)
{
	for (int i=0; i<200; i++)
	{
		CWbxAeMediaBlock* pDataBlock = NULL;
		pDataBlock = new CWbxAeMediaBlock(3000);
		if (NULL != pDataBlock)
		{
			pDataBlock->AddRef();
			if (pDataBlock->GetWriteSpace() >= sizeof(short)*FRAME_LEN  && pPlaybackFile)
			{
				fread(pDataBlock->GetWritePtr(),sizeof(short),FRAME_LEN,pPlaybackFile);
				pDataBlock->AdvanceWritePtr(sizeof(short)*FRAME_LEN);

				pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
				pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
				pDataBlock->GetWaveFormat()->nChannels = 1;
				pDataBlock->GetWaveFormat()->nSamplesPerSec = AE_SAMPLE_RATE_48K;
				pDataBlock->GetWaveFormat()->nAvgBytesPerSec = AE_SAMPLE_RATE_48K*sizeof(short);
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

//				EXPECT_EQ(WBXAE_SUCCESS,pAQE->OnPlaybackData(pDataBlock));
			}
			pDataBlock->Release();
		}	
		pDataBlock = new CWbxAeMediaBlock(3000);
		if (NULL != pDataBlock)
		{
			pDataBlock->AddRef();
			if (pDataBlock->GetWriteSpace() >= sizeof(short)*FRAME_LEN && pCaptureFile)
			{
				fread(pDataBlock->GetWritePtr(),sizeof(short),FRAME_LEN,pCaptureFile);
				pDataBlock->AdvanceWritePtr(sizeof(short)*FRAME_LEN);

				pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
				pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
				pDataBlock->GetWaveFormat()->nChannels = 1;
				pDataBlock->GetWaveFormat()->nSamplesPerSec = AE_SAMPLE_RATE_48K;
				pDataBlock->GetWaveFormat()->nAvgBytesPerSec = AE_SAMPLE_RATE_48K*sizeof(short);
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

				EXPECT_EQ(WBXAE_SUCCESS,pAQE->OnCaptureData(pDataBlock));
			}
			pDataBlock->Release();
		}
		HANDLE m_hThread;

		Sleep(INTETVAL_TIME);

		int inputLev = pAQE->GetInputLevel();
		int outputLev = pAQE->GetOutputLevel();
		int captureDelay = pAQE->GetCaptureDelay();
		int playbackDelay = pAQE->GetPlaybackDelay();
	}	
}

TEST_F(AQETest,OnPlaybackData)
{
	CWbxAeMediaBlock* pDataBlock = NULL;
	pDataBlock = new CWbxAeMediaBlock(3000);
	if (NULL != pDataBlock)
	{
		pDataBlock->AddRef();
		if (pDataBlock->GetWriteSpace() >= sizeof(short)*FRAME_LEN && pPlaybackFile)
		{
			fread(pDataBlock->GetWritePtr(),sizeof(short),FRAME_LEN,pPlaybackFile);
			pDataBlock->AdvanceWritePtr(sizeof(short)*FRAME_LEN);

			pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
			pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
			pDataBlock->GetWaveFormat()->nChannels = 1;
			pDataBlock->GetWaveFormat()->nSamplesPerSec = AE_SAMPLE_RATE_48K;
			pDataBlock->GetWaveFormat()->nAvgBytesPerSec = AE_SAMPLE_RATE_48K*sizeof(short);
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

			EXPECT_EQ(WBXAE_SUCCESS,pAQE->OnPlaybackData(pDataBlock));
		}
		pDataBlock->Release();
	}		
}

TEST_F(AQETest,OnCaptureData)
{
 	CWbxAeMediaBlock* pDataBlock = NULL;
	pDataBlock = new CWbxAeMediaBlock(3000);
	if (NULL != pDataBlock)
	{
		pDataBlock->AddRef();
		if (pDataBlock->GetWriteSpace() >= sizeof(short)*FRAME_LEN && pCaptureFile )
		{
			fread(pDataBlock->GetWritePtr(),sizeof(short),FRAME_LEN,pCaptureFile);
			pDataBlock->AdvanceWritePtr(sizeof(short)*FRAME_LEN);


			pDataBlock->GetWaveFormat()->dwSize = sizeof(WBXWAVEFORMAT);
			pDataBlock->GetWaveFormat()->wFormatTag = WAVE_FORMAT_PCM;
			pDataBlock->GetWaveFormat()->nChannels = 1;
			pDataBlock->GetWaveFormat()->nSamplesPerSec = AE_SAMPLE_RATE_48K;
			pDataBlock->GetWaveFormat()->nAvgBytesPerSec = AE_SAMPLE_RATE_48K*sizeof(short);
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

			EXPECT_EQ(WBXAE_SUCCESS,pAQE->OnCaptureData(pDataBlock));
		}
		pDataBlock->Release();
	}
	
}



TEST_F(AQETest,StopWork)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->StopWork());
}




TEST_F(AQETest,RemoveSinkInfoCollector)
{
	IInfoCollector* pCollector = (IInfoCollector*)66;
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveSinkInfoCollector(pCollector));
}


TEST_F(AQETest,RemoveMediaProcessAll)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveMediaProcessAll());
}

TEST_F(AQETest,RemoveMediaProcess)
{
	IWbxMediaProcess* pProcess_in	= new MediaProcess_AQE("AEC__input");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(pProcess_in, AEC, IN_AEC, TRUE));
	IWbxMediaProcess* pProcess_out	= new MediaProcess_AQE("AEC__output");
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->SetSinkMediaProcess(pProcess_out, AEC, IN_AEC, FALSE));
 	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveMediaProcess(pProcess_in, AEC, IN_AEC, TRUE));
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveMediaProcess(pProcess_out, AEC, IN_AEC, FALSE));
	DELETE_POINTER(pProcess_in);
	DELETE_POINTER(pProcess_out);

}


TEST_F(AQETest,RemoveSinkCaptureData)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveSinkCaptureData(pCaptureReceiver));
}


TEST_F(AQETest,RemoveSinkPlaybackData)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->RemoveSinkPlaybackData(pPlaybackReceiver));
}


TEST_F(AQETest,UnloadAllBasicModules)
{
	EXPECT_EQ(WBXAE_SUCCESS,pAQE->UnloadAllBasicModules());
}




#endif
