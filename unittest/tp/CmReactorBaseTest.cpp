//
//  CmReactorBaseTest.cpp
//  tpTest_mac
//
//  Created by yangzhen on 5/14/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//


#include "gtest/gtest.h"
#define SUPPORT_REACTOR
#include "CmReactorBase.h"
#include "CmThreadManager.h"
#include "CmPipe.h"
#include "atdef.h"
#include <map>
#include "CmThread.h"
#include "CmUtilMisc.h"

class CCmEventHandlerRepositoryTest : public testing::Test
{
public:
    CCmEventHandlerRepositoryTest()
    {
    }
    
    virtual ~CCmEventHandlerRepositoryTest()
    {
    }
    
    static void SetUpTestCase()
    {
#ifdef CM_WIN32
        m_pCmThreadMgr = CCmThreadManager::Instance();
#endif
    }
    static void TearDownTestCase()
    {
#ifdef CM_WIN32
        CCmThreadManager::CleanupOnlyOne();
#endif
    }
    virtual void SetUp()
    {
    }
    
    virtual void TearDown()
    {
    }
    
protected:
    CCmEventHandlerRepository m_EhRepository;
    static CCmThreadManager*  m_pCmThreadMgr;
};

CCmThreadManager*  CCmEventHandlerRepositoryTest::m_pCmThreadMgr = NULL;

class CCmPipeTest : public ACmEventHandler
{
public:
    CCmPipeTest()
    {
        
    }
    virtual ~CCmPipeTest()
    {
        
    }
    virtual CM_HANDLE GetHandle() const
    {
        return m_pipe.GetReadHandle();
    }
public:
    CCmPipe m_pipe;
};

TEST_F(CCmEventHandlerRepositoryTest,OpenClose){
    
    EXPECT_TRUE(CM_OK==m_EhRepository.Open());
    EXPECT_TRUE(CM_OK==m_EhRepository.Close());
        
}


TEST_F(CCmEventHandlerRepositoryTest,ASocketBindTwiceWithoutUnbind)
{
    
    ASSERT_TRUE(CM_OK==m_EhRepository.Open());
    
    CCmPipeTest pipeTest;
    ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Open());
    CM_HANDLE aFd=pipeTest.GetHandle();
    
    CCmEventHandlerRepository::CElement aEle;
    aEle.m_pEh=&pipeTest;
    aEle.m_Mask=ACmEventHandler::ACCEPT_MASK | ACmEventHandler::READ_MASK | ACmEventHandler::CONNECT_MASK;
    
    
    int outCount=3;
    while(0<outCount--)
    {
        //bind
        do
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.Bind(aFd,  aEle));
            EXPECT_TRUE(CM_OK!=m_EhRepository.Bind(aFd,  aEle));
            fd_set fsRead, fsWrite, fsException;
            const int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
            EXPECT_TRUE(nMaxFd == (int)aFd);
            bool found=false;
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(1==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(int i=0; cmHandlesCount>i;++i)
            {
                if(cmHandles[i]==aFd)
                {
                    EXPECT_TRUE(!found);
                    found=true;
                }
            }
#else
            EXPECT_TRUE(1 == fsRead.fd_count);
            EXPECT_TRUE(1 == fsWrite.fd_count);
            EXPECT_TRUE(1 == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count ; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                if (fdGet == aFd)
                {
                    EXPECT_TRUE(!found);
                    found = true;
                }
            }
#endif
        }while(0);
        
        //unbind
        do
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.UnBind(aFd));
            fd_set fsRead, fsWrite, fsException;
            const int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
            EXPECT_TRUE(nMaxFd==-1);
#ifndef CM_WIN32  
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(0==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(int i=0; cmHandlesCount>i;++i)
            {
                EXPECT_TRUE(cmHandles[i]!=aFd);
            }
#else
            EXPECT_TRUE(0 == fsRead.fd_count);
            EXPECT_TRUE(0 == fsWrite.fd_count);
            EXPECT_TRUE(0 == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                if (fdGet == aFd)
                {
                    EXPECT_TRUE(fdGet!=aFd);
                }
            }
#endif
        }while(0);

    };
    EXPECT_TRUE(CM_OK==pipeTest.m_pipe.Close());
    
    ASSERT_TRUE(CM_OK==m_EhRepository.Close());
    
}


