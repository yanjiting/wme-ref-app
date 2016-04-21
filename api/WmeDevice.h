///
///  WmeDevice.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_DEVICE_INTERFACE_H
#define WME_DEVICE_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeEvent.h"
#include "WmeDefine.h"

namespace wme
{

///=============================IWmeMediaDevice============================
/// Enum of device input/output type
typedef enum
{
	WmeDeviceIn=0,			///< Input type
	WmeDeviceOut			///< Output type
}WmeDeviceInOutType;

/// Enum of device capability type
typedef enum
{
	WmeDeviceCapabilityAudio,		///< Audio only device
	WmeDeviceCapabilityVideo		///< Video only device
}WmeDeviceCapabilityType;
    
/// Enum of device changed notification type
typedef enum
{
    WmeDefaultDevice  = 0,
    WmeHeadsetPlugin,     ///Audio only Android platform device changed notification
    WmeHeadsetPlugout,    ///Audio only Android platform device changed notification
    WmeBluetoothHeadsetPlugin,   ///Audio only Android platform device changed notification
    WmeBluetoothHeadsetPlugout,  ///Audio only Android platform device changed notification
}WmeDeviceNotification;

typedef struct
{ 
    wme::WmeVideoRawType type;
	unsigned long width;
	unsigned long height;
	float MinFPS;
	float MaxFPS;
} WmeCameraCapability;

/// Struct of device capability
typedef struct _tagDeviceCapability
{
    wme::WmeDeviceCapabilityType eType;	///< Device capability type
	int iSize;						///< Size of data structure
	void *pCapalibity;				///< Poiter of data structure of specific capability
}WmeDeviceCapability;

/// IWmeMediaDevice interface
static const WMEIID WMEIID_IWmeMediaDevice = 
{ 0x69678db9, 0x7bfe, 0x484b, { 0xbe, 0x1, 0x17, 0x7e, 0x7d, 0x30, 0x47, 0x35 } };

// {D8779451-1C00-4986-AB32-AC5C0C395A62}
static const WMEIID WMEIID_IWmeMediaDeviceProperty =
{ 0xd8779451, 0x1c00, 0x4986, { 0xab, 0x32, 0xac, 0x5c, 0xc, 0x39, 0x5a, 0x62 } };


class WME_NOVTABLE IWmeMediaDeviceProperty : public IWmeUnknown
{
public:
    virtual WMERESULT CanShowProperties() = 0;
    virtual WMERESULT ShowProperties(char* szTitle, int iLen, void* hParent) = 0;
};

class WME_NOVTABLE IWmeMediaDevice : public IWmeUnknown
{
public:
	/// Get corresponding media type
	/*!
		\param eMediaType : [out] Reference of media type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetMediaType(WmeMediaType &eMediaType) = 0;

	/// Get input/output type
	/*!
		\param eInOutType : [out] Reference of input/output type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetInOutType(WmeDeviceInOutType &eInOutType) = 0;

	/// Get unique name
	/*!
		\param szUniqueString : [in] Unique name of the device in UTF-8
		\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetUniqueName(char* szUniqueName, int32_t &iLen) = 0;

	/// Get friendly name
	/*!
		\param szFriendName : [in] Easy to understand name of the device in UTF-8
		\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetFriendlyName(char* szFriendName, int32_t &iLen) = 0;


	/// Get short name, , only for audio on Windows
	/*!
	\param szShortName : [in] Easy to understand short name of the device in UTF-8,
	\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
	\return The error value of the function result
	\note not thread safe
	*/
	virtual WMERESULT GetShortName(char* szShortName, int32_t &iLen) = 0;


	/// Get interface ID, only for audio on Windows
	/*!
	\param szInterfaceID : [in] It's interface ID in system,.
	\param iLen : [in/out] Input the max length and output the occupied length (not including the terminating null character)
	\return The error value of the function result
	\note not thread safe
	*/
	virtual WMERESULT GetInterfaceID(char* szInterfaceID, int32_t &iLen) = 0;


	/// Get DeviceID
	/*!
	\param deviceID : [in/out] device ID.
	\return The error value of the function result
	\note not thread safe
	*/
	virtual WMERESULT GetDeviceID( int32_t &deviceID) = 0;


	/// Get device capabilities
	/*!
		\param iIndex : [in] Index for enumeration
		\param pCapability : [out]	Capability result 
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetCapabilities(int32_t iIndex, WmeDeviceCapability *pCapability) = 0;

	/// Check if another device is same with me
	/*!
		\param pDevice : [in] Another device
		\return :
		- WME_S_OK means true without error
		- WME_S_FALSE means false without error
		- WME_E_XXX is error code
		\note not thread safe
	*/
	virtual WMERESULT IsSameDevice(IWmeMediaDevice* pDevice) = 0;

