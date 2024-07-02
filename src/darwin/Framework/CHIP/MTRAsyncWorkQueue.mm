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

#import "MTRAsyncWorkQueue.h"
#import "MTRDefines_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRUnfairLock.h"

#import <atomic>
#import <os/lock.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, MTRAsyncWorkItemState) {
    MTRAsyncWorkItemMutable,
    MTRAsyncWorkItemComplete,
    MTRAsyncWorkItemEnqueued,
    MTRAsyncWorkItemRunning,
    MTRAsyncWorkItemRetryCountBase = MTRAsyncWorkItemRunning, // values >= MTRAsyncWorkItemRunning encode retryCount
};

MTR_DIRECT_MEMBERS
@implementation MTRAsyncWorkItem {
    dispatch_queue_t _queue;
    MTRAsyncWorkItemState _state; // protected by queue lock once enqueued
}

#pragma mark Configuration by the client

- (instancetype)initWithQueue:(dispatch_queue_t)queue
{
    NSParameterAssert(queue);
    if (self = [super init]) {
        static std::atomic<NSUInteger> nextUniqueID(1);
        _uniqueID = nextUniqueID++;
        _queue = queue;
        _state = MTRAsyncWorkItemMutable;
    }
    return self;
}

- (void)setReadyHandler:(nullable void (^)(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion))readyHandler
{
    [self assertMutable];
    _readyHandler = readyHandler;
}

- (void)setCancelHandler:(nullable void (^)(void))cancelHandler
{
    [self assertMutable];
    _cancelHandler = cancelHandler;
}

- (void)setBatchingID:(NSUInteger)opaqueBatchingID data:(id)opaqueBatchableData handler:(MTRAsyncWorkBatchingHandler)batchingHandler
{
    NSParameterAssert(batchingHandler);
    [self assertMutable];
    _batchingID = opaqueBatchingID;
    _batchableData = opaqueBatchableData;
    _batchingHandler = batchingHandler;
}

- (void)setDuplicateTypeID:(NSUInteger)opaqueDuplicateTypeID handler:(MTRAsyncWorkDuplicateCheckHandler)duplicateCheckHandler
{
    NSParameterAssert(duplicateCheckHandler);
    [self assertMutable];
    _duplicateTypeID = opaqueDuplicateTypeID;
    _duplicateCheckHandler = duplicateCheckHandler;
}

- (void)assertMutable
{
    NSAssert(_state == MTRAsyncWorkItemMutable, @"work item is not mutable (%ld)", static_cast<long>(_state));
}

#pragma mark Management by the work queue (queue lock held)

- (void)markEnqueued
{
    [self assertMutable];
    _state = MTRAsyncWorkItemEnqueued;
}

- (NSInteger)retryCount
{
    switch (_state) {
    case MTRAsyncWorkItemMutable:
    case MTRAsyncWorkItemComplete:
    case MTRAsyncWorkItemEnqueued:
        return 0;
    default:
        return ((NSInteger) _state) - MTRAsyncWorkItemRetryCountBase;
    }
}

- (void)callReadyHandlerWithContext:(id)context completion:(MTRAsyncWorkCompletionBlock)completion
{
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item is not enqueued (%ld)", static_cast<long>(_state));
    NSInteger retryCount = 0;
    if (_state == MTRAsyncWorkItemEnqueued) {
        _state = MTRAsyncWorkItemRunning;
    } else if (_state >= MTRAsyncWorkItemRunning) {
        retryCount = (_state - MTRAsyncWorkItemRetryCountBase) + 1; // increment retryCount
        _state = (MTRAsyncWorkItemState) (MTRAsyncWorkItemRetryCountBase + retryCount);
    } else {
        return; // asserted above
    }

    // Always dispatch even if there is no readyHandler as this avoids synchronously
    // re-entering the MTRAsyncWorkQueueCode, simplifying the implementation.
    auto uniqueID = _uniqueID;
    auto readyHandler = _readyHandler;
    dispatch_async(_queue, ^{
        if (!retryCount) {
            MTR_LOG("MTRAsyncWorkQueue<%@> executing work item [%llu]", context, uniqueID);
        } else {
            MTR_LOG("MTRAsyncWorkQueue<%@> executing work item [%llu] (retry %zd)", context, uniqueID, retryCount);
        }
        if (readyHandler) {
            readyHandler(context, retryCount, completion);
        } else {
            completion(MTRAsyncWorkComplete);
        }
    });
}

- (void)cancel
{
    if (_state != MTRAsyncWorkItemComplete) {
        auto cancelHandler = _cancelHandler;
        [self markComplete];
        if (cancelHandler) {
            // Note that if the work item was running it may call the work
            // completion handler before the cancel handler actually runs,
            // however in this case the work completion handler will return
            // NO, giving the work code the ability to deal with this race if
            // necessary.
            dispatch_async(_queue, cancelHandler);
        }
    }
}

