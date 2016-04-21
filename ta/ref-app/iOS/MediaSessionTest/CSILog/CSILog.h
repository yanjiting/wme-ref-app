//
//  CSILog.h
//  Logger
//
//  Created by Rob Napier on 12/3/08.
//  Copyright 2008 Cisco. All rights reserved.
//

// Logging functions. "Trace" functions log the current method.
// NOTE: The following require CSI_DEBUG be defined:
//		* Logging to both console and log files
//		* Turning on levels below CSILogLevelDebug

/*!
 CSILog provides centralized logging with level control. Setting a particular Log
 level will enable logs for that level and all higher levels ("higher" levels actually
 have lower numbers in the enum).
 
 Logging is performed to a seperate file per-run. Files older than maximumLogAge
 are automatically removed when logging starts.
 
 In debug mode, logs are also sent to the console. In release mode, the console is
 redirected to separate ConsoleLog files. These are also per-run and are aged in
 the same way as regular logs.
 
 CSILog automatically posts things at the Error and Bug level to CSIAnalytics.
 
 CSILog also provides various assertion macros integrated with the CSILogBug level.
 These work very similarly to their NSAssert() parallels, but they always log assertion
 failures, even in Release mode. This means that they are not compiled out in Release
 mode and therefore have the performance overhead of whatever condition is passed to
 them.
 
 CSILog can be configured to run on a background thread, but is hard-coded to run
 on the main thread. If the application crashes while logging on a background thread,
 it is likely that the last log message will be lost (possibly more). There is a major
 tradeoff here because writing to a file is very expensive on iPhone and blocks the 
 main thread. Logging can easily have a non-trivial impact on performance, and can
 swamp the CPU on iPhone. But without detailed logs, it's extremely difficult to 
 debug crashes. You've been warned. Log accordingly.
 
 CSILog can be configured in Info.plist by setting CSILoggingLevel to the integer
 for the appropriate enum value. Typically this will be 4 (CSILogLevelDebug). If this
 key is not set, then the default is Debug in Debug configuration and Info in Release.
 */

#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	//
	// Log Levels
	//
	enum CSILogLevel
	{
		CSILogLevelBug = 0,			// Called "INTERNAL" in the output
		CSILogLevelError,
		CSILogLevelWarning,
		CSILogLevelInfo,
		CSILogLevelDebug,
		CSILogLevelTrace
	};
	typedef enum CSILogLevel CSILogLevel;
	
	//
	// Logging functions. All take printf-style formats
	// Remember, you can't inline varargs, so don't try.
	// Note that this format attribute isn't as powerful as the __printf__ one.
	// It won't protect you against type-mismatch as much as you'd like, but as
	// they improve the compiler, it may someday.
	void CSILogMessage(CSILogLevel level, NSString* message, ...) __attribute__((format(__NSString__, 2, 3)));
	
	
#define CSILogBug(format, args...) CSILogMessage(CSILogLevelBug, format , ## args)
#define CSILogError(format, args...) CSILogMessage(CSILogLevelError, format , ## args)
#define CSILogWarning(format, args...) CSILogMessage(CSILogLevelWarning, format , ## args)
#define CSILogInfo(format, args...) CSILogMessage(CSILogLevelInfo, format , ## args)
#define CSILogDebug(format, args...) CSILogMessage(CSILogLevelDebug, format , ## args)
#define CSILogDebugTrace() CSILogDebug(@"%@", [NSString stringWithUTF8String:__func__])
	
#ifdef __cplusplus
}
#endif

// CSIAssert() macros work exactly like NSAssert() macros except that they also log, even in release mode

#define _CSIAssertBody(condition, desc, arg1, arg2, arg3, arg4, arg5) \
if (!(condition)) {				\
CSILogBug((desc), (arg1), (arg2), (arg3), (arg4), (arg5)); \
_NSAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5)); \
}

#define CSIAssert5(condition, desc, arg1, arg2, arg3, arg4, arg5)	\
_CSIAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5))

#define CSIAssert4(condition, desc, arg1, arg2, arg3, arg4)	\
_CSIAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), 0)

