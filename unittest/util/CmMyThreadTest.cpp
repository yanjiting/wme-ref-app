#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "CmMyThread.h"
#include "CmThreadManager.h"
#include "CmUtil.h"

class CCmMyThreadTest : public testing::Test
{
public:
	CCmMyThreadTest()
	{
		m_pMyThread = NULL;
	}		
	
	virtual ~CCmMyThreadTest()
	{
		if (m_pMyThread) {
			delete m_pMyThread;
			m_pMyThread = NULL;
		}
	}	


	virtual void SetUp()
	{
		CmUtilInit();

		if (!m_pMyThread) 
			m_pMyThread = new CCmMyThread();	
	}

	virtual void TearDown()
	{
		if (m_pMyThread) {
			delete m_pMyThread;
			m_pMyThread = NULL;
		}
	}

protected:
	
	CCmMyThread *m_pMyThread;	

};


TEST_F(CCmMyThreadTest, TestCreateAndStop)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_JOINABLE, FALSE);
        EXPECT_TRUE(rv == CM_OK);
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_RUN);

        m_pMyThread->Stop();
        rv = m_pMyThread->Join();
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_STOP);

	m_pMyThread->Destory(CM_OK);
	m_pMyThread = NULL;
}

/*
TEST_F(CCmMyThreadTest, TestMutipleJoin)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_JOINABLE, FALSE);
        EXPECT_TRUE(rv == CM_OK);
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_RUN);

        m_pMyThread->Stop();
        rv = m_pMyThread->Join();
	//rv = m_pMyThread->Join();
	EXPECT_TRUE(rv == CM_OK); 
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_STOP);
}
*/

TEST_F(CCmMyThreadTest, TestCreateAndStopwithTF_DETACHED)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_DETACHED, FALSE);
        EXPECT_TRUE(rv == CM_OK);
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_RUN);

        m_pMyThread->Stop();

        rv = m_pMyThread->Destory(CM_OK);
	EXPECT_TRUE(rv == CM_OK);

        m_pMyThread = NULL;
}

TEST_F(CCmMyThreadTest, TestCreateAndTerminate)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_DETACHED, FALSE);
        EXPECT_TRUE(rv == CM_OK);
        EXPECT_TRUE(m_pMyThread->get_status() == STATUS_RUN);

        m_pMyThread->Terminate();
        //rv = m_pMyThread->Join();
        //EXPECT_TRUE(m_pMyThread->get_status() == STATUS_STOP);
}


TEST_F(CCmMyThreadTest, TestCreateWithTT_MAINnotregister)
{
	CmResult rv = CM_ERROR_FAILURE;

	rv = m_pMyThread->Create("test", TT_MAIN, TF_JOINABLE, FALSE);
    EXPECT_TRUE(rv == CM_OK);
    m_pMyThread->Stop();
    m_pMyThread->Join();
}

TEST_F(CCmMyThreadTest, TestCreateWithTT_USER_DEFINE_BASEnotregister)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_NONE, FALSE);
        EXPECT_TRUE(rv == CM_OK);
        m_pMyThread->Stop();
        m_pMyThread->Join();
}

TEST_F(CCmMyThreadTest, TestCreateWithTT_MAINregister)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("main", TT_MAIN, TF_JOINABLE, TRUE);
        EXPECT_TRUE(rv == CM_OK);
        m_pMyThread->Stop();
        m_pMyThread->Join();
}


TEST_F(CCmMyThreadTest, TestCreateWithTT_USER_DEFINE_BASEregister)
{
        CmResult rv = CM_ERROR_FAILURE;

        rv = m_pMyThread->Create("test", TT_USER_DEFINE_BASE, TF_JOINABLE, TRUE);
        EXPECT_TRUE(rv == CM_OK);
        m_pMyThread->Stop();
        m_pMyThread->Join();
}
