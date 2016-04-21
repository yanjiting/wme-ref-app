#import "WMEAppDelegate+Calabash.h"
#import "PeerCall.h"
#import "Loopback.h"
#import "Calabash.h"
#import "MediaConnection.h"

#include <ifaddrs.h>
#include <arpa/inet.h>

#include "CmTraceFromT120.h"
extern "C" void __gcov_flush();


@interface WMEAppDelegate ()

@end

BOOL g_isLoopback = NO;


@implementation WMEAppDelegate (Calabash)



- (NSString *)getIPAddress:(NSString *)arg {
    return [self getIPAddress];
}
- (NSString *)getIPAddress {
    
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"]) {
                    char* c_addr = NULL;
                    // Get NSString from C String
                    if (temp_addr->ifa_addr->sa_family == AF_INET) { // check it is IP4
                        // is a valid IP4 Address
                        char addressBuffer[INET_ADDRSTRLEN];
                        c_addr = (char*)inet_ntop(AF_INET, &((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr, addressBuffer, INET_ADDRSTRLEN);
                    }
                    //address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                    address =[NSString stringWithUTF8String:c_addr];
                    
                }
                
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return address;
}

#pragma mark - backdoors
/*
 {
 "p2p":false
 "loopback":false
 "linus":"http://173.39.168.98:5000/"
 "filemode":false
 "multilayer":false
 }
 */

- (NSString *)backdoorTestIOS:(NSString *)param {
    NSLog(@"backdoorTestIOS SUCCESS");
    return @"";
}

- (void)updateFileCaptureRender:(id) json {
    BOOL fileMode = [json[@"filemode"] boolValue];
    BOOL bTimestampMode = [json[@"timestamp"] boolValue];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentPath = [paths objectAtIndex:0];
    
    if(fileMode)
    {
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        
        TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], [[documentPath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], WmeSessionType_Audio);
        
        int resolution = 2;
        if ([json valueForKey:@"resolution"] != nil){
            resolution = [json[@"resolution"] intValue];
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else if (1 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], WmeSessionType_Video);
        }
        
        if ([json valueForKey:@"isLoopFile"] != nil){
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            BOOL bLoopFile = [json[@"isLoopFile"] boolValue];
            capture["isLoop"] = Json::Value(bLoopFile ? true : false);
        }
        
        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
    }
    if([json valueForKey:@"files"]) {
        id files = json[@"files"];
        if ([files valueForKey:@"audio"] && [files[@"audio"] valueForKey:@"source"]) {
            NSString* audio_src = @"wmetest@";
            NSString*audio_source = files[@"audio"][@"source"];
            audio_src = [audio_src stringByAppendingString:audio_source];
            if (audio_source != NULL && [audio_source length] != 0) {
                TestConfig::i().setFilePath([[documentPath stringByAppendingPathComponent:audio_src] UTF8String], [[documentPath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], WmeSessionType_Audio);
                fileMode = true;
            }
            Json::Value &capture = TestConfig::i().m_audioDebugOption["fileCapture"];
            if ([files[@"audio"] valueForKey:@"loop"])
                capture["isLoop"] = (bool)[files[@"audio"][@"loop"] boolValue];
            if ([files[@"audio"] valueForKey:@"timestamp"])
                capture["timestamp"] = (bool)[files[@"audio"][@"timestamp"] boolValue];
        }
        
        if ([files valueForKey:@"video"] && [files[@"video"] valueForKey:@"source"]) {
            NSString* video_src = @"wmetest@";
            video_src = [video_src stringByAppendingString:files[@"video"][@"source"]];
            if (video_src != NULL && [video_src length] != 0) {
                TestConfig::i().setFilePath([[documentPath stringByAppendingPathComponent:video_src] UTF8String], [[documentPath stringByAppendingPathComponent:video_src] UTF8String], WmeSessionType_Video);
                fileMode = true;
            }
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            if ([files[@"video"] valueForKey:@"loop"])
                capture["isLoop"] = (bool)[files[@"video"][@"loop"] boolValue];
            if ([files[@"video"] valueForKey:@"timestamp"])
                capture["timestamp"] = (bool)[files[@"video"][@"timestamp"] boolValue];
        }
    }
}

 - (NSString *)backdoorStartCall:(NSString *)param {
     CM_INFO_TRACE("backdoorStartCall with param:" << [param UTF8String]);
     NSError *error = nil;
     id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
     g_isLoopback = [json[@"loopback"] boolValue];
     
     [self updateFileCaptureRender : json];
     
     if ([json valueForKey:@"bgra"] != nil){
         TestConfig::i().m_videoDebugOption["enableBGRA"] = (bool)[json[@"bgra"] boolValue];
     }
     if ([json valueForKey:@"ice"] != nil){
         TestConfig::i().m_audioDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
         TestConfig::i().m_videoDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
     }
     if ([json valueForKey:@"srtp"] != nil){
         TestConfig::i().m_audioDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
         TestConfig::i().m_videoDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
     }
     if ([json valueForKey:@"mux"] != nil){
         TestConfig::i().m_audioDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
         TestConfig::i().m_videoDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
     }
     if ([json valueForKey:@"mute"] != nil){
         TestConfig::i().m_bMuteAudio = (bool)[json[@"mute"] boolValue];
         TestConfig::i().m_bMuteVideo = (bool)[json[@"mute"] boolValue];
     }
     if ([json valueForKey:@"resolution"] != nil){
         TestConfig::i().m_nVideoSize = [json[@"resolution"] intValue];
     }
     if ([json valueForKey:@"params"] != nil){
         NSData *data = [NSJSONSerialization dataWithJSONObject:json[@"params"] options:0 error:&error];
         NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];

         Json::Reader r;
         Json::Value root;
         r.parse([input UTF8String], root, true);
     
         TestConfig::i().m_videoParam = root["video"];
     }
     TestConfig::i().m_audioParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
     TestConfig::i().m_videoParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
     TestConfig::i().m_shareParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
     
     if ([json valueForKey:@"enableQos"] != nil) {
         TestConfig::i().m_bQoSEnable = (bool)[json[@"enableQos"] boolValue];
     } else {
         TestConfig::i().m_bQoSEnable = true;
     }
     
     NSString *overrideip = json[@"overrideip"];
     if([overrideip length] != 0)
     {
         TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
         TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
     }

     TestConfig::i().m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;

     //all before config should be merge into backdoorAgent->backdoorStartCall
     [NSString stringWithUTF8String:backdoorAgent->backdoorStartCall([param UTF8String]).c_str()];
     [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_TEST_STARTCALL" object:json];
     return @"";
 }

