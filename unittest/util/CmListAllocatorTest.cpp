//
//  CmListAllocatorTest.cpp
//  utilTest_mac
//
//  Created by yangzhen on 7/15/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//


#include "gtest/gtest.h"
#include "CmMessageBlock.h"

#include "CmListAllocator.h"
#include <set>
#include <list>
#include "timer.h"
#include <sstream>
#include <iostream>
#include "CmDebug.h"

#ifdef CM_ANDROID
#include <android/log.h>
#endif

#define FRIEND_GET_IMPLEMENT( class_name, class_variable, return_type)   \
    return_type friend_get_##class_variable(class_name& inst)   \
    {   \
        return inst.class_variable;   \
    }

#define FRIEND_GET_FUNC( class_inst, class_variable)    \
    friend_get_##class_variable(class_inst)


class CCmListAllocatorPoolTest : public testing::Test
{
public:
    CCmListAllocatorPoolTest()
    {
    }
    
    virtual ~CCmListAllocatorPoolTest()
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
    
protected:
    void Clean()
    {
        CCmListAllocatorPool::stat_reset();
    }
    
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_block_size,int);
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_max_count,int);
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_capacity,int);
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_free_count,int);
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_blocks,char**);
    FRIEND_GET_IMPLEMENT(CCmListAllocatorPool,m_step,int);
    
    void checkBeginStatus(CCmListAllocatorPool& pool, int block_size, int max_count,int step)
    {
        
        ASSERT_TRUE(block_size==FRIEND_GET_FUNC(pool, m_block_size));
        ASSERT_TRUE(max_count==FRIEND_GET_FUNC(pool, m_max_count));
        ASSERT_TRUE(step==FRIEND_GET_FUNC(pool, m_step));
        
        ASSERT_TRUE(0==FRIEND_GET_FUNC(pool,m_free_count));
        ASSERT_TRUE(0==FRIEND_GET_FUNC(pool,m_capacity));
        ASSERT_TRUE(NULL!=FRIEND_GET_FUNC(pool, m_blocks));
    }
    void checkEndStatus(CCmListAllocatorPool& pool, int block_size, int max_count,int step)
    {
        
        ASSERT_TRUE(block_size==FRIEND_GET_FUNC(pool, m_block_size));
        ASSERT_TRUE(max_count==FRIEND_GET_FUNC(pool, m_max_count));
        ASSERT_TRUE(step==FRIEND_GET_FUNC(pool, m_step));
        
        ASSERT_TRUE(0==FRIEND_GET_FUNC(pool,m_free_count));
        ASSERT_TRUE(0==FRIEND_GET_FUNC(pool,m_capacity));
        ASSERT_TRUE(NULL==FRIEND_GET_FUNC(pool, m_blocks));
    }
protected:
};


TEST_F(CCmListAllocatorPoolTest, AllocateParamTest)
{
    CCmListAllocatorPool pool(sizeof(int),100);
    int *pInt=(int*)pool.allocate();
    ASSERT_TRUE(NULL!=pInt);
    
    pool.deallocate(pInt);
}


//need to verify all the details before the class is first used
TEST_F(CCmListAllocatorPoolTest, AllocatorAtVeryBeginningTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    int *pInt=(int*)pool.allocate();
    ASSERT_TRUE(NULL!=pInt);
    ASSERT_TRUE(FRIEND_GET_FUNC(pool,m_free_count)==FRIEND_GET_FUNC(pool, m_capacity)-1);
    ASSERT_TRUE(FRIEND_GET_FUNC(pool, m_capacity)==FRIEND_GET_FUNC(pool, m_step));
    
    pool.deallocate(pInt);
    ASSERT_TRUE(FRIEND_GET_FUNC(pool,m_free_count)==FRIEND_GET_FUNC(pool,m_capacity));
    
    pool.~CCmListAllocatorPool();
    
    
    checkEndStatus(pool, sizeof(int), maxCount, step);
}