	/// Check if they are the same device by unique name
	/*!
		\param szUniqueName : [in] Char array of unique name in UTF-8
		\param iLen : [in] Length of string (not including the terminating null character)
		\return :
		- WME_S_OK means true without error
		- WME_S_FALSE means false without error
		- WME_E_XXX is error code
		\note not thread safe
	*/
	virtual WMERESULT IsSameDevice(char* szUniqueName, int32_t iLen) = 0;

protected:
	/// The destructor function
	virtual ~IWmeMediaDevice(){}
};


///=========================IWmeMediaDeviceEnumerator=========================
/// IWmeMediaDeviceEnumerator interface
static const WMEIID WMEIID_IWmeMediaDeviceEnumerator = 
{ 0x32a2e7e3, 0x9b0a, 0x40ba, { 0xb0, 0x65, 0x98, 0x1f, 0x1f, 0xe2, 0xed, 0x22 } };

class WME_NOVTABLE IWmeMediaDeviceEnumerator : public IWmeUnknown
{
public:
	/// Get the total number of devices
	/*!
		\param iNumber : [out] Reference of number
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetDeviceNumber(int32_t &iNumber) = 0;

	/// Get device pointer by index
	/*!
		\param iIndex : [in] Index of device array
		\param ppDevice : [out] Pointer to device handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetDevice(int32_t iIndex, IWmeMediaDevice **ppDevice) = 0;

protected:
	virtual ~IWmeMediaDeviceEnumerator(){}
};

///==========================IWmeMediaDeviceNotifier==========================
/// IWmeMediaDeviceNotifier Event
typedef enum 
{
    WmeDeviceUnknownChange = -1,    ///< Unknown device change
    
    WmeDeviceAdded = 0,				///< Device is added
	WmeDeviceRemoved = 1,			///< Device is removed
	WmeDefaultDeviceChanged = 2,	///< System Default device is changed.
    WmeDeviceInterruptionBegin = 3, ///< Begin device interruption,dont include device info, only for IOS
    WmeDeviceInterruptionEnd = 4,   ///< End device interruption,dont include device info, only for IOS
    WmeDeviceSuspended = 5,         ///< Device become suspended, only for Mac
    WmeDeviceUnsuspended = 6,       ///< Device become unsuspended, only for Mac
}WmeDeviceChangeType;

typedef struct _tagEventDataDeviceChanged
{
	char*	szUniqueName;			///< Unique name of device to identify which device is target
	int32_t	iNameLen;				///< Valid length of unique name
    WmeDeviceChangeType iType;		///< Device change type
}WmeEventDataDeviceChanged;

/// IWmeMediaDeviceNotifier Interface
/*!	
	\note	Supported observers: IWmeMediaDeviceObserver
*/
typedef IWmeMediaEventNotifier IWmeMediaDeviceNotifier;

/// IWmeMediaDeviceObserver Interface
const WMEIID WMEIID_IWmeMediaDeviceObserver = 
{ 0x487a71f3, 0x7f7, 0x4d1e, { 0x91, 0x97, 0x6b, 0x96, 0x5d, 0x79, 0x26, 0x1c } };

class WME_EXPORT WME_NOVTABLE IWmeMediaDeviceObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Notify device changed event to observer(application)
	/*!
		\param pNotifier : [in] Caller's handle
		\param stChangeEvent : [in] A event data struct of device changed
		\return The error value of the function result
	*/
	virtual WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &stChangeEvent) = 0;

};
    
///==========================IWmeMediaDeviceSuspended==========================
/// IWmeMediaDeviceSuspended Interface
static const WMEIID WMEIID_IWmeMediaDeviceSuspended =
{ 0xfe6d2a6d, 0x7ba0, 0x46d3, { 0xaa, 0xaa, 0xd4, 0xc6, 0xe4, 0x39, 0x7f, 0xd2 } };
    
class WME_NOVTABLE IWmeMediaDeviceSuspended : public IWmeUnknown
{
public:
    /// Get device's suspended flag - true or false
    /*!
         \param bSuspended : [out] The output device suspended flag
         \return The error value of the function result
         \note thread safe
     */
    virtual WMERESULT GetSuspended(bool &bSuspended) = 0;
    
protected:
    /// The destructor function
    virtual ~IWmeMediaDeviceSuspended(){}
};

