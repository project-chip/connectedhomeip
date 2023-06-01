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

// The batching handler is called by the work queue when all of the following are true:
//
// 1) A work item that is batchable is about to be dequeued and executed for the first time.
// 2) The next work item in the queue is also batchable.
// 3) The two work items have matching batching ids.
//
// The handler will be passed the opaque data of the two work items: opaqueDataCurrent is the data of the
// item about to be executed and opaqueDataNext is the data for the next item.
//
// The handler is expected to mutate the data as needed to achieve batching.
//
// If after the data mutations opaqueDataNext no longer requires any work, the handler
// should set *fullyMerged to YES to indicate that the next item can be dropped from the queue.
//  Otherwise the handler should set *fullyMerged to NO.
//
// If *fullyMerged is set to YES, this handler may be called again to possibly also batch the work item
// after the one that was dropped.
typedef void (^MTRAsyncCallbackBatchingHandler)(id opaqueDataCurrent, id opaqueDataNext, BOOL * fullyMerged);

// The duplicate check handler is called by the work queue when the client wishes to check whether a work item is a duplicate of an
// existing one, so that the client can decide to not enqueue the new duplicate.
typedef void (^MTRAsyncCallbackDuplicateCheckHandler)(id opaqueItemData, BOOL * isDuplicate);

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

// Optional feature: Work Item Batching
//   When a work item is dequeued to run, if it is of a type that can be combined with similar work items in a batch, this facility
//   gives the client of this API an opportunity to coalesce and merge work items.
//      - The "batching ID" is used for grouping mergeable work items with unique merging strategies. The ID value is opaque to this
//        API, and the API client is responsible for assigning them.
//      - Each work item will only be asked to batch before it's first dequeued to run readyHandler.
// See the MTRAsyncCallbackBatchingHandler definition above and the WorkItem's -setBatchingID:data:handler: method description for
// more details.

// Optional feature: Duplicate Filtering
//   This is a facility that enables the API client to check if a potential work item has already been enqueued. By providing a
//   handler that can answer if a work item's relevant data is a duplicate, it can avoid redundant queuing of requests.
//      - The "duplicate type ID" is used for grouping different types of work items for duplicate checking. The ID value is opaque
//        to this API, and the API client is responsible for assigning them.
// See the MTRAsyncCallbackDuplicateCheckHandler definition above and the WorkItem's -setDuplicateTypeID:handler: method description
// for more details.

// A serial one-at-a-time queue for performing work items
@interface MTRAsyncCallbackWorkQueue : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// The context object is only held and passed back as a reference and is opaque to the work queue
- (instancetype)initWithContext:(id _Nullable)context queue:(dispatch_queue_t)queue;

// Called by the work queue owner to clean up and cancel work items
- (void)invalidate;

// Work items may be enqueued from any queue or thread
// Note: Once a work item is enqueued, its handlers cannot be modified
- (void)enqueueWorkItem:(MTRAsyncCallbackQueueWorkItem *)item;

// Before creating a work item, a client may call this method to check with existing work items that the new potential work item
// data is not a duplicate request. The work queue will then look for all work items matching the duplicate type ID, and call their
// duplicateCheckHandler with the provided opaqueWorkItemData.
//
// Returns YES if any item's duplicateCheckHandler returns a match.
- (BOOL)isDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID workItemData:(id)opaqueWorkItemData;
@end

// An item in the work queue
@interface MTRAsyncCallbackQueueWorkItem : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Both readyHandler and cancelHander will be called on the queue given to initWithQueue
- (instancetype)initWithQueue:(dispatch_queue_t)queue;
@property (nonatomic, strong) MTRAsyncCallbackReadyHandler readyHandler;
@property (nonatomic, strong) dispatch_block_t cancelHandler;

// For work items that can be merged into a batch, set this handler with an identifier and an object that represents the mergeable
// data. When the work queue processes a batchable item, if the next item is also batchable with the same batching identifier, the
// work queue will call the batchingHandler to give the work item an opportunity to merge the data before readyHandler is called.
// Should the two items be completely merged into one batch, the batchingHandler can signal that through the out argument
// "fullyMerged", and the work queue will remove the second item.
- (void)setBatchingID:(NSUInteger)opaqueBatchingID
                 data:(id)opaqueBatchableData
              handler:(MTRAsyncCallbackBatchingHandler)batchingHandler;

// For work items that may have duplicates, set this handler with an identifier and a handler
- (void)setDuplicateTypeID:(NSUInteger)opaqueDuplicateTypeID handler:(MTRAsyncCallbackDuplicateCheckHandler)duplicateCheckHandler;

// Called by the creater of the work item when async work is done and should
// be removed from the queue. The work queue will run the next work item.
// Note: This must only be called from within the readyHandler
- (void)endWork;

// Called by the creater of the work item when async work should be retried.
// The work queue will call this workItem's readyHandler again.
// Note: This must only be called from within the readyHandler
- (void)retryWork;
@end

NS_ASSUME_NONNULL_END
