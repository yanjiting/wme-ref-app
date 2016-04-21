
#include "gtest/gtest.h"
#include "CmMessageBlock.h"

#include "CmUtil.h"
#include "CmLockFree.h"
#include <thread>
#include <mutex>              
#include <condition_variable> 
#include <atomic>
#include "timer.h"
#include <list>
#include <sstream>
#include "CmDebug.h"
#ifdef CM_ANDROID
    #include <android/log.h>
#endif

#ifdef CM_IOS
#define UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
#endif

class Item {
public:
    Item(int nParam){
        m_nPram = nParam;
    }
    int m_nPram;
};
#define LOCKFREE_QUEUE_SIZE_MAX LOCKFREE_QUEUE_SIZE_10K
#define LOCKFREE_QUEUE_NO_LIMIT_TEST_ITEMS 0x1ffff
class CmLockfreeTest : public testing::Test
{
public:
    CmLockfreeTest()
    {
        bStart = false;
        nProduceCount.store(0);
        nConsumerCount.store(0);
        nPlanPushedTotalCount = 0;
        nPushedCountByOneThread = LOCKFREE_QUEUE_SIZE_MAX;
        bTestNoLimit = false;
    }

    virtual ~CmLockfreeTest()
    {
        Clean();
    }

    virtual void SetUp()
    {
        Clean();
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
    
    void produceSpScFun(){
        waitStartRace();
        int i = 0;
        for (i =0;i<nPushedCountByOneThread;i++){
            Item * pItem = new Item(i);
            bool ret = false;
            if(bTestNoLimit){
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
                ret = m_queueSpScTestNoLimit.push_back(pItem);
#endif
            }
            else
                ret = m_queueSpScTest.push_back(pItem);
            
            if(ret)
                nProduceCount++;
            else
                ret = false;
            
            ASSERT_TRUE(ret);
            
            if((i%100)==0)
                std::this_thread::yield();
        }
    }
    void consumerSpScFun(){
        waitStartRace();
        
        int i = 0;
        int iValue = 0;
        Item * pItem = NULL;
        for (i =0;i<nPushedCountByOneThread*100;i++){
            if(iValue>nPushedCountByOneThread)
                break;
            
            bool ret = false;
            if(bTestNoLimit){
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
                ret = m_queueSpScTestNoLimit.pop(pItem);
#endif
            }
            else
                ret = m_queueSpScTest.pop(pItem);
            
            if(!ret) continue;
            ASSERT_TRUE(pItem->m_nPram==iValue);
            iValue++;
            delete pItem;
            pItem = NULL;
            nConsumerCount++;
            
            if((i%100)==0)
                std::this_thread::yield();
        }
    }
    
    void produceFun(){
        waitStartRace();
        
        int i = 0;
        for (i =0;i<nPushedCountByOneThread;i++){
            if((i%10)==0)
                std::this_thread::yield();
            
            Item * pItem = new Item(i);
            bool ret = false;
            if(bTestNoLimit){
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
                ret = m_queueTestNoLimit.push_back(pItem);
#endif
            }
            else
                ret = m_queueTest.push_back(pItem);
            
            if(ret)
                 nProduceCount++;
            ASSERT_TRUE(ret);
        }
    }
    void consumerFun(){
        waitStartRace();
        
        int i = 0;
        Item * pItem = NULL;
        for (i =0;i<nPushedCountByOneThread*100;i++){
            if((i%10)==0)
                std::this_thread::yield();

            if(nConsumerCount.load()>nPlanPushedTotalCount)
                break;
            
            
            bool ret = false;
            if(bTestNoLimit){
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
                ret = m_queueTestNoLimit.pop(pItem);
#endif
            }
            else
                ret = m_queueTest.pop(pItem);
            
            if(!ret) continue;
            
            nConsumerCount++;
            delete pItem;
            pItem = NULL;
        }
    }
    
protected:
    void Clean()
    {
        {
            Item* pItem = NULL;
            while(m_queueTest.pop(pItem)){
                delete pItem;
                pItem = NULL;
            }
        }
        {
            Item* pItem = NULL;
            while(m_queueSpScTest.pop(pItem)){
                delete pItem;
                pItem = NULL;
            }
        }
        bTestNoLimit = false;
    }

protected:
    CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_MAX> m_queueSpScTest;
    CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_MAX> m_queueTest;
    
    
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
    CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_10K> m_queueSpScTestNoLimit;
    CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT> m_queueTestNoLimit;
#endif
    
    std::mutex mtx;
    std::condition_variable_any cv;
    bool bStart;
    
    bool bTestNoLimit;
    
    int nPlanPushedTotalCount;
    int nPushedCountByOneThread;
    std::atomic<int> nProduceCount;
    std::atomic<int> nConsumerCount;
};

//======================================================================
// Queue common function


//======================================================================
//SPSC Queue >>>
TEST_F(CmLockfreeTest, testSpScQueueCommonFunction_DefaultSize)
{
    int nQueueSize = LOCKFREE_QUEUE_SIZE_DEFALT;
    CmLockFreeQueueSpScT<Item*> queueSpScTest;
    int i = 0;
    for (i =0;i<nQueueSize;i++){
        Item * pItem = new Item(i);
        ASSERT_TRUE(queueSpScTest.push_back(pItem));
    }
    Item item(i);
    ASSERT_TRUE(!queueSpScTest.push_back(&item));
    
    Item * pItem = NULL;
    for (i =0;i<nQueueSize;i++){
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        ASSERT_TRUE(pItem!=NULL);
        ASSERT_TRUE(pItem->m_nPram==i);
        delete pItem;
        pItem = NULL;
    }
    ASSERT_TRUE(!queueSpScTest.pop(pItem));
    
    Item item2(i);
    ASSERT_TRUE(queueSpScTest.push_back(&item2));
    pItem = NULL;
    ASSERT_TRUE(queueSpScTest.pop(pItem));
    if(pItem!=NULL){
        ASSERT_TRUE(pItem->m_nPram==i);
    }
}

TEST_F(CmLockfreeTest, testSpScQueueCommonFunction_SIZE_ALL)
{
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_8>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_8;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_64>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_64;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_128>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_128;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_512>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_512;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_1K>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_1K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_2K>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_2K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_10K>
        queueSpScTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_10K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueSpScTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueSpScT<Item*,LOCKFREE_QUEUE_SIZE_10K>   queueSpScTest;
        int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_10K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueSpScTest.push_back(pItem));
        }
        ASSERT_TRUE(queueSpScTest.size()==nQueueSize);
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueSpScTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueSpScTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueSpScTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
}
//======================================================================
//Queue >>

