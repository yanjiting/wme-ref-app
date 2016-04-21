#include "TAManager.h"
#include <wchar.h>
#import "WMEDataProcess.h"

#import "WMERenderView.h"

//extern WMERenderView *s_attendeeView;
//extern WMERenderView *s_selfView;

#define STRACE_MAX_TRACE_LEN  255

TAManager::TAManager()
{
	hComponent = NULL;
	m_theTAData.m_pTASink = NULL;
	m_theTAData.m_pTraceSink = NULL;
    pWMEDataProcess = [WMEDataProcess instance];
}

TAManager::~TAManager()
{

}


void TAManager::CreateTANetwork()
{
	m_theTAData.m_pTASink = this;
	m_theTAData.m_pTraceSink = this;

	hComponent = WBXI_TANetworkNewInstance();
	if(!hComponent)
		return;
	WBXI_TANetworkSetTAData(hComponent, m_theTAData);
}

int TAManager::OnTACommandHandle(unsigned long nEvent, unsigned long wParam, unsigned long lParam)//APP->TA
{
	char* clParam = NULL;
	clParam = (char*)lParam;
    TAResult taResult = {0};


	//WBXI_TANetworkHandleCommand(hComponent, 201, 0,taResult);
    
	if(nEvent == WBXI_EVENTTYPE_TA_OTHER_PERFORMSTART)
	{
        m_time = wParam/1000;

        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_PERFORM_START, 1,&taResult);
        SchedulePerformTimer();

    }

    
    
    if(nEvent == WBXI_EVENTTYPE_TA_OTHER_PERFORMSTOP)
    {
        CancelPerformTimer();

        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_PERFORM_STOP, 1,&taResult);
    }
    
    if(nEvent == WBXI_EVENTTYPE_TA_OTHER_NETUSAGESTART)
	{
        m_time = wParam/1000;
        
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_NETUSAGE_START, 1,&taResult);
        ScheduleNetTimer();
        
    }
    
    if(nEvent == WBXI_EVENTTYPE_TA_OTHER_NETUSAGESTOP)
    {
        CancelNetTimer();
        
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_NETUSAGE_STOP, 1,&taResult);
    }
    
    
    if(nEvent == WBXI_EVENTTYPE_TA_REMOTETRACE_GETTRACEFILE)
    {
        //get trace file
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if([paths count] > 0)
		{
			NSString* document = [paths objectAtIndex:0];
			NSString* file = [document stringByAppendingPathComponent:@"/logs/wbxtra_09022013_073235.wbt"];//wbxtra_09022013_073235.wbt
			//m_file = fopen([file UTF8String], "wb");
            
            NSData *traceFileContent = [NSData dataWithContentsOfFile:file];
            unsigned long returnTADataLen = 1;
            returnTADataLen = traceFileContent.length;
            //NSData *dReturnTADataLen = [NSData dataWithBytes:&returnTADataLen length:sizeof(returnTADataLen)];
            //NSData *dReturnTAData = [NSData dataWithBytes:"0" length:1];
            if (traceFileContent != nil)
            {
                //returnTADataLen = traceFileContent.length;
                //dReturnTADataLen = [NSData dataWithBytes:&returnTADataLen length:returnTADataLen];
                //dReturnTAData = traceFileContent;
                
            }

            taResult.szTAStr = (char*)[traceFileContent bytes];
            taResult.nStrLen = returnTADataLen;

            
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_TRACE_FILENAME, 1,&taResult);
		}
        
        
    }
    
    if(nEvent == 30001)//screen shot
    {
        /*
        //taEvent = [[TAEvent alloc]init];
        //NSData* screenShotData = [NSData dataWithdata:[captureScreenshot]];
        
        UIImage *image = Snapshot(s_attendeeView);
        
        
        NSData *traceFileContent = UIImagePNGRepresentation(image);//UIImage
        unsigned long returnTADataLen = 1;
        returnTADataLen = traceFileContent.length;
        
        
        taResult.szTAStr = (char*)[traceFileContent bytes];
        taResult.nStrLen = returnTADataLen;
        
        
        WBXI_TANetworkHandleCommand(hComponent, 30001, 0,&taResult);
        */
        
    }

    //select audio capability
	if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_SELECTENCODINGPARAM)
	{
        //lParam is the audio encoding param
		//handle the event
        long ret = [pWMEDataProcess setAudioEncodingParam:wParam];
        if(ret == WME_S_OK)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTENCODINGPARAM, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTENCODINGPARAM, 0,NULL);
        }

        //send result to TA
        //WBXI_TANetworkHandleCommand(void* hComponent, unsigned long resultID, unsigned long resultParam,void* resultContent)
        //resultID = WBXI_TA_RESULT_AUDIO_SELECTENCODINGPARAM

	}
    
    //check audio capability
	if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_CHECK_ENCODINGPARAM)
	{
		//handle the event
        
        
        
        
        //send result to TA
        //WBXI_TANetworkHandleCommand(void* hComponent, unsigned long resultID, unsigned long resultParam,void* resultContent)
        //resultID = WBXI_TA_RESULT_AUDIO_CHECK_ENCODINGPARAM
        
	}
    
    
    
    //select video capability
	if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_SELECTENCODINGPARAM)
	{
        //lParam is the video encoding param
		//handle the event
        long ret = [pWMEDataProcess setVideoEncodingParam:wParam];
        if(ret == WME_S_OK)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SELECTENCODINGPARAM, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SELECTENCODINGPARAM, 0,NULL);
        }
        
        //send result to TA
        //WBXI_TANetworkHandleCommand(void* hComponent, unsigned long resultID, unsigned long resultParam,void* resultContent)
        //resultID = WBXI_TA_RESULT_VIDEO_SELECTENCODINGPARAM
        
	}
    
    //check video capability
	if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_CHECK_ENCODINGPARAM)
	{
		//handle the event
        
        
        
        
        //send result to TA
        //WBXI_TANetworkHandleCommand(void* hComponent, unsigned long resultID, unsigned long resultParam,void* resultContent)
        //resultID = WBXI_TA_RESULT_VIDEO_CHECK_ENCODINGPARAM
        
	}
    
    
    if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_CAPTUREMUTE)
	{
        //no handle now
        
        //send error to TA
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CAPTUREMUTE, 0,NULL);
        
	}
    
    if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_CAPTUREUNMUTE)
	{
        //no handle now
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CAPTUREUNMUTE, 0,NULL);
        
	}
    
    if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_CHECK_CAPTUREMUTE)
	{
        //no handle now
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECK_CAPTUREMUTE, 0,NULL);
        
	}
    
    if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_SETCAPTUREVOLUME)
	{
        //no handle now
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SETCAPTUREVOLUME, 0,NULL);
        
	}
    
    if (nEvent == WBXI_EVENTTYPE_TA_AUDIO_GETCAPTUREVOLUME)
	{
        //no handle now
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETCAPTUREVOLUME, 0,NULL);
        
	}
    
    
    
    //start AV sync
	if (nEvent == WBXI_EVENTTYPE_TA_OTHER_AVSYNCSTART)
	{
		//handle the event
        [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_AVSYNCSTART" object:nil userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:wParam] forKey:@"interval"]];
        
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_AVSYNC_START, 1,NULL);
	}
    
    //stop AV sync
	if (nEvent == WBXI_EVENTTYPE_TA_OTHER_AVSYNCSTOP)
	{
        [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_AVSYNCSTOP" object:nil];
        //send result to TA
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_AVSYNC_STOP, 1,NULL);
        
        
	}
    
    if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_SETPORTARITMODE)
	{
        //send result to TA
        long ret = [pWMEDataProcess setRenderAdaptiveAspectRatio:wParam];
        if(ret == WME_S_OK)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SET_PORTARITMODE, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SET_PORTARITMODE, 0,NULL);
        }
        
	}
    
	if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_SETCAPTUREPARAM)
	{
        long ret = [pWMEDataProcess setVideoCameraParam:wParam];
        if(ret == WME_S_OK)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SETCAPTUREPARAM, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SETCAPTUREPARAM, 0,NULL);
        }

	}
    

    if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_STARTVIDEO)
    {
        long ret = WME_S_FALSE;
        
        if(wParam == TA_WME_VIDEO_TRACK_TYPE_REMOTE)
        {
            ret = [pWMEDataProcess startVideoClient:WME_RECVING];
        }
        else
        {
            if (wParam == TA_WME_VIDEO_TRACK_TYPE_LOCAL) {
                pWMEDataProcess.bVideoSending = YES;
            }
            else
            {
                pWMEDataProcess.bVideoSending = NO;
            }
            
            ret = [pWMEDataProcess startVideoClient:WME_SENDING];
        }
        
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STARTVIDEO, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STARTVIDEO, 0,NULL);
        }
            
    }
    
    if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_STOPVIDEO)
    {
        
        long ret = WME_S_FALSE;
        
        if(wParam == TA_WME_VIDEO_TRACK_TYPE_REMOTE)
        {
            ret = [pWMEDataProcess stopVideoClient:WME_RECVING];
        }
        else
        {
            ret = [pWMEDataProcess stopVideoClient:WME_SENDING];
        }
        
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STOPVIDEO, 1,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STOPVIDEO, 0,NULL);
        }
        
    }
    if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_SWITCHCAMERA)
    {
        //long ret = [pWMEDataProcess setVideoCameraDevice:(NSInteger)index];
        [pWMEDataProcess switchCameraDevice];
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SWITCHCAMERA, 1,NULL);
    }
    
    if (nEvent == WBXI_EVENTTYPE_TA_VIDEO_GETENCODELEVEL)
    {
        WmeVideoMediaCapability vMC;
        
        long ret = [pWMEDataProcess getVideoMediaCapability:vMC];
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_GETENCODELEVEL, vMC.height,NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_GETENCODELEVEL, 0,NULL);
        }
        
    }
    
    //3-19-2014
    if (WBXI_EVENTTYPE_TA_ACTION_ENABLEMYVIDEO == nEvent)
    {
        long ret = [pWMEDataProcess enableMyVideo:wParam];
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_ENABLEMYVIDEO, 1, NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_ENABLEMYVIDEO, 0,NULL);
        }
        
    }
    
    if (WBXI_EVENTTYPE_TA_ACTION_ENABLEMYAUDIO == nEvent)
    {
        long ret = [pWMEDataProcess enableMyAudio:wParam];
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_ENABLEMYAUDIO, 1, NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_ENABLEMYAUDIO, 0,NULL);
        }
        
    }
    
    if (WBXI_EVENTTYPE_TA_AUDIO_CHECKAUDIOPAUSESTATUS == nEvent)
    {
        long ret = [pWMEDataProcess checkAudioPauseStatus];
        if (YES == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECKAUDIOPAUSESTATUS, 1, NULL);
        }
        else if (NO == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECKAUDIOPAUSESTATUS, 0, NULL);
        }
        
    }
    
    if (WBXI_EVENTTYPE_TA_AUDIO_CHECKCURRENTSPEAKER == nEvent)
    {
        //long ret = [pWMEDataProcess checkAudioPauseStatus];
        
        //WBXI_TANetworkHandleCommand(hComponent, WBXI_EVENTTYPE_TA_AUDIO_CHECKCURRENTSPEAKER, ret, NULL);
    }

    
    /////////////////////////////////////////////////////////
    
    if (WBXI_EVENTTYPE_TA_AUDIO_PAUSEAUDIO == nEvent)
    {
        
        long ret = WME_S_FALSE;
        
        if (1 == wParam)
        {
            ret = [pWMEDataProcess stopAudioClient:WME_SENDING];
        }
        else if (0 == wParam)
        {
            ret = [pWMEDataProcess startAudioClient:WME_SENDING];
        }
        
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_PAUSEAUDIO, 1, NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_PAUSEAUDIO, 0, NULL);
        }
        
    }
    
    if (WBXI_EVENTTYPE_TA_AUDIO_SELECTSPEAKER == nEvent)
    {
        long ret = [pWMEDataProcess setAudioSpeaker:wParam];
        if (WME_S_OK == ret)
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTSPEAKER, 1, NULL);
        }
        else
        {
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTSPEAKER, 0,NULL);
        }
        
    }
    
    if (WBXI_EVENTTYPE_TA_ACTION_CHECKVIDEOENABLESTATUS == nEvent)
    {
        long ret = [pWMEDataProcess checkVideoEnableStatus];
        
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_CHECKVIDEOENABLESTATUS, ret, NULL);
        
    }
    
    if (WBXI_EVENTTYPE_TA_ACTION_CHECKAUDIOENABLESTATUS == nEvent)
    {
        long ret = [pWMEDataProcess checkAudioEnableStatus];
        
        WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_CHECKAUDIOENABLESTATUS, ret, NULL);
        
    }

    
	return 0;
}


