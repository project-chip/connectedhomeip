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

#import "MTRAsyncCallbackWorkQueue.h"

NS_ASSUME_NONNULL_BEGIN

@class MTRDevice;

// Optional feature: Work Item Batching
//   When a work item is dequeued to run, if it is of a type that can be combined with similar work items in a batch, this facility
//   gives the client of this API an opportunity to coalesce and merge work items.
//      - The "batching ID" is used for grouping mergeable work items with unique merging strategies. The ID value is opaque to this
//        API, and the API client is responsible for assigning them.
//      - Each work item will only be asked to batch before it's first dequeued to run readyHandler.
// See the MTRAsyncCallbackBatchingHandler definition for more details.

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
// If after the data mutations opaqueDataNext no longer requires any work, the handler should set *fullyMerged to YES to indicate
// that the next item can be dropped from the queue. Otherwise the handler should set *fullyMerged to NO.
//
// If *fullyMerged is set to YES, this handler may be called again to possibly also batch the work item
// after the one that was dropped.
typedef void (^MTRAsyncCallbackBatchingHandler)(id opaqueDataCurrent, id opaqueDataNext, BOOL * fullyMerged);

// Optional feature: Duplicate Filtering
//   This is a facility that enables the API client to check if a potential work item has already been enqueued. By providing a
//   handler that can answer if a work item's relevant data is a duplicate, it can avoid redundant queuing of requests.
//      - The "duplicate type ID" is used for grouping different types of work items for duplicate checking. The ID value is opaque
//        to this API, and the API client is responsible for assigning them.
// See the MTRAsyncCallbackDuplicateCheckHandler definition and the WorkQueue's -isDuplicateForTypeID:workItemData: method
// descriptions for more details.

// The duplicate check handler is called by the work queue when the client wishes to check whether a work item is a duplicate of an
// existing one, so that the client may decide to not enqueue a duplicate work item.
//
// The handler will be passed the opaque data of a potential duplicate work item.
//
// If the handler determines the data is indeed duplicate work, it should set *stop to YES, and set *isDuplicate to YES.
//
// If the handler determines the data is not duplicate work, it should set *stop to YES, and set *isDuplicate to NO.
//
// If the handler is unable to determine if the data is duplicate work, it should set *stop to NO.
// In this case, the value of *isDuplicate is not examined.
typedef void (^MTRAsyncCallbackDuplicateCheckHandler)(id opaqueItemData, BOOL * isDuplicate, BOOL * stop);

@interface MTRAsyncCallbackWorkQueue ()
// The MTRDevice object is only held and passed back as a reference and is opaque to the queue
- (instancetype)initWithContext:(id _Nullable)context queue:(dispatch_queue_t)queue;

// Before creating a work item, a client may call this method to check with existing work items that the new potential work item
// data is not a duplicate request.
//    - This method will call the duplicate check handler for all work items matching the duplicate type ID, starting from the last
//      item in the queue, and if a handler sets *stop to YES, this method will return the value the handler sets for *isDuplicate
//    - If no duplicate check handlers set *stop to YES, this method will return NO.
- (BOOL)isDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID workItemData:(id)opaqueWorkItemData;
@end

@interface MTRAsyncCallbackQueueWorkItem ()
// Batching
@property (nonatomic, readonly) BOOL batchable;
@property (nonatomic, readonly) NSUInteger batchingID;
@property (nonatomic, readonly) id batchableData;
@property (nonatomic, readonly) MTRAsyncCallbackBatchingHandler batchingHandler;
- (void)setBatchingID:(NSUInteger)opaqueBatchingID
                 data:(id)opaqueBatchableData
              handler:(MTRAsyncCallbackBatchingHandler)batchingHandler;

// Duplicate check
@property (nonatomic, readonly) BOOL supportsDuplicateCheck;
@property (nonatomic, readonly) NSUInteger duplicateTypeID;
@property (nonatomic, readonly) MTRAsyncCallbackDuplicateCheckHandler duplicateCheckHandler;
- (void)setDuplicateTypeID:(NSUInteger)opaqueDuplicateTypeID handler:(MTRAsyncCallbackDuplicateCheckHandler)duplicateCheckHandler;
@end

NS_ASSUME_NONNULL_END
