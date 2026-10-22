/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * The incoming message or framework event that caused a callback.  Its traceID also
 * appears in log lines and in MTRMetricData recorded while handling it.
 */
NS_SWIFT_SENDABLE
MTR_NEWLY_AVAILABLE
@interface MTRTraceContext : NSObject <NSCopying, NSSecureCoding>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, readonly, copy) NSUUID * traceID;

// The trace of the framework callback running on this thread, or nil outside one.
@property (class, nonatomic, readonly, nullable) MTRTraceContext * currentContext;

@end

NS_ASSUME_NONNULL_END