TEST_F(CCmEventHandlerRepositoryTest,CallBindUnbindDuringLoopingCmHandles)
{
    
    ASSERT_TRUE(CM_OK==m_EhRepository.Open());
    
    CCmPipeTest pipeTest;
    ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Open());
    CM_HANDLE aFd=pipeTest.GetHandle();
    
    CCmEventHandlerRepository::CElement aEle;
    aEle.m_pEh=&pipeTest;
    aEle.m_Mask=ACmEventHandler::ACCEPT_MASK | ACmEventHandler::READ_MASK | ACmEventHandler::CONNECT_MASK;
    
    
    int outCount=3;
    while(0<outCount--)
    {
        EXPECT_TRUE(CM_OK==m_EhRepository.Bind(aFd,  aEle));
        EXPECT_TRUE(CM_OK!=m_EhRepository.Bind(aFd,  aEle));
        fd_set fsRead, fsWrite, fsException;
        int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd==(int)aFd);

        //unbind during looping cm handles
        do
        {
            bool found=false;
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(1==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(int i=0; cmHandlesCount>i;++i)
            {
                if(cmHandles[i]==aFd)
                {
                    EXPECT_TRUE(!found);
                    EXPECT_TRUE(CM_OK==m_EhRepository.UnBind(aFd));
                    found=true;
                }
            }
#else
            EXPECT_TRUE(1 == fsRead.fd_count);
            EXPECT_TRUE(1 == fsWrite.fd_count);
            EXPECT_TRUE(1 == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                if (fdGet == aFd)
                {
                    EXPECT_TRUE(!found);
                    EXPECT_TRUE(CM_OK == m_EhRepository.UnBind(aFd));
                    found = true;
                }
            }
#endif
        }while(0);
        
        //not invoke FillFdSets,so the cm handles should not be changed
        do
        {
            bool found=false;
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(1==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(int i=0; cmHandlesCount>i;++i)
            {
                if(cmHandles[i]==aFd)
                {
                    EXPECT_TRUE(!found);
                    EXPECT_TRUE(CM_OK==m_EhRepository.UnBind(aFd));
                    found=true;
                }
            }
#else
            EXPECT_TRUE(1 == fsRead.fd_count);
            EXPECT_TRUE(1 == fsWrite.fd_count);
            EXPECT_TRUE(1 == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                if (fdGet == aFd)
                {
                    EXPECT_TRUE(!found);
                    EXPECT_TRUE(CM_OK == m_EhRepository.UnBind(aFd));
                    found = true;
                }
            }
#endif
        }while(0);
        //refresh the fd set
        nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd==-1);
        //verify the result
        do
        {
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(0==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
#else
            EXPECT_TRUE(0 == fsRead.fd_count);
            EXPECT_TRUE(0 == fsWrite.fd_count);
            EXPECT_TRUE(0 == fsException.fd_count);
#endif
        }while(0);
        
    };
    EXPECT_TRUE(CM_OK==pipeTest.m_pipe.Close());
    ASSERT_TRUE(CM_OK==m_EhRepository.Close());
    
}

struct CCmReactorBaseTestStatInfo
{
    CCmReactorBaseTestStatInfo()
    {
        m_aFd=0;
        m_pipeTest=NULL;
        m_aEle=NULL;
        m_count=0;
    }
    void doCheck(CM_HANDLE aFd,CCmEventHandlerRepository::CElement& aEle,const int statCount)
    {
        ASSERT_TRUE(0<m_aFd);
        ASSERT_TRUE(NULL!=m_pipeTest);
        ASSERT_TRUE(NULL!=m_aEle);
        EXPECT_TRUE(statCount==m_count);
        
        EXPECT_TRUE(aFd==m_aFd);
        EXPECT_TRUE(!aEle.IsCleared());
        EXPECT_TRUE(aEle.m_pEh=m_aEle->m_pEh);
        
        ++m_count;
    }
    CM_HANDLE                               m_aFd;
    CCmPipeTest*                            m_pipeTest;
    CCmEventHandlerRepository::CElement*    m_aEle;
    int                                     m_count;
};