- (BOOL)isComplete
{
    return _state == MTRAsyncWorkItemComplete;
}

- (void)markComplete
{
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item was not enqueued (%ld)", static_cast<long>(_state));
    _state = MTRAsyncWorkItemComplete;

    // Clear all handlers in case any of them captured this object.
    _readyHandler = nil;
    _cancelHandler = nil;
    _batchingHandler = nil;
    _duplicateCheckHandler = nil;
}

- (NSString *)description
{
    NSString * state;
    switch (_state) {
    case MTRAsyncWorkItemMutable:
        state = @"mutable";
        break;
    case MTRAsyncWorkItemComplete:
        state = @"complete";
        break;
    case MTRAsyncWorkItemEnqueued:
        state = @"enqueued";
        break;
    default:
        return [NSString stringWithFormat:@"<%@ %llu running retry: %ld>", self.class, _uniqueID, static_cast<long>(self.retryCount)];
    }
    return [NSString stringWithFormat:@"<%@ %llu %@>", self.class, _uniqueID, state];
}

@end

MTR_DIRECT_MEMBERS
@implementation MTRAsyncWorkQueue {
    os_unfair_lock _lock;
    __weak id _context;
    NSMutableArray<MTRAsyncWorkItem *> * _items;
    NSUInteger _runningWorkItemCount;
    NSUInteger _width;
}

// A helper struct that facilitates access to _context while
//  - only reading the _context weak reference once and retaining a strong
//    reference for the duration of a particular queue method call
//  - avoiding calls to `[context description]` under our lock
struct ContextSnapshot {
    id _Nullable reference;
    NSString * _Nullable description;
    ContextSnapshot(MTRAsyncWorkQueue * queue)
    {
        os_unfair_lock_assert_not_owner(&queue->_lock);
        reference = queue->_context;
        description = [reference description];
    }
};

- (instancetype)initWithContext:(id)context
{
    return [self initWithContext:context width:1];
}

- (instancetype)initWithContext:(id)context width:(NSUInteger)width
{
    NSParameterAssert(context);
    if (self = [super init]) {
        _context = context;
        _items = [NSMutableArray array];
        _width = width;
    }
    return self;
}

- (NSString *)description
{
    ContextSnapshot context(self);
    std::lock_guard lock(_lock);
    return [NSString stringWithFormat:@"<%@ context: %@, items count: %lu>", self.class, context.description, static_cast<unsigned long>(_items.count)];
}

- (void)enqueueWorkItem:(MTRAsyncWorkItem *)item
{
    [self enqueueWorkItem:item description:nil];
}

- (void)enqueueWorkItem:(MTRAsyncWorkItem *)item descriptionWithFormat:(NSString *)format, ...
{
    va_list args;
    va_start(args, format);
    NSString * description = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self enqueueWorkItem:item description:description];
}

- (void)enqueueWorkItem:(MTRAsyncWorkItem *)item
            description:(nullable NSString *)description
{
    NSParameterAssert(item);
    ContextSnapshot context(self); // outside of lock
    NSAssert(context.reference, @"context has been lost");

    std::lock_guard lock(_lock);
    [item markEnqueued];
    [_items addObject:item];

    if (description) {
        // Logging the description once is enough because other log messages
        // related to the work item (execution, completion etc) can easily be
        // correlated using the unique id.
        MTR_LOG("MTRAsyncWorkQueue<%@, items count: %lu> enqueued work item [%llu]: %@", context.description, static_cast<unsigned long>(_items.count), item.uniqueID, description);
    } else {
        MTR_LOG("MTRAsyncWorkQueue<%@, items count: %lu> enqueued work item [%llu]", context.description, static_cast<unsigned long>(_items.count), item.uniqueID);
    }

    [self _callNextReadyWorkItemWithContext:context];
}

- (void)invalidate
{
    ContextSnapshot context(self); // outside of lock
    std::lock_guard lock(_lock);
    MTR_LOG("MTRAsyncWorkQueue<%@> invalidate %lu items", context.description, static_cast<unsigned long>(_items.count));
    for (MTRAsyncWorkItem * item in _items) {
        [item cancel];
    }
    [_items removeAllObjects];
}

