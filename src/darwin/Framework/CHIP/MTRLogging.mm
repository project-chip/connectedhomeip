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
#include <platform/Darwin/Logging.h>

// MARK: - MTRLogging

@implementation MTRLoggingConfiguration

static id<MTRLogger> custom_logger;

// queue used to serialize all work performed by the MTRLogger
static dispatch_queue_t log_work_queue;

/**
 * A log redirection function that will be used to capture logs from the Matter C++ SDK.
 */
static ENFORCE_FORMAT(3, 0) void cpp_log_redirect_callback(const char * module, uint8_t category, const char * msg, va_list args)
{
    char * formatted_msg = new char[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    chip::Logging::Platform::getDarwinLogMessageFormat(
        module, category, msg, args, CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE, formatted_msg);
    NSString * msg_string = [[NSString alloc] initWithCString:formatted_msg encoding:NSUTF8StringEncoding];

    // Delegate the log to the MTRLogging system
    dispatch_async([MTRLoggingConfiguration getLogWorkQueue], ^{
        switch(category) {
        case chip::Logging::LogCategory::kLogCategory_Error:
            [custom_logger logAtLevel:MTRLogLevelError message:msg_string];
            break;

        case chip::Logging::LogCategory::kLogCategory_Progress:
            [custom_logger logAtLevel:MTRLogLevelProgress message:msg_string];
            break;

        case chip::Logging::LogCategory::kLogCategory_Detail:
            [custom_logger logAtLevel:MTRLogLevelDetail message:msg_string];
            break;

        default:
            [custom_logger logAtLevel:MTRLogLevelProgress message:msg_string];
            break;
        }
    });
}

+ (void)setLogger:(id<MTRLogger>)logger
{
    custom_logger = logger;
    chip::Logging::SetLogRedirectCallback(cpp_log_redirect_callback);
}

+ (id<MTRLogger>)getLogger
{
    return custom_logger;
}

+ (dispatch_queue_t)getLogWorkQueue
{
    if (log_work_queue == nil) {
        log_work_queue = dispatch_queue_create("com.csa.matter.framework.log.workqueue", DISPATCH_QUEUE_SERIAL);
    }
    return log_work_queue;
}

@end