TEST_F(CCmListAllocatorPoolTest, AllocateAndDeallocate)
{
    const int maxPoolCounts[]={1,19,99,100,149,150};
    const int steps[]={1,19,20,100,300};
    
    const int arrTestCounts[][8]={
        {    1,    2,     1,      2,      1,      2,      1,      1,      },
        {   10,   15,    18,     10,     11,     13,     17,     18,      },
        {   19,   10,     5,     19,      7,     14,     19,      8,      },
        {   20,    5,     3,     18,     20,     19,     20,      3,      },
        {   21,    4,    21,      3,     19,     20,     21,     17,      },
        {   39,    6,    19,     39,     20,     37,     15,     39,      },
        {   40,   10,    20,     40,     30,     39,     19,     40,      },
        {   41,   10,    40,     41,     30,     19,     39,     40,      },
        {   99,   39,    40,     99,     79,     99,     59,     98,      },
        {  100,   36,    31,     49,     50,     70,    100,     99,      },
        {  101,  100,   101,     20,     59,     79,     99,    101,      },
        {  150,  120,     1,     19,    150,     18,     20,     78,      },
        {  200,  150,    19,    120,      2,     88,    199,     99,      },
    };
    
    //loop maxPoolCounts
    for(int poolCountIndex=0;sizeof(maxPoolCounts)/sizeof(maxPoolCounts[0])>poolCountIndex;++poolCountIndex)
    {
        const int maxPoolCount=maxPoolCounts[poolCountIndex];
        
        //loop steps
        for(int stepIndex=0; sizeof(steps)/sizeof(steps[0])>stepIndex;++stepIndex)
        {
            
            //loop useCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0])
            const int maxUsedCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0]);
            for(int usedCount=0; maxUsedCount>usedCount;++usedCount)
            {
                const int step=steps[stepIndex];
                CCmListAllocatorPool pool(sizeof(int),maxPoolCount,step);
                int freeCount=0;
                int capacity=0;
                int allocatedCount=0;
                
                checkBeginStatus(pool, sizeof(int), maxPoolCount, step);
                
                //loop sizeof(arrTestCounts)/sizeof(arrTestCounts[0])
                for(int arrTestCountsIndex=0; sizeof(arrTestCounts)/sizeof(arrTestCounts[0])>arrTestCountsIndex;++arrTestCountsIndex)
                {
                    
                    //loop usedCount
                    for(int testIndex=0; usedCount>testIndex;++testIndex)
                    {
                        const int testCount=arrTestCounts[arrTestCountsIndex][testIndex];
                        
                        if(capacity<testCount)
                        {
                            if(testCount>maxPoolCount)
                                capacity=maxPoolCount;
                            else if(testCount<step)
                            {
                                capacity=step;
                            }
                            else
                            {
                                const int modValue=testCount%step;
                                const int divValue=testCount/step;
                                if(0==modValue)
                                    capacity=divValue*step;
                                else
                                    capacity=(divValue+1)*step;
                            }
                            if(capacity>maxPoolCount)
                                capacity=maxPoolCount;
                        }
                        
                        std::set<int*> pintSet;
                        int index=0;
                        
                        //allocate
                        for(index=0; index<testCount;++index)
                        {
                            ++allocatedCount;
                            int* pint=(int*)pool.allocate();
                            ASSERT_TRUE(NULL!=pint)<<",step="<<step<<",testCount="<<testCount;
                            ASSERT_TRUE(pintSet.end()==pintSet.find(pint));
                            pintSet.insert(pint);
                        }
                        ASSERT_TRUE(FRIEND_GET_FUNC(pool, m_capacity)==capacity)<<",capacity="<<capacity<<",testCount="<<testCount
                            <<",step="<<step<<",maxPoolCount="<<maxPoolCount<<",realCapacity="<<FRIEND_GET_FUNC(pool, m_capacity);
                        freeCount=allocatedCount>=capacity ? 0 : capacity-allocatedCount;
                        ASSERT_TRUE(FRIEND_GET_FUNC(pool, m_free_count)==freeCount);
                        
                        //deallocate
                        for(std::set<int*>::iterator iter=pintSet.begin();
                            pintSet.end()!=iter;++iter)
                        {
                            --allocatedCount;
                            pool.deallocate(*iter);
                        }
                        freeCount=allocatedCount>=capacity ? 0 : capacity-allocatedCount;
                        ASSERT_TRUE(FRIEND_GET_FUNC(pool, m_free_count)==freeCount);
                    
                    }
                }
                //destructure
                pool.~CCmListAllocatorPool();
                checkEndStatus(pool, sizeof(int), maxPoolCount, step);
            }
        }
    }
}


