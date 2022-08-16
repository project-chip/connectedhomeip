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

// MTRAsyncCallbackQueue high level description
//   The MTRAsyncCallbackQueue was made to call one readyHandler
//   block at a time asynchronously, and the readyHandler is
//   expected to start/schedule a task. When the task finishes
//   asynchronously in the future (at any time, from any queue
//   or thread), it is expected to ask the workItem object to
//   either endWork or retryWork.

// Sequence of steps when queuing a work item:
//   - Create MTRAsyncCallbackQueueWorkItem object
//   - Create ready handler block (MTRAsyncCallbackReadyHandler)
//      - block is called when it's the WorkItem's turn to do work
//      - its body is to perform a task that is expected to end asynchronously in the future
//      - at the end of work, call on the work item object:
//         - endWork for success or failure
//         - retryWork for temporary failures
//   - Set the readyHandler block on the WorkItem object
//   - Call enqueueWorkItem on a MTRAsyncCallbackQueue

// A serial one-at-a-time queue for performing work items
@interface MTRAsyncCallbackWorkQueue : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// The context object is only held and passed back as a reference and is opaque to the work queue
- (instancetype)initWithContext:(id _Nullable)context queue:(dispatch_queue_t)queue;

// Called by the work queue owner to clean up and cancel work items
- (void)invalidate;

// Work items may be enqueued from any queue or thread
- (void)enqueueWorkItem:(MTRAsyncCallbackQueueWorkItem *)item;

// TODO: Add a "set concurrency width" method to allow for more than 1 work item at a time
@end

// An item in the work queue
@interface MTRAsyncCallbackQueueWorkItem : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Both readyHandler and cancelHander will be called on the queue given to initWithQueue
- (instancetype)initWithQueue:(dispatch_queue_t)queue;
@property (nonatomic, strong) MTRAsyncCallbackReadyHandler readyHandler;
@property (nonatomic, strong) dispatch_block_t cancelHandler;

// Called by the creater of the work item when async work is done and should
// be removed from the queue. The work queue will run the next work item.
- (void)endWork;

// Called by the creater of the work item when async work should be retried.
// The work queue will call this workItem's readyHandler again.
- (void)retryWork;
@end

NS_ASSUME_NONNULL_END
