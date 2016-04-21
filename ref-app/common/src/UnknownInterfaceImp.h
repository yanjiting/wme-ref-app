///
///  UnknownInterfaceImp.h
///
///
///  Created by ziyzhang@cisco.com on 2013-5-3.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef UNKNOWN_INTERFACE_IMP_H
#define UNKNOWN_INTERFACE_IMP_H

#include "WmeUnknown.h"
#include "WmeError.h"
#include "WmeCommonTypes.h"

//#include "WseMutex.h"		//Should be replaced by Whitney mutex
#include "CmMutex.h"

using namespace wme;

class CUnknownInterfaceImp :
	public IWmeUnknown
{
public:
	CUnknownInterfaceImp():m_uRef(0){}
	virtual ~CUnknownInterfaceImp(){}
	WMEMETHOD_(uint32_t,AddRef)()
	{
#ifdef WIN32
		uint32_t uRef = InterlockedIncrement((volatile long *)&m_uRef);
#else
		CCmMutexGuardT<MutexType> theGuard(m_Mutex);
		uint32_t uRef = ++m_uRef;
		theGuard.UnLock();
#endif
		return uRef;
	}
	WMEMETHOD_(uint32_t,Release)()
	{

#ifdef WIN32
		uint32_t uRef = InterlockedDecrement((volatile long *)&m_uRef);
#else
		CCmMutexGuardT<MutexType> theGuard(m_Mutex);
		uint32_t uRef = --m_uRef;
		theGuard.UnLock();
#endif
		if(0 == uRef)
		{
			++m_uRef;
			delete this;
			return 0;
		}
		return uRef;
	}
	WMEMETHOD_(WMERESULT,QueryInterface)(REFWMEIID iid,void **ppvObject)
	{
		if(ppvObject == NULL)
			return WME_E_POINTER;
		if(IsEqualWMEUUID(iid,WMEIID_IWmeUnknown)) {

			*ppvObject = dynamic_cast<IWmeUnknown*>(this);
			( dynamic_cast<IWmeUnknown*>(this))->AddRef();
			return WME_S_OK;

		} else {
			*ppvObject = NULL;
			return WME_E_NOINTERFACE;
		}
	}
protected:
	volatile uint32_t m_uRef;

#ifndef WIN32
	typedef CCmMutexThreadRecursive MutexType;
	MutexType m_Mutex;
#endif
};

#define IMPLEMENT_UNKNOWN_REFERENCE												\
	WMEMETHOD_(uint32_t,AddRef)(){return CUnknownInterfaceImp::AddRef();};		\
	WMEMETHOD_(uint32_t,Release)(){return CUnknownInterfaceImp::Release();};	\
	WMEMETHOD_(WMERESULT,QueryInterface)(REFWMEIID iid,void **ppvObject){return CUnknownInterfaceImp::QueryInterface(iid,ppvObject);};


#endif //WME_UNKNOWN_IMP_H