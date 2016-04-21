///
///  WmeSyncBox.h
///
///
///  Created by Sand Pei on 2013-1-23.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_SYNC_BOX_INTERFACE_H
#define WME_SYNC_BOX_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeTrack.h"
#include "WmeStatistics.h"

namespace wme
{

static const WMEIID WMEIID_IWmeMediaSyncBox = 
{ 0x5b4df442, 0x82eb, 0x4379, { 0x92, 0xc1, 0xcf, 0x43, 0x2b, 0xcd, 0xc7, 0xa3 } };

/// IWmeMediaSyncBox interface
class IWmeMediaSyncBox : public IWmeUnknown
{
public:
	/// Get stream label
	/*!
		\param uLabel : [out] Reference of stream label which must be unique
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetLabel(uint32_t &uLabel) = 0;

	/// Set stream label
	/*!
		\param uLabel : [in] Stream label which must be unique
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetLabel(uint32_t uLabel) = 0;
    
	/// Add a track to stream
	/*!
		\param pTrack : [in] Media track handle
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT AddTrack(IWmeMediaTrack *pTrack) = 0;
	
	/// Remove a specified track from stream
	/*!
		\param pTrack : [in] Media track handle
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT RemoveTrack(IWmeMediaTrack *pTrack) = 0;

	/// Find the track, specified by label
	/*!
		\param pbFound : [out] Indicate found or not
		\param ppTrack : [out] Pointer to media track handle. If found, overwrite the handle
		\param uLabel : [in] Label to identify a track
		\param type : [in] Option to identify audio, video or other track type. If don't know the type, set it to WmeTrackType_Uknown
		\return The error value of the function result
	*/
	//virtual WMERESULT FindTrack(bool* pbFound, IWmeMediaTrack** ppTrack, uint32_t uLabel, WmeTrackType type) = 0;
    
    /// Get statistics of sync box
	/*!
        \param stStat : [out] reference of WmeSyncStatistics
        \return The error value of the funtion result
	 */
	virtual WMERESULT GetStatistics(WmeSyncStatistics &stStat) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeMediaSyncBox(){}
};

}

#endif // WME_SYNC_BOX_INTERFACE_H
