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

#ifndef MTRLogger_h
#define MTRLogger_h

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// MARK: - MTRLogLevel

/**
 * An enumeration of the log levels supported by the Matter SDK.
 */
typedef NS_ENUM(NSUInteger, MTRLogLevel) {
    MTRLogLevelFault = 0,
    MTRLogLevelError,
    MTRLogLevelInfo,
    MTRLogLevelDebug
};

// MARK: - MTRLogger

/**
 * An entity that can be used to log messages from the Matter SDK.
 */
@protocol MTRLogger

/**
 * Tests if logs may be emitted at the given log level.
 *
 * @param level The log level to test.
 * @returns `true` if logs may be emitted at the given log level; `false` otherwise.
 */
- (BOOL)isLoggingEnabledAtLevel: (MTRLogLevel)level;

/**
  * Logs a message at the given level.
 *
 * @note Messages will only be passed to this method if `isLoggingEnabledAtLevel` is `true` for the given log level.
 */
- (void)logAtLevel: (MTRLogLevel)level
           message: (NSString *)message;

@end

// MARK: - MTRLogging

/**
 * A static entry point for configuring logging in the Matter C++ and Objective C SDKs.
 */
@interface MTRLoggingConfiguration : NSObject {}

// Disable construction
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Sets the `MTRLogger` that will be used to log messages from the Matter SDK.
 *
 * @param logger The `MTRLogger` that will be used to log messages from the Matter SDK.
 */
+ (void)setLogger: (id<MTRLogger>)logger;

/**
 * Gets the `MTRLogger` that will be used to log messages from the Matter SDK.
 * 
 * If no logger has been [set](@ref setLogger), all messages at all log levels will be logged
 * to the `os_log`.
 * 
 * @returns The `MTRLogger` that will be used to log messages from the Matter SDK.
 */
+ (id<MTRLogger>)getLogger;

@end

NS_ASSUME_NONNULL_END

#endif /* MTRLogger_h */