- (void)_postProcessWorkItem:(MTRAsyncWorkItem *)workItem
                     context:(ContextSnapshot const &)context
                       retry:(BOOL)retry
{
    os_unfair_lock_assert_owner(&_lock);

    BOOL foundWorkItem = NO;
    NSUInteger indexOfWorkItem = 0;
    for (NSUInteger i = 0; i < _width; i++) {
        if (_items[i] == workItem) {
            foundWorkItem = YES;
            indexOfWorkItem = i;
            break;
        }
    }
    if (!foundWorkItem) {
        NSAssert(NO, @"work item to post-process is not running");
        return;
    }

    // already part of the running work items allowed by width - retry directly
    if (retry) {
        MTR_LOG("MTRAsyncWorkQueue<%@> retry needed for work item [%llu]", context.description, workItem.uniqueID);
        [self _callWorkItem:workItem withContext:context];
        return;
    }

    [workItem markComplete];
    [_items removeObjectAtIndex:indexOfWorkItem];
    MTR_LOG("MTRAsyncWorkQueue<%@, items count: %lu> completed work item [%llu]", context.description, static_cast<unsigned long>(_items.count), workItem.uniqueID);

    // sanity check running work item count is positive
    if (_runningWorkItemCount == 0) {
        NSAssert(NO, @"running work item count should be positive");
        return;
    }

    _runningWorkItemCount--;
    [self _callNextReadyWorkItemWithContext:context];
}

- (void)_callWorkItem:(MTRAsyncWorkItem *)workItem withContext:(ContextSnapshot const &)context
{
    os_unfair_lock_assert_owner(&_lock);

    mtr_weakify(self);
    [workItem callReadyHandlerWithContext:context.reference completion:^(MTRAsyncWorkOutcome outcome) {
        mtr_strongify(self);
        BOOL handled = NO;
        if (self) {
            ContextSnapshot context(self); // re-acquire a new snapshot
            std::lock_guard lock(self->_lock);
            if (!workItem.isComplete) {
                [self _postProcessWorkItem:workItem context:context retry:(outcome == MTRAsyncWorkNeedsRetry)];
                handled = YES;
            }
        }
        return handled;
    }];
}

- (void)_callNextReadyWorkItemWithContext:(ContextSnapshot const &)context
{
    os_unfair_lock_assert_owner(&_lock);

    // sanity check not running more than allowed
    if (_runningWorkItemCount > _width) {
        NSAssert(NO, @"running work item count larger than the maximum width");
        return;
    }

    // sanity check consistent counts
    if (_items.count < _runningWorkItemCount) {
        NSAssert(NO, @"work item count is less than running work item count");
        return;
    }

    // can't run more work items if already running at max concurrent width
    if (_runningWorkItemCount == _width) {
        return;
    }

    // no more items to run
    if (_items.count == _runningWorkItemCount) {
        return; // nothing to run
    }

    if (!context.reference) {
        MTR_LOG_ERROR("MTRAsyncWorkQueue<%@> context has been lost, dropping queued work items", (id) nil);
        [_items removeAllObjects];
        return;
    }

    NSUInteger nextWorkItemToRunIndex = _runningWorkItemCount;
    MTRAsyncWorkItem * workItem = _items[nextWorkItemToRunIndex];
    _runningWorkItemCount++;

    // Check if batching is possible or needed.
    auto batchingHandler = workItem.batchingHandler;
    if (batchingHandler) {
        while (_items.count > _runningWorkItemCount) {
            NSUInteger firstNonRunningItemIndex = _runningWorkItemCount;
            MTRAsyncWorkItem * nextWorkItem = _items[firstNonRunningItemIndex];
            if (!nextWorkItem.batchingHandler || nextWorkItem.batchingID != workItem.batchingID) {
                goto done; // next item is not eligible to merge with this one
            }

            switch (batchingHandler(workItem.batchableData, nextWorkItem.batchableData)) {
            case MTRNotBatched:
                goto done; // can't merge anything else
            case MTRBatchedPartially:
                MTR_LOG("MTRAsyncWorkQueue<%@> partially merged work item [%llu] into %llu",
                    context.description, nextWorkItem.uniqueID, workItem.uniqueID);
                goto done; // can't merge anything else
            case MTRBatchedFully:
                MTR_LOG("MTRAsyncWorkQueue<%@> fully merged work item [%llu] into %llu",
                    context.description, nextWorkItem.uniqueID, workItem.uniqueID);
                [_items removeObjectAtIndex:1];
                continue; // try to batch the next item (if any)
            }
        }
    done:;
    }

    [self _callWorkItem:workItem withContext:context];
}

- (BOOL)hasDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID workItemData:(id)opaqueWorkItemData
{
    std::lock_guard lock(_lock);
    // Start from the last item
    for (MTRAsyncWorkItem * item in [_items reverseObjectEnumerator]) {
        auto duplicateCheckHandler = item.duplicateCheckHandler;
        if (duplicateCheckHandler && item.duplicateTypeID == opaqueDuplicateTypeID) {
            BOOL stop = NO;
            BOOL isDuplicate = NO;
            duplicateCheckHandler(opaqueWorkItemData, &isDuplicate, &stop);
            if (stop) {
                return isDuplicate;
            }
        }
    }

    return NO;
}

@end

NS_ASSUME_NONNULL_END