TEST_F(CmLockfreeTest, testQueueCommonFunction)
{
    int nQueueSize = LOCKFREE_QUEUE_SIZE_DEFALT;
    CmLockFreeQueueT<Item*> queueSpScTest;
    int i = 0;
    for (i =0;i<nQueueSize;i++){
        Item * pItem = new Item(i);
        ASSERT_TRUE(queueSpScTest.push_back(pItem));
    }
    Item item(i);
    ASSERT_TRUE(!queueSpScTest.push_back(&item));
    
    Item * pItem = NULL;
    for (i =0;i<nQueueSize;i++){
        ASSERT_TRUE(queueSpScTest.pop(pItem));
        ASSERT_TRUE(pItem!=NULL);
        ASSERT_TRUE(pItem->m_nPram==i);
        delete pItem;
        pItem = NULL;
    }
    ASSERT_TRUE(!queueSpScTest.pop(pItem));
    
    Item item2(i);
    ASSERT_TRUE(queueSpScTest.push_back(&item2));
    pItem = NULL;
    ASSERT_TRUE(queueSpScTest.pop(pItem));
    if(pItem!=NULL){
        ASSERT_TRUE(pItem->m_nPram==i);
    }
}
TEST_F(CmLockfreeTest, testQueueCommonFunction_ALLSIZE)
{
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_8>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_8;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_64>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_64;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_128>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_128;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_512>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_512;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_1K>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_1K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_2K>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_2K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }

    
    
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_10K>
        queueTest;int nQueueSize = (int)LOCKFREE_QUEUE_SIZE_10K;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        Item item(i);
        ASSERT_TRUE(!queueTest.push_back(&item));
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
    
    
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT>   queueTest;
        int nQueueSize = (int)LOCKFREE_QUEUE_NO_LIMIT_TEST_ITEMS;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
        Item item2(i);
        ASSERT_TRUE(queueTest.push_back(&item2));
        pItem = NULL;
        ASSERT_TRUE(queueTest.pop(pItem));
        if(pItem!=NULL){
            ASSERT_TRUE(pItem->m_nPram==i);
        }
    }
