//
//  UdpTrace.cpp
//  perfbase
//
//  Created by yangzhen on 4/30/15.
//  Copyright (c) 2015 com.cisco.wme.yangzhen. All rights reserved.
//

#include "UdpTrace.h"

#include <unistd.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/policy.h>
#include <mach/task_info.h>
#include <mach/thread_info.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <list>
#include <stdlib.h>


static in_addr_t g_to_addr=0;

typedef struct {
    unsigned int lastUsedTicks;
    unsigned int lastTotalTicks;
    float used;
} CPU_STATISTIC;
typedef struct _tag_cpu_usage{
    float total_cpu_usage;
    std::list<float> cores_cpu_usage;
} CPU_USAGE;

static int get_cpu_usage(CPU_USAGE & usage){
    usage.cores_cpu_usage.clear();
    static CPU_STATISTIC s_metrics[100];
    
    unsigned numCPUs;
    int mib[2U] = { CTL_HW, HW_NCPU };
    size_t sizeOfNumCPUs = sizeof(numCPUs);
    int status = sysctl(mib, 2U, &numCPUs, &sizeOfNumCPUs, NULL, 0U);
    if(status)
        numCPUs = 1;
    
    processor_cpu_load_info_t cpu_load;
    mach_msg_type_number_t cpu_msg_count;
    natural_t cpu_count;
    
    kern_return_t kret = host_processor_info(mach_host_self(),
                                             PROCESSOR_CPU_LOAD_INFO,
                                             &cpu_count,
                                             (processor_info_array_t *)&cpu_load,
                                             &cpu_msg_count);
    if(kret != KERN_SUCCESS){
        return -1;
    }
    
    float total_used = 0.0;
    for(unsigned int index = 0; index < numCPUs && index < sizeof(s_metrics)/sizeof(CPU_STATISTIC); ++index)
    {
        CPU_STATISTIC &metrics = s_metrics[index];
        
        unsigned int userTicks = cpu_load[index].cpu_ticks[CPU_STATE_USER];
        unsigned int niceTicks = cpu_load[index].cpu_ticks[CPU_STATE_NICE];
        unsigned int systemTicks = cpu_load[index].cpu_ticks[CPU_STATE_SYSTEM];
        unsigned int idleTicks = cpu_load[index].cpu_ticks[CPU_STATE_IDLE];
        
        unsigned int usedTicks = systemTicks + userTicks + niceTicks;
        unsigned int totalTicks = usedTicks + idleTicks;
        unsigned int deltaUsed = usedTicks - metrics.lastUsedTicks;
        
        float deltaTotal = static_cast<float>(totalTicks
                                              - metrics.lastTotalTicks);
        if(deltaTotal > 0)
        {
            metrics.used = (static_cast<float>(deltaUsed)
                            / deltaTotal * 100.0f);
        }
        total_used += metrics.used;
        usage.cores_cpu_usage.push_back(metrics.used);
        metrics.lastTotalTicks = totalTicks;
        metrics.lastUsedTicks = usedTicks;;
    }
    vm_deallocate(mach_task_self(), (vm_address_t)cpu_load, cpu_msg_count*sizeof(processor_info_array_t));
    
    usage.total_cpu_usage = total_used;
    
    return 0;
}
/*
static void mydump_system(char* buf,const int len,int& writeLen)
{
    CPU_USAGE usage;
    get_cpu_usage(usage);
    writeLen=0;
    int remainLen=len;
    if(0<remainLen)
    {
        int newWriteLen=snprintf(buf+writeLen,remainLen,"total cpu=%10g\r\n",
                                 usage.total_cpu_usage);
        if(0<newWriteLen)
        {
            writeLen+=newWriteLen;
            remainLen-=newWriteLen;
        }
    }

}
*/