TEST_F(CCmEventHandlerRepositoryTest,LargeFdCount)
{
    
    ASSERT_TRUE(CM_OK==m_EhRepository.Open());
#ifndef CM_WIN32
    const int count = 128;
#else
    const int count=60;
#endif
    CCmPipeTest pipeTests[count];
    CCmEventHandlerRepository::CElement aEles[count];
    CM_HANDLE aFds[count];
    int index=0;
    
       //create the Event Handles
    std::map<CM_HANDLE,CCmReactorBaseTestStatInfo> theStats;
    for(index=0;count>index;++index)
    {
        CCmPipeTest& pipeTest=pipeTests[index];
        CCmEventHandlerRepository::CElement& aEle=aEles[index];
        CM_HANDLE& aFd=aFds[index];
        
        ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Open())<<",index="<<index;
        aFd=pipeTest.GetHandle();
        
        aEle.m_pEh=&pipeTest;
        aEle.m_Mask=ACmEventHandler::ACCEPT_MASK | ACmEventHandler::READ_MASK | ACmEventHandler::CONNECT_MASK;
        
        CCmReactorBaseTestStatInfo si;
        si.m_aFd=aFd;
        si.m_pipeTest=&pipeTest;
        si.m_aEle=&aEle;
        si.m_count=0;
        theStats[aFd]=si;
    }
    
    int statCount=0;
    int outCount=2;
    while(0<outCount--)
    {
        //bind
        for(index=0;count>index;++index)
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.Bind(aFds[index],  aEles[index]));
            EXPECT_TRUE(CM_OK!=m_EhRepository.Bind(aFds[index],  aEles[index]));
        }
        fd_set fsRead, fsWrite, fsException;
        int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd!=-1);
        
        //verify one by one
        do
        {
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(count==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(index=0; cmHandlesCount>index && count>index;++index)
            {
                CM_HANDLE aFd=cmHandles[index];
                CCmEventHandlerRepository::CElement aEle;
                EXPECT_TRUE(CM_OK==m_EhRepository.Find(cmHandles[index], aEle));
                theStats[aFd].doCheck(aFd, aEle,statCount);
            }
#else
            EXPECT_TRUE(count == fsRead.fd_count);
            EXPECT_TRUE(count == fsWrite.fd_count);
            EXPECT_TRUE(count == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                CCmEventHandlerRepository::CElement aEle;
                EXPECT_TRUE(CM_OK == m_EhRepository.Find(fdGet, aEle));
                theStats[fdGet].doCheck(fdGet,aEle,statCount);
            }
#endif
        }while(0);
        ++statCount;
        //unbind
        for(index=0;count>index;++index)
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.UnBind(aFds[index]));
        }

        //refresh the fd set
        nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd==-1);
        //verify the result
        do
        {
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(0==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;

#else
            EXPECT_TRUE(0 == fsRead.fd_count);
            EXPECT_TRUE(0 == fsWrite.fd_count);
            EXPECT_TRUE(0 == fsException.fd_count);
#endif
        }while(0);
        for(index=0; count>index;++index)
        {
            CCmEventHandlerRepository::CElement aEle;
            EXPECT_TRUE(CM_OK!=m_EhRepository.Find(aFds[index], aEle));
        }
        
    };
    
    for(index=0;count>index;++index)
    {
        CCmPipeTest& pipeTest=pipeTests[index];
        
        ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Close());
    }
    ASSERT_TRUE(CM_OK==m_EhRepository.Close());
    
}



