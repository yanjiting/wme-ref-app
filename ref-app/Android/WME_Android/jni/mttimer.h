//////////////////////////////////////////////////////////////////////////////////
//  CTimer
//      
//
//  Copyright (C) 2009 WebEx Communications Inc.
//  All rights reserved                                                     
//
//
//  Author: 
//      Jiong Tu 
//
//  Histroy :
//      10/26/2009      create;
//
// attention:
//
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef MTTIMER_H
#define MTTIMER_H

#include <stdio.h>
#include "CmThreadManager.h"
#include "CmThread.h"
#include "DemoParameters.h"
#include "MediaClient.h"


typedef struct TPInfo {
    CMediaClient *client;
    bool host;
    bool enableCtrl;
    char hostIP[32];

    int hostDataPort;
    NetConnType connDataType;
    int hostCtrlPort;
    NetConnType connCtrlType;
}TPInfo_t;

class CTimerAndroid
{
    enum {
        S_INIT = 0,
        S_WAIT,
        S_RUN,
        S_QUIT,
    };
protected:
    std::vector<TPInfo_t> m_msg;

    pthread_t m_tid;
    unsigned long m_interval; // ms
    long m_state;
    bool m_quit;

    pthread_mutex_t m_mtx;
    pthread_cond_t m_cond;

    static void * timer_callback (void *info)
    {
        CTimerAndroid* thiz = (CTimerAndroid*)info;
        if (thiz)
            thiz->Run();
        return NULL;
    }

    void Run();

public:
    explicit CTimerAndroid () : m_interval(0), m_tid(-1), m_state(S_INIT)
    {
        m_msg.clear();
        pthread_mutex_init(&m_mtx, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    ~CTimerAndroid ()
    {
        m_msg.clear();
        KillTimer();
        pthread_mutex_destroy(&m_mtx);
        pthread_cond_destroy(&m_cond);
    }

    void Pause()
    {
        pthread_mutex_lock(&m_mtx);
        m_state = S_WAIT;
        pthread_mutex_unlock(&m_mtx);
    }

    void Continue()
    {
        pthread_mutex_lock(&m_mtx);
        m_state = S_RUN;
        pthread_mutex_unlock(&m_mtx);
    }

    void PopMsg()
    {
        pthread_mutex_lock(&m_mtx);
        m_msg.clear();
        pthread_mutex_unlock(&m_mtx);
    }

    void PushMsg(TPInfo_t & msg)
    {
        pthread_mutex_lock(&m_mtx);
        m_msg.push_back(msg);
        pthread_mutex_unlock(&m_mtx);
    }

    bool SetTimer (unsigned int interval);
    void KillTimer();
};



#endif // #ifndef MTTIMER_H