TEST_F(CCmListAllocatorPoolTest, AllocatorTillPoolMaxCapacityTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    int* tmpVec[200]={0};
    int index=0;
    
    //make capacity full
    for(index=0; index<maxCount;++index)
    {
        tmpVec[index]=(int*)pool.allocate();
        ASSERT_TRUE(NULL!=tmpVec[index]);
    }
    ASSERT_TRUE(0==FRIEND_GET_FUNC(pool, m_free_count));
    
    //return all blocks
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));

    //destructure
    pool.~CCmListAllocatorPool();
    checkEndStatus(pool, sizeof(int), maxCount, step);
    
}

TEST_F(CCmListAllocatorPoolTest, AllocatorAfterPoolMaxCapacityTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    int* tmpVec[200]={0};
    int index=0;
    
    //make capacity full
    for(index=0; index<maxCount;++index)
    {
        tmpVec[index]=(int*)pool.allocate();
        ASSERT_TRUE(NULL!=tmpVec[index]);
    }

    //allocate a block
    tmpVec[index++]=(int*)pool.allocate();
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_capacity));
    
    //return all blocks
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //destructure
    pool.~CCmListAllocatorPool();
    checkEndStatus(pool, sizeof(int), maxCount, step);
    
}

TEST_F(CCmListAllocatorPoolTest, AllocatorAfterPoolFullTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    //make capacity full
    int* tmpVec[200]={0};
    int index=0;
    for(index=0; index<maxCount;++index)
    {
        tmpVec[index]=(int*)pool.allocate();
        ASSERT_TRUE(NULL!=tmpVec[index]);
    }
    
    //make pool full
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //allocate a block
    index=0;
    tmpVec[index++]=(int*)pool.allocate();
    ASSERT_TRUE(FRIEND_GET_FUNC(pool, m_free_count)+1==FRIEND_GET_FUNC(pool, m_capacity));
    
    //return all blocks
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //destructure
    pool.~CCmListAllocatorPool();
    checkEndStatus(pool, sizeof(int), maxCount, step);
    
}


