#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeUtils.h"
#include "WmeNetworkIndicatorImp.h"

class CNetworkIndicatorTest : public testing::Test{
    
public:
	CNetworkIndicatorTest(){
		m_pEngine = NULL;
	}
	virtual ~CNetworkIndicatorTest(){
        
	}
    
	void SetUp()
	{
		m_pEngine = NULL;
		wme::WmeCreateMediaEngine(&m_pEngine);
		//
		if(m_pEngine){
			WMERESULT ret = m_pEngine->CreateNetworkIndicator(&m_pNetworkIndicator);
            ASSERT_EQ(ret, WME_S_OK);
        }
        
	}
	void TearDown()
	{
        SAFE_RELEASE(m_pNetworkIndicator);
		SAFE_RELEASE(m_pEngine);
	}
    
    
  protected:
	wme::IWmeMediaEngine* m_pEngine;
    wme::IWmeNetworkIndicator * m_pNetworkIndicator;
};

TEST_F(CNetworkIndicatorTest, TestGetMappingDirection)
{
    wme::CWmeNetworkIndicatorImp * pIndicator = reinterpret_cast<wme::CWmeNetworkIndicatorImp *>(m_pNetworkIndicator);

    wrtp::CNetworkIndicator::NetworkDirection d = pIndicator->GetMappingDirection(wme::DIRECTION_UPLINK);
    ASSERT_EQ(d, wrtp::CNetworkIndicator::DIRECTION_UPLINK);
    
    d = pIndicator->GetMappingDirection(wme::DIRECTION_DOWNLINK);
    ASSERT_EQ(d, wrtp::CNetworkIndicator::DIRECTION_DOWNLINK);
    
    d = pIndicator->GetMappingDirection(wme::DIRECTION_BOTHLINK);
    ASSERT_EQ(d, wrtp::CNetworkIndicator::DIRECTION_BOTHLINK);
}

TEST_F(CNetworkIndicatorTest, TestGetMappingIndex)
{
    wme::CWmeNetworkIndicatorImp * pIndicator = reinterpret_cast<wme::CWmeNetworkIndicatorImp *>(m_pNetworkIndicator);

    wme::WmeNetworkIndex idx;
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_UNKNOWN);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_UNKNOWN);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_USELESS);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_USELESS);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_SEVERE);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_SEVERE);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_NOT_ACCEPTABLE);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_POOR);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_POOR);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_GOOD);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_GOOD);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_VERY_GOOD);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_VERY_GOOD);
    
    idx = pIndicator->GetMappingIndex(wrtp::CNetworkIndicator::NETWORK_EXCELLENT);
    ASSERT_EQ(idx, wme::WmeNetworkIndex::NETWORK_EXCELLENT);
}


