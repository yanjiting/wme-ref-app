//
//  CWseUtil.cpp
//  sharkUnitTestApp
//
//  Created by guangx on 5/5/14.
//  Copyright (c) 2014 WME. All rights reserved.
//
#if defined (WIN32)
#include <windows.h>
#elif defined(ANDROID) || defined(LINUX) || defined(MACOS) || defined (IPHONEOS)
#include <unistd.h>
#endif

#include "CWseUtil.h"


void WseSleep(unsigned int ms)
{

#if defined (WIN32)
    Sleep(ms);
#elif defined(ANDROID) || defined(LINUX) || defined(MACOS) || defined(MACOS) || defined (IPHONEOS)
    usleep(ms);
#endif
}