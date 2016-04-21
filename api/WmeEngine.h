///
///  WmeEngine.h
///
///
///  Created by Sand Pei on 2013-1-23.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_ENGINE_INTERFACE_H
#define WME_ENGINE_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeSession.h"
#include "WmeSyncBox.h"
#include "WmeTrack.h"
#include "WmeRender.h"
#include "WmeScreenSource.h"
#include "WmeCodec.h"
#include "WmeExternalCapturer.h"
#include "WmeMediaInfo.h"
#include "WmeNetworkIndicator.h"

namespace wme
{

static const WMEIID WMEIID_IWmeMediaEngine = 
{ 0xf6af2b6e, 0x60f4, 0x4eb1, { 0xa1, 0x31, 0x48, 0x66, 0x8c, 0x93, 0x1f, 0xfd } };

/// IWmeMediaEngine interface
class IWmeMediaEngine : public IWmeUnknown
{
public:
	/// Create media session 
	/*!
		\param ppMediaSession : [out] Pointer to the handle of media session
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateMediaSession(WmeSessionType eType, IWmeMediaSession **ppMediaSession) = 0;

    /// Create media sync box 
	/*!
		\param ppMediaSyncBox : [out] Pointer to the handle of media sync box
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateMediaSyncBox(IWmeMediaSyncBox **ppMediaSyncBox) = 0;

	/// Create local video track
	/*!
		\param ppLocalVideoTrack : [out] Pointer to the handle of local video track, which is used for sending video
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateLocalVideoTrack(IWmeLocalVideoTrack **ppLocalVideoTrack) = 0;

	/// Create local audio track
	/*!
		\param ppLocalAudioTrack : [out] Pointer to the handle of local audio track, which is used for sending audio
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateLocalAudioTrack(IWmeLocalAudioTrack **ppLocalAudioTrack) = 0;

	/// Create local video track of external source
	/*!
		\param ppLocalVideoExternalTrack : [out] Pointer to the handle of local video track, which is used for sending video
		\return The error value of the function result
		\note no implementation 
	*/
    virtual WMERESULT CreateLocalVideoExternalTrack(IWmeLocalVideoExternalTrack **ppLocalVideoExternalTrack, WmeSourceType eType) = 0;

	/// Create local audio track of external source
	/*!
		\param ppLocalAudioExternalTrack : [out] Pointer to the handle of local audio track, which is used for sending audio
		\return The error value of the function result
		\note no implementation
	*/
    virtual WMERESULT CreateLocalAudioExternalTrack(IWmeLocalAudioExternalTrack **ppLocalAudioExternalTrack) = 0;

	/// Create remote video track
	/*!
		\param ppRemoteVideoTrack : [out] Pointer to the handle of remote video track, which is used for receiving video
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateRemoteVideoTrack(IWmeRemoteVideoTrack **ppRemoteVideoTrack) = 0;

	/// Create remote audio track
	/*!
		\param ppRemoteAudioTrack : [out] Pointer to the handle of remote audio track, which is used for receiving audio
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateRemoteAudioTrack(IWmeRemoteAudioTrack **ppRemoteAudioTrack) = 0;	
	/// Create video preview track
	/*!
		\param ppVideoPreviewTrack : [out] Pointer to the handle of video preview track
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateVideoPreviewTrack(IWmeVideoPreviewTrack** ppVideoPreviewTrack) = 0;

    /// Create the default media device instance
	/*!
		\param eMediaType : [in] Media type
		\param eInOutType : [in] In/Out type
		\param ppDevice : [out] Pointer to the default device handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetSystemDefaultDevice(WmeMediaType eMediaType, 
                                             WmeDeviceInOutType eInOutType, 
                                             IWmeMediaDevice **ppDevice) = 0;

	/// Create enumerator of media devices
	/*!
		\param eMediaType : [in] Media type
		\param eInOutType : [in] In/Out type
		\param ppDeviceEnumerator : [out] Pointer to enumerator handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateMediaDeviceEnumerator(WmeMediaType eMediaType, 
                                                  WmeDeviceInOutType eInOutType, 
                                                  IWmeMediaDeviceEnumerator **ppDeviceEnumerator) = 0;

	/// Create notifier of media device
	/*!
		\param eMediaType : [in] Media type
		\param eInOutType : [in] In/Out type
		\param ppDevicesNotifier : [out] Pointer to notifier handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateMediaDevicesNotifier(WmeMediaType eMediaType, 
                                                 WmeDeviceInOutType eInOutType, 
                                                 IWmeMediaDeviceNotifier **ppDevicesNotifier) = 0;
    
    /// Create enumerator of media codec
	/*!
		\param eMediaType : [in] Media type
		\param ppCodecEnumerator : [out] Pointer to notifier handle
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateMediaCodecEnumerator(WmeMediaType eMediaType, 
                                                 IWmeMediaCodecEnumerator **ppCodecEnumerator) = 0;    
	
	/// Create media file capture
	/*!
		\param ppFileCapturer : [out] Pointer to media file capture handle
		\return The error value of the function result
	*/
    virtual WMERESULT CreateMediaFileCapturer(IWmeMediaFileCapturer **ppFileCapturer) = 0;