#endif
    
}
TEST_F(CmLockfreeTest, testQueueCommonFunction_PerformanceCompare)
{
    int i =0;
    int nTimes = 100;
    const LockFreeQueueSize eFreeQueueSize = LOCKFREE_QUEUE_SIZE_10K;
    int nQueueSize = (int) eFreeQueueSize;
    
    uint64_t nNoLimitQueueUseTime = 0;
    uint64_t nQueueUseTime = 0;
    uint64_t nSpScQueueUseTime = 0;
    uint64_t nStdListQueueUseTimeWithLock = 0;
    uint64_t nStdListQueueUseTime = 0;
    
    uint64_t start = 0;
    uint64_t end = 0 ;
    
    start=ticker::now();
    std::mutex mtx;
    for(i=0;i<nTimes;i++)
    {
        int nCount = 0;
        std::list<Item*> queueTest;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            std::lock_guard<std::mutex> lck (mtx);
            Item * pItem = new Item(i);
            queueTest.push_back(pItem);
            //nCount ++;
            //ASSERT_TRUE(queueTest.size()==nCount);
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            std::lock_guard<std::mutex> lck (mtx);
            pItem = queueTest.front();
            queueTest.pop_front();
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
            //nCount--;
            //ASSERT_TRUE(queueTest.size()==nCount);
		}
		ASSERT_TRUE(queueTest.empty());
    }
    end=ticker::now();
    nStdListQueueUseTimeWithLock = end - start;
    
    start=ticker::now();
    for(i=0;i<nTimes;i++)
    {
        int nCount = 0;
        std::list<Item*> queueTest;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            //std::lock_guard<std::mutex> lck (mtx);
            Item * pItem = new Item(i);
            queueTest.push_back(pItem);
            //nCount ++;
            //ASSERT_TRUE(queueTest.size()==nCount);
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            //std::lock_guard<std::mutex> lck (mtx);
            pItem = queueTest.front();
            queueTest.pop_front();
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
            //nCount--;
            //ASSERT_TRUE(queueTest.size()==nCount);
		}
        ASSERT_TRUE(queueTest.empty());
    }
    end=ticker::now();
    nStdListQueueUseTime = end - start;
    
    start=ticker::now();
    
#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
    for(i=0;i<nTimes;i++)
    {
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT>   queueTest;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
    }
