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
 * Marks an MTRDevice command invoke as a user command, bounds how long it may wait in the device's work queue,
 * and lets its caller cancel it while it is still queued.
 *
 * A user command is queued ahead of reads that have not sent anything yet.  The caller creates the token, passes
 * it with the invoke, and keeps it to cancel the invoke later.  One token is for one invoke.
 */
MTR_PROVISIONALLY_AVAILABLE
@interface MTRUserInvokeToken : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * @param timeoutMs  how long, in milliseconds, the invoke may wait in the queue before it is sent.  An invoke still
 *                   unsent when it elapses completes with MTRErrorCodeTimeout and is never sent.  If nil, the wait
 *                   is not bounded.  A value shorter than one session attempt is refused when the invoke is made,
 *                   with MTRErrorCodeInvalidArgument.
 */
- (instancetype)initWithTimeout:(NSNumber * _Nullable)timeoutMs;

@property (nonatomic, readonly, copy, nullable) NSNumber * timeoutMs;

/**
 * Cancels the invoke if it has not been sent yet.  A cancelled invoke completes with MTRErrorCodeCancelled and is
 * never sent.
 *
 * Returns YES if the invoke was still queued and will never be sent, or if the token has not been passed to an
 * invoke yet: an invoke given a token cancelled that way completes with MTRErrorCodeCancelled and is never sent, on
 * any device.  Returns NO if the invoke was already sent or finished, and for an XPC device once the invoke was
 * made.  An invoke already sent completes as usual.
 */
- (BOOL)cancel;

@end

NS_ASSUME_NONNULL_END