    /// Create media file renderer
	/*!
		\param ppFileRenderer : [out] Pointer to media file render handle
		\return The error value of the function result
	*/
    virtual WMERESULT CreateMediaFileRenderer(IWmeMediaFileRenderer **ppFileRenderer) = 0;


    /// Create audio device controller 
	/*!
		\param ppDeviceContr : [out] Pointer to audio device controller handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateAudioDeviceController(IWmeAudioDeviceController **ppDeviceContr) = 0;

	/// Create audio volume controller 
	/*!
		\param ppVolContr : [out] Pointer to audio volume controller handle
		\param iType : [in] Audio device in or out type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateAudioVolumeController(IWmeAudioVolumeController **ppVolContr ,WmeDeviceInOutType iType)=0;
    /// Get and Set audio engine info
    /*!
     \param WMEAUDIOENGINESTATUS*pStatusInfo: the struct of status to store the value
         int* if the infoType = WmeMediaInfo_AE_CaptureDataTimes
         int* if the infoType = WmeMediaInfo_AE_AECType
         bool* if the infoType = WmeMediaInfo_AE_VPIOMode
         WMEAE_QOEMSTATICS* if the infoTpe = WmeMediaInfo_AE_QoEM_Statics
         bool* if infoType = WmeMediaInfo_AE_QoEM_Enable
     \param int nInfoLen, the length of the pInfo, to check the pointer length to ensure no error
     \param WmeMediaInfo_AudioEngine infoType, indicate the type of audio engine info
         WmeMediaInfo_AE_CaptureDataTimes =0,//
         WmeMediaInfo_AE_AECType,        //for AEC using in audio engine,0:default engine,1:webrtc aec,2:tc aec,3:Alpha aec
         WmeMediaInfo_AE_VPIOMode,       ///for ios platform, is the caputre engine using VPIO mode
         WmeMediaInfo_AE_QoEM_Statics,   ///for QOEM, get the statics
         WmeMediaInfo_AE_QoEM_Enable,    ///for QOEM, enable or disable qoem function, the default value in
     \return value, 0:succeed, others:failed
     */
    virtual WMERESULT GetAudioEngineInfo(void* pInfo,int nInfoLen,WmeMediaInfo_AudioEngine infoType)=0;
    virtual WMERESULT SetAudioEngineInfo(void* pInfo,int nInfoLen,WmeMediaInfo_AudioEngine infoType)=0;
	/// Create media info getter
	/*!
		\param eMediaType : [in] Media type
		\param ppMediaInfoGetter : [out] Pointer to MediaInfoGetter handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateMediaInfoGetter(WmeMediaType eMediaType, IWmeMediaInfoGetter** ppMediaInfoGetter) = 0;

    /// Create network indicator 
    /*!
        \param ppNetworkIndicator : [out] Pointer to network indicator handle
        \return The error value of the function result
        \note thread safe
    */
    virtual WMERESULT CreateNetworkIndicator(IWmeNetworkIndicator ** ppNetworkIndicator) = 0;