///==========================IWmeVideoCapDevicePosition==========================
typedef enum
{
	WME_VIDEO_CAP_DEVICE_POSITION_UNKNOWN = 0,
	WME_VIDEO_CAP_DEVICE_POSITION_BACK,
	WME_VIDEO_CAP_DEVICE_POSITION_FRONT,
}WmeVideoCapDevicePosition;

/// IWmeVideoCapDevicePosition Interface
static const WMEIID WMEIID_IWmeVideoCapDevicePosition = 
{ 0xc404750e, 0xcf15, 0x434f, { 0xbf, 0x7e, 0xb9, 0xea, 0xac, 0x53, 0x7f, 0xa7 } };

class WME_NOVTABLE IWmeVideoCapDevicePosition : public IWmeUnknown
{
public:
	/// Get device's position - front or back
	/*!
		\param position : [out] Pointer to the output device position
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetPosition(WmeVideoCapDevicePosition* position)=0;

protected:
	/// The destructor function
	virtual ~IWmeVideoCapDevicePosition(){}
};

///==========================IWmeAudioDeviceController==========================
/// IWmeAudioDeviceController Interface
static const WMEIID WMEIID_IWmeAudioDeviceController = 
{ 0x233965ea, 0x805a, 0x4a22, { 0x86, 0x78, 0xb8, 0x5e, 0xed, 0x6, 0x69, 0xe } };

class WME_NOVTABLE IWmeAudioDeviceController : public IWmeUnknown
{
public:
	/// Set the device for capture
	/*!
		\param pDevice : [in] Pointer to device
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetCaptureDevice(IWmeMediaDevice *pDevice) = 0;	

	/// Get the device for capture
	/*!
		\param ppDevice : [out] Pointer to the handle of device
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetCaptureDevice(IWmeMediaDevice **ppDevice) = 0;

	/// Set audio format for capture
	/*!
		\param pFormat : [in] Pointer to audio format
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetCaptureFormat(WmeAudioRawFormat *pFormat) = 0;

	/// Get audio format for capture
	/*!
		\param format : [out] Reference of audio format
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetCaptureFormat(WmeAudioRawFormat &format) = 0;
    
	/// Set the device for play
	/*!
		\param pDevice : [in] Pointer to device
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetPlayDevice(IWmeMediaDevice *pDevice) = 0;

	/// Get the device for play
	/*!
		\param ppDevice : [out] Reference of device handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetPlayDevice(IWmeMediaDevice **ppDevice) = 0;

	/// Set audio play format
	/*!
		\param pFormat : [in] Pointer to audio format
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetPlayFormat(WmeAudioRawFormat *pFormat) = 0;

	/// Get audio play format
	/*!
		\param format : [out] Reference of audio format
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetPlayFormat(WmeAudioRawFormat &format) = 0;
    
    ///add by Ark for Audio device changed notifycation
    /// Notificate the device changed message to wme
    /*!
     \WmeDeviceChangedNotification notificationType): [in] device changed tpye enum
     \return The error value of the function result
     */
    virtual WMERESULT DeviceChangedNotification(WmeDeviceNotification notificationType,int deviceValue = 0, int moreValue = 0) = 0;

protected:
	/// The destructor function
	virtual ~IWmeAudioDeviceController(){}
};

///==========================IWmeSysDefaultDevice==========================
/// IWmeSysDefaultDevice Interface
static const WMEIID WMEIID_IWmeSysDefaultDevice = 
{ 0x49c5b0b7, 0xfbfb, 0x4b0e, { 0xbf, 0xf0, 0x89, 0xab, 0xfe, 0xb1, 0x11, 0x70 } };

class WME_NOVTABLE IWmeSysDefaultDevice : public IWmeUnknown
{
public:
	//Get default device

	/*!
		\param ppDevice : [out] Reference of device handle		
		\param eInOutType : [in] Reference of input/output type
		\return The error value of the function result
		\note not thread safe
	*/
	 virtual WMERESULT GetDefaultDeice(IWmeMediaDevice **ppDevice,WmeDeviceInOutType eInOutType) = 0;

protected:
	/// The destructor function
	virtual ~IWmeSysDefaultDevice(){}
};

///==========================IWmeAudioVolumeNotifier==========================
/// IWmeMediaDeviceNotifier Event
typedef enum 
{
	WmeAudioVolumeChange = 0,	///< Device is added
	WmeAudioVolumeMute = 1		///< Device is removed
}WmeAudioVolumeChangeType;

typedef enum
{
	WME_VOL_APPLICATION = 0,
	WME_VOL_SYSTEM,
    WME_VOL_DIGITAL
}WmeAudioVolumeCtrlType;

