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
typedef NS_ENUM(NSUInteger, MTRLogLevel) { MTRLogLevelError = 0, MTRLogLevelProgress, MTRLogLevelDetail };

// MARK: - MTRLogger

/**
 * An entity that can be used to log messages from the Matter SDK.
 */
@protocol MTRLogger

/**
 * Logs a message at the given level. Logs will be written asynchronously using a logger dispatch queue.
 *
 * @note Messages will only be passed to this method if `isLoggingEnabledAtLevel` is `true` for the given log level.
 */
- (void)logAtLevel:(MTRLogLevel)level message:(NSString *)message;

@end

// MARK: - MTRLogging

/**
 * A static entry point for configuring logging in the Matter C++ and Objective C SDKs.
 */
@interface MTRLoggingConfiguration : NSObject

// Disable construction
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Sets the `MTRLogger` that will be used to log messages from the Matter SDK.
 *
 * @param logger The `MTRLogger` that will be used to log messages from the Matter SDK.
 */
+ (void)setLogger:(id<MTRLogger>)logger;

/**
 * Gets the `MTRLogger` that will be used to log messages from the Matter SDK.
 *
 * @returns The `MTRLogger` that will be used to log messages from the Matter SDK if it was previously set.
 *          Will return null if the logger was never set.
 */
+ (id<MTRLogger>)getLogger;

@end

NS_ASSUME_NONNULL_END

#endif /* MTRLogger_h */