#endif
    end=ticker::now();
    nNoLimitQueueUseTime = end - start;
    
    start=ticker::now();
    for(i=0;i<nTimes;i++)
    {
        CmLockFreeQueueT<Item*,eFreeQueueSize>   queueTest;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
    }
    end=ticker::now();
    nQueueUseTime = end - start;
    
    start=ticker::now();
    for(i=0;i<nTimes;i++)
    {
        CmLockFreeQueueSpScT<Item*,eFreeQueueSize>   queueTest;
        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            pItem = NULL;
        }
        ASSERT_TRUE(!queueTest.pop(pItem));
        
    }
    end=ticker::now();
    nSpScQueueUseTime = end - start;
    /*
    ASSERT_TRUE(nStdListQueueUseTime<=nStdListQueueUseTimeWithLock);
    ASSERT_TRUE(nNoLimitQueueUseTime<=nStdListQueueUseTimeWithLock);
    ASSERT_TRUE(nQueueUseTime<=nStdListQueueUseTimeWithLock);
    ASSERT_TRUE(nSpScQueueUseTime<=nStdListQueueUseTimeWithLock);
    
    
    ASSERT_TRUE(nSpScQueueUseTime<=nQueueUseTime);
    */
    
    std::stringstream stream;
    stream << "  push/pop count  = " << nQueueSize*nTimes << std::endl
    << "    nStdListQueueUseTimeWithLock = " << nStdListQueueUseTimeWithLock << std::endl
    << "    nStdListQueueUseTime        = " << nStdListQueueUseTime << std::endl
    << "    nNoLimitQueueUseTime        = " << nNoLimitQueueUseTime << std::endl
    << "    nQueueUseTime       = " << nQueueUseTime  << std::endl
    << "    nSpScQueueUseTime       = " << nSpScQueueUseTime << std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CmLockfreeTest","%s",stream.str().c_str());
#else
    std::cout << stream.str();
#endif
    CM_INFO_TRACE(stream.str());
    /*
     Mac: https://wme-jenkins.cisco.com:8443/job/WME-GIT-UT-MAC64/2374/consoleFull
     push/pop count                 = 1024000
     nStdListQueueUseTimeWithLock   = 450000
     nStdListQueueUseTime           = 300000
     nNoLimitQueueUseTime           = 370000
     nQueueUseTime                  = 260000
     nSpScQueueUseTime              = 200000
     
     Windows: https://wme-jenkins.cisco.com:8443/job/WME-GIT-UT-WIN/2460/consoleFull
     push/pop count                 = 1024000
     nStdListQueueUseTimeWithLock   = 547000
     nStdListQueueUseTime           = 438000
     nNoLimitQueueUseTime           = 343000
     nQueueUseTime                  = 235000
     nSpScQueueUseTime              = 140000
     
     iOS(iPhone 5s):
     [ RUN      ] CmLockfreeTest.testQueueCommonFunction_PerformanceCompare
     push/pop count                 = 1024000
     nStdListQueueUseTimeWithLock   = 900000
     nStdListQueueUseTime           = 550000
     nNoLimitQueueUseTime           = 1090000
     nQueueUseTime                  = 740000
     nSpScQueueUseTime              = 480000
     
     Android(MI2):
     I/CmLockfreeTest(  624):       push/pop count              = 1024000
     I/CmLockfreeTest(  624):     nStdListQueueUseTimeWithLock  = 2080000
     I/CmLockfreeTest(  624):     nStdListQueueUseTime          = 1400000
     I/CmLockfreeTest(  624):     nNoLimitQueueUseTime          = 3880000
     I/CmLockfreeTest(  624):     nQueueUseTime                 = 3660000
     I/CmLockfreeTest(  624):     nSpScQueueUseTime             = 1820000
     
     Android(N5):
     I/CmLockfreeTest(21460):   push/pop count                  = 1024000
     I/CmLockfreeTest(21460):     nStdListQueueUseTimeWithLock  = 1360000
     I/CmLockfreeTest(21460):     nStdListQueueUseTime          = 1000000
     I/CmLockfreeTest(21460):     nNoLimitQueueUseTime          = 3180000
     I/CmLockfreeTest(21460):     nQueueUseTime                 = 2420000
     I/CmLockfreeTest(21460):     nSpScQueueUseTime             = 1170000
     
     Windows Phone 8.1 (Nokia):
     push/pop count  = 1024000
     nStdListQueueUseTimeWithLock   = 4266000
     nStdListQueueUseTime           = 1593000
     nNoLimitQueueUseTime           = 3875000
     nQueueUseTime                  = 2766000
     nSpScQueueUseTime              = 1344000
     */
}
//======================================================================
// Queue thread contention
//======================================================================
const int nPerformanceQueueSize =  10240;
std::atomic<int> nPerformanceQueueSizeCtrlProduce;
std::atomic<int> nPerformanceQueueSizeCtrlConsume;
std::atomic<int> nPerformanceQueueSizeCurProduce;
std::atomic<int> nPerformanceQueueSizeCurConsume;
std::atomic<int> nPerformanceQueueSizePushIncorrect;
typedef CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_10K> PERFORMANCE_TEST_LOCKFREE_QUEUE;
typedef std::list<Item*> PERFORMANCE_TEST_LIST;
std::mutex mtx_list;

