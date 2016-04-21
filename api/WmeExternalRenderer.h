///
///  WmeExternalRenderer.h
///
///
///  Created by Sand Pei on 2013-2-4.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_EXTERNAL_RENDERER_INTERFACE_H
#define WME_EXTERNAL_RENDERER_INTERFACE_H

#include "WmeDefine.h"
#include "WmeUnknown.h"
#include "WmePackage.h"

namespace wme
{

/// IWmeExternalOutputter interface
class IWmeExternalOutputter : public IWmeUnknown
{
public:
	/// Output media data in package format
	/*!
		\param pPackage : [in] Media package for input
		\return The error value of the function result
	*/
    virtual WMERESULT OutputMediaData(IWmeMediaPackage *pPackage) = 0;
    
protected:
    virtual ~IWmeExternalOutputter(){}
};

/// IWmeScreenViewer
typedef IWmeExternalOutputter	IWmeScreenViewer;

/// IWmeExternalRenderAnswerer interface
class IWmeExternalRenderAnswerer {
public:
	/// Request data, pull mode
	/*!
		\param uTimestamp : [out] Timestamp of the data
		\param eType : [out] Data format type
		\param pFormat : [out] Pointer to data format
		\param pData : [out] Buffer pointer
		\param uLength : [in,out] Buffer capacity, and buffer dirty length
		\return The error value of the function result
	*/
	virtual WMERESULT OnRequestData(uint32_t& uTimestamp, WmeMediaFormatType& eType, void* pFormat, unsigned char *pData, uint32_t& uLength) = 0;
};

/// IWmeExternalRenderer interface
class IWmeExternalRenderer
{
public:
	/// PUSH MODE:
	/// Query if support PUSH mode
	/*!
		\param isSupport : [out] Flag
		\return The error value of the function result
	*/
	virtual WMERESULT IsPushModeSupport(bool& isSupport) = 0;
	
    /// Render media in package format
	/*!
		\param pPackage : [in] Media data to be rendered
		\return The error value of the function result
		\note This is for PUSH mode only. If video raw is rendered, the WmeMediaFormatType should be WmeMediaFormatVideoRawWithStride.
	*/
    virtual WMERESULT RenderMediaData(IWmeMediaPackage *pPackage) = 0;

    /// Render media in data buffer format
	/*!
		\param uTimestamp : [in] Timestamp of the data
		\param eType : [in] Data format type
		\param pFormat : [in] Pointer to data format
		\param pData : [in] Buffer pointer
		\param uLength : [in] Buffer length
		\return The error value of the function result
		\note This is for PUSH mode only. If video raw is rendered, the WmeMediaFormatType should be WmeMediaFormatVideoRawWithStride.
	*/
    virtual WMERESULT RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength) = 0;

    /// PULL MODE:
    /// Query if support PULL mode
	/*!
		\param isSupport : [out] Flag
		\return The error value of the function result
	*/
	virtual WMERESULT IsPullModeSupport(bool& isSupport) = 0;

    /// Register answer for request callback
    /*!
		\param pAnswerer : [in] Request answerer handle
		\return The error value of the function result
		\note This is for PULL mode only. If answerer exists, it will be PULL mode. Once invoked, OnRequestData will come imediately
	*/
    virtual WMERESULT RegisterRequestAnswerer(IWmeExternalRenderAnswerer* pAnswerer) = 0;

 	/// Unregister answer for request callback
    /*!
		\return The error value of the function result
		\note This is for PULL mode only. If answerer exists, it will be PULL mode
	*/
    virtual WMERESULT UnregisterRequestAnswerer() = 0;
    
	virtual WmeMediaExternalRenderTypeConfig GetTypeExt() { return WmeExternalRender_Type_Normal; }
	virtual void SetTypeExt(WmeMediaExternalRenderTypeConfig eType) { return; }
protected:
    virtual ~IWmeExternalRenderer(){}
};

/// IWmeMediaFileRenderSink interface
class IWmeMediaFileRenderSink
{
public:
    /// Output media data in package format
    /*!
     \param uTimestamp : [out] Timestamp of the data
     \param eType : [out] Data format type
     \param pFormat : [out] Pointer to data format
     \param pData : [out] Buffer pointer
     \param uLength : [in,out] Buffer capacity, and buffer dirty length
     \return The error value of the function result
     */
    virtual WMERESULT OnOutputMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength) = 0;
};
    
/// IWmeMediaFileRenderer interface
class IWmeMediaFileRenderer : public IWmeUnknown, public IWmeExternalRenderer
{
public: 
	/// Set file path
	/*!
		\param pFilePath : [in] Path in string
		\param iPathLenght : [in] String length
		\return The error value of the function result
		\note The target file path to write in
	*/
    virtual WMERESULT SetTargetFile(char *pFilePath, int32_t iPathLength) = 0;

    /// Set media file config
	/*!
		\param eConfig : [in] Config value
		\param pValue : [in] Pointer to content, it depends on config
		\param uSize : [in] Size of content, it depends on config
        \return The error value of the function result
		\note For video, use push mode; For audio, use pull mode
	*/
	virtual WMERESULT SetConfig(WmeMediaFileConfig eConfig, void* pValue, uint32_t uSize) = 0;
    
    /// Set a data copy sink if users want to get the render data
    /*!
        \param pSink : [in] Handle to a sink
        \return The error value of the function result
        \note For video, you can get the rgb/yuv data; For audio, you can get the pcm data; Push mode only
     */
    virtual WMERESULT SetRenderSink(IWmeMediaFileRenderSink* pSink) = 0;

protected:
    virtual ~IWmeMediaFileRenderer(){}
};


}

#endif // WME_EXTERNAL_RENDERER_INTERFACE_H
