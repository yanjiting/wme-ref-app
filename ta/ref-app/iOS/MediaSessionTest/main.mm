//
//  main.m
//  MediaSessionTest
//
//  Created by Soya Li on 3/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMEAppDelegate.h"
#import "CSILog.h"
#import <execinfo.h>

void dump_frames(void) {
	void *backtraceFrames[128];
	int frameCount = backtrace(&backtraceFrames[0], 128);
	char **frameStrings = backtrace_symbols(&backtraceFrames[0], frameCount);
	
	if(frameStrings != NULL) {
		int x = 0;
		for(x = 0; x < frameCount; x++) {
			if(frameStrings[x] == NULL) { break; }
			CSILogBug(@"%s", frameStrings[x]);
		}
		free(frameStrings);
	}
}

int GetiOSDocumentPath(char *pPath, int &uPathLen)
{
    if(NULL == pPath || 0 == uPathLen)
    {
        return 1;
    }
    
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if([paths count] == 0)
    {
        return 1;
    }
    
    NSString* document = [paths objectAtIndex:0];
    unsigned long uDocumentPathLen = [document length];
    uPathLen = (uDocumentPathLen <= uPathLen) ? uDocumentPathLen : uPathLen;
    memcpy(pPath, [document UTF8String], uPathLen);
    
    return 0;
}

void AppSigHandler(int sig, siginfo_t* info, void* context)
{
	CSILogInfo(@"=======================sig crash call stack===================");
	CSILogDebug(@"sig:%d", sig);
    
	struct sigaction mySigAction;
	memset(&mySigAction, 0, sizeof(mySigAction));
	mySigAction.sa_handler = SIG_DFL;
	sigemptyset(&mySigAction.sa_mask);
	
	sigaction(SIGQUIT, &mySigAction, NULL);
	sigaction(SIGILL, &mySigAction, NULL);
	sigaction(SIGTRAP, &mySigAction, NULL);
	sigaction(SIGABRT, &mySigAction, NULL);
	sigaction(SIGEMT, &mySigAction, NULL);
	sigaction(SIGFPE, &mySigAction, NULL);
	sigaction(SIGBUS, &mySigAction, NULL);
	sigaction(SIGSEGV, &mySigAction, NULL);
	sigaction(SIGSYS, &mySigAction, NULL);
	//sigaction(SIGPIPE, &mySigAction, NULL);
	sigaction(SIGALRM, &mySigAction, NULL);
	sigaction(SIGXCPU, &mySigAction, NULL);
	sigaction(SIGXFSZ, &mySigAction, NULL);
    
	dump_frames();
}

void handleUncaughtException(NSException* exception)
{
	CSILogInfo(@"=======================crash call stack===================");
	
	NSArray* callstacks = [exception callStackSymbols];
	for(NSString* stack in callstacks)
	{
		CSILogBug(@"%@", stack);
	}
}


#ifdef __cplusplus
extern "C"
{
#endif
    
    void InitCmdArguments(int argc, char **argv);
    
#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[]) {
    
	struct sigaction mySigAction;
	mySigAction.sa_sigaction = AppSigHandler;
	mySigAction.sa_flags = SA_SIGINFO;
	sigemptyset(&mySigAction.sa_mask);
	sigaction(SIGQUIT, &mySigAction, NULL);
	sigaction(SIGILL, &mySigAction, NULL);
	sigaction(SIGTRAP, &mySigAction, NULL);
	sigaction(SIGABRT, &mySigAction, NULL);
	sigaction(SIGEMT, &mySigAction, NULL);
	sigaction(SIGFPE, &mySigAction, NULL);
	sigaction(SIGBUS, &mySigAction, NULL);
	sigaction(SIGSEGV, &mySigAction, NULL);
	sigaction(SIGSYS, &mySigAction, NULL);
	//sigaction(SIGPIPE, &mySigAction, NULL);
	sigaction(SIGALRM, &mySigAction, NULL);
	sigaction(SIGXCPU, &mySigAction, NULL);
	sigaction(SIGXFSZ, &mySigAction, NULL);
	
	NSSetUncaughtExceptionHandler(&handleUncaughtException);
    InitCmdArguments(argc,(char**)argv);
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([WMEAppDelegate class]));
    }
}