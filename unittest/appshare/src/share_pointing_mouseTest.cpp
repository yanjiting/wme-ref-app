
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "share_pointing.h"

using namespace testing;

class CSharePointingTest : public Test, ISharePointingSink
{
public:
    CSharePointingTest()
    {
        m_pPointing = NULL;

    }
    virtual ~CSharePointingTest()
    {

    }
    virtual void SetUp()
    {
        if (NULL == m_pPointing) {
            m_pPointing = CreateSharePointing(SHARE_POINTING_TYPE_MOUSE, kFullDisplayID);
            m_pPointing->AddSink(this);
        }
    }
    virtual void TearDown()
    {
        if (NULL != m_pPointing) {
            m_pPointing->RemoveSink(this);
            DestroySharePointing(m_pPointing);
            m_pPointing = NULL;
        }
    }

    WBXResult OnPointingPosChanged(WBXInt32 x, WBXInt32 y)
    {
        return 0;
    };
    WBXResult OnPointingImgChanged(ISharePointingImg *pISharePointingImg)
    {
        return 0;
    };
protected:
    ISharePointing *m_pPointing;
};

//ISharePointing *CSharePointingTest::m_pPointing = NULL;

TEST_F(CSharePointingTest, Start)
{
    m_pPointing->Start();
}

TEST_F(CSharePointingTest, Stop)
{
    m_pPointing->Stop();
}

TEST_F(CSharePointingTest, CSharePointingMouse)
{
    m_pPointing->Start();
    m_pPointing->CapturePoint();
    m_pPointing->CapturePoint();
}
