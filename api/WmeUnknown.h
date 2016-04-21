///
///  WmeUnknown.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_UNKNOWN_INTERFACE_H
#define WME_UNKNOWN_INTERFACE_H

#include "WmeUuid.h"
#include "WmeError.h"

#ifdef WIN32
#define WME_NOVTABLE		__declspec(novtable)
#define WMECALLTYPE         __stdcall
#else
#define WME_NOVTABLE
#define WMECALLTYPE
#endif

#define WMEMETHOD_(type,method) virtual type WMECALLTYPE method

namespace wme
{

	/// WMEIID
	typedef WMEUUID			WMEIID;
	#define REFWMEIID		const wme::WMEIID &
	#define IsEqualWMEIID(riid1, riid2) IsEqualWMEUUID(riid1, riid2)

///============================IWmeUnknown=============================
/// IWmeUnknown IID
/// {00000000-2013-0114-0123-456789ABCDEF}
static const WMEIID WMEIID_IWmeUnknown = 
{ 0x00000000, 0x2013, 0x0114, { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef } };

/// IWmeUnknown interface
class WME_NOVTABLE IWmeUnknown
{
public:
    virtual ~IWmeUnknown() {}
	/// Add reference count
	/*!
		\return Reference count after add one
	*/
	virtual uint32_t WMECALLTYPE AddRef()=0;
	
	/// Subtract reference count
	/*!
		\return Reference count after subtract one
	*/
	virtual uint32_t WMECALLTYPE Release()=0;
	
	/// Query pointer of interface by REFWMEIID
	/*!
		\param idd : [in] REFWMEIID valude
		\param ppvObject: [out] Pointer of interface
		\return The error value of the funtion result
	*/
	virtual WMERESULT WMECALLTYPE QueryInterface(REFWMEIID iid, void **ppvObject)=0;
    
    /// Get reference count
    virtual uint32_t WMECALLTYPE GetReference()=0;
};

}

#endif // WME_UNKNOWN_INTERFACE_H
