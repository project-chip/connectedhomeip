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

#include "MTRLogger.h"
#import <MTRLogging.h>
#include <dispatch/dispatch.h>
#import <lib/support/logging/CHIPLogging.h>
#import <os/log.h>
#include <platform/Darwin/Logging.h>
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

- (BOOL)isLoggingEnabledAtLevel:(MTRLogLevel)level
{
    return true;
}

- (void)logAtLevel:(MTRLogLevel)level message:(NSString *)message
{
    switch (level) {
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

static id<MTRLogger> custom_logger;
static id<MTRLogger> os_logger;

// queue used to serialize all work performed by the MTRLogger
static dispatch_queue_t log_work_queue;

/**
 * A log redirection function that will be used to capture logs from the Matter C++ SDK.
 */
static ENFORCE_FORMAT(3, 0) void cpp_log_redirect_callback(const char * module, uint8_t category, const char * msg, va_list args)
{
    char * formatted_msg = ((char *) malloc(CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE));
    chip::Logging::Platform::getDarwinLogMessageFormat(
        module, category, msg, args, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE, formatted_msg);

    // Delegate the log to the MTRLogging system
    dispatch_async([MTRLoggingConfiguration getLogWorkQueue], ^{
        switch (category) {
        case chip::Logging::LogCategory::kLogCategory_Error:
            [MTRLogging logAtLevel:MTRLogLevelError format:@"%s", formatted_msg];
            break;

        case chip::Logging::LogCategory::kLogCategory_Progress:
            [MTRLogging logAtLevel:MTRLogLevelInfo format:@"%s", formatted_msg];
            break;

        case chip::Logging::LogCategory::kLogCategory_Detail:
            [MTRLogging logAtLevel:MTRLogLevelDebug format:@"%s", formatted_msg];
            break;

        default:
            [MTRLogging logAtLevel:MTRLogLevelInfo format:@"%s", formatted_msg];
            break;
        }

        free(formatted_msg);
    });
}

+ (void)setLogger:(id<MTRLogger>)logger
{
    custom_logger = logger;
    chip::Logging::SetLogRedirectCallback(cpp_log_redirect_callback);
}

+ (id<MTRLogger>)getLogger
{
    if (custom_logger == nil) {
        return [MTRLoggingConfiguration getOsLogger];
    }

    return custom_logger;
}

+ (id<MTRLogger>)getOsLogger
{
    if (os_logger == nil) {
        os_logger = [OsLogMTRLogger sharedInstance];
    }
    return os_logger;
}

+ (dispatch_queue_t)getLogWorkQueue
{
    if (log_work_queue == nil) {
        log_work_queue = dispatch_queue_create("com.csa.matter.framework.log.workqueue", DISPATCH_QUEUE_SERIAL);
    }
    return log_work_queue;
}

@end

@implementation MTRLogging

+ (void)logAtLevel:(MTRLogLevel)level format:(NSString *)format, ... NS_FORMAT_FUNCTION(2, 3)
{
    va_list args;
    va_start(args, format);
    [MTRLogging logAtLevelV:level format:format args:args];
    va_end(args);
}

+ (void)logAtLevelV:(MTRLogLevel)level format:(NSString *)format args:(va_list)args NS_FORMAT_FUNCTION(2, 0)
{
    id<MTRLogger> os_logger = [MTRLoggingConfiguration getOsLogger];

    // Fetch the currently-active MTRLogger
    id<MTRLogger> logger = [MTRLoggingConfiguration getLogger];

    // Early return if logging is not currently enabled at the requested level.
    if (logger != nil && ![logger isLoggingEnabledAtLevel:level]) {
        return;
    }

    // Format the message
    NSString * message = [[NSString alloc] initWithFormat:format arguments:args];
    if (logger != nil) {
        dispatch_async([MTRLoggingConfiguration getLogWorkQueue], ^{
            // Log the message to the MTRLogger
            [logger logAtLevel:level message:message];
        });
    }
    [os_logger logAtLevel:level message:message];
}

@end