	//>>>>>>>>Start Screen Sharing	
	///---------------------------------------------------------------------------------------------------------------------------------

	///=================================================================
	///Screen Tracker Interfaces
	///=================================================================
	///***************common content and data track***************///
	/// Create local screen share video track
	/*!
		\param ppWmeLocalScreenShareTrack : [out] Pointer to the handle of local screen share video track, which is used for sending screen video
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateLocalScreenShareTrack(IWmeLocalScreenShareTrack **ppWmeLocalScreenShareTrack) = 0;

	/// Create remote screen share video track
	/*!
		\param ppRemoteScreenShareTrack : [out] Pointer to the handle of remote screen share video track, which is used for receiving screen video
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT CreateRemoteScreenShareTrack(IWmeRemoteScreenShareTrack **ppRemoteScreenShareTrack) = 0;

	///***************Screen share user customize track **********///
	/// Create local screen share external track
	/*! user customize track, 
		1. external capturer to capture and feed data
			1.1 use API InputMediaData
				- get IWmeExternalInputter from IWmeLocalScreenShareExternalTrack;
				- capture data by self, 
				- package data into IWmeMediaPackage and call IWmeExternalInputter::InputMediaData(IWmeMediaPackage *pPackage)
			1.2	use 
		2. external CODEC to encode data
			
		3. external process to Pre- Post- data process 
			
		\param ppIWmeLocalScreenShareExternalTrack : [out] Pointer to the handle of remote screen share external track, which is used for receiving screen video
		\return The error value of the function result
		\note no implementation
	*/
	virtual WMERESULT CreateLocalScreenExternalTrack(IWmeLocalScreenShareExternalTrack **ppIWmeLocalScreenShareExternalTrack) = 0;

	///***************Screen share composite track that include multi tracks e.g. content and data track***************///
	/// Create local screen share track
	/*!
		\param bBindAdditionalData : [in] Need bind screen share addition data or not
		\param ppLocalScreenShareTrack : [out] Pointer to the handle of local screen share track, which is used for sending screen video and additional data
		\return The error value of the function result
		\note no implementation
	*/
    virtual WMERESULT CreateLocalCompositeScreenShareTrack(IWmeLocalCompositeScreenShareTrack **ppLocalScreenShareTrack) = 0;

	/// Create remote screen share track
	/*!
		\param bBindAdditionalData : [in] Need bind screen share addition data or not
		\param ppRemoteScreenShareTrack : [out] Pointer to the handle of remote screen share track, which is used for receiving screen video and additional data
		\return The error value of the function result
		\note no implementation
	*/
    virtual WMERESULT CreateRemoteCompositeScreenShareTrack(IWmeRemoteCompositeScreenShareTrack **ppRemoteScreenShareTrack) = 0;


	///=================================================================
	///Screen Source Interfaces
	///=================================================================
	/// Create screen source enumerator
	/*!
		\param ppScreenSourceEnum : [out] Pointer to screen source control handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT CreateScreenSourceEnumerator(IWmeScreenSourceEnumerator **ppScreenSourceEnum ,WmeScreenSourceType wmeScreenSourceType) = 0;
	
	///=================================================================
	///Screen Capturer Interfaces
	///=================================================================


	///---------------------------------------------------------------------------------------------------------------------------------
	///<<<<<<<<End Screen Sharing

	virtual WMERESULT GetSoundEffectState4Device(const IWmeMediaDevice &wmeDevice,bool &bChecked, long lReserver = 0) = 0;
	///Get sound effect state of the device.
	///Return:
	/// Succeed :	WBXAE_SUCCESS
	/// Failed:		WBXAE_ERROR_NOTIMPL 
	///				WBXAE_ERROR_KEYNOTFOUND
	///				WBXAE_ERROR_INVALID_VALUE
	///
	///Input:
	/// deviceID:	The device ID.
	/// lReserver:  Option type. Currently, only support 0 (Disable all enhancements)
	///
	///Output:
	///bChecked:    0: The option is unchecked.   1: The option is checked.
	///The API support windows only.
protected:
    virtual ~IWmeMediaEngine(){}
};



};
#endif // WME_ENGINE_INTERFACE_H
