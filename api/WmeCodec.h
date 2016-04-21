///
///  WmeCodec.h
///
///
///  Created by Sand Pei on 2013-1-24.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_CODEC_INTERFACE_H
#define WME_CODEC_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeDefine.h"

namespace wme
{

/// IWmeMediaCodec interface
static const WMEIID WMEIID_IWmeMediaCodec = 
{ 0xedcf1e6c, 0xbd54, 0x4134, { 0x9c, 0x8c, 0x32, 0x87, 0x9c, 0x56, 0x72, 0xa2 } };
class WME_NOVTABLE IWmeMediaCodec : public IWmeUnknown
{
public:
	/// Get media type
	/*!
		\param eMediaType : [out] Reference of WmeMediaType
		\return The error value of the funtion result
		\note thread safe
	*/
	virtual WMERESULT GetMediaType(WmeMediaType &eMediaType) = 0;
	
	/// Get codec type
	/*!
		\param eCodecType : [out] Reference of WmeCodecType
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetCodecType(WmeCodecType &eCodecType) = 0;
	
	/// Get codec name
	/*!
		\param szCodecName : [out] String of codec name in UTF-8
		\param iLen : [out] Input the max length and output the occupied length (not including the terminating null character)
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetCodecName(char* szCodecName, int32_t &iLen) = 0;
	
	/// Get RTP payload type
	/*!
		\param uPayloadType : [out] RTP payload type
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetPayloadType(uint32_t &uPayloadType) = 0;
	
	/// Get RTP clock rate
	/*!
		\param uClockRate : [out] Clock rate
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetClockRate(uint32_t &uClockRate) = 0;
    
	virtual WMERESULT SetCodecBandwidth(uint32_t ubps) = 0;
	virtual WMERESULT GetCodecBandwidth(uint32_t &ubps) = 0;
protected:
	/// The destructor function
    virtual ~IWmeMediaCodec(){}
};

/// IWmeMediaCodecEnumerator interface
static const WMEIID WMEIID_IWmeMediaCodecEnumerator = 
{ 0xd299d752, 0x5cf1, 0x40de, { 0x88, 0x81, 0x5, 0x22, 0x3c, 0xd, 0x55, 0x48 } };
class WME_NOVTABLE IWmeMediaCodecEnumerator : public IWmeUnknown
{
public:
	/// Get number of supported codecs
	/*!
		\param iNumber : [out] Number of codecs
		\return The error value of the funtion result
		\note not thread safe for audio codec enumerator and thread safe for video enumerator
	*/
	virtual WMERESULT GetNumber(int32_t &iNumber) = 0;
	
	/// Get codec specified by index
	/*!
		\param iIndex : [in] Index of codec, according to the GetNumber
		\param ppCodec : [out] Pointer of codec handle
		\return The error value of the funtion result
		\note not thread safe for audio codec enumerator and thread safe for video enumerator
	*/
	virtual WMERESULT GetCodec(int32_t iIndex, IWmeMediaCodec **ppCodec) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeMediaCodecEnumerator(){}
};

}

#endif // WME_CODEC_INTERFACE_H
