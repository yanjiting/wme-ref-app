#include "gtest/gtest.h"

#include <thread>
#include <mutex>              
#include <condition_variable> 
#include <atomic>
#include "timer.h"
#include <list>
#include <sstream>
#include <random>

#ifdef CM_ANDROID
    #include <android/log.h>
#endif

//#define CmPerformanceTest_Enable 1

#ifdef CmPerformanceTest_Enable
    #define CmPerformanceTestT CmPerformanceTest
#else
    #define CmPerformanceTestT DISABLED_CmPerformanceTest
#endif

class RunningTimeTotal{
public:
    RunningTimeTotal(uint64_t * pTimeTotal){
        m_pTimeTotal = pTimeTotal;
        m_nStart = ticker::now();
        //std::cout << "start=" << m_nStart << std::endl;
    }
    ~RunningTimeTotal(){
        uint64_t end = ticker::now();
        //std::cout << "  end=" << end << std::endl;
        uint64_t usage = end - m_nStart;
        if(m_pTimeTotal)
            *m_pTimeTotal = usage;
    }
protected:
    uint64_t * m_pTimeTotal;
    uint64_t m_nStart;
};

class CmPerformanceTestT : public testing::Test
{
public:
    virtual void SetUp()
    {
        m_buff = NULL;
        Clean();
        m_buff = new char [1024];
    }

    virtual void TearDown()
    {
        Clean();
    }

    void waitStartRace(){
        std::unique_lock<std::mutex> lck(mtx);
        while (!bStart) cv.wait(lck);
    }
    void StartRace(){
        std::unique_lock<std::mutex> lck(mtx);
        bStart = true;
        cv.notify_all();
    }
    void ResetRace(){
        std::unique_lock<std::mutex> lck(mtx);
        bStart = false;
    }
    
    
    char * GetBuff(){
        std::unique_lock<std::mutex> lck(mtx);
        return m_buff;
    }
    void ReleaseBuff(char *pBuff){
        std::unique_lock<std::mutex> lck(mtx);
        m_buff = pBuff;
    }
    
protected:
    void Clean()
    {
        if(m_buff)
            delete []m_buff;
        m_buff = NULL;
    }
    
protected:
    std::mutex mtx;
    std::condition_variable_any cv;
    bool bStart;
    char * m_buff;
};
void testFun(int &i){
    if(i<=0) return;
    i--;
    //std::cout << "i=" << i << std::endl;
    return testFun(i);
}
void testFun3(int &i,int p1, int p2){
    if(i<=0) return;
    i--;
    //std::cout << "i=" << i << std::endl;
    return testFun3(i,p1,p2);
}
TEST_F(CmPerformanceTestT, testStackFramesPerformanceCompare)
{
    int i = 0;
    int nLoop = 10240000;
    
    uint64_t start = 0;
    uint64_t end = 0;
    timer::now();
    
    uint64_t m_nTime_StackFrame_8 = 0;
    start = timer::now();
    {
        i = 8;
        //RunningTimeTotal runningTimeTotal8(&m_nTime_StackFrame_8);
        for(int j=0;j<nLoop;j++){
            testFun(i);
            ASSERT_TRUE(i==0);
        }
    }
    end = timer::now();
    m_nTime_StackFrame_8 = end - start;
    
    uint64_t m_nTime_StackFrame_16 = 0;
    start = timer::now();
    {
        i = 16;
        //RunningTimeTotal runningTimeTotal16(&m_nTime_StackFrame_16);
        for(int j=0;j<nLoop;j++){
            testFun(i);
            ASSERT_TRUE(i==0);
        }

    }
    end = timer::now();
    m_nTime_StackFrame_16 = end - start;
    
    uint64_t m_nTime_StackFrame_32 = 0;
    start = timer::now();
    {
        i = 32;
        //RunningTimeTotal runningTimeTotal32(&m_nTime_StackFrame_32);
        for(int j=0;j<nLoop;j++){
            testFun(i);
            ASSERT_TRUE(i==0);
        }
    }
    end = timer::now();
    m_nTime_StackFrame_32 = end - start;
    
    uint64_t m_nTime_StackFrame_64 = 0;
    start = timer::now();
    {
        i = 64;
        //RunningTimeTotal runningTimeTotal64(&m_nTime_StackFrame_64);
        for(int j=0;j<nLoop;j++){
            testFun(i);
            ASSERT_TRUE(i==0);
        }

    }
    end = timer::now();
    m_nTime_StackFrame_64 = end - start;
    
    
    uint64_t m_nTime_StackFrame_64_3param = 0;
    start = timer::now();
    {
        i = 64;
        //RunningTimeTotal runningTimeTotal643(&m_nTime_StackFrame_64_3param);
        for(int j=0;j<nLoop;j++){
            testFun3(i,0,0);
            ASSERT_TRUE(i==0);
        }
    }
    end = timer::now();
    m_nTime_StackFrame_64_3param = end - start;
    
    std::stringstream ss;
    ss<<" run loop count:         "<< std::setw(10)<<  nLoop <<std::endl;
    ss<<" m_nTime_StackFrame_8:         "<< std::setw(10)<<  m_nTime_StackFrame_8 <<std::endl;
    ss<<" m_nTime_StackFrame_16:        "<< std::setw(10)<<  m_nTime_StackFrame_16 <<std::endl;
    ss<<" m_nTime_StackFrame_32:        "<< std::setw(10)<<  m_nTime_StackFrame_32 <<std::endl;
    ss<<" m_nTime_StackFrame_64:        "<< std::setw(10)<<  m_nTime_StackFrame_64 <<std::endl;
    ss<<" m_nTime_StackFrame_64_3param: "<< std::setw(10)<<  m_nTime_StackFrame_64_3param <<std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CmLockfreeTest","%s",ss.str().c_str());
#else
    std::cout << ss.str();
#endif

}

