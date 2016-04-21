///
///  WmeExternalCapturer.h
///
///
///  Created by Sand Pei on 2013-2-4.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_EXTERNAL_CAPTURER_INTERFACE_H
#define WME_EXTERNAL_CAPTURER_INTERFACE_H

#include "WmeDefine.h"
#include "WmeUnknown.h"
#include "WmePackage.h"
#include "WmeScreenSource.h"
#include "WmeStatistics.h"

namespace wme
{

/// IWmeExternalInputter interface
static const WMEIID WMEIID_IWmeExternalInputter = 
{ 0x169da7e6, 0x490d, 0x4eeb, { 0xbf, 0xc, 0x48, 0x60, 0xf9, 0xfe, 0xab, 0xfe } };

class IWmeExternalInputter : public IWmeUnknown
{
public:
	/// Input media data in package format
	/*!
		\param pPackage : [in] Media package for input
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT InputMediaData(IWmeMediaPackage *pPackage) = 0;

	/// Input media data in buffer format
	/*!
		\param uTimestamp : [in] Timestamp of the data
		\param eType : [in] Data format type
		\param pFormat : [in] Pointer to data format
		\param pData : [in] Buffer pointer
		\param uLength : [in] Buffer length
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT InputMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength) = 0;
    
protected:
    virtual ~IWmeExternalInputter(){}
};

/// An enum of external captuer status
typedef enum
{
    WmeExternalCapturerStatusUnknown = 0,		///< Unknown status
    WmeExternalCapturerStatusStarting,			///< Staring status, capturer is starting to capture
    WmeExternalCapturerStatusStarted,			///< Started status, capturer has started to capture
    WmeExternalCapturerStatusPaused,			///< Paused status, capturer has paused to capture 
    WmeExternalCapturerStatusResuming,			///< Resuming status, capturer is resuming to capture
    WmeExternalCapturerStatusStopping,			///< Stopping status, capture is stopping to capture
    WmeExternalCapturerStatusStopped,			///< Stopped status, capture has stopped to capture
    
    WmeExternalCapturerStatusError = 100		///< Error status, capture happen some error
}WmeExternalCapturerStatus;

/// IWmeExternalCapturer interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeExternalCapturer = 
{ 0x2b42e35f, 0xe1bf, 0x487e, { 0xa8, 0x96, 0x68, 0xfc, 0x9e, 0x13, 0xbe, 0xc9 } };


class IWmeExternalCapturer : public IWmeUnknown
{
public:
	virtual WMERESULT Capture(IWmeMediaPackage *pPackage) = 0;

protected:
	virtual ~IWmeExternalCapturer(){}
};
/// IWmeExternalCaptureEngine interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeExternalCaptureEngine = 
{ 0x4d93df3d, 0x3894, 0x4dcb, { 0xba, 0x8f, 0x4f, 0x48, 0xde, 0xf, 0xbe, 0x26 } };

class IWmeExternalCaptureEngine : public IWmeMediaEventNotifier
{
public:
	/// Set external inputter
	/*!
		\param eType : [in] Media type
		\param pInputter : [in] External inputter handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetExternalInputter(WmeMediaType eType, IWmeExternalInputter *pInputter) = 0;
    
	/// Start to capture media data
	/*!
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT Start() = 0;

	/// Stop to capture media data
	/*!
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT Stop() = 0;
    
    /// Pause to capture media data
	/*!
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT Pause() = 0;
    
	/// Resume to capture media data
	/*!
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT Resume() = 0;
    
    /// Get capturer status
	/*!
        \param eStatus : [out] Capturer status
        \return The error value of the function result
        \note not thread safe
    */
    virtual WMERESULT GetCapturerStatus(WmeExternalCapturerStatus &eStatus) = 0;

protected:
    virtual ~IWmeExternalCaptureEngine(){}
};


/// IWmeMediaFileCapturer interface
/*!	
	\note	Supported observers: IWmeMediaFileCapturerObserver
*/
static const WMEIID WMEIID_IWmeMediaFileCapturer = 
{ 0x59a75211, 0xb4d8, 0x4fc8, { 0xa3, 0x1c, 0xad, 0x3c, 0xcc, 0x2a, 0x39, 0xbf } };

class IWmeMediaFileCapturer : public IWmeExternalCaptureEngine
{    
public:
	/// Set file path
	/*!
		\param pFilePath : [in] Path in string
		\param iPathLenght : [in] String length
		\return The error value of the function result
		\note Raw data file path to read from
	*/
    virtual WMERESULT SetCaptureFile(char *pFilePath, int32_t iPathLength) = 0;
	
	/// Set media file config
	/*!
		\param eConfig : [in] Config value
		\param pValue : [in] Pointer to content, it depends on config
		\param uSize : [in] Size of content, it depends on config
		\note For some media file(eg: video YUV420), you have to tell WME the essential info, so that the file could be parsed right
	*/
	virtual WMERESULT SetConfig(WmeMediaFileConfig eConfig, void* pValue, uint32_t uSize) = 0;
    
protected:
    virtual ~IWmeMediaFileCapturer(){}
};
    
/// IWmeMediaFileCapturerObserver interface
static const WMEIID WMEIID_IWmeMediaFileCapturerObserver =
{ 0x98e5bcd4, 0x72aa, 0x5c25, { 0x8f, 0x73, 0xde, 0x82, 0x3e, 0xa0, 0x8d, 0x18 } };

class WME_EXPORT WME_NOVTABLE IWmeMediaFileCapturerObserver : virtual public IWmeMediaEventObserver
{
public:
    