int TAManager::OnTABigDataSent(unsigned long nEvent, unsigned long wParam, unsigned long lParam)
{

	return 0;
}


int TAManager::TATrace(unsigned long trace_level, const char* szInfo, int len)
{

	if( !szInfo || szInfo[0] == 0 )
		return -1;
	

	
	return 0;

}



void TAManager::OnTimer(CCmTimerWrapperID *aId)
{
    TAPerformResult taPerformResult;
    TANetUsageResult taNetUsageResult;
    TACPUInfo cpuInfo;
    TAMemInfo memInfo;
    TANetUsageInfo netUsageInfo;
    TAResult taResult = {0};
    
    if(aId == NULL)
    {
        return;
    }
    
    if(aId == &m_PerformTimerId)
    {
        if(0 == WBXI_TAGetHostInfo(&cpuInfo, &memInfo))
        {
            taPerformResult.nUserCPU = cpuInfo.nUser;
            taPerformResult.nSysCPU = cpuInfo.nSys;
            taPerformResult.nIdleCPU = cpuInfo.nIdle;
            taPerformResult.nUsedMem = memInfo.nUsed;
            taPerformResult.nFreeMem = memInfo.nFree;
            taPerformResult.nWiredMem = memInfo.nWired;
            taResult.szTAStr = (char*)&taPerformResult;
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_PERFORM, 0,&taResult);
            
        }
        
    }
    
    
    if(aId == &m_NetTimerId)
    {
        if(0 == WBXI_TAGetNetUsageInfo(&netUsageInfo))
        {
            taNetUsageResult.nSent = netUsageInfo.nSent;
            taNetUsageResult.nReceived = netUsageInfo.nReceived;
            taResult.szTAStr = (char*)&taNetUsageResult;

            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_NETUSAGE, 0,&taResult);
            
        }
        
    }
    
	return;
}

