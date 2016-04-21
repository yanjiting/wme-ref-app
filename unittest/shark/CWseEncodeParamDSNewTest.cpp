#include <time.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WseCommonTypes.h"
#include "WseEncodeParamGenerator.h"
#include <cmath>
using namespace shark;
class CWseEncodeParamDSNewWrapper : public CWseEncodeParamDSNew{
public:
    CWseEncodeParamDSNewWrapper(){
        
    }
    bool updateStandardParamTestMaxDPB(){
        _ResetParams();
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880) * GetMBSize(1800);
        m_sVideoEncCapability.uMaxMBPS = 60.0 * GetMBSize(2880) * GetMBSize(1800);
        
        //=============================
        m_bLegacyLandscape = false;
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(2880) * GetMBSize(1800) * 3/8 + 8;//1.0 frame, 1KB = 8/3 macro blocks
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(2880*0.9) * GetMBSize(1800*0.9) * 3/8 + 8;//1.0 frame, 1KB = 8/3 macro blocks
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(1920) * GetMBSize(1080) * 3/8 + 8;
        updateStandardParams();
        if(!VerifySize(1136)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(1280) * GetMBSize(720) * 3/8 + 8;
        updateStandardParams();
        if(!VerifySize(752)) return false;
        
        //=============================
        m_bLegacyLandscape = true;
        //!!!! if < 1080p , need check both width and height < standared both width and height
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(2880) * GetMBSize(1800) * 3/8 + 8;//1.0 frame, 1KB = 8/3 macro blocks
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(2880*0.9) * GetMBSize(1800*0.9) * 3/8 + 8;//1.0 frame, 1KB = 8/3 macro blocks
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(1920) * GetMBSize(1080) * 3/8 + 8;
        updateStandardParams();
        if(!VerifySize(1080)) return false;
        
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(1280) * GetMBSize(720) * 3/8 + 8;
        updateStandardParams();
        if(!VerifySize(720)) return false;
        //=============================
        
        return true;
    }
    bool updateStandardParamTestMaxMBPS(){
        _ResetParams();
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880) * GetMBSize(1800);
        m_sVideoEncCapability.uMaxDPB = 10.0 * GetMBSize(2880) * GetMBSize(1800) * 3/8 + 8;//
        
        //=============================
        m_bLegacyLandscape = false;
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(2880) * GetMBSize(1800);//at least 1 fps
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(2880*0.9) * GetMBSize(1800*0.9);//at least 1 fps
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(1920) * GetMBSize(1080);
        updateStandardParams();
        if(!VerifySize(1136)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(1280) * GetMBSize(720);
        updateStandardParams();
        if(!VerifySize(752)) return false;
        
        //=============================
        m_bLegacyLandscape = true;
        //!!!! if < 1080p , need check both width and height < standared both width and height
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(2880) * GetMBSize(1800);//at least 1 fps
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(2880*0.9) * GetMBSize(1800*0.9);//at least 1 fps
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(1920) * GetMBSize(1080);
        updateStandardParams();
        if(!VerifySize(1080)) return false;
        
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(1280) * GetMBSize(720);
        updateStandardParams();
        if(!VerifySize(720)) return false;
         //=============================
        
        return true;
    }
    bool updateStandardParamTestFS(){
        _ResetParams();
        m_sVideoEncCapability.uMaxDPB = 1.0 * GetMBSize(2880) * GetMBSize(1800) * 3/8 + 8;//1.0 frame, 1KB = 8/3 macro blocks
        m_sVideoEncCapability.uMaxMBPS = 1.0 * GetMBSize(2880) * GetMBSize(1800);//at least 1 fps
        
        //=============================
        m_bLegacyLandscape = false;
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880) * GetMBSize(1800);
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880*0.9) * GetMBSize(1800*0.9);
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1920) * GetMBSize(1200);//1920/1.6 = 1200
        updateStandardParams();
        if(!VerifySize(1200)) return false;
        
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1920) * GetMBSize(1200);//1919/1.6 = 1199.37
        updateStandardParams();
        if(!VerifySize(1200)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1818) * GetMBSize(1136);
        updateStandardParams();
        if(!VerifySize(1136)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1920) * GetMBSize(1080);
        updateStandardParams();
        if(!VerifySize(1136)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1280) * GetMBSize(720);
        updateStandardParams();
        if(!VerifySize(752)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1152) * GetMBSize(720);
        updateStandardParams();
        if(!VerifySize(720)) return false;
        
        //=============================
        m_bLegacyLandscape = true;
        //!!!! if < 1080p , need check both width and height < standared both width and height
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880) * GetMBSize(1800);
        updateStandardParams();
        if(!VerifySize(1800)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(2880*0.9) * GetMBSize(1800*0.9);
        updateStandardParams();
        if(!VerifySize(1800*0.9)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1920) * GetMBSize(1200);//1920/1.6 = 1200
        updateStandardParams();
        if(!VerifySize(1200)) return false;

        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1919) * (GetMBSize(1199)-1);//1919/1.6 = 1199.37
        updateStandardParams();
        if(!VerifySize(1080)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1818) * GetMBSize(1136);
        updateStandardParams();
        if(!VerifySize(1080)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1920) * GetMBSize(1080);
        updateStandardParams();
        if(!VerifySize(1080)) return false;
        
        m_sVideoEncCapability.uMaxFS = GetMBSize(1280) * GetMBSize(720);
        updateStandardParams();
        if(!VerifySize(720)) return false;
         //=============================
        
        return true;
    }

protected:
    
    inline bool VerifySize(int nTargetHeigth){
        if(m_nSourceHeight!=nTargetHeigth) return false;
        if(std::abs(1.0 * m_nSourceHeight * m_nSourceOriginWidth/m_nSourceOriginHeight-m_nSourceWidth)>=1.0) return false;
        return true;
    }
    
    void _ResetParams(){
        memset(&m_sVideoEncCapability,sizeof(m_sVideoEncCapability),0);
        m_sVideoEncCapability.uProfileLevelID = 4325407;
        m_sVideoEncCapability.uMaxMBPS = 108000;
        m_sVideoEncCapability.uMaxFS = 20340;
        m_sVideoEncCapability.uMaxFPS = 30;
        m_sVideoEncCapability.uMaxBitRate = 14000000;
        m_sVideoEncCapability.uMaxDPB = 30510;
        m_nSourceOriginWidth = 2880;
        m_nSourceOriginHeight = 1800;
        
        m_fFrameRate = m_sVideoEncCapability.uMaxFPS;
        m_bLegacyLandscape = false;
    }
};
class CWseEncodeParamDSNewTest : public testing::Test
{
public:
	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
        
	}

public:
	CWseEncodeParamDSNewWrapper wseEncodeParamDS;
};

TEST_F(CWseEncodeParamDSNewTest, updateStandardParamsTest)
{
    ASSERT_TRUE(wseEncodeParamDS.updateStandardParamTestMaxDPB());
    ASSERT_TRUE(wseEncodeParamDS.updateStandardParamTestMaxMBPS());
    ASSERT_TRUE(wseEncodeParamDS.updateStandardParamTestFS());
}
