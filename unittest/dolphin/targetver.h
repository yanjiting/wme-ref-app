#if !defined(WBXAE_TARGETVER_H_INCLUDED_)
#define WBXAE_TARGETVER_H_INCLUDED_


#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif // _WIN32_WINNT

#include "WseCommonTypes.h"
#include "WseUtil.h"// protect redefine
#include "IWBXAEBase.h"
#include "Mmdeviceapi.h"
#include "Audioclient.h"
#include "Audiopolicy.h"
#include "functiondiscoverykeys.h"
#include "dsound.h"
#include "dsconf.h"
#include <string>
#include <list>
#include "dbt.h"
#include "uuids.h"

using namespace std;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif//SAFE_RELEASE

#define WM_NOTIFY_DEVICE_CHANGE		WM_USER + 320
#define WM_NOTIFY_VOLUME_CHANGE		WM_USER + 321
#define WM_NOTIFY_DEFAULT_CHANGE	WM_USER + 322

#endif //!define WBXAE_TARGETVER_H_INCLUDED_
