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

// A helper struct that facilitates access to _context while
//  - only reading the _context weak reference once and retaining a strong
//    reference for the duration of a particular queue method call
//  - avoiding calls to `[context description]` under our lock
struct ContextSnapshot {
    id _Nullable reference;
    NSString * _Nullable description;
    ContextSnapshot(id _Nullable context)
    {
        reference = context;
        description = [context description];
    }
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
    NSAssert(_state == MTRAsyncWorkItemMutable, @"work item is not mutable (%ld)", (long) _state);
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
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item is not enqueued (%ld)", (long) _state);
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
            MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> executing work item [%llu]", context, uniqueID);
        } else {
            MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> executing work item [%llu] (retry %zd)", context, uniqueID, retryCount);
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
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item was not enqueued (%ld)", (long) _state);
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
        return [NSString stringWithFormat:@"<%@ %llu running retry: %tu>", self.class, _uniqueID, self.retryCount];
    }
    return [NSString stringWithFormat:@"<%@ %llu %@>", self.class, _uniqueID, state];
}

@end

MTR_DIRECT_MEMBERS
@implementation MTRAsyncWorkQueue {
    os_unfair_lock _lock;
    __weak id _context;
    NSMutableArray<MTRAsyncWorkItem *> * _items;
    NSInteger _runningWorkItemCount;
}

- (instancetype)initWithContext:(id)context
{
    NSParameterAssert(context);
    if (self = [super init]) {
        _context = context;
        _items = [NSMutableArray array];
    }
    return self;
}

- (NSString *)description
{
    NSUInteger itemsCount;
    os_unfair_lock_lock(&_lock);
    itemsCount = _items.count;
    os_unfair_lock_unlock(&_lock);
    return [NSString stringWithFormat:@"<%@ context: %@ items count: %tu>", self.class, _context, itemsCount];
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
    ContextSnapshot context(_context); // outside of lock
    NSAssert(context.reference, @"context has been lost");

    os_unfair_lock_lock(&_lock);
    [item markEnqueued];
    [_items addObject:item];

    if (description) {
        // Logging the description once is enough because other log messages
        // related to the work item (execution, completion etc) can easily be
        // correlated using the unique id.
        MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> enqueued work item [%llu]: %@", context.description, item.uniqueID, description);
    } else {
        MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> enqueued work item [%llu]", context.description, item.uniqueID);
    }

    [self _callNextReadyWorkItemWithContext:context];
    os_unfair_lock_unlock(&_lock);
}

- (void)invalidate
{
    ContextSnapshot context(_context); // outside of lock
    std::lock_guard lock(_lock);
    MTR_LOG_INFO("MTRAsyncWorkQueue<%@> invalidate %tu items", context.description, _items.count);
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

    MTRAsyncWorkItem * runningWorkItem = (_runningWorkItemCount) ? _items.firstObject : nil;
    if (workItem != runningWorkItem) {
        NSAssert(NO, @"work item to post-process is not running");
        return;
    }

    if (retry) {
        MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> retry needed for work item [%llu]", context.description, workItem.uniqueID);
    } else {
        [workItem markComplete];
        [_items removeObjectAtIndex:0];
        MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> completed work item [%llu]", context.description, workItem.uniqueID);
    }

    // when "concurrency width" is implemented this will be decremented instead
    _runningWorkItemCount = 0;
    [self _callNextReadyWorkItemWithContext:context];
}

- (void)_callNextReadyWorkItemWithContext:(ContextSnapshot const &)context
{
    os_unfair_lock_assert_owner(&_lock);

    // when "concurrency width" is implemented this will be checked against the width
    if (_runningWorkItemCount) {
        return; // can't run next work item until the current one is done
    }

    if (!_items.count) {
        return; // nothing to run
    }

    if (!context.reference) {
        MTR_LOG_ERROR("MTRAsyncWorkQueue<%@> context has been lost, dropping queued work items", (id) nil);
        [_items removeAllObjects];
        return;
    }

    // when "concurrency width" is implemented this will be incremented instead
    _runningWorkItemCount = 1;

    MTRAsyncWorkItem * workItem = _items.firstObject;

    // Check if batching is possible or needed. Only ask work item to batch once for simplicity
    auto batchingHandler = workItem.batchingHandler;
    if (batchingHandler && workItem.retryCount == 0) {
        while (_items.count >= 2) {
            MTRAsyncWorkItem * nextWorkItem = _items[1];
            if (!nextWorkItem.batchingHandler || nextWorkItem.batchingID != workItem.batchingID) {
                goto done; // next item is not eligible to merge with this one
            }

            switch (batchingHandler(workItem.batchableData, nextWorkItem.batchableData)) {
            case MTRNotBatched:
                goto done; // can't merge anything else
            case MTRBatchedPartially:
                MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> partially merged work item [%llu] into %llu",
                    context.description, nextWorkItem.uniqueID, workItem.uniqueID);
                goto done; // can't merge anything else
            case MTRBatchedFully:
                MTR_LOG_DEFAULT("MTRAsyncWorkQueue<%@> fully merged work item [%llu] into %llu",
                    context.description, nextWorkItem.uniqueID, workItem.uniqueID);
                [_items removeObjectAtIndex:1];
                continue; // try to batch the next item (if any)
            }
        }
    done:;
    }

    mtr_weakify(self);
    [workItem callReadyHandlerWithContext:context.reference completion:^(MTRAsyncWorkOutcome outcome) {
        mtr_strongify(self);
        BOOL handled = NO;
        if (self) {
            ContextSnapshot context(self->_context); // re-acquire a new snapshot
            os_unfair_lock_lock(&self->_lock);
            if (!workItem.isComplete) {
                [self _postProcessWorkItem:workItem context:context retry:(outcome == MTRAsyncWorkNeedsRetry)];
                handled = YES;
            }
            os_unfair_lock_unlock(&self->_lock);
        }
        return handled;
    }];
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
