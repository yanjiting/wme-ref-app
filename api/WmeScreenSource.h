///
///  WmeScreenSource.h
///
///
///  Created by Sand Pei on 2013-1-24.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_SCREEN_SOURCE_INTERFACE_H
#define WME_SCREEN_SOURCE_INTERFACE_H

#include "WmeDefine.h"
#include "WmeUnknown.h"
#include "WmeEvent.h"
#include "WmePackage.h"

namespace wme
{

/// An enum of screen source type
typedef enum
{
    WmeScreenSourceTypeUnknow = 0,
	WmeScreenSourceTypeDesktop,				///< Desktop source type
	WmeScreenSourceTypeApplication,				///< Application source type
    WmeScreenSourceTypeContent                  ///< etc. for IOS share
}WmeScreenSourceType;



/// IWmeScreenSource interface
static const WMEIID WMEIID_IWmeScreenSource =  
{ 0x57dbe889, 0x8be, 0x4d71, { 0x80, 0x53, 0x64, 0x85, 0xf5, 0x2f, 0x96, 0xd6 } };

class WME_NOVTABLE IWmeScreenSource : public IWmeUnknown
{  
public:
	/// Get source type
	/*!
		\param eSourceType : [out] Source type of the screen source
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetSourceType(WmeScreenSourceType &eSourceType) = 0;

	/// Get unique name
	/*!
		\param szUniqueName : [out] Unique name of the source used as identifier in UTF-8
		\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetUniqueName(char* szUniqueName, int32_t &iLen) = 0;

	/// Get friendly name
	/*!
		\param szFriendName : [out] Easy to understand name of the source, may not be unique, in UTF-8
		\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetFriendlyName(char* szFriendName, int32_t &iLen) = 0;
    

    
    /// Get rectangle
	/*!
        \param stRect : [out] Screen source rectangle value
        \return The error value of the function result
        \note not thread safe
    */
    virtual WMERESULT GetRect(WmeRect &stRect) = 0;
    

    
    /// Check if the screen source is shared
	/*!
        \param bShared : [out] The share status
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT IsShared(bool &bShared) = 0;
	virtual void* GetSourceHandle() = 0;

protected:
    virtual ~IWmeScreenSource(){}
};

    
/// IWmeScreenSourceFilter interface
static const WMEIID WMEIID_IWmeScreenSourceFilter = 
{ 0x1eaf31b4, 0xd3d8, 0x420f, { 0x99, 0xad, 0xe5, 0xa3, 0x9d, 0x57, 0x0, 0xa1 } };

class WME_NOVTABLE IWmeScreenSourceFilter : public IWmeUnknown
{
public:
    /// Get filter name
	/*!
        \param szFilterName : [out] Filter name in UTF-8
        \param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
        \return The error value of the function result
        \note no implementation
    */
	virtual WMERESULT GetFilterName(char* szFilterName, int32_t &iLen) = 0;
    
    /// Get filter name
	/*!
        \param pScreenSource : [in] The pointer of screen source to filter
		\param bFiltered : [out] The filter result
        \return The error value of the function result
        \note no implementation
    */
    virtual WMERESULT Filter(IWmeScreenSource *pScreenSource, bool& bFiltered) = 0;
    
    virtual WMERESULT GetFilter(void ** ppFilter) = 0;
};
    
    
/// IWmeScreenSourceEnumerator interface
static const WMEIID WMEIID_IWmeScreenSourceEnumerator =
{ 0xd62940e9, 0xb60e, 0x4538, { 0xa6, 0x93, 0x7d, 0xee, 0xca, 0x11, 0x34, 0x8 } };


class WME_NOVTABLE IWmeScreenSourceEnumerator : public IWmeMediaEventNotifier
{
public:
    /// Get total number
    /*!
     \param iNumber : [out] Total number of application sources
     \return The error value of the function result
     \note not thread safe
     */
    virtual WMERESULT GetNumber(int32_t &iNumber) = 0;
    
    /// Get source by index
    /*!
     \param iIndex : [in] Index value
     \param ppSource : [out] Pointer to application source handle
     \return The error value of the function result
     \note not thread safe
     */
    virtual WMERESULT GetSource(int32_t iIndex, IWmeScreenSource **ppSource) = 0;
    
    
    virtual WMERESULT Refresh() = 0;
    
    
    virtual WMERESULT SetFilter(IWmeScreenSourceFilter *pFilter) = 0;
    
    
    
protected:
    virtual ~IWmeScreenSourceEnumerator(){}
};

typedef enum
{
    WmeScreenSourceUnknown = -1,
    WmeScreenSourceAdded = 0,
    WmeScreenSourceRemoved,
    WmeScreenSourceShared,
    WmeScreenSourceUnshared
} WmeScreenSourceEvent;

/// IWmeScreenSourceEnumeratorEventObserver interface
/// Notes: Now IWmeScreenSourceController is not supported to add IWmeScreenSourceControllerEventObserver, and it will be supported in subsequent release.
static const WMEIID WMEIID_IWmeScreenSourceEnumeratorEventObserver =
{ 0x152f4ee2, 0xc1af, 0x4dfe, { 0x86, 0xda, 0xde, 0x63, 0xe9, 0xdd, 0x2d, 0xad } };

class WME_EXPORT WME_NOVTABLE IWmeScreenSourceEnumeratorEventObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Notify screen source status changed event
	/*!
        \param pNotifier : [in] Caller's handle
        \param eChangedEvent : [in] The structure of screen source status changed event
        \return The error value of the function result
    */
	virtual WMERESULT OnScreenSourceChanged(IWmeMediaEventNotifier *pNotifier, WmeScreenSourceEvent event,
                                            IWmeScreenSource *pScreenSource) = 0;
};

}

#endif  // WME_SCREEN_SOURCE_INTERFACE_H
