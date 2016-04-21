//
//  UdpTrace.h
//  perfbase
//
//  Created by yangzhen on 4/30/15.
//  Copyright (c) 2015 com.cisco.wme.yangzhen. All rights reserved.
//

#ifndef __perfbase__UdpTrace__
#define __perfbase__UdpTrace__

#include <stdio.h>

namespace udptrace
{
    extern void start();
    extern void send_trace(const char* format,...);
    extern void trace_tid(const char* info);
}

#endif /* defined(__perfbase__UdpTrace__) */