void produceStdList(PERFORMANCE_TEST_LIST & list,CmLockfreeTest * pCmLockfreeTest){
    if(pCmLockfreeTest==NULL) return;
    pCmLockfreeTest->waitStartRace();
    
    for(int i=0;i<nPerformanceQueueSize*100;i++){
        std::lock_guard<std::mutex> lck (mtx_list);
        
        if(nPerformanceQueueSizeCurProduce.load()>=nPerformanceQueueSize)
            break;
        
        Item * pItem = new Item(i);
        list.push_back(pItem);
        nPerformanceQueueSizeCurProduce++;
    }
    
}
void consumerStdList(PERFORMANCE_TEST_LIST & list,CmLockfreeTest * pCmLockfreeTest){
    if(pCmLockfreeTest==NULL) return;
    pCmLockfreeTest->waitStartRace();
    
    for(int i=0;i<nPerformanceQueueSize*100;i++){
        std::lock_guard<std::mutex> lck (mtx_list);
        
        if(nPerformanceQueueSizeCurConsume.load()>=nPerformanceQueueSize)
            break;
        
        if(list.size()==0) {
            std::this_thread::yield();
            continue;
        }
        Item * pItem = NULL;
        pItem = list.front();
        if(pItem)
            delete pItem;
        pItem = NULL;
        list.pop_front();
        nPerformanceQueueSizeCurConsume++;
    }

}
void produceLockfree(PERFORMANCE_TEST_LOCKFREE_QUEUE & queue,CmLockfreeTest * pCmLockfreeTest){
    if(pCmLockfreeTest==NULL) return;
    pCmLockfreeTest->waitStartRace();
    
    for(int i=0;i<nPerformanceQueueSize*100;i++){
        if(nPerformanceQueueSizeCtrlProduce.load()>=nPerformanceQueueSize)
            break;
        int nPrevious=nPerformanceQueueSizeCtrlProduce.fetch_add(1);
        if(nPrevious>=nPerformanceQueueSize)
            break;
        Item * pItem = new Item(i);
        if(queue.push_back(pItem))
            nPerformanceQueueSizeCurProduce++;
        else
            nPerformanceQueueSizePushIncorrect++;
        //nPerformanceQueueSizeCtrlProduce++;
    }

}
void consumerLockfree(PERFORMANCE_TEST_LOCKFREE_QUEUE & queue,CmLockfreeTest * pCmLockfreeTest){
    if(pCmLockfreeTest==NULL) return;
    pCmLockfreeTest->waitStartRace();
    
    for(int i=0;i<nPerformanceQueueSize*100;i++){
        if(nPerformanceQueueSizeCtrlConsume.load()>=nPerformanceQueueSize)
            break;
        Item * pItem = NULL;
        queue.pop(pItem);
        if(!pItem){ //just checking point,
            std::this_thread::yield();
            continue;
        }
        if(pItem){
            delete pItem;
        }
        pItem = NULL;
        
        nPerformanceQueueSizeCurConsume++;
        nPerformanceQueueSizeCtrlConsume++;
}


}
TEST_F(CmLockfreeTest, testQueueCommonFunction_MultiThreadPerformanceCompare)
{
    const int nProduceThreadCount = 2;
    const int nConsumerThreadCount = 2;
    
    uint64_t nLockUseTime = 0;
    uint64_t nLockFreeUseTime = 0;
    uint64_t start = 0;
    uint64_t end = 0;
    
    nPerformanceQueueSizeCtrlProduce=0;
    nPerformanceQueueSizeCtrlConsume=0;
    nPerformanceQueueSizeCurProduce = 0;
    nPerformanceQueueSizeCurConsume = 0;
    nPerformanceQueueSizePushIncorrect=0;
    ResetRace();
    start=ticker::now();
    {
        PERFORMANCE_TEST_LOCKFREE_QUEUE list;
        std::thread threads_prod[nProduceThreadCount];
        for(int i =0;i<nProduceThreadCount;i++){
            threads_prod[i] = std::thread(produceLockfree, std::ref(list),this);
        }
        
        std::thread threads_cust[nConsumerThreadCount];
        for(int i =0;i<nConsumerThreadCount;i++){
            threads_cust[i] = std::thread(consumerLockfree, std::ref(list), this);
        }
        
        StartRace();
        
        //
        for(int i =0;i<nProduceThreadCount;i++){
            threads_prod[i].join();
        }
        for(int i =0;i<nConsumerThreadCount;i++){
            threads_cust[i].join();
        }
        
        ASSERT_TRUE( (nPerformanceQueueSizeCurProduce>=nPerformanceQueueSize) &&
                    (nPerformanceQueueSizeCurProduce <= (nPerformanceQueueSize + nProduceThreadCount -1)) )
            <<",nPerformanceQueueSizeCurConsume="<<nPerformanceQueueSizeCurConsume
            <<",nPerformanceQueueSizeCurProduce="<<nPerformanceQueueSizeCurProduce
            <<",list.size()="<<list.size()
            <<",nPerformanceQueueSizePushIncorrect="<<nPerformanceQueueSizePushIncorrect
        ;
        ASSERT_TRUE(nPerformanceQueueSizeCurConsume==nPerformanceQueueSizeCurProduce)
            <<",nPerformanceQueueSizeCurConsume="<<nPerformanceQueueSizeCurConsume
            <<",nPerformanceQueueSizeCurProduce="<<nPerformanceQueueSizeCurProduce
            <<",list.size()="<<list.size()
            <<",nPerformanceQueueSizePushIncorrect="<<nPerformanceQueueSizePushIncorrect
        ;
        ASSERT_TRUE(list.size()==0)
            <<",list.size()="<<list.size()
            <<",nPerformanceQueueSizePushIncorrect="<<nPerformanceQueueSizePushIncorrect
        ;
    }
    end=ticker::now();
    nLockFreeUseTime = end - start;
    
    nPerformanceQueueSizeCtrlProduce=0;
    nPerformanceQueueSizeCtrlConsume=0;
    nPerformanceQueueSizeCurProduce = 0;
    nPerformanceQueueSizeCurConsume = 0;
    nPerformanceQueueSizePushIncorrect=0;
    ResetRace();
    start=ticker::now();
    {
        PERFORMANCE_TEST_LIST list;
        std::thread threads_prod[nProduceThreadCount];
        for(int i =0;i<nProduceThreadCount;i++){
            threads_prod[i] = std::thread(produceStdList, std::ref(list),this);
        }
        
        std::thread threads_cust[nConsumerThreadCount];
        for(int i =0;i<nConsumerThreadCount;i++){
            threads_cust[i] = std::thread(consumerStdList, std::ref(list), this);
        }
        
        StartRace();
        
        //
        for(int i =0;i<nProduceThreadCount;i++){
            threads_prod[i].join();
        }
        for(int i =0;i<nConsumerThreadCount;i++){
            threads_cust[i].join();
        }
        ASSERT_TRUE(nPerformanceQueueSizeCurProduce==nPerformanceQueueSize);
        ASSERT_TRUE(nPerformanceQueueSizeCurConsume==nPerformanceQueueSize);
        ASSERT_TRUE(list.size()==0);
        
    }
    end=ticker::now();
    nLockUseTime = end - start;
    
    std::stringstream stream;
    stream << "  produce/customer count  = " << nPerformanceQueueSize << std::endl
    << "    nLockUseTime        = " << nLockUseTime << std::endl
    << "    nLockFreeUseTime    = " << nLockFreeUseTime << std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CmLockfreeTest","%s",stream.str().c_str());
#else
    std::cout << stream.str();
#endif
    CM_INFO_TRACE(stream.str());
}
//======================================================================
//SPSC Queue >>>
void produceSpScFunThread(CmLockfreeTest*pCmLockfreeTest){
    if(pCmLockfreeTest)
        pCmLockfreeTest->produceSpScFun();
}