TEST_F(CCmListAllocatorPoolTest, DeallocatorAfterPoolFullTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    //make capacity full
    int* tmpVec[200]={0};
    int index=0;
    for(index=0; index<maxCount;++index)
    {
        tmpVec[index]=(int*)pool.allocate();
        ASSERT_TRUE(NULL!=tmpVec[index]);
    }
    
    //allocate a block
    tmpVec[index++]=(int*)pool.allocate();
    ASSERT_TRUE(0==FRIEND_GET_FUNC(pool, m_free_count));
    
    //make pool full
    for(index=index-1; index>=1;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //deallocate a block
    ASSERT_TRUE(0==index);
    pool.deallocate(tmpVec[index--]);
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //return all blocks
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //destructure
    pool.~CCmListAllocatorPool();
    checkEndStatus(pool, sizeof(int), maxCount, step);
    
}


TEST_F(CCmListAllocatorPoolTest, DeallocatorWhenPoolEmptyTest)
{
    const int maxCount=100;
    const int step=20;
    
    CCmListAllocatorPool pool(sizeof(int),maxCount,step);
    
    checkBeginStatus(pool, sizeof(int), maxCount, step);
    
    //make capacity full
    int* tmpVec[200]={0};
    int index=0;
    for(index=0; index<maxCount;++index)
    {
        tmpVec[index]=(int*)pool.allocate();
        ASSERT_TRUE(NULL!=tmpVec[index]);
    }
    
    //pool empty now
    //deallocate a block
    pool.deallocate(tmpVec[--index]);
    ASSERT_TRUE(1==FRIEND_GET_FUNC(pool, m_free_count));
    
    //return all blocks
    for(index=index-1; index>=0;--index)
    {
        pool.deallocate(tmpVec[index]);
    }
    ASSERT_TRUE(maxCount==FRIEND_GET_FUNC(pool, m_free_count));
    
    //destructure
    pool.~CCmListAllocatorPool();
    checkEndStatus(pool, sizeof(int), maxCount, step);
    
}


class CCmListAllocatorTest : public testing::Test
{
public:
    CCmListAllocatorTest()
    {
    }
    
    virtual ~CCmListAllocatorTest()
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
    
protected:
    void Clean()
    {
        CCmListAllocatorPool::stat_reset();
    }
    
protected:
};


TEST_F(CCmListAllocatorTest, IntAllocateAndDeallocate)
{
    const int maxPoolCounts[]={1,19,99,100,149,150};
    const int steps[]={1,19,20,100,300};
    
    const int arrTestCounts[][8]={
        {    1,    2,     1,      2,      1,      2,      1,      1,      },
        {   10,   15,    18,     10,     11,     13,     17,     18,      },
        {   19,   10,     5,     19,      7,     14,     19,      8,      },
        {   20,    5,     3,     18,     20,     19,     20,      3,      },
        {   21,    4,    21,      3,     19,     20,     21,     17,      },
        {   39,    6,    19,     39,     20,     37,     15,     39,      },
        {   40,   10,    20,     40,     30,     39,     19,     40,      },
        {   41,   10,    40,     41,     30,     19,     39,     40,      },
        {   99,   39,    40,     99,     79,     99,     59,     98,      },
        {  100,   36,    31,     49,     50,     70,    100,     99,      },
        {  101,  100,   101,     20,     59,     79,     99,    101,      },
        {  150,  120,     1,     19,    150,     18,     20,     78,      },
        {  200,  150,    19,    120,      2,     88,    199,     99,      },
    };
    
    //loop maxPoolCounts
    for(int poolCountIndex=0;sizeof(maxPoolCounts)/sizeof(maxPoolCounts[0])>poolCountIndex;++poolCountIndex)
    {
        
        //loop steps
        for(int stepIndex=0; sizeof(steps)/sizeof(steps[0])>stepIndex;++stepIndex)
        {
            
            //loop useCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0])
            const int maxUsedCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0]);
            for(int usedCount=0; maxUsedCount>usedCount;++usedCount)
            {
                std::list<int, CCmListAllocator<int,100, 20> > theList;
                
                
                //loop sizeof(arrTestCounts)/sizeof(arrTestCounts[0])
                for(int arrTestCountsIndex=0; sizeof(arrTestCounts)/sizeof(arrTestCounts[0])>arrTestCountsIndex;++arrTestCountsIndex)
                {
                    
                    //loop usedCount
                    for(int testIndex=0; usedCount>testIndex;++testIndex)
                    {
                        const int testCount=arrTestCounts[arrTestCountsIndex][testIndex];
                        
                        int index=0;
                        //allocate
                        for(index=0; index<testCount;++index)
                        {
                            theList.push_back(index);
                        }
                                               //deallocate
                        for(index=0; index<testCount;++index)
                        {
                            theList.pop_back();
                        }
                        
                    }
                }
            }
        }
    }
}

class CCmListAllocatorTest_UnitClass
{
public:
    CCmListAllocatorTest_UnitClass()
    {
        setI(0);
    }
    virtual ~CCmListAllocatorTest_UnitClass()
    {
        setI(-1);
    }
    
    void setI(int i)
    {
        m_i=i;
    }
public:
    int m_i;
};



