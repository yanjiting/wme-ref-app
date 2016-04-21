///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///
///
///  @file WmeError.h
///  @brief WME Error Definitions

#ifndef WME_ERROR_DEFINE_H
#define WME_ERROR_DEFINE_H

#define WMERESULT           int32_t
///Check if a return code is sueccess
#define WME_SUCCEEDED(Status)	((int32_t)((Status) & 0x0000F000) == 0)
///check if a return code is failure
#define WME_FAILED(Status)		((int32_t)((Status) & 0x0000F000) != 0)

///Success Code
#define WME_S_OK							(0x00000000)
///Partial Success Code
#define WME_S_FALSE							(0x46000001)

///  Unspecified error
#define WME_E_FAIL							(0x46004001)

///  Ran out of memory
#define WME_E_OUTOFMEMORY					(0x46004002)

///  One or more arguments are invalid
#define WME_E_INVALIDARG					(0x46004003)

///  Not implemented
#define WME_E_NOTIMPL						(0x46004004)

///  No such interface supported
#define WME_E_NOINTERFACE					(0x46004005)

///  Invalid pointer
#define WME_E_POINTER						(0x46004006)

/// Deprecated feature
#define WME_E_DEPRECATED					(0x46004007)

///  Don't Init
#define WME_E_NO_INIT                       (0x46004008)

///  Device special error:
///  Video capture device(camera) access error
#define WME_E_VIDEO_CAMERA_FAIL             (0x46024101)
///  Video capture device(camera) is not authorized
#define WME_E_VIDEO_CAMERA_NOT_AUTHORIZED   (0x46024102)
///  There is no video capture device(camera)
#define WME_E_VIDEO_CAMERA_NO_DEVICE        (0x46024103)
///  Camera runtime fatal error(e.g camera service die)
#define WME_E_VIDEO_CAMERA_RUNTIME_DIE      (0x46024104)

/// Video encode fail
#define WME_E_VIDEO_ENCODE_FAIL             (0x46024104)

/// Video decode fail
#define WME_E_VIDEO_DECODE_FAIL             (0x46024105)



/// Auio sepcial error:
/// It hasn't any audio capture device
#define WME_E_AUDIO_NO_CAPTURE_DEVICE        (0x46014101)

/// It hasn't any audio playback device
#define WME_E_AUDIO_NO_PLAYBACK_DEVICE       (0x46014102)

/// It hasn't any audio capture device
#define WME_E_AUDIO_START_CAPTURE_FAILED     (0x46014103)

/// It hasn't any audio playback device
#define WME_E_AUDIO_START_PLAYBACK_FAILED    (0x46014104)


/// When setting device, it can't get the device from system
/// Suggest to use another device
#define WME_E_AUDIO_CANNT_USE_THIS_DEVICE	 (0x46014105)

/// Audio service abnormal, suggest to
/// 1. Set another device
/// 2. Exit APP, and restart APP
/// 3. if 1&2 doesn't work, please restart machine
#define WME_E_AUDIO_AUDIO_SERVICE_RUN_OUT	 (0x46014106)


/// can not create
#define WME_E_AUDIO_CREATE_TRANSPORT_FAILED      (0x46015101)
/// screen share capture fail
#define WME_E_SCREEN_SHARE_CAPTURE_FAIL                     (0x46034101)

/// shared display is plug-out,
/// WME User Action: show waring UI and let user make decision ( stop sharing ; or select other monitor. )
#define WME_E_SCREEN_SHARE_CAPTURE_DISPLAY_PLUGOUT          (0x46034102)

/// all shared application is removed, the current sharing app is zero,
/// WME User Action: show waring UI and let user make decision(select at least one app or stop sharing);
#define WME_E_SCREEN_SHARE_CAPTURE_NO_APP_SOURCE            (0x46034103)

/// add one source that is not exist
/// WME User Action: need check waht logic cause cached sources in client layer is not sync with sources of WME internal.
#define WME_E_SCREEN_SHARE_SOURCE_NOT_EXIST                 (0x46034104)

/// fail start capture withou adding source
/// WME User Action: need to add at least one source  in order to start screen capture.
#define WME_E_SCREEN_SHARE_NO_SOURCE                        (0x46034105)

///Mediasession layer error
#define WME_E_NEGOTIATION				(0x46004101)
///Error invalid status to call that interface
#define WME_E_INVALIDSTATUS				(0x46004102)
///Error, cannot find the target
#define WME_E_NOTFOUND                  (0x46004103)


#endif // WME_ERROR_DEFINE_H
