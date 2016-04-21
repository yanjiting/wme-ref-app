//
//  CAudioDefaultSettingsTest.cpp
//  dolphinUnitTestApp
//
//  Created by Boris Lin on 14-10-14.
//  Copyright (c) 2014年 WME. All rights reserved.
//


#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#define private     public
#define protected   public


#include "AudioDefaultSettings.h"
#include "WbxAudioEngineImpl.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAeAudioDefaultSettingsTest : public testing::Test
{
public:
    CWbxAeAudioDefaultSettingsTest()
    {
    }
    
    virtual ~CWbxAeAudioDefaultSettingsTest()
    {
    }
    
    virtual void SetUp()
    {
		
    }
    
    virtual void TearDown()
    {
		
    }

};


TEST_F(CWbxAeAudioDefaultSettingsTest, load)
{
#if  defined(MX_IOS) || defined(ANDROID) || defined(MACOS) || defined(WIN32)
    CAudioDefaultSettings &inst = CAudioDefaultSettings::getInstance();
    inst.getInstance();
    EXPECT_EQ(true, inst.m_isLoaded);
    
    int nTotal =(int) (inst.m_arrDumpModules.size());
    AudioDataModuleType value;
    if (nTotal <= 0)
    {
        EXPECT_EQ(0, (inst.GetNextDumpModules(value)));
    }
    else
    {
        int nRet = -1;
        do
        {
            nRet = inst.GetNextDumpModules(value);
            EXPECT_TRUE((nRet <= nTotal && nRet >= 0));
        }while(nRet > 0);
    }
#endif
}