TEST_F(CmPerformanceTestT, testNewDeletePerformanceCompare)
{
    int i = 0;
    int nLoop = 10240000;
    
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(8,10240);
    int buff_len = distribution(generator);  // generates number in the range 1..6
    
    uint64_t m_nTime_NewDel_Random = 0;
    {
        RunningTimeTotal runningTimeTotal32(&m_nTime_NewDel_Random);
        for(i=0; i<nLoop; i++){
            char * pBuff = NULL;
            pBuff = new char[buff_len];
            if(pBuff)
                delete [] pBuff;
            pBuff = NULL;
        }
    }
    
    uint64_t m_nTime_NewDel_32 = 0;
    {
        RunningTimeTotal runningTimeTotal32(&m_nTime_NewDel_32);
        for(i=0; i<nLoop; i++){
            char * pBuff = NULL;
            pBuff = new char[32];
            if(pBuff)
                delete [] pBuff;
            pBuff = NULL;
        }
    }

    
    uint64_t m_nTime_NewDel_1K = 0;
    {
        RunningTimeTotal runningTimeTotal32(&m_nTime_NewDel_1K);
        for(i=0; i<nLoop; i++){
            char * pBuff = NULL;
            pBuff = new char[1024];
            if(pBuff)
                delete [] pBuff;
            pBuff = NULL;
        }
    }

    uint64_t m_nTime_Cache = 0;
    {
        RunningTimeTotal runningTimeTotal32(&m_nTime_Cache);
        for(i=0; i<nLoop; i++){
            char * pBuff = GetBuff();
            if(pBuff)
                ReleaseBuff(pBuff);
            pBuff = NULL;
        }
    }
    
    
    std::stringstream ss;
    ss<<" run loop count:           "<< std::setw(10)<<  nLoop <<std::endl;
    ss<<" m_nTime_Cache:            "<< std::setw(10)<<  m_nTime_Cache <<std::endl;
    ss<<" m_nTime_NewDel_Random:    "<< std::setw(10)<<  m_nTime_NewDel_Random <<std::endl;
    ss<<" m_nTime_NewDel_32:        "<< std::setw(10)<<  m_nTime_NewDel_32 <<std::endl;
    ss<<" m_nTime_NewDel_1K:        "<< std::setw(10)<<  m_nTime_NewDel_1K <<std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CmLockfreeTest","%s",ss.str().c_str());
#else
    std::cout << ss.str();
#endif
    
}

