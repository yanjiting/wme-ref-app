///
///  WmeCommonTypes.h
///
///
///  Created by Sand Pei on 2013-1-30.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_COMMON_TYPES_DEFINE_H
#define WME_COMMON_TYPES_DEFINE_H

#if (defined(WIN32)) || (defined(WP8))

#pragma warning (disable : 4786)
#if (!defined(_WIN32_WINNT)) && (!defined(WP8)) 
#define _WIN32_WINNT 0x0400
#elif (!defined(_WIN32_WINNT)) && (defined(WP8))
#define _WIN32_WINNT 0x0602
#endif // !defined(_WIN32_WINNT)
#if !defined(POINTER_64) && _MSC_VER > 1200
#define POINTER_64 __ptr64
#endif // !defined(POINTER_64) && _MSC_VER > 1200

//MUST include winsock2.h before windows.h to avoid including winsock.h
#include <winsock2.h>
#include <windows.h>

#include <windef.h>
#include <string>
#include <map>
#include <list>
#include <sstream>

#if _MSC_VER >= 1600
#include <stdint.h>
#else
#include "stdint_msc.h"
#endif


#else

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#endif	// !defined(WIN32)

#endif // WME_COMMON_TYPES_DEFINE_H