static void mydump_threads(char* buf,const int len,int& writeLen)
{
    pid_t pid = ::getpid();
    task_t port;
    task_for_pid(mach_task_self(), pid, &port);
    
    task_info_data_t tinfo;
    mach_msg_type_number_t task_info_count;
    
    task_info_count = TASK_INFO_MAX;
    kern_return_t kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
    if (kr != KERN_SUCCESS) {
        writeLen=snprintf(buf,len,"task_info failed\r\n");
        return;
    }
    
    task_basic_info_t        basic_info;
    thread_array_t         thread_list;
    mach_msg_type_number_t thread_count;
    
    thread_info_data_t     thinfo;
    mach_msg_type_number_t thread_info_count;
    
    
    
    thread_info_data_t          tidStoreInfo;
    thread_identifier_info_t    tidinfo;
    
    
    thread_basic_info_t basic_info_th;
    uint32_t stat_thread = 0; // Mach threads
    
    basic_info = (task_basic_info_t)tinfo;
    
    // get threads in the task
    kr = task_threads(mach_task_self(), &thread_list, &thread_count);
    if (kr != KERN_SUCCESS) {
        writeLen=snprintf(buf,len,"task_threads failed\r\n");
        return;
    }
    if (thread_count > 0)
        stat_thread += thread_count;
    
    //long tot_sec = 0;
    //long tot_usec = 0;
    //long tot_cpu = 0;
    int j;
    
    writeLen=0;
    int remainLen=len;
    
    
    if(0<remainLen)
    {
        int newWriteLen=snprintf(buf+writeLen,remainLen,"thread count:%d\r\n",
                                 thread_count);
        if(0<newWriteLen)
        {
            writeLen+=newWriteLen;
            remainLen-=newWriteLen;
        }
    }
    float tot_cpu=0;
    
    for (j = 0; j < (int)thread_count; j++) {
        
        thread_info_count = THREAD_INFO_MAX;
        kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
                         (thread_info_t)thinfo, &thread_info_count);
        if (kr != KERN_SUCCESS) {
            if(0<remainLen)
            {
                int newWriteLen=snprintf(buf+writeLen,remainLen,"%d:thread info THREAD_BASIC_INFO failed\r\n",
                                         j);
                if(0<newWriteLen)
                {
                    writeLen+=newWriteLen;
                    remainLen-=newWriteLen;
                }
            }
            
            continue;
        }
        basic_info_th = (thread_basic_info_t)thinfo;
        
        
        kr = thread_info(thread_list[j],THREAD_IDENTIFIER_INFO,
                         (thread_info_t)tidStoreInfo,&thread_info_count);
        if(KERN_SUCCESS!=kr)
        {
            if(0<remainLen)
            {
                int newWriteLen=snprintf(buf+writeLen,remainLen,"%d:thread info THREAD_IDENTIFIER_INFO failed\r\n",
                                         j);
                if(0<newWriteLen)
                {
                    writeLen+=newWriteLen;
                    remainLen-=newWriteLen;
                }
            }
            
            continue;
        }
        tot_cpu+=basic_info_th->cpu_usage;
        tidinfo=(thread_identifier_info_t)tidStoreInfo;
        if(0<remainLen)
        {
            char name[512]={0};
            pthread_t pt = pthread_from_mach_thread_np(thread_list[j]);
            pthread_getname_np(pt, name,sizeof(name));
            int newWriteLen=snprintf(buf+writeLen,remainLen,"%3d %10lld %10llx %10llx %10g %s\r\n",
                                     j,
                                     tidinfo->thread_id,
                                     tidinfo->thread_id,
                                     tidinfo->thread_handle,
                                     basic_info_th->cpu_usage/(float)TH_USAGE_SCALE * 100.0,
                                     name);
            if(0<newWriteLen)
            {
                writeLen+=newWriteLen;
                remainLen-=newWriteLen;
            }
        }
        /*
         if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
         tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
         tot_usec = tot_usec + basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
         tot_cpu = tot_cpu + basic_info_th->cpu_usage;
         }
         */
        
    } // for each thread
    if(0<thread_count)
        vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));
    if(0<remainLen)
    {
        int newWriteLen=snprintf(buf+writeLen,remainLen,"loop end: j=%d,thread_count=%d,total cpu=%10g\r\n",
                                 j,thread_count,tot_cpu/(float)TH_USAGE_SCALE * 100.0);
        if(0<newWriteLen)
        {
            writeLen+=newWriteLen;
            remainLen-=newWriteLen;
        }
    }

    
}
static void* _traceAllThreads(void*)
{
    udptrace::trace_tid("udp trace");
    int s=socket(AF_INET,SOCK_DGRAM,0);
    if(0>=s)
        return NULL;
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=g_to_addr;
    addr.sin_port=htons(10000);
    
    char* buf=new char[65535];
    while(true)
    {
        memset(buf,'0',65535);
        //int len=snprintf(buf,65535,"%s","test1\r\n");
        //int len=60000;
        int len=0;
        mydump_threads(buf,65535,len);
        //mydump_system(buf,65535,len);
        if(0<len)
        {
            ::sendto(s,buf,len,0,(sockaddr*)&addr,sizeof(addr));
        }
        usleep(1000*1000*3);
    }
    return NULL;
}
static void traceAllThreads()
{
    pthread_t pt;
    ::pthread_create(&pt,NULL,_traceAllThreads,NULL);
}
static void _send_trace(const char* buf,int len)
{
    static int s=-1;
    static bool inited=false;
    static sockaddr_in addr;
    if(!inited)
    {
        inited=true;
        s=socket(AF_INET,SOCK_DGRAM,0);
        memset(&addr,0,sizeof(addr));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=g_to_addr;
        addr.sin_port=htons(10001);
    }
    if(0<s)
        ::sendto(s,buf,len,0,(sockaddr*)&addr,sizeof(addr));
}

#include "CMDebug.h"

namespace udptrace
{
    void start()
    {
        const char* svrAddr=getenv("WME_PERFORMANCE_TEST_udp");
        if(NULL==svrAddr)
            return;
        CM_INFO_TRACE("udptrace svrAddr=" << svrAddr);
        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        inet_pton(AF_INET, svrAddr,&(serv.sin_addr));
        //g_to_addr=inet_addr(svrAddr);
        g_to_addr = serv.sin_addr.s_addr;
        
        trace_tid("main");
        usleep(1000*100);
        traceAllThreads();
    }
    void send_trace(const char* format,...)
    {
        char buf[2048]={0};
        va_list args;
        va_start(args, format);
        int len = vsnprintf( buf, sizeof(buf) - 1, format, args);
        if(0<len)
            _send_trace(buf,len);
    }
    void trace_tid(const char* info)
    {
        char buf[2048]={0};
        int len=snprintf(buf,sizeof(buf),"%s tid=%d\r\n",info,syscall(SYS_thread_selfid));
        if(0<len)
        {
            _send_trace(buf,len);
        }
    }
}