- (NSString *)backdoorCheckHWSupport:(NSString *) device {
    NSString *ret = @"false";
    float osVer = [[[UIDevice currentDevice] systemVersion] floatValue];
    if (osVer >= 8.0) {
        ret = @"true";
    }
    return ret;
}

 - (NSString *)backdoorGetLocalSdp:(NSString *)ignored {
     return [NSString stringWithUTF8String:PeerCall::Instance()->m_endCaller->m_localSdp.c_str()];
 }

- (NSString *)backdoorSetRemoteSdp:(NSString *)remoteSdp {
    PeerCall::Instance()->m_endCaller->answerReceived([remoteSdp UTF8String]);
    return nil;
}

-(NSString*)backdoorStartStopTrack:(NSString*)param{
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorStartStopTrack([param UTF8String]).c_str()];
}

-(NSString*)backdoorRequestFloor:(NSString*)param{
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorRequestFloor([param UTF8String]).c_str()];
}


-(NSString*)backdoorOutputTrace:(NSString*)param{
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorOutputTrace([param UTF8String]).c_str()];
}

- (NSString *)backdoorMuteUnMute:(NSString *)param {
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    NSString *type = json[@"type"];
	WmeSessionType sessType = WmeSessionType_Audio;
	if ([type isEqualToString:@"audio"])
		sessType = WmeSessionType_Audio;
	else if ([type isEqualToString:@"video"])
		sessType = WmeSessionType_Video;
	else if ([type isEqualToString:@"sharing"])
		sessType = WmeSessionType_ScreenShare;
    
    BOOL bMute = [json[@"mute"] boolValue];
    BOOL bSpeaker = [json[@"speaker"] boolValue];
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    NSLog(@"backdoorMuteUnMute, loopback call=%d, sessType=%d, bMute=%d, bSpeaker=%d",bLoopbackCall, sessType, bMute, bSpeaker);
	if (bLoopbackCall){
        LoopbackCall::Instance()->mute(sessType, bMute, bSpeaker);
    }
	else{
        PeerCall::Instance()->mute(sessType, bMute, bSpeaker);
    }
    
	return nil;
}

