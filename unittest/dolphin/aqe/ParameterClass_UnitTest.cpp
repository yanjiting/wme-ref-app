#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtest/gtest-param-test.h"

#include "WbxAeParameterClass.h"


using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;


CAQE_Para* pAQE_Para = new CAQE_Para();

class AQE_ParaTest : public testing::TestWithParam<int>
{
public:
	AQE_ParaTest()
	{
		
	}

	virtual ~AQE_ParaTest()
	{
	}

	virtual void SetUp()
	{
		pAQE_Para->init();
	}

	virtual void TearDown()
	{
	}

protected:
};


// TEST_P(AQE_ParaTest, SetOpt)
// {
// 	ParaType b = (ParaType)GetParam();
// 	int value = 10;
// 	EXPECT_EQ(TRUE,pAQE_Para ->SetOpt(b,&value));
// 
// }
// //INSTANTIATE_TEST_CASE_P(SetOpt,AQE_ParaTest,testing::Range(0, 20,1));
// INSTANTIATE_TEST_CASE_P(SetOpt,AQE_ParaTest,testing::
// 						Values(INTERVALTIME,NET_IN_SAMPRATE,NET_OUT_SAMPRATE,MIC_IN_SAMPRATE,SPK_OUT_SAMPRATE));
// 
// 
// TEST_P(AQE_ParaTest, GetOpt)
// {
// 	ParaType b = (ParaType)GetParam();
// 	int value = 10;
// //	pAQE_Para ->SetOpt(b,&value);
// 
// 	void* returnValue = NULL;
// 	EXPECT_EQ(TRUE,pAQE_Para ->GetOpt(b,returnValue));
// 	
// }
// //INSTANTIATE_TEST_CASE_P(GetOpt,AQE_ParaTest,testing::Range(0, 20,1));
// INSTANTIATE_TEST_CASE_P(GetOpt,AQE_ParaTest,testing::
// 						Values(INTERVALTIME,NET_IN_SAMPRATE,NET_OUT_SAMPRATE,MIC_IN_SAMPRATE,SPK_OUT_SAMPRATE));
// 
// 


