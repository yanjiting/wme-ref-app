//
//  CCAudioMetricsTest
//  dolphinUnitTestApp
//
//  Created by Audio Engine on 14-10-14.
//  Copyright (c) 2014年 WME. All rights reserved.
//


#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#define private     public
#define protected   public


#include "WbxAeAudioMetrics.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CAudioMetricsTest : public testing::Test
{
public:
    CAudioMetricsTest()
    {
    }
    
    virtual ~CAudioMetricsTest()
    {
    }
    
    virtual void SetUp()
    {
        m_pMetrics = new CAudioMetrics();
    }
    
    virtual void TearDown()
    {
        delete m_pMetrics;
        m_pMetrics = NULL;

    }
protected:
    CAudioMetrics *m_pMetrics;
};

TEST_F(CAudioMetricsTest, InitializeCaptureParam)
{
    m_pMetrics->InitializeCaptureParam();
}

TEST_F(CAudioMetricsTest, InitializeNetworkParam)
{
    m_pMetrics->InitializeNetworkParam();
}

TEST_F(CAudioMetricsTest, InitializeRenderParam)
{
    m_pMetrics->InitializeRenderParam();
}

TEST_F(CAudioMetricsTest, GetCurrentCapMetrice)
{
    m_pMetrics->GetCurrentCapMetrice(10);

}

TEST_F(CAudioMetricsTest, CalculateCapWholeCallMetrics)
{
    m_pMetrics->CalculateCapWholeCallMetrics();
    
}

TEST_F(CAudioMetricsTest, GetCurrentRenderMetrice)
{
    m_pMetrics->GetCurrentRenderMetrice(90);
    
}

TEST_F(CAudioMetricsTest, CalculateRenderWholeCallMetrics)
{
    m_pMetrics->CalculateRenderWholeCallMetrics();
    
}

TEST_F(CAudioMetricsTest, GetCurrentNetworkMetrice)
{
    m_pMetrics->GetCurrentNetworkMetrice(40);
    
}

TEST_F(CAudioMetricsTest, CalculateNetworkWholeCallMetrics)
{
    m_pMetrics->CalculateNetworkWholeCallMetrics();
    
}

