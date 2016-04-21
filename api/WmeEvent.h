///
///  WmeEvent.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_EVENT_INTERFACE_H
#define WME_EVENT_INTERFACE_H

#include "WmeUuid.h"
#include "WmeUnknown.h"
#include "WmeDefine.h"

namespace wme
{

class IWmeMediaEventNotifier;
///============================IWmeMediaEventNotifier============================
/// IWmeMediaEventObserver interface
static const WMEIID WMEIID_IWmeMediaEventObserver = 
{ 0x35dcf88a, 0xec09, 0x4850, { 0xb5, 0x5b, 0xb6, 0xc8, 0xae, 0x8c, 0x8f, 0xa9 } };

class WME_EXPORT WME_NOVTABLE IWmeMediaEventObserver
{
public:
	/// Event callback: observer will be added to notifier
	/*!
		\param pNotifer : [in] Caller's handle
		\param ulObserverID : [in] Unique ID of observer, for differentiate
		\return The error value of the function result
	*/
	virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) = 0;

	/// Event callback: observer will be removed from notifier
	/*!
		\param pNotifer : [in] Caller's handle
		\param ulObserverID : [in] Unique ID of observer, for differentiate
		\return The error value of the function result
	*/
	virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) = 0;
	//Any derived function should contain the parameter of IWmeMediaEventNotifier.
};

/// IWmeMediaEventNotifier interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeMediaEventNotifier = 
{ 0x218280e9, 0x33e3, 0x4719, { 0xaa, 0xba, 0xd6, 0x36, 0x2, 0x4e, 0xc8, 0x54 } };

class WME_NOVTABLE IWmeMediaEventNotifier : public /*virtual*/ IWmeUnknown
{
public:
	/// Add one observer to notifier
	/*!
		\param ulObserverID : [in] Unique ID of observer
		\param pObserver	: [in] Pointer to observer, to be added
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT AddObserver(REFWMEIID ulObserverID, IWmeMediaEventObserver *pObserver) = 0;
	
	/// Remove one observer from notifier
	/*!
		\param ulObserverID : [in] Unique ID of observer
		\param pObserver	: [in] Pointer to observer, to be removed
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT RemoveObserver(REFWMEIID ulObserverID, IWmeMediaEventObserver *pObserver) = 0;

	/// Remove one observer from notifier
	/*!
		\param pObserver	: [in] Pointer to observer, to be removed
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT RemoveObserver(IWmeMediaEventObserver *pObserver) = 0;

protected:
	/// The destructor function
	virtual ~IWmeMediaEventNotifier(){}
};


}	//namespace wme

#endif	// WME_EVENT_INTERFACE_H