TEST_F(CCmListAllocatorTest, ClassAllocateAndDeallocate)
{
    const int maxPoolCounts[]={1,19,99,100,149,150};
    const int steps[]={1,19,20,100,300};
    
    const int arrTestCounts[][8]={
        {    1,    2,     1,      2,      1,      2,      1,      1,      },
        {   10,   15,    18,     10,     11,     13,     17,     18,      },
        {   19,   10,     5,     19,      7,     14,     19,      8,      },
        {   20,    5,     3,     18,     20,     19,     20,      3,      },
        {   21,    4,    21,      3,     19,     20,     21,     17,      },
        {   39,    6,    19,     39,     20,     37,     15,     39,      },
        {   40,   10,    20,     40,     30,     39,     19,     40,      },
        {   41,   10,    40,     41,     30,     19,     39,     40,      },
        {   99,   39,    40,     99,     79,     99,     59,     98,      },
        {  100,   36,    31,     49,     50,     70,    100,     99,      },
        {  101,  100,   101,     20,     59,     79,     99,    101,      },
        {  150,  120,     1,     19,    150,     18,     20,     78,      },
        {  200,  150,    19,    120,      2,     88,    199,     99,      },
    };
    
    //loop maxPoolCounts
    for(int poolCountIndex=0;sizeof(maxPoolCounts)/sizeof(maxPoolCounts[0])>poolCountIndex;++poolCountIndex)
    {
        
        //loop steps
        for(int stepIndex=0; sizeof(steps)/sizeof(steps[0])>stepIndex;++stepIndex)
        {
            
            //loop useCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0])
            const int maxUsedCount=sizeof(arrTestCounts[0])/sizeof(arrTestCounts[0][0]);
            for(int usedCount=0; maxUsedCount>usedCount;++usedCount)
            {
                std::list<CCmListAllocatorTest_UnitClass, CCmListAllocator<CCmListAllocatorTest_UnitClass,100, 20> > theList;
                
                
                //loop sizeof(arrTestCounts)/sizeof(arrTestCounts[0])
                for(int arrTestCountsIndex=0; sizeof(arrTestCounts)/sizeof(arrTestCounts[0])>arrTestCountsIndex;++arrTestCountsIndex)
                {
                    
                    //loop usedCount
                    for(int testIndex=0; usedCount>testIndex;++testIndex)
                    {
                        const int testCount=arrTestCounts[arrTestCountsIndex][testIndex];
                        
                        int index=0;
                        //allocate
                        for(index=0; index<testCount;++index)
                        {
                            theList.push_back(CCmListAllocatorTest_UnitClass());
                        }
                        //deallocate
                        for(index=0; index<testCount;++index)
                        {
                            theList.pop_back();
                        }
                        
                    }
                }
            }
        }
    }
}


#include <string>

TEST_F(CCmListAllocatorTest, PerformanceTest)
{
    std::list<int> normalList;
    std::list<int, CCmListAllocator<int,160,20> > customList;
 
    const uint64_t now = time(NULL);
    bool traceDetail = now > 125 * 365 * 24 * 3600;

    int outerIndex=0;
    const int outerCount=10000;
    
    int innerIndex=0;
    const int innerCount=160;
    int unitIndex=0;
    
    uint64_t before;
    uint64_t after;
    
    before=ticker::now();
    unitIndex=0;
    for(outerIndex=0;outerCount>outerIndex;++outerIndex)
    {
        EXPECT_TRUE(0==customList.size());
        for(innerIndex=0;innerCount>innerIndex;++innerIndex)
        {
            customList.push_back(++unitIndex);
            if (traceDetail)
            {
                CM_INFO_TRACE("customList size "<<customList.size());
            }
        }
        EXPECT_TRUE(innerCount==customList.size());
        customList.clear();
    }
    after=ticker::now();
    uint64_t customListDelta=after-before;

    
    before=ticker::now();
    unitIndex=0;
    for(outerIndex=0;outerCount>outerIndex;++outerIndex)
    {
        EXPECT_TRUE(0==customList.size());
        for(innerIndex=0;innerCount>innerIndex;++innerIndex)
        {
            normalList.push_back(++unitIndex);
            if (traceDetail)
            {
                CM_INFO_TRACE("normalList size " << normalList.size());
            }
        }
        EXPECT_TRUE(innerCount==normalList.size());
        normalList.clear();
    }
    after=ticker::now();
    uint64_t normalListDelta=after-before;
    
    std::stringstream ss;
    struct comma_facet : public std::numpunct<char>
    {
        protected:
        virtual std::string do_grouping() const { return "\003" ; }
        virtual ~comma_facet(){}
    };
    ss.imbue(std::locale( ss.getloc(), new comma_facet ));
    ss<<std::setw(20)<<" std::allocator("<<outerCount<<"x"<<innerCount<<"):  "<< std::setw(10)<<  normalListDelta <<std::endl;
    ss<<std::setw(20)<<" CCmListAllocator("<<outerCount<<"x"<<innerCount<<"):  "<< std::setw(10)<<  customListDelta <<std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CCmListAllocatorTest","%s",ss.str().c_str());
#else
    std::cout << ss.str();
#endif
    CM_INFO_TRACE(ss.str());
    
    /*
     android release:
        std::allocator(10,000x160):     420,000
        CCmListAllocator(10,000x160):    40,000
     
     
     mac release:
        std::allocator(10,000x160):     330,000
        CCmListAllocator(10,000x160):    20,000
     
     
     mac debug:
        std::allocator(10,000x160):     260,000
        CCmListAllocator(10,000x160):   130,000
     
     ios release:
        std::allocator(10,000x160):     310,000
        CCmListAllocator(10,000x160):    30,000
     
     ios debug:
        std::allocator(10,000x160):     490,000
        CCmListAllocator(10,000x160):   270,000
     
     windows release:
        std::allocator(10,000x160):     719,000
        CCmListAllocator(10,000x160):   547,000
     windows debug:
        std::allocator(10,000x160):   4,891,000
        CCmListAllocator(10,000x160): 3,484,000

     windows phone 8.1 release:
        std::allocator(10,000x160):     1,172,000
        CCmListAllocator(10,000x160):      78,000
     windows phone 8.1 debug:
        std::allocator(10,000x160):     10,797,000
        CCmListAllocator(10,000x160):    4,406,000
     */

}

