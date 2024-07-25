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

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, MTRAsyncWorkOutcome) {
    MTRAsyncWorkComplete,
    MTRAsyncWorkNeedsRetry,
};

/// The type of completion handler passed to `MTRAsyncWorkItem`.
/// Return YES if the completion call was valid, or NO if the
/// work item was already completed previously (e.g. due to
/// being cancelled).
typedef BOOL (^MTRAsyncWorkCompletionBlock)(MTRAsyncWorkOutcome outcome);

typedef NS_ENUM(NSInteger, MTRBatchingOutcome) {
    MTRNotBatched = 0,
    MTRBatchedPartially, // some work was batched but the source item has work remaining
    MTRBatchedFully, // the source item is now empty and can be dropped from the queue
};

/// An optional handler that controls batching of MTRAsyncWorkItem.
///
/// When a work item is dequeued to run, if it is of a type that can be
/// combined with similar work items in a batch, this facility provides an
/// opportunity to coalesce and merge work items.
///
/// The batching handler is called by the work queue when all of the following
/// are true:
/// 1) A work item that is batchable is about to be executed for the first time
/// 2) The next work item in the queue is also batchable
/// 3) The two work items have identical batching ids
///
/// The handler will be passed the opaque data of the two work items:
/// `opaqueDataCurrent` is the data of the item about to be executed and
/// `opaqueDataNext` is the data for the next item. The `fullyMerged` parameter
/// will be initialized to NO by the caller.
///
/// The handler is expected to mutate the data as needed to achieve batching,
/// and return an `MTRBatchingOutcome` to indicate if any or all of the work
/// from the next item was merged into the current item. A return value of
/// `MTRBatchedFully` indicates that `opaqueDataNext` no longer requires any
/// work and should be dropped from the queue. In this case, the handler may be
/// called again to possibly also batch the work item after the one that was
/// dropped.
///
/// @see MTRAsyncWorkItem
typedef MTRBatchingOutcome (^MTRAsyncWorkBatchingHandler)(id opaqueDataCurrent, id opaqueDataNext);

/// An optional handler than enables duplicate checking for MTRAsyncWorkItem.
///
/// The duplicate check handler is called when the client wishes to check
/// whether a new candidate work item is a duplicate of an existing queued
/// item, so that the client may decide to not enqueue the duplicate work.
/// Duplicate checking is performed in reverse queue order, i.e. more
/// recently enqueued items will be checked first.
///
/// The handler will be passed the opaque data of the candidate work item. The
/// `stop` and `isDuplicate` parameters will be initialized to NO by the caller.
///
/// If the handler determines the data is indeed duplicate work, it should
/// set `stop` to YES, and set `isDuplicate` to YES.
///
/// If the handler determines the data is not duplicate work, it should set
/// `stop` to YES, and set `isDuplicate` to NO.
///
/// If the handler is unable to determine if the data is duplicate work, it
/// should set `stop` to NO; the value of `isDuplicate` will be ignored.
///
/// @see MTRAsyncWorkItem
typedef void (^MTRAsyncWorkDuplicateCheckHandler)(id opaqueItemData, BOOL * isDuplicate, BOOL * stop);

/// A unit of work that can be run on a `MTRAsyncWorkQueue`.
///
/// A work item can be configured with a number of hander blocks called by the
/// async work queue in various situations. Generally work items will have at
/// least a `readyHandler` (though it is technically optional).
///
/// This class is not thread-safe, and once a work item has been submitted to
/// the queue via `enqueueWorkItem` ownership of the work item passes to the
/// queue. No further modifications may be made to it after that point.
///
/// @see -[MTRAsyncWorkQueue enqueueWorkItem:]
MTR_TESTABLE
@interface MTRAsyncWorkItem<__contravariant ContextType> : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/// Creates a work item that will run on the specified dispatch queue.
- (instancetype)initWithQueue:(dispatch_queue_t)queue;

/// A unique (modulo overflow) ID automatically assigned to each work item for
/// the purpose of correlating log messages from the work queue.
@property (readonly, nonatomic) uint64_t uniqueID;

/// Called by the work queue to start this work item.
///
/// Will be called on the dispatch queue associated with this item.
///
/// This handler block must, synchronously or asynchronously from any thread,
/// call the provided completion block exactly once. Passing an outcome of
/// MTRAsyncWorkComplete removes it from the queue and allows the queue to move
/// on to the next work item (if any).
///
/// Passing an outcome of MTRAsyncWorkNeedsRetry causes the queue to start the
/// work item again with an incremented retryCount. The retryCount is 0 when a
/// work item is executed for the first time.
@property (nonatomic, strong, nullable) void (^readyHandler)
    (ContextType context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion);