void consumerSpScFunThread(CmLockfreeTest*pCmLockfreeTest){
    if(pCmLockfreeTest)
        pCmLockfreeTest->consumerSpScFun();
}

TEST_F(CmLockfreeTest, testSpScQueueThreads)
{
    std::thread threadProduce(produceSpScFunThread, this);
    std::thread threadConsumer(consumerSpScFunThread, this);
    nPushedCountByOneThread = LOCKFREE_QUEUE_SIZE_MAX;
    nPlanPushedTotalCount = nPushedCountByOneThread;

    StartRace();
    
    threadProduce.join();
    threadConsumer.join();
    
    ASSERT_TRUE(nPlanPushedTotalCount==nProduceCount);
    ASSERT_TRUE(nPlanPushedTotalCount==nConsumerCount);
    
    Item*pItem=NULL;
    ASSERT_TRUE(!m_queueSpScTest.pop(pItem));
    ASSERT_TRUE(m_queueSpScTest.size()==0);
}

//======================================================================
//Queue >>
void produceFunThread(CmLockfreeTest*pCmLockfreeTest){
    if(pCmLockfreeTest)
        pCmLockfreeTest->produceFun();
}

void consumerFunThread(CmLockfreeTest*pCmLockfreeTest){
    if(pCmLockfreeTest)
        pCmLockfreeTest->consumerFun();
}
TEST_F(CmLockfreeTest, testQueueThreadsPushCount)
{
    const int thread_count_prod = 4;
    std::thread threads[thread_count_prod];
    nPushedCountByOneThread = 103;
    for(int i =0;i<thread_count_prod;i++){
        threads[i] = std::thread(produceFunThread, this);
    }
    nPlanPushedTotalCount = thread_count_prod*nPushedCountByOneThread;
    
    StartRace();
    
    for(int i =0;i<thread_count_prod;i++){
        threads[i].join();
    }
    
    ASSERT_TRUE(nProduceCount==nPlanPushedTotalCount);
    ASSERT_TRUE(m_queueTest.size()==nPlanPushedTotalCount);
}

