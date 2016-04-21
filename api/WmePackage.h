///
///  WmePackage.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_PACKAGE_INTERFACE_H
#define WME_PACKAGE_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeDefine.h"

namespace wme
{

///============================IWmeMediaPackage==============================
/// IWmeMediaPackage interface
class WME_NOVTABLE IWmeMediaPackage : public IWmeUnknown
{
public:
	/// Get timestamp of the package
	/*!
		\param uTimestamp : [out] Reference of timestamp
		\return The error value of the function result
	*/
	virtual WMERESULT GetTimestamp(uint32_t &uTimestamp) = 0;

	/// Set timestamp of the package
	/*!
		\param uTimestamp : [in] Timestamp value
		\return The error value of the function result
	*/
	virtual WMERESULT SetTimestamp(uint32_t uTimestamp) = 0;

	/// Get media format type
	/*!
		\param eType : [out] Reference of media format type
		\return The error value of the function result
	*/
	virtual WMERESULT GetFormatType(WmeMediaFormatType &eType) = 0;

	/// Get size of format data
	/*!
		\param uSize : [out] Reference of size value
		\return The error value of the function result
	*/
	virtual WMERESULT GetFormatSize(uint32_t &uSize) = 0;

	/// Get pointer of format data
	/*!
		\param ppBuffer : [out] Reference of format data handle
		\return The error value of the function result
	*/
	virtual WMERESULT GetFormatPointer(unsigned char **ppBuffer) = 0;

	/// Get size of data buffer
	/*!
		\param uSize : [out] Reference of size value
		\return The error value of the function result
	*/
	virtual WMERESULT GetDataSize(uint32_t &uSize) = 0;

	/// Get data buffer
	/*!
		\param ppBuffer : [out] Pointer to the buffer
		\return The error value of the function result
	*/
	virtual WMERESULT GetDataPointer(unsigned char **ppBuffer) = 0;

	/// Get offset of data buffer
	/*!
		\param uOffset : [out] Reference of offset value
		\return The error value of the function result
	*/
    virtual WMERESULT GetDataOffset(uint32_t &uOffset) = 0;

	/// Get length of data buffer
	/*!
		\param uLength : [out] Reference of length value
		\return The error value of the function result
	*/
	virtual WMERESULT GetDataLength(uint32_t &uLength) = 0;

	/// Set offset of data buffer
	/*!
		\param uOffset : [in] new offset value
		\return The error value of the function result
	*/
    virtual WMERESULT SetDataOffset(uint32_t uOffset) = 0;

	/// Set length of data buffer
	/*!
		\param uLength : [in] new length value
		\return The error value of the function result
	*/
	virtual WMERESULT SetDataLength(uint32_t uLength) = 0;
    
    virtual ~IWmeMediaPackage(){};
    
    
};

///========================IWmeMediaPackageAllocator==========================
class WME_NOVTABLE IWmeMediaPackageAllocator : public IWmeUnknown
{
public:
	/// Allocate package with specified format size and data size
	/*!
		\param uFormatSize : [in] Size of format data
		\param uDataSize : [in] Size of buffer
		\param ppPackage : [out] Pointer to package handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetPackage(WmeMediaFormatType eFormatType, uint32_t uFormatSize, uint32_t uDataSize, IWmeMediaPackage **ppPackage) = 0;
};

}

#endif	// WME_PACKAGE_INTERFACE_H
