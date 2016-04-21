#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
// Windows
#include <windows.h>

//C rumtime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS 

#include <atlbase.h>
#include <atlapp.h>
#include <atlstr.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlscrl.h>


#if !defined(SAFE_DELETE)
#define SAFE_DELETE(x) { if (x) delete x; x = NULL; }
#endif

#if !defined(SAFE_ARRAY_DELETE)
#define SAFE_ARRAY_DELETE(x) { if (x) delete [] x; x = NULL; }
#endif
