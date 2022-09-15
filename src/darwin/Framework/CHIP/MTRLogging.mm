/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#import <lib/support/logging/CHIPLogging.h>
#import <MTRLogging.h>
#import <os/log.h>
#import <pthread.h>

// MARK: - OsLogMTRLogger

/**
 * An implementation of the MTRLogger protocol that logs messages to `os_log`.
 */
@interface OsLogMTRLogger : NSObject <MTRLogger>

// Disable construction
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * @returns The singleton OsLogMTRLogger
 */
+ (instancetype)sharedInstance;

@end

@implementation OsLogMTRLogger

+ (instancetype)sharedInstance
{
    static OsLogMTRLogger * singleton = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the factory.
        singleton = [[OsLogMTRLogger alloc] init];
    });
    return singleton;
}

- (instancetype)init
{
    return self;
}

- (BOOL)isLoggingEnabledAtLevel: (MTRLogLevel)level {
    return true;
}

- (void)logAtLevel: (MTRLogLevel)level
           message: (NSString *)message {
    switch (level) {
        case MTRLogLevelFault:
            os_log_fault(OS_LOG_DEFAULT, "%@", message);
            break;
        case MTRLogLevelError:
            os_log_error(OS_LOG_DEFAULT, "%@", message);
            break;
        case MTRLogLevelInfo:
            os_log_info(OS_LOG_DEFAULT, "%@", message);
            break;
        case MTRLogLevelDebug:
            os_log_debug(OS_LOG_DEFAULT, "%@", message);
            break;
        default:
            break;
    }
}

@end

// MARK: - MTRLogging

@implementation MTRLoggingConfiguration

static id<MTRLogger> singleton_logger;

/**
 * A log redirection function that will be used to capture logs from the Matter C++ SDK.
 */
static ENFORCE_FORMAT(3, 0) void cpp_log_redirect_callback(const char * module, uint8_t category, const char * msg, va_list args)
{
    // Pre-format the message. Additional information will be prepended and appended before
    // logging.
    char formatted_msg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    int32_t formatted_msg_length = vsnprintf(formatted_msg, sizeof(formatted_msg), msg, args);
    if (formatted_msg_length < 0)
    {
        // In the event that we could not format the message, simply copy over the
        // unformatted message. It won't contain all the information that we need, but
        // it's better than dropping a log on the floor.
        strncpy(formatted_msg, msg, sizeof(formatted_msg));
        return;
    }

    // Explicitly NULL-terminate the formatted string for safety.
    formatted_msg[sizeof(formatted_msg) - 1] = '\0';

    // Collect the process and thread information to match the original CHIPLogging implementation
    // in src/platform/Darwin/Logging.cpp
    uint64_t ktid;
    pthread_threadid_np(NULL, &ktid);
    uint64_t pid = getpid();

    // Delegate the log to the MTRLogging system
    switch (category) {
        case chip::Logging::LogCategory::kLogCategory_Error:
            [MTRLogging logAtLevel:MTRLogLevelError
                            format:@"%@ [%lld:%lld] CHIP: [%s] %s",
                 @"🔴",
                 (long long) pid,
                 (long long) ktid,
                 module,
                 formatted_msg];
            break;

        case chip::Logging::LogCategory::kLogCategory_Progress:
            [MTRLogging logAtLevel:MTRLogLevelInfo
                            format:@"%@ [%lld:%lld] CHIP: [%s] %s",
                 @"🔵",
                 (long long) pid,
                 (long long) ktid,
                 module,
             formatted_msg];
            break;

        case chip::Logging::LogCategory::kLogCategory_Detail:
            [MTRLogging logAtLevel:MTRLogLevelDebug
                            format:@"%@ [%lld:%lld] CHIP: [%s] %s",
                 @"🟢",
                 (long long) pid,
                 (long long) ktid,
                 module,
                 formatted_msg];
            break;
        default:
            [MTRLogging logAtLevel:MTRLogLevelInfo
                            format:@"[%lld:%lld] CHIP: [%s] %s",
                (long long) pid,
                (long long) ktid,
                module,
                formatted_msg];
            break;
    }
}

+ (void)setLogger: (id<MTRLogger>)logger
{
    singleton_logger = logger;
    chip::Logging::SetLogRedirectCallback(cpp_log_redirect_callback);
}

+ (id<MTRLogger>)getLogger
{
    return singleton_logger;
}

@end

@implementation MTRLogging

+ (void)logAtLevel: (MTRLogLevel)level
            format: (NSString*) format, ... NS_FORMAT_FUNCTION(2,3) {
    va_list args;
    va_start(args, format);
    [MTRLogging logAtLevelV:level
                     format:format
                       args:args];
    va_end(args);
}

+ (void)logAtLevelV: (MTRLogLevel)level
             format: (NSString*) format
               args: (va_list) args NS_FORMAT_FUNCTION(2,0)
{
    // Fetch the currently-active MTRLogger
    id<MTRLogger> logger = [MTRLoggingConfiguration getLogger];
    if (logger == nil)
    {
        logger = [OsLogMTRLogger sharedInstance];
    }

    // Early return if logging is not currently enabled at the requested level.
    if (![logger isLoggingEnabledAtLevel:level])
    {
        return;
    }

    // Format the message
    NSString *message = [[NSString alloc] initWithFormat:format
                                               arguments:args];

    // Log the message to the MTRLogger
    [logger logAtLevel:level message:message];
}

@end

