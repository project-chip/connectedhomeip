/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Matter/MTRAttributeValueWaiter.h>
#import <Matter/MTRBaseDevice.h> // For MTRAttributePath.
#import <Matter/MTRCluster.h>
#import <Matter/MTRDevice.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

// Represents the state of a single attribute being waited for: has the path and
// value being waited for in the response-value and whether the value has been
// reached.
MTR_DIRECT_MEMBERS
@interface MTRAwaitedAttributeState : NSObject
@property (nonatomic, assign, readwrite) BOOL valueSatisfied;
@property (nonatomic, retain, readonly) MTRDeviceDataValueDictionary value;

- (instancetype)initWithValue:(MTRDeviceDataValueDictionary)value;
@end

@interface MTRAttributeValueWaiter ()

@property (nonatomic, readonly) BOOL allValuesSatisfied;

- (instancetype)initWithDevice:(MTRDevice *)device values:(NSDictionary<MTRAttributePath *, MTRDeviceDataValueDictionary> *)values queue:(dispatch_queue_t)queue completion:(MTRStatusCompletion)completion;

// Returns YES if after this report the waiter might be done waiting, NO otherwise.
- (BOOL)_attributeValue:(MTRDeviceDataValueDictionary)value reportedForPath:(MTRAttributePath *)path byDevice:(MTRDevice *)device;

- (void)_notifyWithError:(NSError * _Nullable)error;

// Starts the timer for our timeout, using the device's queue as the dispatch
// queuue for the timer firing.
- (void)_startTimerWithTimeout:(NSTimeInterval)timeout;

// Cancels the waiter without trying to remove it from the MTRDevice's
// collection of waiters (unlike "cancel", which does that removal).  This is
// exposed so that MTRDevice can do it when invalidating.
- (void)_notifyCancellation;

@end

NS_ASSUME_NONNULL_END