typedef struct _tagEventDataVolumeChanged
{
    int nVolume;							//the changed volume
    bool bMute;		
    WmeAudioVolumeChangeType iChangedType;	// volume change type:volume,mute
    WmeDeviceInOutType iDeviceType;			//in or out device
	WmeAudioVolumeCtrlType iVolumeCrtlType;
}WmeEventDataVolumeChanged;

/// IWmeAudioVolumeNotifier interface
/*!	
	\note	Supported observers: IWmeAudioVolumeObserver
*/
static const WMEIID WMEIID_IWmeAudioVolumeNotifier= 
{ 0x6f8bc1c, 0x4edd, 0x4e9b, { 0x97, 0x35, 0x28, 0x94, 0xbf, 0x48, 0x31, 0x8f } };
class WME_NOVTABLE IWmeAudioVolumeNotifier : /*virtual*/ public IWmeMediaEventNotifier
{
public:

protected:
	/// The destructor function
	virtual ~IWmeAudioVolumeNotifier(){}
};

/// IWmeAudioVolumeObserver interface
static const WMEIID WMEIID_IWmeAudioVolumeObserver = 
{ 0x54db6dcc, 0xfe27, 0x4121, { 0xa2, 0xc3, 0x8, 0xa8, 0x64, 0xc2, 0xb2, 0x1d } };

class WME_EXPORT WME_NOVTABLE IWmeAudioVolumeObserver: virtual public IWmeMediaEventObserver
{
public:
	/// Notify audio device volume changed event to observer(application)
	/*!
		\param pNotifier : [in] Caller's handle
		\param stChangeEvent : [in] A event data struct of volume changed
		\return The error value of the function result
	*/
	virtual void OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &stChangeEvent) = 0;
};

///==========================IWmeAudioVolumeController==========================


typedef struct WmeAudioMixerInfo
{
	unsigned int  dwLineType;								//Can be WbxDestLineType
	unsigned int  dwLineCount;								//The count of active source line in this mixer device
	unsigned int dwMixerID;									//Mixer ID for application if it wants to control it in advance.
//	WBXAELINEINFO lineInfo[WBXAE_MAXLINE_COUNT];			//Detail line information.
	unsigned int  dwMaxLineCount;							//Total source line count
}WMEAUDIOMIXERINFO, *LPWMEAUDIOMIXERINFO;

/// IWmeAudioVolumeController interface
static const WMEIID WMEIID_IWmeAudioVolumeController = 
{ 0xa078e16d, 0x3240, 0x4fcf, { 0xb8, 0x6e, 0x31, 0xf5, 0xc0, 0x26, 0xf2, 0xf8 } };

class WME_NOVTABLE IWmeAudioVolumeController:public IWmeUnknown
{
public:
	 //Get the binded device
	 /*
		\note not thread safe
	 */        
    virtual void GetDevice(IWmeMediaDevice **ppDevice) = 0;

	/// set volume
	/*!
		\param nVolumeValue : [in] the volume value range[0,65535]
		\return value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetVolume(unsigned int nVolumeValue,WmeAudioVolumeCtrlType nType = WME_VOL_APPLICATION) = 0;
	
	/// get volume
	/*!
		\param void
		\return the volume value, 0~65535
		\note not thread safe
	*/
    virtual unsigned int GetVolume(WmeAudioVolumeCtrlType nType = WME_VOL_APPLICATION) = 0;
	
	/// set the volume mute
	/*!
		\param WmeDeviceInOutType nType:[in] the volume controler type
		   WME_VOL_APPLICATION  or  WME_VOL_SYSTEM
		\return the set result
		\note not thread safe
	*/    
    virtual int Mute(WmeAudioVolumeCtrlType nType = WME_VOL_DIGITAL) = 0;

	/// set the volume unmute
	/*!
		\param WmeDeviceInOutType nType:[in] the volume controler type
		   WME_VOL_APPLICATION  or  WME_VOL_SYSTEM
		\return the set result
		\note not thread safe
	*/
    virtual int UnMute(WmeAudioVolumeCtrlType nType = WME_VOL_DIGITAL) = 0;

	/// check the volume mute status
	/*!
		\param WmeDeviceInOutType nType:[in] the volume controler type
		   WME_VOL_APPLICATION  or  WME_VOL_SYSTEM
		\return the mute status true:mute,false:unmute
		\note not thread safe
	*/
    virtual bool IsMute(WmeAudioVolumeCtrlType nType = WME_VOL_DIGITAL) = 0;

protected:
	virtual ~IWmeAudioVolumeController(){}
};

}

#endif	// WME_DEVICE_INTERFACE_H
