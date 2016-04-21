//
//  CSILog.m
//  Logger
//
//  Created by Rob Napier on 12/3/08.
//  Copyright 2008 Cisco. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <stdio.h>

#import "CSILog.h"
#import <sys/xattr.h>

#include <pthread.h>

#if TARGET_OS_MAC
#include <fcntl.h>      /* For O_* constants */
#include <sys/stat.h>   /* For mode constants */
const char* getLogName();
#endif

static CSILog *theSharedLog = nil;
unsigned long CSILogLevel2WBXLevel(CSILogLevel lv);

static CSILogLevel kCSIDefaultLogLevel = CSILogLevelDebug;

//static NSArray *theLevelStrings;

static NSString *kLogFileExtension = @"log";
static NSString *kLogFilesDirectory = @"Logs";
static NSTimeInterval kDefaultMaximumLogAge = 1;//60 * 60 * 24/*76*/;	// Three days and four hours.

//
// Logging functions
//
void CSILogMessage(CSILogLevel level, NSString* message, ...)
{
	if( level <= [(CSILog*) [CSILog sharedLog] level] )
	{
		va_list argList;
		va_start(argList, message);
        @autoreleasepool {
		NSString *formattedMessage = [[NSString alloc] initWithFormat:message arguments:argList];
		[theSharedLog logMessage:formattedMessage level:level];
		va_end(argList);
#if !__has_feature(objc_arc)
        [formattedMessage release];
#endif
        }
	}
}


//////////////////
// Private Methods
//////////////////
@interface CSILog ()
@property (readwrite, copy) NSString* logDirectory;
@property (readwrite, retain) NSFileHandle * fileHandle;
@property (readwrite, copy) NSString* logName;
- (void)writeLogMessage:(NSString*)message;
- (void)removeLogsBeforeDate:(NSDate*)aDate;

- (void)startRunLoop:(id)sender;
@end

/////////
// CSILog
/////////
@implementation CSILog
@synthesize fileHandle=myFileHandle;
@synthesize thread=myThread;
@synthesize level=myLevel;
@synthesize dateFormatter=myDateFormatter;
@synthesize isStarted=myIsStarted;
@synthesize logDirectory=myLogDirectory;
@synthesize maximumLogAge=myMaximumLogAge;
@synthesize logName=myLogName;
@synthesize analytics=myAnalytics;


//////////////////////////
#pragma mark Class methods
//////////////////////////
#if TARGET_OS_MAC

int g_mutex = 0;

int get_process_count()
{
    int i = 0;
    for(i = 0; i < 10; i++) {
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        char mutex_name[1024] = {0};
        sprintf(mutex_name, "/tmp/mediasessiontest_lock_%d", i);
        int mutex = open(mutex_name, O_CREAT, mode);
        if(mutex == -1) {
            printf("open file failed: %d\n", errno);
            continue;
        }
        int ret = flock(mutex, LOCK_EX | LOCK_NB);
        if(0 == ret) {
            g_mutex = mutex;
            break;
        }else {
            printf("try wait failed: %d\n", errno);
        }
        close(mutex);
    }
    return i;
}

void release_mutex() {
    if(g_mutex != 0) {
        flock(g_mutex, LOCK_NB | LOCK_UN);
        close(g_mutex);
        g_mutex = 0;
    }
}
#endif

+ (void)initialize
{
	if (self == [CSILog class])
	{
		//theLevelStrings = [[NSArray alloc] initWithObjects:@"INTERNAL", @"ERROR", @"WARNING", @"INFO", @"DEBUG", @"TRACE", nil];

		// Create shared logger
		
		NSString *logName = @"wme_ta_ios";
		
	#if TARGET_OS_IPHONE
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
		NSString *docDirectory = [paths objectAtIndex:0];
		NSString *directory = [docDirectory stringByAppendingPathComponent:kLogFilesDirectory];
        NSString *logFileName = logName;
	#elif TARGET_OS_MAC
        //int processID = [[NSProcessInfo processInfo] processIdentifier];
        logName = @"wme_ta_mac";
        NSString *logFileName = logName;
        if(getLogName() != NULL) {
            logFileName = [NSString stringWithUTF8String:getLogName()];
        }else{
            int processIndex = get_process_count();
            if(processIndex > 0)
                logFileName = [NSString stringWithFormat:@"%@%i", logName, processIndex];
        }
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *directory = [[[paths objectAtIndex:0] stringByAppendingPathComponent:kLogFilesDirectory]
                               stringByAppendingPathComponent:logName];
	#else
	#error Not a supported platform.
	#endif
		
		theSharedLog = [[self alloc] initWithDirectory:directory logName:logFileName];
	}
}

