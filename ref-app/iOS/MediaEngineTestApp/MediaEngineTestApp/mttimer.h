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
#import <CoreFoundation/CFRunLoop.h>
#import "DemoClient.h"
typedef CFRunLoopTimerRef ATHTimer;




class CTimerProc
{
public:
    virtual ~CTimerProc() {};
    virtual void OnTimer (void*, unsigned int, ATHTimer idEvent, unsigned long dwTime) = 0;
};


class CTimerID
{
protected:
    ATHTimer   m_tmID;

    static void timer_callback (CFRunLoopTimerRef timer, void *info)
    {
//        CTimerProc* proc = (CTimerProc*)info;
//        proc->OnTimer (0, 0, timer, 0);
        
        DemoClient::HeartBeat();
    }

public:
    explicit CTimerID (ATHTimer tmID = 0 )
        : m_tmID (tmID)
    {
    }

    ~CTimerID ()
    {
        KillTimer();
    }

    operator ATHTimer () const { return m_tmID; }

    CTimerID& operator = (ATHTimer id)
    {
        assert ( !m_tmID );
        m_tmID = id;
        return *this;
    }
    
    ATHTimer SetTimer (unsigned int interval, CTimerProc* pProc)
    {
        assert ( !m_tmID );
        CFRunLoopTimerContext context = { 0, pProc, NULL, NULL, NULL };
        CFTimeInterval ti = 0.001 * (CFTimeInterval)interval;
		CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + ti;// + (1.0e-3 * timeout);
        m_tmID = CFRunLoopTimerCreate ( NULL, fireDate, ti, 0, 0, timer_callback, &context);
		//FIXME: Temporarily adding this to debug crash on resume. To be removed after. (yaprasad, 9-21-10)
		printf("AT::CTimerID::SetTimer: Setting timer %p with interval %u and target of %p\n", m_tmID, interval, pProc);
        CFRunLoopAddTimer(CFRunLoopGetCurrent(), m_tmID, kCFRunLoopCommonModes);
        return m_tmID;
    }
    
    void KillTimer()
    {
        if (m_tmID)
        {
			//FIXME: Temporarily adding this to debug crash on resume. To be removed after. (yaprasad, 9-21-10)
			printf("AT::CTimerID::KillTimer: Killing timer %p\n", m_tmID);
            CFRunLoopTimerInvalidate (m_tmID);
            CFRelease (m_tmID);
            m_tmID = NULL;
        }
    }
};



#endif // #ifndef MTTIMER_H
