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

#import <os/lock.h>

typedef NS_ENUM(NSInteger, MTRAsyncWorkItemState) {
    MTRAsyncWorkItemMutable,
    MTRAsyncWorkItemComplete,
    MTRAsyncWorkItemEnqueued,
    MTRAsyncWorkItemRunning,
    // values > MTRAsyncWorkItemRunning encode retryCount
};

MTR_DIRECT_MEMBERS
@implementation MTRAsyncWorkItem {
    dispatch_queue_t _queue;
    MTRAsyncWorkItemState _state; // protected by queue lock once enqueued
}

- (instancetype)initWithQueue:(dispatch_queue_t)queue
{
    NSParameterAssert(queue);
    if (self = [super init]) {
        _queue = queue;
        _state = MTRAsyncWorkItemMutable;
    }
    return self;
}

- (void)assertMutable
{
    NSAssert(_state == MTRAsyncWorkItemMutable, @"work item is not mutable (%ld)", (long) _state);
}

- (void)setReadyHandler:(void (^)(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion))readyHandler
{
    [self assertMutable];
    _readyHandler = readyHandler;
}

- (void)setCancelHandler:(void (^)(void))cancelHandler
{
    [self assertMutable];
    _cancelHandler = cancelHandler;
}

- (void)markedEnqueued
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
        return ((NSInteger) _state) - MTRAsyncWorkItemRunning;
    }
}

- (void)callReadyHandlerWithContext:(id)context completion:(MTRAsyncWorkCompletionBlock)completion
{
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item is not enqueued (%ld)", (long) _state);
    NSInteger retryCount = 0;
    if (_state == MTRAsyncWorkItemEnqueued) {
        _state = MTRAsyncWorkItemRunning;
    } else if (_state >= MTRAsyncWorkItemRunning) {
        retryCount = (_state - MTRAsyncWorkItemRunning) + 1; // increment retryCount
        _state = (MTRAsyncWorkItemState) (MTRAsyncWorkItemRunning + retryCount);
    } else {
        return; // asserted above
    }

    // Always dispatch even if there is no readyHandler as this avoids synchronously
    // re-entering the MTRAsyncWorkQueueCode, simplifying the implementation.
    auto readyHandler = _readyHandler;
    dispatch_async(_queue, ^{
        if (readyHandler) {
            readyHandler(context, retryCount, completion);
        } else {
            completion(MTRAsyncWorkComplete);
        }
    });
}

- (BOOL)isComplete
{
    return _state == MTRAsyncWorkItemComplete;
}

- (void)markComplete
{
    NSAssert(_state >= MTRAsyncWorkItemEnqueued, @"work item was not enqueued (%ld)", (long) _state);
    _state = MTRAsyncWorkItemComplete;
}

- (void)cancel
{
    if (_state != MTRAsyncWorkItemComplete) {
        _state = MTRAsyncWorkItemComplete;
        auto cancelHandler = _cancelHandler;
        if (cancelHandler) {
            // Note that this does not prevent a race against
            // the readyHandler calling the work completion.
            dispatch_async(_queue, cancelHandler);
        }
    }
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
    os_unfair_lock_lock(&_lock);
    auto * result = [NSString stringWithFormat:@"<%@ context: %@ items count: %tu>", self.class, _context, _items.count];
    os_unfair_lock_unlock(&_lock);
    return result;
}

- (void)enqueueWorkItem:(MTRAsyncWorkItem *)item
{
    NSParameterAssert(item);
    NSAssert(_context, @"context has been lost");
    [item markedEnqueued];

    os_unfair_lock_lock(&_lock);
    [_items addObject:item];
    [self _callNextReadyWorkItem];
    os_unfair_lock_unlock(&_lock);
}

- (void)invalidate
{
    os_unfair_lock_lock(&_lock);
    MTR_LOG_INFO("MTRAsyncWorkQueue<%@> invalidate %tu items", _context, _items.count);
    for (MTRAsyncWorkItem * item in _items) {
        [item cancel];
    }
    [_items removeAllObjects];
    os_unfair_lock_unlock(&_lock);
}

- (void)_postProcessWorkItem:(MTRAsyncWorkItem *)workItem retry:(BOOL)retry
{
    MTRAsyncWorkItem * runningWorkItem = (_runningWorkItemCount) ? _items.firstObject : nil;
    if (workItem != runningWorkItem) {
        NSAssert(NO, @"work item to post-process is not running");
        return;
    }

    // if work item is done (no need to retry), remove from queue and call ready on the next item
    if (!retry) {
        [workItem markComplete];
        [_items removeObjectAtIndex:0];
    }

    // when "concurrency width" is implemented this will be decremented instead
    _runningWorkItemCount = 0;
    [self _callNextReadyWorkItem];
}

// assume lock is held while calling this
- (void)_callNextReadyWorkItem
{
    // when "concurrency width" is implemented this will be checked against the width
    if (_runningWorkItemCount) {
        return; // can't run next work item until the current one is done
    }

    if (!_items.count) {
        return; // nothing to run
    }

    id context = _context;
    if (!context) {
        MTR_LOG_ERROR("MTRAsyncWorkQueue context has been lost, dropping queued work items");
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
                break; // next item is not eligible to merge with this one
            }

            BOOL fullyMerged = NO;
            batchingHandler(workItem.batchableData, nextWorkItem.batchableData, &fullyMerged);
            if (!fullyMerged) {
                break; // not removing the next item, so we can't merge anything else
            }

            [_items removeObjectAtIndex:1];
        }
    }

    mtr_weakify(self);
    [workItem callReadyHandlerWithContext:context completion:^(MTRAsyncWorkOutcome outcome) {
        mtr_strongify(self);
        BOOL handled = NO;
        if (self) {
            os_unfair_lock_lock(&self->_lock);
            if (!workItem.isComplete) {
                [self _postProcessWorkItem:workItem retry:(outcome == MTRAsyncWorkNeedsRetry)];
                handled = YES;
            }
            os_unfair_lock_unlock(&self->_lock);
        }
        return handled;
    }];
}

- (BOOL)hasDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID workItemData:(id)opaqueWorkItemData
{
    BOOL hasDuplicate = NO;
    os_unfair_lock_lock(&_lock);
    // Start from the last item
    for (MTRAsyncWorkItem * item in [_items reverseObjectEnumerator]) {
        auto duplicateCheckHandler = item.duplicateCheckHandler;
        if (duplicateCheckHandler && item.duplicateTypeID == opaqueDuplicateTypeID) {
            BOOL stop = NO;
            BOOL isDuplicate = NO;
            duplicateCheckHandler(opaqueWorkItemData, &isDuplicate, &stop);
            if (stop) {
                hasDuplicate = isDuplicate;
                break;
            }
        }
    }
    os_unfair_lock_unlock(&_lock);
    return hasDuplicate;
}

@end