TEST_F(CmLockfreeTest, testQueueThreads)
{
    //
    const int thread_count_prod = 4;
    std::thread threads_prod[thread_count_prod];
	nPushedCountByOneThread = LOCKFREE_QUEUE_SIZE_10K/thread_count_prod;
    for(int i =0;i<thread_count_prod;i++){
        threads_prod[i] = std::thread(produceFunThread, this);
    }
    nPlanPushedTotalCount = thread_count_prod*nPushedCountByOneThread;

    //
    const int thread_count_cust = 8;
    std::thread threads_cust[thread_count_cust];
    for(int i =0;i<thread_count_cust;i++){
        threads_cust[i] = std::thread(consumerFunThread, this);
    }
    
    StartRace();
    
    //
    for(int i =0;i<thread_count_prod;i++){
        threads_prod[i].join();
    }
    for(int i =0;i<thread_count_cust;i++){
        threads_cust[i].join();
    }
    
    ASSERT_TRUE(nPlanPushedTotalCount==nProduceCount);
    ASSERT_TRUE(nPlanPushedTotalCount==nConsumerCount);
    
    Item*pItem=NULL;
    ASSERT_TRUE(!m_queueTest.pop(pItem));
    ASSERT_TRUE(m_queueTest.size()==0);
}
//=============================================
//