+ (id)sharedLog
{
	// Already set up in +initialize
	return theSharedLog;
}

+ (void)setSharedLog:(CSILog*)aLog
{
	if (theSharedLog != aLog)
	{
		[theSharedLog stopLogging];
		theSharedLog = aLog;
	}
}

/////////////////////////////////////
#pragma mark Constructors/Destructors
/////////////////////////////////////

//
// Designated initializer
//
- (id)initWithDirectory:(NSString*)directory logName:(NSString*)logName
{
	self = [super init];
    if(self != nil){
        self.dateFormatter = [[NSDateFormatter alloc] init];
        [self.dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss.SSSZZZZZ"];
        [self.dateFormatter setTimeZone:[NSTimeZone timeZoneWithAbbreviation:@"UTC"]];
        self.maximumLogAge = kDefaultMaximumLogAge;
        self.isStarted = NO;
	
        self.logDirectory = directory;
        self.logName = logName;
        
        self.level = kCSIDefaultLogLevel;
	}
	return self;
}

////////////////////
#pragma mark Actions
////////////////////

- (BOOL)addSkipBackupAttributeToItemAtURL:(NSString *)path
{
    NSURL* URL = [NSURL fileURLWithPath:path isDirectory: NO];
    
    NSError *error = nil;
    BOOL success = [URL setResourceValue: [NSNumber numberWithBool: YES]
                                  forKey: NSURLIsExcludedFromBackupKey error: &error];
    if(!success){
        NSLog(@"Error excluding %@ from backup %@", [URL lastPathComponent], error);
    }
    return success;
}

- (void)startLogging
{
	if (self.fileHandle == nil)
	{
		NSFileManager *fm = [NSFileManager defaultManager];
		BOOL canCreateFile = YES;
		BOOL isDirectory;
		NSString *logDirectory = self.logDirectory;
		if ( ! [fm fileExistsAtPath:logDirectory isDirectory:&isDirectory] )
		{
			NSError *error;
			if ( ! [fm createDirectoryAtPath:logDirectory withIntermediateDirectories:YES attributes:nil error:&error] )
			{
				// FIXME: Make look like regular log message
				NSLog(@"ERROR: Could not create log file. Logging to console: %@", error);
				canCreateFile = NO;
			}
		}
		
		if (canCreateFile)
		{
			//NSDateFormatter *dateFormat = [[NSDateFormatter alloc] init];
			//[dateFormat setDateFormat:@"yyyy-MM-dd-HHmmssSS"];
			
			//NSString *logBasename = [NSString stringWithFormat:@"%@-%@", self.logName, [dateFormat stringFromDate:[NSDate date]]];
			NSString *logPath = [[logDirectory stringByAppendingPathComponent:self.logName] stringByAppendingPathExtension:kLogFileExtension];
			
            if ([fm createFileAtPath:logPath contents:nil attributes:nil])
			{
				self.fileHandle = [NSFileHandle fileHandleForWritingAtPath:logPath];
				if (self.fileHandle == nil)
				{
					NSLog(@"ERROR: Could not open file for writing. Logging to console: %@", logPath);
				}
                [self addSkipBackupAttributeToItemAtURL: logPath];
			}
			else
			{
				NSLog(@"ERROR: Could not create file. Logging to console: %@", logPath);
			}
		}
	}
/*	
	if (self.thread != nil)
	{
		[self.thread cancel];
		self.thread = nil;
		[NSThread sleepForTimeInterval:1]; //wait for 1s
	}
*/	
	// Log on a background thread.
	self.isStarted = YES;
    
    
#if TARGET_OS_IPHONE == 1
    // on iOS, we dont call autorelease because of ARC, while on OS X,
    // as long as we have not enabled ARC we will still use autorelease.
	self.thread = [[NSThread alloc] initWithTarget:self selector:@selector(startRunLoop:) object:nil];
#else
    self.thread = [[[NSThread alloc] initWithTarget:self selector:@selector(startRunLoop:) object:nil] autorelease];
#endif
    
	[self.thread start];
	
	// Log on the main thread.
    if(self.thread == nil)
	    self.thread = [NSThread mainThread];
	
	
	CSILogInfo(@"Started Logging");
	CSILogInfo(@"Identifier: %@", (__bridge NSString *)CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleIdentifierKey));
	CSILogInfo(@"Version: %@", (__bridge NSString *)CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleVersionKey));
	
	// Remove old logs:
	[self removeOldLogs];
}

- (void)stopLogging
{
	CSILogInfo(@"Stopping Logging");
	self.isStarted = NO;
	[self.fileHandle closeFile];
	self.fileHandle = nil;
	// It's not easy to stop console logging here. http://c-faq.com/stdio/undofreopen.html
	// There may be a way, but it's probably not worth the trouble.

#if TARGET_OS_MAC
    release_mutex();
#endif
}