TEST_F(CCmListAllocatorTest, StdSwap2CustomAssignPerformanceTest)
{
    int index=0;
    const int count=100*10000;
    const int maxListSize = 5;

    const uint64_t now = time(NULL);
    bool traceDetail = now > 125 * 365 * 24 * 3600;
    
    uint64_t std_list_swap_time=0;
    {
        std::list<int> theList;
        std::list<int> theList2;
        
        uint64_t before;
        uint64_t after;
        
        before=ticker::now();
        for(index=0; count>index;++index)
        {
            for(int i=0;maxListSize>i;++i)
            {
                theList.push_back(index+i);
                if (traceDetail)
                {
                    CM_INFO_TRACE("size "<<theList.size());
                }
            }
            theList2.swap(theList); 
            if (traceDetail)
            {
                CM_INFO_TRACE("size " << theList2.size());
            }
            theList2.clear();
            if (traceDetail)
            {
                CM_INFO_TRACE("size " << theList2.size());
            }
        }
        after=ticker::now();
        std_list_swap_time=after-before;
    }
    
    
    
    uint64_t cm_list_swap_time=0;
    {
        std::list<int, CCmListAllocator<int, 20,5> > theList;
        std::list<int, CCmListAllocator<int, 20,5> > theList2;
        
        uint64_t before;
        uint64_t after;
        
        before=ticker::now();
        for(index=0; count>index;++index)
        {
            for(int i=0;maxListSize>i;++i)
            {
                theList.push_back(index + i);
                if (traceDetail)
                {
                    CM_INFO_TRACE("size " << theList.size());
                }
            }
            theList2.assign(theList.begin(), theList.end());
            if (traceDetail)
            {
                CM_INFO_TRACE("size " << theList2.size());
            }
            theList2.clear();
            if (traceDetail)
            {
                CM_INFO_TRACE("size " << theList2.size());
            }
            theList.clear();
            ASSERT_TRUE(0== CCmListAllocatorPool::get_allocated_directly());
        }
        after=ticker::now();
        cm_list_swap_time=after-before;
    }
    
    
    std::stringstream ss;
    struct comma_facet : public std::numpunct<char>
    {
    protected:
        virtual std::string do_grouping() const { return "\003" ; }
        virtual ~comma_facet(){}
    };
    ss.imbue(std::locale( ss.getloc(), new comma_facet ));
    ss<<std::setw(20)<<" std swap("<<count<<"):  "<< std::setw(10)<<  std_list_swap_time <<std::endl;
    ss<<std::setw(20)<<" custom assign("<<count<<"):  "<< std::setw(10)<<  cm_list_swap_time <<std::endl;
#ifdef CM_ANDROID
    __android_log_print(ANDROID_LOG_INFO,"CCmListAllocatorTest SwapVsAssign","%s",ss.str().c_str());
#else
    std::cout << ss.str();
#endif
    CM_INFO_TRACE(ss.str());
    
    
/*
    mac os release:
        std swap(1,000,000):            520,000
        custom assign(1,000,000):        70,000
    mac os debug:
        std swap(1,000,000):            840,000
        custom assign(1,000,000):       700,000
 
    ios debug:
        std swap(1,000,000):            1,750,000
        custom assign(1,000,000):       4,280,000
 
    ios release:
        std swap(1,000,000):            1,010,000
        custom assign(1,000,000):         170,000
 
    android release:
        std swap(1,000,000):            1,980,000
        custom assign(1,000,000):         860,000
 
    windows debug:
        std swap(1,000,000):            16,547,000
        custom assign(1,000,000):       30,375,000
   
    windows release:
        std swap(1,000,000):            2,265,000
        custom assign(1,000,000):       3,219,000

    windows phone 8.1 debug:
        std swap(1,000,000):            34,609,000
        custom assign(1,000,000):       52,500,000

    windows phone 8.1 release:
         std swap(1,000,000):           3,610,000
      custom assign(1,000,000):           531,000
 */
}