- (NSString *)backdoorUpdateSdp:(NSString *)param {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorUpdateSdp([param UTF8String]).c_str()];
}

- (NSString *)backdoorSetRemoteOffer:(NSString *)param {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorSetRemoteOffer([param UTF8String]).c_str()];
}

- (NSString*)backdoorSwitchCamera:(NSString*)cameraFacing{
    WMERESULT res = WME_S_OK;
    if (g_isLoopback) {
        std::map<std::string, IWmeMediaDevice *> cameraMap = LoopbackCall::Instance()->m_endCaller.getCameraList();
        if (cameraMap.find([cameraFacing UTF8String]) == cameraMap.end() ){
            return @"true";
        }
        res = LoopbackCall::Instance()->m_endCaller.setCamera([cameraFacing UTF8String]);
        if (WME_S_OK != res) {
            return @"false";
        }
    }
    return @"true";
}

- (NSString *)backdoorGetStatistics:(NSString *)ignored {
    if (!backdoorAgent)
        return @"false";

    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetStatistics([@"" UTF8String]).c_str()];
}

- (NSString *)backdoorGetFeatureToggles:(NSString *)ignored {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetFeatureToggles([@"" UTF8String]).c_str()];
}

- (NSString *)backdoorGetNetworkMetrics:(NSString *)ignored {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetNetworkMetrics([@"" UTF8String]).c_str()];
}

- (NSString *)backdoorStopCall:(NSString *)ignored {
    /*if (g_isLoopback)
        LoopbackCall::Instance()->stopLoopback();
    else
        PeerCall::Instance()->m_endCaller->stopCall();
    */
    [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_TEST_STOPCALL" object:nil];
    
    return @"";
}

- (NSString *)backdoorSetParam:(NSString *)arg {
    if (nil == arg || [arg length] < 3) {
        NSLog(@"backdoorSetParam received invalid param");
    }
    NSString *paramType = [arg substringToIndex:1];
    NSLog(@"paramType=%@", paramType);
    NSString *param = [arg substringFromIndex:2];
    NSLog(@"param=%@", param);

    if (g_isLoopback)
        LoopbackCall::Instance()->m_endCaller.setParam((PARAM_TYPE_NUM)[paramType intValue], [param UTF8String]);
    else
        PeerCall::Instance()->m_endCaller->setParam((PARAM_TYPE_NUM)[paramType intValue], [param UTF8String]);
    
    return @"";
}

- (NSString *)backdoorSetAudioParam:(NSString *)param {
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    BOOL isCaller = [json[@"caller"] boolValue];
    NSData *data = [NSJSONSerialization dataWithJSONObject:json[@"param"] options:0 error:&error];
    NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    if (g_isLoopback){
        if(isCaller)
            LoopbackCall::Instance()->m_endCaller.setAudioParam([input UTF8String]);
        else
            LoopbackCall::Instance()->m_endCallee->setAudioParam([input UTF8String]);
    }
    else{
        PeerCall::Instance()->m_endCaller->setAudioParam([input UTF8String]);
    }
    
    return @"";
}

- (NSString *)backdoorSetVideoParam:(NSString *)param {
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    BOOL isCaller = [json[@"caller"] boolValue];
    NSData *data = [NSJSONSerialization dataWithJSONObject:json[@"param"] options:0 error:&error];
    NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
     if (g_isLoopback){
         if(isCaller)
             LoopbackCall::Instance()->m_endCaller.setVideoParam([input UTF8String]);
         else
             LoopbackCall::Instance()->m_endCallee->setVideoParam([input UTF8String]);
     }
     else{
         PeerCall::Instance()->m_endCaller->setVideoParam([input UTF8String]);
     }
    
    return @"";
}

- (NSString *)backdoorSetFecParam:(NSString *)param {
    if (!backdoorAgent)
        return @"false";

    return [NSString stringWithUTF8String:backdoorAgent->backdoorSetFecParam([param UTF8String]).c_str()];
}

- (NSString *)backdoorSetFeatureToggles:(NSString *)param {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorSetFeatureToggles([param UTF8String]).c_str()];
}

