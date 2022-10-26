/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#import "MTRLogger.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A static entry point for logging in the Matter Objective C SDK.
 *
 * @note We declare the public API of `MTRLogging` in the `MTRLogger.h` public header file, while the non-public
 *       module API is declared in the original `MTRLogging.h`.
 */
@interface MTRLogging : NSObject

// Disable construction
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Logs a message at the given level.
 *
 * @param level The level at which the message should be logged.
 * @param format The format of the message to be logged.
 * @param ... Optional arguments to be supplied with the format specifier.
 */
+ (void)logAtLevel:(MTRLogLevel)level format:(NSString *)format, ... NS_FORMAT_FUNCTION(2, 3);

/**
 * Logs a message at the given level.
 *
 * @param level The level at which the message should be logged.
 * @param format The format of the message to be logged.
 * @param args The variadic list of arguments to be supplied to the format specifier.
 */
+ (void)logAtLevelV:(MTRLogLevel)level format:(NSString *)format args:(va_list)args NS_FORMAT_FUNCTION(2, 0);

@end

/**
 * A macro that converts a C string (`char *`) into an `NSString *`.
 */
#define MTR_C_STRING_TO_NSSTRING(x) ((NSString *) CFSTR(x))

/**
 * An always-inlined function that allows us to reliably invoke an Objective C function from a macro without having to deal
 * with non-standard behavior with trailing commas and logs with no arguments.
 */
NS_INLINE NS_FORMAT_FUNCTION(2, 3) void mtr_log(MTRLogLevel level, NSString * format, ...)
{
    va_list args;
    va_start(args, format);
    [MTRLogging logAtLevelV:level format:format args:args];
    va_end(args);
}

#define MTR_LOG_ERROR(format, ...) (mtr_log(MTRLogLevelError, MTR_C_STRING_TO_NSSTRING(format), ##__VA_ARGS__))

#define MTR_LOG_INFO(format, ...) (mtr_log(MTRLogLevelInfo, MTR_C_STRING_TO_NSSTRING(format), ##__VA_ARGS__))

#define MTR_LOG_DEBUG(format, ...) (mtr_log(MTRLogLevelDebug, MTR_C_STRING_TO_NSSTRING(format), ##__VA_ARGS__))

#define MTR_LOG_METHOD_ENTRY()                                                                                                     \
    ({ MTR_LOG_DEBUG("[<%@: %p> %@]", NSStringFromClass([self class]), self, NSStringFromSelector(_cmd)); })

NS_ASSUME_NONNULL_END