TEST_F(CCmListAllocatorTest, ListCopyTest)
{
    const int count=1000;
    int index=0;
    
    
    std::list<int, CCmListAllocator<int, 100, 20> > theList;
    theList.push_back(-2);
    theList.push_back(-1);

    std::list<int, CCmListAllocator<int, 100, 20> > theList2=theList;
    ASSERT_TRUE(2==theList2.size());
    theList2.clear();
    ASSERT_TRUE(0==theList2.size());
    theList2.push_back(-1);
    theList2.push_back(-2);
    
    std::list<int, CCmListAllocator<int, 100, 20> > theList3(theList);
    ASSERT_TRUE(2==theList3.size());
    theList3.clear();
    ASSERT_TRUE(theList3.empty());
    theList3.push_back(-1);
    theList3.push_back(-2);
    
    
    for(index=0;count>index;++index)
    {
        theList.push_back(index);
        theList.pop_front();
        ASSERT_TRUE(index-1==theList.front());
        
        theList2.insert(theList2.begin(),index);
        theList2.pop_back();
        ASSERT_TRUE(index-1==theList2.back());
        
        theList3.push_front(index);
        theList3.erase(--theList3.end());
        ASSERT_TRUE(index-1==theList3.back());
    }
    theList2.pop_front();
    theList.assign(theList2.begin(), theList2.end());
    ASSERT_TRUE(theList.size()==theList2.size());
    
    theList3.pop_back();
    theList.assign(theList3.begin(), theList3.end());
    ASSERT_TRUE(theList.size()==theList3.size());
}



TEST_F(CCmListAllocatorTest, PoolMetricsTest)
{
    std::list<int, CCmListAllocator<int, 100,5> > theList;
    int index=0;
    //use 100*2, 5*2 
    //because some compiler( ms, ...) will allocate more blocks
    for(index=0;105>index;++index)
        theList.push_back(index);
    ASSERT_TRUE(100*2 > CCmListAllocatorPool::get_allocated_from_pool());
    ASSERT_TRUE(5*2 > CCmListAllocatorPool::get_allocated_directly());
 
    theList.clear();
    ASSERT_TRUE(100*2 > CCmListAllocatorPool::get_deallocated_to_pool());
    ASSERT_TRUE(5*2 > CCmListAllocatorPool::get_deallocated_directly());
    
    
    for(index=0;105>index;++index)
        theList.push_back(index);
    ASSERT_TRUE(200*2 > CCmListAllocatorPool::get_allocated_from_pool());
    ASSERT_TRUE(10*2 > CCmListAllocatorPool::get_allocated_directly());
    
    //theList2.swap(theList);
    
    theList.clear();
    ASSERT_TRUE(200*2 > CCmListAllocatorPool::get_deallocated_to_pool());
    ASSERT_TRUE(10*2 > CCmListAllocatorPool::get_deallocated_directly());
}