- (NSString *)backdoorGetMemory:(NSString *)ignored {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetMemory([@"" UTF8String]).c_str()];
}

- (NSString *)backdoorGetVideoParameters:(NSString *)param {
    NSError *error = nil;
    std::string value;
    if (g_isLoopback){
        value = LoopbackCall::Instance()->m_endCaller.getParameters(PARAM_VIDEO);
    }
    else{
        value = PeerCall::Instance()->m_endCaller->getParameters(PARAM_VIDEO);
    }

    NSString * output = [NSString stringWithFormat:@"%s", value.c_str()];
    id json_output = [NSJSONSerialization JSONObjectWithData:[output dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    if ([json_output[@"video"] isKindOfClass:[NSDictionary class]]) {
        NSDictionary *video_param = (NSDictionary*)json_output[@"video"];
        NSNumber *ret = [video_param objectForKey:param];
        return [ret stringValue];
    }


    return @"";
}


- (NSString *)backdoorIsFileCaptureEnded:(NSString *)arg {
    BOOL ret = NO;
    if(arg == NULL || [arg length] == 0 || [arg isEqualToString: @"all"]) {
        if (g_isLoopback) {
            ret = LoopbackCall::Instance()->m_endCaller.m_audioFileEnded && LoopbackCall::Instance()->m_endCaller.m_videoFileEnded;
        }else {
            ret = PeerCall::Instance()->m_endCaller->m_audioFileEnded && PeerCall::Instance()->m_endCaller->m_videoFileEnded;
        }
    }else if([arg isEqualToString: @"audio"]) {
        if (g_isLoopback) {
            ret = LoopbackCall::Instance()->m_endCaller.m_audioFileEnded;
        }else {
            ret = PeerCall::Instance()->m_endCaller->m_audioFileEnded;
        }
    }else if([arg isEqualToString: @"video"]) {
        if (g_isLoopback) {
            ret = LoopbackCall::Instance()->m_endCaller.m_videoFileEnded;
        }else {
            ret = PeerCall::Instance()->m_endCaller->m_videoFileEnded;
        }
    }
    return ret?@"ended":@"playing";
}

- (NSString *)backdoorGetCSIChangeHistory:(NSString *)ignored {
    std::string sRet;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        sRet = LoopbackCall::Instance()->m_endCaller.getCalabash()->GetCSIChangeHistory();
    }
    else{
        sRet = PeerCall::Instance()->m_endCaller->getCalabash()->GetCSIChangeHistory();
    }
    return [NSString stringWithUTF8String:sRet.c_str()];
}


- (NSString *)backdoorSetPlaybackDevice:(NSString *)dev {
    bool ret = false;
    if ([dev isEqualToString:@"Speaker"]) {
        ret = PeerCall::Instance()->m_endCaller->setAudioPlaybackDevice([dev UTF8String]);
    }

    return ret?@"done":@"false";
}

- (NSString *)backdoorGetAudioParameters:(NSString *)param {
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    BOOL isCaller = [json[@"caller"] boolValue];
    std::string ret;
    if (g_isLoopback){
        if(isCaller)
            ret = LoopbackCall::Instance()->m_endCaller.getParameters(PARAM_AUDIO);
        else
            ret = LoopbackCall::Instance()->m_endCallee->getParameters(PARAM_AUDIO);
    }
    else{
        ret = PeerCall::Instance()->m_endCaller->getParameters(PARAM_AUDIO);
    }
    NSString * option = [NSString stringWithFormat:@"%s", ret.c_str()];
    
    id jsonRet = [NSJSONSerialization JSONObjectWithData:[option dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    
    if ([jsonRet[@"audio"] isKindOfClass:[NSDictionary class]]) {
        NSDictionary* audio = (NSDictionary*)jsonRet[@"audio"];
        NSNumber * value = [audio objectForKey:param];

        return ([value boolValue] ? @"true" : @"false");
    }
    return @"";
}


- (NSString *)backdoorCheckSyncStatus:(NSString *)param {
    bool ret = false;
    
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    NSString *result = json[@"result"];
    int rate = [json[@"rate"] intValue];
    
    if (g_isLoopback) {
        if(LoopbackCall::Instance()->m_endCallee) {
            ret = LoopbackCall::Instance()->m_endCallee->checkSyncStatus([result UTF8String], rate);
        }
    }
    else{
        if(PeerCall::Instance()->m_endCaller) {
            ret = PeerCall::Instance()->m_endCaller->checkSyncStatus([result UTF8String], rate);
        }
    }
    
    return ret?@"pass":@"false";
}

- (NSString *)backdoorCheckOnMediaReady:(NSString *)param {
    Calabash * myCalabash = NULL;
    std::vector<Calabash::OnMediaReadyArgs> myList;
    std::vector<Calabash::OnMediaReadyArgs>::iterator ite;
    int count = 0;
    
    if (g_isLoopback) {
        // Loopback has special case: it instantiates 3 tracks total. The "Video In" track is not used for
        // loopback mode
        myCalabash = LoopbackCall::Instance()->getCalabashCaller();
    } else {
        myCalabash = PeerCall::Instance()->getCalabash();
    }
    
    if (myCalabash != NULL) {
        myList = myCalabash->getOnMediaReadyInfoList();
    }
 
    for (ite = myList.begin(); ite != myList.end(); ite++) {
        if (ite->direction == WmeDirection::WmeDirection_Send && ite->mediaType == WmeSessionType::WmeSessionType_Audio) {
            count++;
            NSLog(@"backdoorCheckOnMediaReady, Audio Out track generated, num of tracks total = %d", count);
        }

        if (ite->direction == WmeDirection::WmeDirection_Send && ite->mediaType == WmeSessionType::WmeSessionType_Video) {
            count++;
            NSLog(@"backdoorCheckOnMediaReady, Video Out track generated, num of tracks total = %d", count);
        }
        
        if (ite->direction == WmeDirection::WmeDirection_Recv && ite->mediaType == WmeSessionType::WmeSessionType_Audio) {
            count++;
            NSLog(@"backdoorCheckOnMediaReady, Audio In track generated, num of tracks total = %d", count);
        }
        
        if (ite->direction == WmeDirection::WmeDirection_Recv && ite->mediaType == WmeSessionType::WmeSessionType_Video) {
            count++;
            NSLog(@"backdoorCheckOnMediaReady, Video In track generated, num of tracks total = %d", count);
        }
        
        // After confluence, there should be four tracks opened: Audio In/Out, Video In/Out
        if (count >= 4) {
            return @"pass";
        }

    }
    
    return @"fail";
}



- (NSString *)backdoorStopSharing:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorStopSharing([arg UTF8String]).c_str()];

}


- (NSString *)backdoorGetSharingStatus:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetSharingStatus([arg UTF8String]).c_str()];

}