    /// Handle EOS
    /*!
     \param pCapturer : [in] The media file capturer who meet EOS
     \return The error value of the function result
     */
    virtual WMERESULT OnEndOfStream(IWmeMediaFileCapturer* pCapturer) = 0;
};

//==================================================================
//Screen Capturer



/// IWmeExternalCapturer interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeScreenCapturer = 
{ 0x2b42e35f, 0xe1bf, 0x487e, { 0xa8, 0x96, 0x68, 0xfc, 0x9e, 0x13, 0xbe, 0xc9 } };


class IWmeScreenCapturer : public IWmeUnknown
{
public:
	virtual WMERESULT Capture(IWmeMediaPackage *pPackage) = 0;
    virtual WMERESULT GetCurrentCaptureSourceSize(int32_t &nWidth,int32_t &nHeight) = 0;
    virtual WMERESULT GetCurrentCaptureSize(int32_t &nWidth,int32_t &nHeight) = 0;
    virtual WMERESULT SetCaptureMaxSize(const int32_t nWidth,const int32_t nHeight) = 0;

protected:
	virtual ~IWmeScreenCapturer(){}
};

/// IWmeScreenCaptureEngine interface
/*!	
	\note	Supported observers: IWmeScreenCaptureEngineEventObserver
*/
static const WMEIID WMEIID_IWmeScreenCaptureEngine =
{ 0xe5aacb2b, 0x7585, 0x4366, { 0x9c, 0x7e, 0x4b, 0xbe, 0x40, 0x7b, 0x6f, 0x19 } };

class IWmeScreenCaptureEngine : public IWmeExternalCaptureEngine
{
public:
	/// Add screen source
	/*!
		\param pSource : [in] Screen source handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT AddScreenSource(IWmeScreenSource *pSource) = 0;

	/// Remove screen source
	/*!
		\param pSource : [in] Screen source handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT RemoveScreenSource(IWmeScreenSource *pSource) = 0;	
    
    virtual WMERESULT SetFilterSelf(bool bFilterSelf) = 0;

	virtual WMERESULT AddSharedWindow(void* handle) = 0;
	virtual WMERESULT RemoveSharedWindow(void* handle) = 0;
	/// Set external capturer
	/*!
		\param pIWmeScreenCapturer : [in] Media type
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetExternalCapturer(IWmeScreenCapturer * pIWmeScreenCapturer) = 0;

	/// Check whether desktop is sharing in the screen capturer 
	/*!
        \param bSharing : [out] Sharing result
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT IsDesktopSharing(bool &bSharing) = 0;
    
    /// Check whether the application is sharing in the screen capturer
	/*!
        \param bSharing : [out] Sharing result
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT IsApplicationShared(WmeWindowHandle hWindow, uint32_t uProcessID, bool &bSharing) = 0;
    
    /// Get total number of shared applications
	/*!
        \param iNumber : [out] Total number of shared application sources
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT GetSharedApplicationNumber(int32_t &iNumber) = 0;
    
	virtual WMERESULT GetShareCapturerStatistics(WmeScreenShareCaptureStatistics & stWmeScreenShareCaptureStatistics) = 0;
    virtual WMERESULT SetCaptureFps(uint32_t nFps) = 0;
    virtual WMERESULT SetCaptureMaxFrameSize(uint32_t nWidth,uint32_t nHeight) = 0;
    virtual WMERESULT GetCaptureFps() = 0;
	virtual WMERESULT Refresh() = 0;
protected:
    virtual ~IWmeScreenCaptureEngine(){}
};
/// [Remove end]
    
/// IWmeScreenCaptureEngineEventObserver interface
static const WMEIID WMEIID_IWmeScreenCaptureEngineEventObserver = 
{ 0x4924fe6b, 0x2195, 0x47ea, { 0xb8, 0x76, 0x57, 0x3f, 0x2e, 0x4, 0x42, 0x12 } };

class WME_EXPORT WME_NOVTABLE IWmeScreenCaptureEngineEventObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Notify screen captuer status changed event
	/*!
        \param pNotifier : [in] Caller's handle
        \param eCapturerStatus : [in] Screen capture status
        \return The error value of the function result
    */
    virtual WMERESULT OnCaptureStatusChanged(IWmeMediaEventNotifier *pNotifier, WmeExternalCapturerStatus eCapturerStatus) = 0;
    
	/// Notify screen capture size changed event
	/*!
        \param pNotifier : [in] Caller's handle
        \param stCaptureSize : [in] Screen capture size
        \return The error value of the function result
    */
    virtual WMERESULT OnCaptureDisplaySizeChanged(IWmeMediaEventNotifier *pNotifier, WmeSize &stCaptureSize) = 0;

	/// Notify screen capture data changed event
	/*!
        \param pNotifier : [in] Caller's handle
        \param pIWmeMediaPackage : [in] Screen capture size
        \return The error value of the function result
    */
    virtual WMERESULT OnCaptureData(IWmeMediaEventNotifier *pNotifier,IWmeMediaPackage *pIWmeMediaPackage) = 0;
    
    virtual WMERESULT OnError(IWmeMediaEventNotifier *pNotifier, uint32_t nErrorCode) = 0;
};


//==================================================================
}

#endif // WME_EXTERNAL_CAPTURER_INTERFACE_H