- (void)logMessage:(NSString*)message level:(CSILogLevel)level
{
	//NSString *levelString = [theLevelStrings objectAtIndex:level];
	NSString *logMessage = nil;
	
	if (self.isStarted == YES)
	{
		// If we're logging on the current thread, then keep it synchronous so we don't lose log messages if we crash in this run-loop.
		if ([[NSThread currentThread] isEqual:self.thread])
		{
			//logMessage = [[NSString alloc] initWithFormat:@"%@ - %@", levelString, message];
			[self writeLogMessage:message];
		}
		else
		{
			if ( [[[NSThread currentThread] name] length] == 0 )
			{
				//
				// No thread name set
				pthread_t t = pthread_self();
                uint64_t tid;
                pthread_threadid_np(NULL, &tid);
				
				[[NSThread currentThread] setName:[NSString stringWithFormat:@"%x-%llu", (int)t,tid]];
			}
			logMessage = [[NSString alloc] initWithFormat:@"[%@] - %@", [[NSThread currentThread] name], message];
			
			[self performSelector:@selector(writeLogMessage:) onThread:self.thread withObject:logMessage waitUntilDone:NO];
		}
	}
	else
	{
		//logMessage = [[NSString alloc] initWithFormat:@"%@ - %@", levelString, message];
		NSLog(@"(WARNING: CSILog is not running) %@", message);
	}
    
#if !__has_feature(objc_arc)
    if (logMessage) {
        [logMessage release];
    }
#endif
}

- (void)removeOldLogs
{
	[self performSelectorInBackground:@selector(removeLogsBeforeDate:) withObject:[NSDate dateWithTimeIntervalSinceNow:-self.maximumLogAge]];
}

///////////////////////////////////////////////////////////////////////////////
#pragma mark Threaded methods
///////////////////////////////////////////////////////////////////////////////

//
// (Logging Thread) startRunLoop:
//
- (void)startRunLoop:(id)sender
{
	do
	{
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
	} while (self.isStarted);
	
	
	[self.fileHandle closeFile];
	self.fileHandle = nil;
	self.thread = nil;
}

//
// (Logging Thread) writeLogMessage:
//
- (void)writeLogMessage:(NSString*)message
{
	// Prepend the date and a delimiter '-- ' to the message that is being logged.
	// This will make messages fast and easy to iterate through.
	NSString *dateString = [self.dateFormatter stringFromDate:[NSDate date]];
	NSString *datedMessage = [NSString stringWithFormat:@"%@ %@\n", dateString, message];

    if ( [[[NSThread currentThread] name] length] == 0 )
    {
        
        [[NSThread currentThread] setName:[NSString stringWithFormat:@"CSILog"]];
    }
#if !defined(TA_TEST_MODE) && defined(DEBUG)
	NSLog(@"%@", message);
#endif
	if( self.fileHandle != nil )
	{
		@try
		{
			[self.fileHandle writeData:[datedMessage dataUsingEncoding:NSUTF8StringEncoding]];
		}
		@catch (NSException * e)
		{
			NSLog(@"ERROR: Could not write to log file. Switching to console: %@", e);
			NSLog(@"%@", message);
			self.fileHandle = nil;
		}
	}
	else
	{
		NSLog(@"%@", message);
	}
}

//
// (Background Thread) removeLogsBeforeDate:
//
- (void)removeLogsBeforeDate:(NSDate*)aDate
{
	[NSThread sleepForTimeInterval:30.0];	// Don't even try to do this unless it's a "long run"
	CSILogInfo(@"Cleaning logs older than %@", aDate);
	
	NSString *logDirectory = self.logDirectory;
	NSFileManager *fm = [NSFileManager defaultManager];
	NSDirectoryEnumerator *dirEnum = [fm enumeratorAtPath:logDirectory];
	[dirEnum skipDescendents];
	
	for (NSString *file in dirEnum)
	{
		if ([[file pathExtension] isEqualToString:kLogFileExtension])
		{
			NSDate *fileModDate = [[dirEnum fileAttributes] objectForKey:NSFileModificationDate];
			if( [fileModDate compare:aDate] == NSOrderedAscending )
			{
				CSILogDebug(@"Removing log file: %@", file);
				NSError *error;
				if (![fm removeItemAtPath:[logDirectory stringByAppendingPathComponent:file] error:&error])
				{
					CSILogError(@"Could not remove file (%@): %@", file, [error localizedDescription]);
				}
			}
		}
		[NSThread sleepForTimeInterval:1.0];	// No need to hurry
	}
}

@end