- (NSString *)backdoorGetSharingResult:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetSharingResult([arg UTF8String]).c_str()];

}



- (NSString *)backdoorStartSharing:(NSString *)param {
    
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    g_isLoopback = [json[@"loopback"] boolValue];
    
    
    TestConfig::i().m_bSharer = false;
	TestConfig::i().m_bAppshare = true;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_TEST_STARTSCREENSHARE" object:json];

    return @"";
}

- (NSString *)backdoorGetCSICount:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"0";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetCSICount([arg UTF8String]).c_str()];
    
}

- (NSString *)backdoorSubscribe:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorSubscribe([arg UTF8String]).c_str()];
    
}

- (NSString *)backdoorGetMediaStatus:(NSString *)arg {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetMediaStatus([arg UTF8String]).c_str()];
}

- (NSString *)backdoorGetVideoCSI:(NSString *)arg {
    
    if (!backdoorAgent)
        return @"0";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetVideoCSI([arg UTF8String]).c_str()];
    
}

 - (NSString *)backdoorTestEcho:(NSString *)param
{
    CM_INFO_TRACE("backdoorTestEcho with param:" << [param UTF8String]);
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    g_isLoopback = [json[@"loopback"] boolValue];
    BOOL fileMode = [json[@"filemode"] boolValue];
    BOOL bTimestampMode = [json[@"timestamp"] boolValue];
    
    if(fileMode)
    {
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentPath = [paths objectAtIndex:0];
        
        TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], [[documentPath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], WmeSessionType_Audio);
        
        int resolution = 2;
        if ([json valueForKey:@"resolution"] != nil){
            resolution = [json[@"resolution"] intValue];
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else if (1 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], WmeSessionType_Video);
        }
        
        if ([json valueForKey:@"isLoopFile"] != nil){
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            BOOL bLoopFile = [json[@"isLoopFile"] boolValue];
            capture["isLoop"] = Json::Value(bLoopFile ? true : false);
        }
        
        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
    }
    
    
    if ([json valueForKey:@"bgra"] != nil){
        TestConfig::i().m_videoDebugOption["enableBGRA"] = (bool)[json[@"bgra"] boolValue];
    }
    if ([json valueForKey:@"ice"] != nil){
        TestConfig::i().m_audioDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
        TestConfig::i().m_videoDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
    }
    if ([json valueForKey:@"srtp"] != nil){
        TestConfig::i().m_audioDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
        TestConfig::i().m_videoDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
    }
    if ([json valueForKey:@"mux"] != nil){
        TestConfig::i().m_audioDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
        TestConfig::i().m_videoDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
    }
    if ([json valueForKey:@"mute"] != nil){
        TestConfig::i().m_bMuteAudio = (bool)[json[@"mute"] boolValue];
        TestConfig::i().m_bMuteVideo = (bool)[json[@"mute"] boolValue];
    }
    if ([json valueForKey:@"resolution"] != nil){
        TestConfig::i().m_nVideoSize = [json[@"resolution"] intValue];
    }
    if ([json valueForKey:@"params"] != nil){
        NSData *data = [NSJSONSerialization dataWithJSONObject:json[@"params"] options:0 error:&error];
        NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        
        Json::Reader r;
        Json::Value root;
        r.parse([input UTF8String], root, true);
        
        TestConfig::i().m_videoParam = root["video"];
    }
    TestConfig::i().m_audioParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    TestConfig::i().m_videoParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    TestConfig::i().m_shareParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    
    if ([json valueForKey:@"enableQos"] != nil) {
        TestConfig::i().m_bQoSEnable = (bool)[json[@"enableQos"] boolValue];
    } else {
        TestConfig::i().m_bQoSEnable = true;
    }
    
    NSString *overrideip = json[@"overrideip"];
    if([overrideip length] != 0)
    {
        TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
        TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
    }
    
    TestConfig::i().m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;
    
    //all before config should be merge into backdoorAgent->backdoorStartCall
    [NSString stringWithUTF8String:backdoorAgent->backdoorTestEcho([param UTF8String]).c_str()];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_TEST_STARTCALL" object:json];
    return @"";
}