void TAManager::CancelPerformTimer()
{
    
	m_PerformTimerId.Cancel();
}

void TAManager::SchedulePerformTimer()
{
    
	CancelPerformTimer();

    m_PerformTimerId.Schedule(this, CCmTimeValue(m_time, 0));

}

void TAManager::CancelNetTimer()
{
    
	m_NetTimerId.Cancel();
}

void TAManager::ScheduleNetTimer()
{
    
	CancelNetTimer();
    
    m_NetTimerId.Schedule(this, CCmTimeValue(m_time, 0));
    
}


#if 0
UIImage* TAManager::GetCaptureScreenShot()
{
    CGSize imageSize = [[UIScreen mainScreen] bounds].size;
    if (NULL != UIGraphicsBeginImageContextWithOptions)
        UIGraphicsBeginImageContextWithOptions(imageSize, NO, 0);
    else
        UIGraphicsBeginImageContext(imageSize);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    // Iterate over every window from back to front
    for (UIWindow *window in [[UIApplication sharedApplication] windows])
    {
        if (![window respondsToSelector:@selector(screen)] || [window screen] == [UIScreen mainScreen])
        {
            // -renderInContext: renders in the coordinate space of the layer,
            // so we must first apply the layer's geometry to the graphics context
            CGContextSaveGState(context);
            // Center the context around the window's anchor point
            CGContextTranslateCTM(context, [window center].x, [window center].y);
            // Apply the window's transform about the anchor point
            CGContextConcatCTM(context, [window transform]);
            // Offset by the portion of the bounds left of and above the anchor point
            CGContextTranslateCTM(context,
                                  -[window bounds].size.width * [[window layer] anchorPoint].x,
                                  -[window bounds].size.height * [[window layer] anchorPoint].y);
            
            // Render the layer hierarchy to the current context
            [[window layer] renderInContext:context];
            
            // Restore the context
            CGContextRestoreGState(context);
        }
    }
    
    // Retrieve the screenshot image
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;

}