#define CSIAssert3(condition, desc, arg1, arg2, arg3)	\
_CSIAssertBody((condition), (desc), (arg1), (arg2), (arg3), 0, 0)

#define CSIAssert2(condition, desc, arg1, arg2)		\
_CSIAssertBody((condition), (desc), (arg1), (arg2), 0, 0, 0)

#define CSIAssert1(condition, desc, arg1)		\
_CSIAssertBody((condition), (desc), (arg1), 0, 0, 0, 0)

#define CSIAssert(condition, desc)			\
_CSIAssertBody((condition), (desc), 0, 0, 0, 0, 0)


// CSICAssert() macros work exactly like NSCAssert() macros except that they also log, even in release mode
#define _CSICAssertBody(condition, desc, arg1, arg2, arg3, arg4, arg5) \
if (!(condition)) {				\
CSILogBug((desc), (arg1), (arg2), (arg3), (arg4), (arg5)); \
_NSCAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5)); \
}

#define CSICAssert5(condition, desc, arg1, arg2, arg3, arg4, arg5)	\
_CSICAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), (arg5))

#define CSICAssert4(condition, desc, arg1, arg2, arg3, arg4)	\
_CSICAssertBody((condition), (desc), (arg1), (arg2), (arg3), (arg4), 0)

#define CSICAssert3(condition, desc, arg1, arg2, arg3)	\
_CSICAssertBody((condition), (desc), (arg1), (arg2), (arg3), 0, 0)

#define CSICAssert2(condition, desc, arg1, arg2)		\
_CSICAssertBody((condition), (desc), (arg1), (arg2), 0, 0, 0)

#define CSICAssert1(condition, desc, arg1)		\
_CSICAssertBody((condition), (desc), (arg1), 0, 0, 0, 0)

#define CSICAssert(condition, desc)			\
_CSICAssertBody((condition), (desc), 0, 0, 0, 0, 0)

//////////////////////////////////////////////////////////////////////////////

#define CSIAbstract() CSIAssert1(NO, @"Called abstract method: %s",  __func__)

//
// CSILog
//
@class CSIAnalytics;

@interface CSILog : NSObject
{
	NSString *myLogDirectory;
	NSString *myLogName;
	NSFileHandle *myFileHandle;
	CSILogLevel myLevel;
	NSDateFormatter *myDateFormatter;
	NSThread *myThread;
	BOOL myIsStarted;
	NSTimeInterval myMaximumLogAge;
	CSIAnalytics* myAnalytics;
}

// These are intentionally atomic to remind you that these are accessed on multiple threads.
// Atomicy doesn't actually matter for non-objects.
@property (readwrite, assign) CSILogLevel level;
@property (readwrite, assign) BOOL isStarted;
@property (readwrite, assign) NSTimeInterval maximumLogAge;
@property (readwrite, retain) NSThread * thread;
@property (readwrite, retain) NSDateFormatter * dateFormatter;
@property (nonatomic, readwrite, retain) CSIAnalytics* analytics;

// Shared instance
+ (id)sharedLog;
+ (void)setSharedLog:(CSILog*)log;

/*!
 Designated initializer. Generally +sharedLog can be used instead of this.
 \param directory : Path to the directory to place logs in. This directory heirarchy will be created if needed
 \param logName : Base name of the log. The dateTime will be appended to this, along with an extension.
 "-Console" will be appended for the console log.
 */
- (id)initWithDirectory:(NSString*)directory logName:(NSString*)logName;

/*!
 Begins logging, including console logging in Release mode. We may want to make
 console logging an option, but so far everyone wants it and it's easy to add later.
 */
- (void)startLogging;

/*!
 Stops logging. Doesn't stop console logging. Nothing can stop console logging.
 http://c-faq.com/stdio/undofreopen.html
 */
- (void)stopLogging;

/*!
 Log a message at a level. Generally it is easier to use the CSILog...() macros.
 */

- (void)logMessage:(NSString*)message level:(CSILogLevel)level;

/*!
 Removes old logs in the background based on maximimumLogAge. Log aging is based on
 modification date, not the filename.
 */
- (void)removeOldLogs;

- (BOOL)addSkipBackupAttributeToItemAtURL:(NSString *)path;

@end