- (NSString *)backdoorCalMos:(NSString *)param
{
    CM_INFO_TRACE("backdoorCalMos with param:" << [param UTF8String]);
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:[param dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&error];
    g_isLoopback = [json[@"loopback"] boolValue];
    BOOL fileMode = [json[@"filemode"] boolValue];
    BOOL bTimestampMode = [json[@"timestamp"] boolValue];
    
    if(fileMode)
    {
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentPath = [paths objectAtIndex:0];
        
        TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], [[documentPath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], WmeSessionType_Audio);
        
        int resolution = 2;
        if ([json valueForKey:@"resolution"] != nil){
            resolution = [json[@"resolution"] intValue];
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_160_90_6_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else if (1 == resolution) {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_320_180_12_i420.yuv"] UTF8String], WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], WmeSessionType_Video);
        }
        
        if ([json valueForKey:@"isLoopFile"] != nil){
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            BOOL bLoopFile = [json[@"isLoopFile"] boolValue];
            capture["isLoop"] = Json::Value(bLoopFile ? true : false);
        }
        
        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
    }
    
    
    if ([json valueForKey:@"bgra"] != nil){
        TestConfig::i().m_videoDebugOption["enableBGRA"] = (bool)[json[@"bgra"] boolValue];
    }
    if ([json valueForKey:@"ice"] != nil){
        TestConfig::i().m_audioDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
        TestConfig::i().m_videoDebugOption["enableICE"]  = (bool)[json[@"ice"] boolValue];
    }
    if ([json valueForKey:@"srtp"] != nil){
        TestConfig::i().m_audioDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
        TestConfig::i().m_videoDebugOption["enableSRTP"]  = (bool)[json[@"srtp"] boolValue];
    }
    if ([json valueForKey:@"mux"] != nil){
        TestConfig::i().m_audioDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
        TestConfig::i().m_videoDebugOption["enableRTCPMux"]  = (bool)[json[@"mux"] boolValue];
    }
    if ([json valueForKey:@"mute"] != nil){
        TestConfig::i().m_bMuteAudio = (bool)[json[@"mute"] boolValue];
        TestConfig::i().m_bMuteVideo = (bool)[json[@"mute"] boolValue];
    }
    if ([json valueForKey:@"resolution"] != nil){
        TestConfig::i().m_nVideoSize = [json[@"resolution"] intValue];
    }
    if ([json valueForKey:@"params"] != nil){
        NSData *data = [NSJSONSerialization dataWithJSONObject:json[@"params"] options:0 error:&error];
        NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        
        Json::Reader r;
        Json::Value root;
        r.parse([input UTF8String], root, true);
        
        TestConfig::i().m_videoParam = root["video"];
    }
    TestConfig::i().m_audioParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    TestConfig::i().m_videoParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    TestConfig::i().m_shareParam["supportCmulti"] = true; //(bool)[json[@"multilayer"] boolValue];
    
    if ([json valueForKey:@"enableQos"] != nil) {
        TestConfig::i().m_bQoSEnable = (bool)[json[@"enableQos"] boolValue];
    } else {
        TestConfig::i().m_bQoSEnable = true;
    }
    
    NSString *overrideip = json[@"overrideip"];
    if([overrideip length] != 0)
    {
        TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
        TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = [overrideip UTF8String];
    }
    
    TestConfig::i().m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;
    
    //all before config should be merge into backdoorAgent->backdoorStartCall
    [NSString stringWithUTF8String:backdoorAgent->backdoorCalMos([param UTF8String]).c_str()];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTIFICATION_TEST_STARTCALL" object:json];
    return @"";
}

- (NSString *)backdoorGetStunTraceResult:(NSString *)arg {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetStunTraceResult([arg UTF8String]).c_str()];
}

- (NSString *)backdoorGetTraceServerResult:(NSString *)arg {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorGetTraceServerResult([arg UTF8String]).c_str()];
}

- (NSString *)backdoorConfig:(NSString *)arg {
    if (!backdoorAgent)
        return @"false";
    
    return [NSString stringWithUTF8String:backdoorAgent->backdoorConfig([arg UTF8String]).c_str()];
}

- (NSString *)backdoorGetAssertionsCount:(NSString *)param {
    NSLog(@"backdoorGetAssertionsCount SUCCESS");
    long nCount = cm_get_assertions_count();
    return [@(nCount) stringValue];
}

@end