UIImage* TAManager::GetCaptureScreenShot()
{
/*
    UIImage *image = NULL;
    
    CGSize imageSize = [[UIScreen mainScreen] bounds].size;
    if (NULL != UIGraphicsBeginImageContextWithOptions)
        UIGraphicsBeginImageContextWithOptions(imageSize, NO, 0);
    else
        UIGraphicsBeginImageContext(imageSize);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    // Iterate over every window from back to front
    for (UIWindow *window in [[UIApplication sharedApplication] windows])
    {
        if (![window respondsToSelector:@selector(screen)] || [window screen] == [UIScreen mainScreen])
        {
            image = Snapshot(s_attendeeView);
        }
    }
    
    UIGraphicsEndImageContext();
    
    return image;
 */
    
    return Snapshot(s_selfView);
}
#endif




#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#include <OPENGLES/ES1/gl.h>
#include <OPENGLES/ES1/glext.h>

#include <OPENGLES/ES2/gl.h>
#include <OPENGLES/ES2/glext.h>

UIImage* TAManager::Snapshot(UIView* eaglview)
{
    EAGLContext *context = [EAGLContext currentContext];
    if( context == nil )
        return nil;
    
    GLint backingWidth, backingHeight;
       
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    NSInteger x = 0, y = 0, width = backingWidth, height = backingHeight;
    NSInteger dataLength = width * height * 4;
    GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));
    
    
    // Read pixel data from the framebuffer
    
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Create a CGImage with the pixel data
    
    // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
    
    // otherwise, use kCGImageAlphaPremultipliedLast
    
    CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
    
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    
    CGImageRef iref = CGImageCreate(width, height, 8, 32, width * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                    ref, NULL, true, kCGRenderingIntentDefault);
    
    
    
    // OpenGL ES measures data in PIXELS
    
    // Create a graphics context with the target size measured in POINTS
    
    NSInteger widthInPoints, heightInPoints;
    
    if (NULL != UIGraphicsBeginImageContextWithOptions) {
        
        // On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
        
        // Set the scale parameter to your OpenGL ES view's contentScaleFactor
        
        // so that you get a high-resolution snapshot when its value is greater than 1.0
        
        CGFloat scale = eaglview.contentScaleFactor;
        
        widthInPoints = width / scale;
        
        heightInPoints = height / scale;
        
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(widthInPoints, heightInPoints), NO, scale);
        
    }
    
    else {
        
        // On iOS prior to 4, fall back to use UIGraphicsBeginImageContext
        
        widthInPoints = width;
        
        heightInPoints = height;
        
        UIGraphicsBeginImageContext(CGSizeMake(widthInPoints, heightInPoints));
        
    }
    
    
    
    CGContextRef cgcontext = UIGraphicsGetCurrentContext();
    
    
    
    // UIKit coordinate system is upside down to GL/Quartz coordinate system
    
    // Flip the CGImage by rendering it to the flipped bitmap context
    
    // The size of the destination area is measured in POINTS
    
    CGContextSetBlendMode(cgcontext, kCGBlendModeCopy);
    
    CGContextDrawImage(cgcontext, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);
    
    
    
    // Retrieve the UIImage from the current context
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    
    
    
    UIGraphicsEndImageContext();
    
    
    
    // Clean up
    
    free(data);
    
    CFRelease(ref);
    
    CFRelease(colorspace);
    
    CGImageRelease(iref);
    
    
    return image;
    
}




