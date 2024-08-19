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

#import <Foundation/Foundation.h>
#import "MTRDefines_Internal.h"

@protocol MTRDeviceDelegate;

NS_ASSUME_NONNULL_BEGIN

// container of MTRDevice delegate weak reference, its queue, and its interested paths for attribute reports
MTR_DIRECT_MEMBERS
@interface MTRDeviceDelegateInfo : NSObject {
@private
    void * _delegatePointerValue;
    __weak id _delegate;
    dispatch_queue_t _queue;
    NSArray * _Nullable _interestedPathsForAttributes;
    NSArray * _Nullable _interestedPathsForEvents;
}

// Array of interested cluster paths, attribute paths, or endpointID, for attribute report filtering.
@property (readonly, nullable) NSArray * interestedPathsForAttributes;

// Array of interested cluster paths, attribute paths, or endpointID, for event report filtering.
@property (readonly, nullable) NSArray * interestedPathsForEvents;

// Expose delegate
@property (readonly) id delegate;

// Pointer value for logging purpose only
@property (readonly) void * delegatePointerValue;

- (instancetype)initWithDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents;

// Returns YES if delegate and queue are both non-null, and the block is scheduled to run.
- (BOOL)callDelegateWithBlock:(void (^)(id<MTRDeviceDelegate>))block;

#ifdef DEBUG
// Only used for unit test purposes - normal delegate should not expect or handle being called back synchronously.
- (BOOL)callDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate>))block;
#endif

@end

NS_ASSUME_NONNULL_END