TEST_F(CCmEventHandlerRepositoryTest,DisorderedLargeFdCount)
{
    
    ASSERT_TRUE(CM_OK==m_EhRepository.Open());
    
#ifndef CM_WIN32
    const int MaxHandleCount = 128;
#else
    const int MaxHandleCount=60;
#endif
    
    struct CLoopInfo
    {
        int                             m_beginIndex;
        int                             m_count;
        std::map<CM_HANDLE,CCmReactorBaseTestStatInfo>   m_theStats;
    };
#ifndef CM_WIN32
    CLoopInfo loopInfos[]={
        {  0,   MaxHandleCount},
        {  50,  70},
        {  25,  60},
    };
#else
    CLoopInfo loopInfos[] = {
        { 0,    MaxHandleCount},
        { 10,   50},
        { 25,   20},
    };
#endif

    

    CCmPipeTest pipeTests[MaxHandleCount];
    CCmEventHandlerRepository::CElement aEles[MaxHandleCount];
    CM_HANDLE aFds[MaxHandleCount];
    int index=0;
    //create the Event Handles
    
    for(index=0;MaxHandleCount>index;++index)
    {
        CCmPipeTest& pipeTest=pipeTests[index];
        CCmEventHandlerRepository::CElement& aEle=aEles[index];
        CM_HANDLE& aFd=aFds[index];
        
        ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Open())<<",index="<<index;
        aFd=pipeTest.GetHandle();
        
        aEle.m_pEh=&pipeTest;
        aEle.m_Mask=ACmEventHandler::ACCEPT_MASK | ACmEventHandler::READ_MASK | ACmEventHandler::CONNECT_MASK;
        
        CCmReactorBaseTestStatInfo si;
        si.m_aFd=aFd;
        si.m_pipeTest=&pipeTest;
        si.m_aEle=&aEle;
        si.m_count=0;
        for(int loopIndex=0;sizeof(loopInfos)/sizeof(loopInfos[0])>loopIndex;++loopIndex)
        {
            CLoopInfo& loopInfo=loopInfos[loopIndex];
            if(loopInfo.m_beginIndex<=index &&loopInfo.m_beginIndex+loopInfo.m_count>index)
                loopInfo.m_theStats[aFd]=si;
        }
    }
    
    const int loopCount=sizeof(loopInfos)/sizeof(loopInfos[0]);
    for(int loopIndex=0; loopCount>loopIndex;++loopIndex)
    {
        CLoopInfo& loopInfo=loopInfos[loopIndex];
        
        std::map<CM_HANDLE,CCmReactorBaseTestStatInfo>& theStats=loopInfo.m_theStats;
        int statCount=0;
        
        ASSERT_TRUE(0<=loopInfo.m_beginIndex && MaxHandleCount>loopInfo.m_beginIndex);
        ASSERT_TRUE(0<=loopInfo.m_count);
#ifndef CM_WIN32
        ASSERT_TRUE(loopInfo.m_count+loopInfo.m_beginIndex<=m_EhRepository.GetMaxHandlers())
            <<",beginIndex="<<loopInfo.m_beginIndex<<",count="<<loopInfo.m_count
        <<",maxHandlers="<<m_EhRepository.GetMaxHandlers();
#endif
        ASSERT_TRUE(MaxHandleCount>=loopInfo.m_beginIndex+loopInfo.m_count);
        //bind
        int offset=0;
        for(offset=0;loopInfo.m_count>offset;++offset)
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.Bind(aFds[loopInfo.m_beginIndex+offset],  aEles[loopInfo.m_beginIndex+offset]));
            EXPECT_TRUE(CM_OK!=m_EhRepository.Bind(aFds[loopInfo.m_beginIndex+offset],  aEles[loopInfo.m_beginIndex+offset]));
        }
        fd_set fsRead, fsWrite, fsException;
        int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd!=-1);
        
        //verify one by one
        do
        {
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(loopInfo.m_count==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
            for(offset=0; cmHandlesCount>offset; ++offset)
            {
                CM_HANDLE aFd=cmHandles[offset];
                CCmEventHandlerRepository::CElement aEle;
                EXPECT_TRUE(CM_OK==m_EhRepository.Find(cmHandles[offset], aEle));
                ASSERT_TRUE(!aEle.IsCleared());
                theStats[aFd].doCheck(aFd,aEle,statCount);
            }
#else
            EXPECT_TRUE(loopInfo.m_count == fsRead.fd_count);
            EXPECT_TRUE(loopInfo.m_count == fsWrite.fd_count);
            EXPECT_TRUE(loopInfo.m_count == fsException.fd_count);
            fd_set aFdSet = fsRead;
            for (unsigned i = 0; i < aFdSet.fd_count; ++i) {
                CM_HANDLE fdGet = (CM_HANDLE)aFdSet.fd_array[i];
                CCmEventHandlerRepository::CElement aEle;
                EXPECT_TRUE(CM_OK == m_EhRepository.Find(fdGet, aEle));
                theStats[fdGet].doCheck(fdGet, aEle, statCount);
            }
#endif
        }while(0);

        //unbind
        for(offset=0;loopInfo.m_count>offset;++offset)
        {
            EXPECT_TRUE(CM_OK==m_EhRepository.UnBind(aFds[loopInfo.m_beginIndex+offset]));
        }
        
        //refresh the fd set
        nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
        EXPECT_TRUE(nMaxFd==-1);
        //verify the result
        do
        {
#ifndef CM_WIN32
            CM_HANDLE* cmHandles=m_EhRepository.GetCmHandles();
            ASSERT_TRUE(NULL!=cmHandles);
            const int cmHandlesCount=m_EhRepository.GetCmHandlesCount();
            EXPECT_TRUE(0==cmHandlesCount)<<"cmHandlesCount="<<cmHandlesCount;
#else
            EXPECT_TRUE(0 == fsRead.fd_count);
            EXPECT_TRUE(0 == fsWrite.fd_count);
            EXPECT_TRUE(0 == fsException.fd_count);
#endif
        }while(0);
        for(offset=0; loopInfo.m_count>offset;++offset)
        {
            CCmEventHandlerRepository::CElement aEle;
            EXPECT_TRUE(CM_OK!=m_EhRepository.Find(aFds[loopInfo.m_beginIndex+offset], aEle));
        }
        
    };
    
    for(index=0;MaxHandleCount>index;++index)
    {
        CCmPipeTest& pipeTest=pipeTests[index];
        
        ASSERT_TRUE(CM_OK==pipeTest.m_pipe.Close());
    }
    ASSERT_TRUE(CM_OK==m_EhRepository.Close());
    
}