#ifndef UT_DISABLE_LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT
TEST_F(CmLockfreeTest, testSpScQueueNoLimitThreads)
{
    return ;
    bTestNoLimit = true;
    nPushedCountByOneThread = LOCKFREE_QUEUE_NO_LIMIT_TEST_ITEMS;
    std::thread threadProduce(produceSpScFunThread, this);
    std::thread threadConsumer(consumerSpScFunThread, this);
    StartRace();
    
    threadProduce.join();
    threadConsumer.join();
    
    ASSERT_TRUE(nPlanPushedTotalCount==nProduceCount);
    ASSERT_TRUE(nPlanPushedTotalCount==nConsumerCount);
    
    Item*pItem=NULL;
    ASSERT_TRUE(!m_queueSpScTest.pop(pItem));
    ASSERT_TRUE(m_queueSpScTest.size()==0);
}
TEST_F(CmLockfreeTest, testQueueNoLimitThreads)
{
    //
    bTestNoLimit = true;
    const int thread_count_prod = 4;
    std::thread threads_prod[thread_count_prod];
    nPushedCountByOneThread = LOCKFREE_QUEUE_NO_LIMIT_TEST_ITEMS;
    for(int i =0;i<thread_count_prod;i++){
        threads_prod[i] = std::thread(produceFunThread, this);
    }
    nPlanPushedTotalCount = thread_count_prod*nPushedCountByOneThread;
    
    //
    const int thread_count_cust = 8;
    std::thread threads_cust[thread_count_cust];
    for(int i =0;i<thread_count_cust;i++){
        threads_cust[i] = std::thread(consumerFunThread, this);
    }
    
    StartRace();
    
    //
    for(int i =0;i<thread_count_prod;i++){
        threads_prod[i].join();
    }
    for(int i =0;i<thread_count_cust;i++){
        threads_cust[i].join();
    }
    
    ASSERT_TRUE(nPlanPushedTotalCount==nProduceCount);
    ASSERT_TRUE(nPlanPushedTotalCount==nConsumerCount);
    
    Item*pItem=NULL;
    ASSERT_TRUE(!m_queueTest.pop(pItem));
    ASSERT_TRUE(m_queueTest.size()==0);
}
#endif


//TEST_F(CmLockfreeTest, testDebug){
TEST_F(CmLockfreeTest, DISABLED_testDebug){
    int nTimes = 100;
    int nQueueSize = 10240;
    nTimes = 100;
    nQueueSize = 10240;
    int i = 0;
    int nDeleteCount = 0;
    int nNewCount  = 0;
    for(i=0;i<nTimes;i++)
    {
        //CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_DYNAMIC_NO_LIMIT>   queueTest;
        CmLockFreeQueueT<Item*,LOCKFREE_QUEUE_SIZE_10K>   queueTest;

        int i = 0;
        for (i =0;i<nQueueSize;i++){
            Item * pItem = new Item(i);
            nNewCount++;
            ASSERT_TRUE(queueTest.push_back(pItem));
        }
        
        Item * pItem = NULL;
        for (i =0;i<nQueueSize;i++){
            ASSERT_TRUE(queueTest.pop(pItem));
            ASSERT_TRUE(pItem!=NULL);
            ASSERT_TRUE(pItem->m_nPram==i);
            delete pItem;
            nDeleteCount++;
            pItem = NULL;
        }
        ASSERT_TRUE(nDeleteCount==nNewCount);
        ASSERT_TRUE(!queueTest.pop(pItem));
    }
    
}
