
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CmStdCpp.h"

#include "WbxAeAQE.h"

// #include "WbxAePrepareAECData_Module.h"
// #include "WbxAeSignalAnalysis_Module.h"
// #include "WbxAeAEC_Module.h"
// #include "WbxAeNR_Module.h"
// #include "WbxAeBuffer_Module.h"
// #include "WbxAeAnalogAGCDecide_Module.h"
// #include "WbxAeDigitalAGC_Module.h"


// 
// 
// 
// using testing::Return;
// using testing::_;
// using testing::Invoke;
// using testing::ReturnRef;
// using testing::Matcher;



// 
//  
// class ModulesTest : public testing::Test
// {
// public:
// 	ModulesTest()
// 	{
// 		pBaseModule = new CBaseModule();
// 		pPrepareAECData = new CPrepareAECData(BEFORE_AEC);
// 		pSignalAnalysis_NetIn = new CSignalAnalysis(POST_PROC_BEFORE_AEC);
// 		pSignalAnalysis_MicIn = new CSignalAnalysis(PRE_PROC_BEFORE_AEC);
// 		pSignalAnalysis_SpkOut = new CSignalAnalysis(POST_PROC_AFTER_AEC);
// 		pSignalAnalysis_NetOut = new CSignalAnalysis(PRE_PROC_AFTER_AEC);
// 		pAEC = new CAEC_Module(IN_AEC);
// 		pNR = new CNR_Module(PRE_PROC_AFTER_AEC);
// 		pDigitalAGC_Post = new CDigitalAGC(POST_PROC_BEFORE_AEC);
// 		pDigitalAGC_Pre = new CDigitalAGC(PRE_PROC_AFTER_AEC);
// 		pNetBuffer = new CBuffer_Module(POST_PROC_BEFORE_AEC);
// 		pMicBuffer = new CBuffer_Module(PRE_PROC_BEFORE_AEC);
// 		pAnaAGCDecide = new CAnalogAGCDecide(PRE_PROC_BEFORE_AEC);
// 	}
// 
// 	virtual ~ModulesTest()
// 	{
// 		if (NULL != pBaseModule)
// 		{
// 			delete pBaseModule;
// 			pBaseModule = NULL;
// 
// 		}
// 		if (NULL != pPrepareAECData)
// 		{
// 			delete pPrepareAECData;
// 			pPrepareAECData = NULL;
// 		}
// 		if (NULL != pSignalAnalysis_NetIn)
// 		{
// 			delete pSignalAnalysis_NetIn;
// 			pSignalAnalysis_NetIn = NULL;
// 		}
// 		if (NULL != pSignalAnalysis_MicIn)
// 		{
// 			delete pSignalAnalysis_MicIn;
// 			pSignalAnalysis_MicIn = NULL;
// 		}
// 		if (NULL != pSignalAnalysis_SpkOut)
// 		{
// 			delete pSignalAnalysis_SpkOut;
// 			pSignalAnalysis_SpkOut = NULL;
// 		}
// 		if (NULL != pSignalAnalysis_NetOut)
// 		{
// 			delete pSignalAnalysis_NetOut;
// 			pSignalAnalysis_NetOut = NULL;
// 		}
// 		if (NULL != pAEC)
// 		{
// 			delete pAEC;
// 			pAEC = NULL;
// 		}
// 		if (NULL != pNR)
// 		{
// 			delete pNR;
// 			pNR = NULL;
// 		}
// 		if (NULL != pDigitalAGC_Post)
// 		{
// 			delete pDigitalAGC_Post;
// 			pDigitalAGC_Post = NULL;
// 		}
// 		if (NULL != pDigitalAGC_Pre)
// 		{
// 			delete pDigitalAGC_Pre;
// 			pDigitalAGC_Pre = NULL;
// 		}
// 		if (NULL != pNetBuffer)
// 		{
// 			delete pNetBuffer;
// 			pNetBuffer = NULL;
// 		}
// 		if (NULL != pMicBuffer)
// 		{
// 			delete pMicBuffer;
// 			pMicBuffer = NULL;
// 		}
// 		if (NULL != pAnaAGCDecide)
// 		{
// 			delete pAnaAGCDecide;
// 			pAnaAGCDecide = NULL;
// 		}
// 	}
// 
// 	virtual void SetUp()
// 	{
// 		pSignalAnalysis_NetIn->Connect(pDigitalAGC_Post);
// 		pDigitalAGC_Post->Connect(pNetBuffer);
// 
// 		pSignalAnalysis_MicIn->Connect(pAnaAGCDecide);
// 		pAnaAGCDecide->Connect(pMicBuffer);
// 		
// 		pPrepareAECData->ConnectBuffer(pNetBuffer,pMicBuffer);
// 		pPrepareAECData->Connect(pAEC);
// 		pAEC->Connect(pSignalAnalysis_NetOut,pNR);
// 
// 		pNR->Connect(pDigitalAGC_Pre);
// 		pDigitalAGC_Pre->Connect(pSignalAnalysis_NetOut);
// 		
// 	}
// 
// 	virtual void TearDown()
// 	{
// 	}
// 
// protected:
// 	CBaseModule* pBaseModule ;
// 	CPrepareAECData* pPrepareAECData ;
// 	CSignalAnalysis* pSignalAnalysis_NetIn;
// 	CSignalAnalysis* pSignalAnalysis_MicIn;
// 	CSignalAnalysis* pSignalAnalysis_SpkOut;
// 	CSignalAnalysis* pSignalAnalysis_NetOut;
// 	CAEC_Module*		pAEC;
// 	CNR_Module*			pNR;
// 	CDigitalAGC*		pDigitalAGC_Post ;
// 	CDigitalAGC*		pDigitalAGC_Pre;
// 	CBuffer_Module*		pNetBuffer;
// 	CBuffer_Module*		pMicBuffer;
// 	CAnalogAGCDecide*	pAnaAGCDecide;
// };

// TEST_F(ModulesTest, Call)
// {
// 
// 
// 	float level = 2;
// 	pSignalAnalysis_NetIn->GetPara()->SetOpt(NOISE_LEVEL,&level);
// 
// 	void* pLevel;
// 	pSignalAnalysis_NetIn->GetPara()->GetOpt(NOISE_LEVEL,pLevel);
// 	float flevel = *(float*)pLevel;
// 
// 
// }



