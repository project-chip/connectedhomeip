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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRAsyncCallbackQueueWorkItem;

typedef void (^MTRAsyncCallbackReadyHandler)(id context, NSUInteger retryCount);

// How to queue a new work item:
//   - Create MTRAsyncCallbackQueueWorkItem object
//   - Create ready handler block (MTRAsyncCallbackReadyHandler)
//      - block is called when it's the work item's turn to do work
//      - its body is to do work with the device
//      - at the end of work, call on the work item object:
//         - endWork for success or failure
//         - retryWork for temporary failures
//   - Set the work handler block to the Item object
//   - Call enqueueWorkItem on the MTRDevice's work queue property

// A serial one-at-a-time queue for performing work items
@interface MTRAsyncCallbackWorkQueue : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (void)enqueueWorkItem:(MTRAsyncCallbackQueueWorkItem *)item;

// TODO: Add a "set concurrency width" method to allow for more than 1 work item at a time
@end

// An item in the work queue
@interface MTRAsyncCallbackQueueWorkItem : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithQueue:(dispatch_queue_t)queue;
@property (nonatomic, strong) MTRAsyncCallbackReadyHandler readyHandler;
@property (nonatomic, strong) dispatch_block_t cancelHandler;

// Called by Cluster object's after async work is done
- (void)endWork;
- (void)retryWork;
@end

NS_ASSUME_NONNULL_END