/// Called by the work queue to cancel the work item.
///
/// Will be called on the dispatch queue associated with this item.
/// The work item may or may not have been started already.
@property (nonatomic, strong, nullable) void (^cancelHandler)(void);

@property (nonatomic, readonly) NSUInteger batchingID;
@property (nonatomic, readonly, nullable) id batchableData;
@property (nonatomic, readonly, nullable) MTRAsyncWorkBatchingHandler batchingHandler;

/// Sets the batching handler and associated data for this work item.
///
/// Note: This handler is NOT called on the dispatch queue associated with
/// this work item. Thread-safety is managed by the work queue internally.
///
/// If no `batchingHandler` is set using this method, the work item will not
/// participate in batching, and the `batchingID` and `batchableData`
/// properties are  meaningless.
///
/// @see MTRAsyncWorkBatchingHandler
- (void)setBatchingID:(NSUInteger)opaqueBatchingID
                 data:(id)opaqueBatchableData
              handler:(MTRAsyncWorkBatchingHandler)batchingHandler;

@property (nonatomic, readonly) NSUInteger duplicateTypeID;
@property (nonatomic, readonly, nullable) MTRAsyncWorkDuplicateCheckHandler duplicateCheckHandler;

/// Sets the duplicate check type and handler for this work item.
///
/// Note: This handler is NOT called on the dispatch queue associated with
/// this work item. Thread-safety is managed by the work queue internally.
///
/// If no `duplicateCheckHandler` is set using this method, the work item
/// will not participate in duplicate checking, and the `duplicateTypeID`
/// property is meaningless.
///
/// @see MTRAsyncWorkDuplicateCheckHandler
- (void)setDuplicateTypeID:(NSUInteger)opaqueDuplicateTypeID
                   handler:(MTRAsyncWorkDuplicateCheckHandler)duplicateCheckHandler;

@end

/// A serial one-at-a-time queue for performing asynchronous work items.
///
/// Units of work are represented by MTRAsyncWorkItem objects that are
/// configured with one or more handler blocks before being passed to
/// `enqueueWorkItem:`.
///
/// MTRAsyncWorkQueue is thread-safe.
MTR_TESTABLE
@interface MTRAsyncWorkQueue<ContextType> : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/// Creates a work queue with the given context object.
///
/// The context object is weakly held and passed to the readyHandler of work
/// items. This avoids work item blocks accidentally creating a retain cycle
/// by strongly closing over the context object themselves (since the context
/// object will generally be holding a strong reference to the work queue
/// itself). The owner of the queue is responsible for keeping the context
/// object alive; no further work items will be executed if the context object
/// is lost.
- (instancetype)initWithContext:(ContextType)context;

/// Creates a work queue with the given context object and a queue width.
///
/// The queue will call readyHandler on up to "width" number of work items
/// concurrently. Once "width" number of work items have started, no other
/// work items will get a readyHandler call until one of the running work items
/// has called its completion block with MTRAsyncWorkComplete.
///
/// This allows the a MTRAsyncWorkQueue object to manage a pool of
/// resources that can be use concurrently at any given time.
- (instancetype)initWithContext:(ContextType)context width:(NSUInteger)width;

/// Enqueues the specified work item, making it eligible for execution.
///
/// Once a work item is enqueued, ownership of it passes to the queue and
/// no further modifications may be made to it. Work item objects cannot be
/// re-used.
- (void)enqueueWorkItem:(MTRAsyncWorkItem<ContextType> *)item;

/// Same as `enqueueWorkItem:` but includes the description in queue logging.
- (void)enqueueWorkItem:(MTRAsyncWorkItem<ContextType> *)item
            description:(nullable NSString *)description;

/// Convenience for `enqueueWorkItem:description:` with a formatted string.
- (void)enqueueWorkItem:(MTRAsyncWorkItem<ContextType> *)item
    descriptionWithFormat:(NSString *)format, ... NS_FORMAT_FUNCTION(2, 3);

/// Checks whether the queue already contains a work item matching the provided
/// details. A client may call this method to avoid enqueueing duplicate work.
///
/// This method will call the duplicate check handler for all work items
/// matching the duplicate type ID, starting from the last item in the queue
///
/// @see MTRAsyncWorkDuplicateCheckHandler
- (BOOL)hasDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID
                 workItemData:(id)opaqueWorkItemData;

/// Cancels and removes all work items.
- (void)invalidate;
@end

NS_ASSUME_NONNULL_END