class CLastEvent : public ICmEvent {
public:
    virtual CmResult OnEventFire() {
        CM_INFO_TRACE("CLastEvent::OnEventFire");
        return 0;
    }
};

class CCleanupEvent : public ICmEvent {
public:
    virtual CmResult OnEventFire() {
        CLastEvent *pLastEvent = new CLastEvent;
        ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
        pMain->GetEventQueue()->PostEvent(pLastEvent);
        CCmThreadManager::Instance()->CleanupOnlyOne();
        return 0;
    }

};

class CInQueueEvent : public ICmEvent {
public:
    CInQueueEvent(ACmThread *pMain) : m_pMain(pMain) {
        
    }
    virtual CmResult OnEventFire() {
        ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
        EXPECT_TRUE(m_pMain == pMain);
        return 0;
    }
private:
    ACmThread *m_pMain;
};

TEST(ReacotrTest, TestCleanupInEvents1) {
    CCmThreadManager::Instance();
    ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    CCleanupEvent *pCleanup = new CCleanupEvent;
    CInQueueEvent *pInQueue = new CInQueueEvent(pMain);
    
    pMain->GetEventQueue()->PostEvent(pCleanup);
    pMain->GetEventQueue()->PostEvent(pInQueue);

    SleepMsWithLoop(10);
}

TEST(ReacotrTest, TestCleanupInNetThread) {
    CCmThreadManager::Instance();
    ACmThread *pNet = CCmThreadManager::Instance()->GetThread(TT_NETWORK);
    
    auto fnClean = [] ()-> CmResult {
        auto fnUninit = [] () -> CmResult {
            CCmThreadManager::Instance()->CleanupOnlyOne();
            return 0;
        };
        ICmEvent *pEventUninit = new CCmInvokeEvent<decltype(fnUninit)>(fnUninit);
        ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
        pMain->GetEventQueue()->PostEvent(pEventUninit);
        
        CCmThreadManager::EnsureThreadManagerExited();
        
        EXPECT_TRUE(true);
        return 0;
    };
    
    ICmEvent *pEventClean = new CCmInvokeEvent<decltype(fnClean)>(fnClean);
    pNet->GetEventQueue()->PostEvent(pEventClean);

    SleepMsWithLoop(50);
}

TEST(ReacotrTest, TestCleanupInUserThread) {
    CCmThreadManager::Instance();
    ACmThread *pUser = nullptr;
    CreateUserTaskThread("t-cleanup-user", pUser);
    
    auto fnClean = [] ()-> CmResult {
        auto fnUninit = [] () -> CmResult {
            CCmThreadManager::Instance()->CleanupOnlyOne();
            return 0;
        };
        ICmEvent *pEventUninit = new CCmInvokeEvent<decltype(fnUninit)>(fnUninit);
        ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
        pMain->GetEventQueue()->PostEvent(pEventUninit);
        
        CCmThreadManager::EnsureThreadManagerExited();
        
        EXPECT_TRUE(true);
        return 0;
    };
    
    ICmEvent *pEventClean = new CCmInvokeEvent<decltype(fnClean)>(fnClean);
    pUser->GetEventQueue()->PostEvent(pEventClean);
    
    SleepMsWithLoop(50);
    pUser->Stop();
    pUser->Join();
}

