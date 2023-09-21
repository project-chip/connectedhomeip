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

#import <dispatch/dispatch.h>
#import <os/lock.h>

#import "MTRAsyncWorkQueue_Internal.h"
#import "MTRLogging_Internal.h"

#pragma mark - Class extensions

@interface MTRAsyncWorkQueue ()
// The lock protects the internal state of the work queue so that these may be called from any queue or thread:
//   -enqueueWorkItem:
//   -invalidate
//   -endWork:
//   -retryWork:
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic, strong, readonly) id context;
@property (nonatomic, strong, readonly) dispatch_queue_t queue;
@property (nonatomic, strong, readonly) NSMutableArray<MTRAsyncWorkItem *> * items;
@property (nonatomic, readwrite) NSUInteger runningWorkItemCount;

// For WorkItem's use only - the parameter is for sanity check
- (void)endWork:(MTRAsyncWorkItem *)workItem;
- (void)retryWork:(MTRAsyncWorkItem *)workItem;
@end

@interface MTRAsyncWorkItem ()
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic, strong, readonly) dispatch_queue_t queue;
@property (nonatomic, readwrite) NSUInteger retryCount;
@property (nonatomic, strong) MTRAsyncWorkQueue * workQueue;
@property (nonatomic, readonly) BOOL enqueued;
// Called by the queue
- (void)markedEnqueued;
- (void)callReadyHandlerWithContext:(id)context;
- (void)cancel;
@end

#pragma mark - Class implementations

@implementation MTRAsyncWorkQueue
- (instancetype)initWithContext:(id)context queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _context = context;
        _queue = queue;
        _items = [NSMutableArray array];
        MTR_LOG_INFO("MTRAsyncCallbackWorkQueue init for context %@", context);
    }
    return self;
}

- (NSString *)description
{
    os_unfair_lock_lock(&_lock);

    auto * desc = [NSString
        stringWithFormat:@"MTRAsyncCallbackWorkQueue context: %@ items count: %lu", self.context, (unsigned long) self.items.count];

    os_unfair_lock_unlock(&_lock);

    return desc;
}

- (void)enqueueWorkItem:(MTRAsyncWorkItem *)item
{
    if (item.enqueued) {
        MTR_LOG_ERROR("MTRAsyncCallbackWorkQueue enqueueWorkItem: item cannot be enqueued twice");
        return;
    }

    [item markedEnqueued];

    os_unfair_lock_lock(&_lock);
    item.workQueue = self;
    [self.items addObject:item];

    [self _callNextReadyWorkItem];
    os_unfair_lock_unlock(&_lock);
}

- (void)invalidate
{
    os_unfair_lock_lock(&_lock);
    NSMutableArray * invalidateItems = _items;
    _items = nil;
    os_unfair_lock_unlock(&_lock);

    MTR_LOG_INFO(
        "MTRAsyncCallbackWorkQueue invalidate for context %@ items count: %lu", _context, (unsigned long) invalidateItems.count);
    for (MTRAsyncWorkItem * item in invalidateItems) {
        [item cancel];
    }
    [invalidateItems removeAllObjects];
}

// called after executing a work item
- (void)_postProcessWorkItem:(MTRAsyncWorkItem *)workItem retry:(BOOL)retry
{
    os_unfair_lock_lock(&_lock);
    // sanity check if running
    if (!self.runningWorkItemCount) {
        // something is wrong with state - nothing is currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("MTRAsyncCallbackWorkQueue endWork: no work is running on work queue");
        return;
    }

    // sanity check the same work item is running
    // when "concurrency width" is implemented need to check first N items
    MTRAsyncWorkItem * firstWorkItem = self.items.firstObject;
    if (firstWorkItem != workItem) {
        // something is wrong with this work item - should not be currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("MTRAsyncCallbackWorkQueue endWork: work item is not first on work queue");
        return;
    }

    // if work item is done (no need to retry), remove from queue and call ready on the next item
    if (!retry) {
        [self.items removeObjectAtIndex:0];
    }

    // when "concurrency width" is implemented this will be decremented instead
    self.runningWorkItemCount = 0;
    [self _callNextReadyWorkItem];
    os_unfair_lock_unlock(&_lock);
}

- (void)endWork:(MTRAsyncWorkItem *)workItem
{
    [self _postProcessWorkItem:workItem retry:NO];
}

- (void)retryWork:(MTRAsyncWorkItem *)workItem
{
    [self _postProcessWorkItem:workItem retry:YES];
}

// assume lock is held while calling this
- (void)_callNextReadyWorkItem
{
    // when "concurrency width" is implemented this will be checked against the width
    if (self.runningWorkItemCount) {
        // can't run next work item until the current one is done
        return;
    }

    // only proceed to mark queue as running if there are items to run
    if (self.items.count) {
        // when "concurrency width" is implemented this will be incremented instead
        self.runningWorkItemCount = 1;

        MTRAsyncWorkItem * workItem = self.items.firstObject;

        // Check if batching is possible or needed. Only ask work item to batch once for simplicity
        if (workItem.batchable && workItem.batchingHandler && (workItem.retryCount == 0)) {
            while (self.items.count >= 2) {
                MTRAsyncWorkItem * nextWorkItem = self.items[1];
                if (!nextWorkItem.batchable || (nextWorkItem.batchingID != workItem.batchingID)) {
                    // next item is not eligible to merge with this one
                    break;
                }

                BOOL fullyMerged = NO;
                workItem.batchingHandler(workItem.batchableData, nextWorkItem.batchableData, &fullyMerged);
                if (!fullyMerged) {
                    // We can't remove the next work item, so we can't merge anything else into this one.
                    break;
                }

                [self.items removeObjectAtIndex:1];
            }
        }

        [workItem callReadyHandlerWithContext:self.context];
    }
}

- (BOOL)isDuplicateForTypeID:(NSUInteger)opaqueDuplicateTypeID workItemData:(id)opaqueWorkItemData
{
    os_unfair_lock_lock(&_lock);
    // Start from the last item
    for (NSUInteger i = self.items.count; i > 0; i--) {
        MTRAsyncWorkItem * item = self.items[i - 1];
        BOOL isDuplicate = NO;
        BOOL stop = NO;
        if (item.supportsDuplicateCheck && (item.duplicateTypeID == opaqueDuplicateTypeID) && item.duplicateCheckHandler) {
            item.duplicateCheckHandler(opaqueWorkItemData, &isDuplicate, &stop);
            if (stop) {
                os_unfair_lock_unlock(&_lock);
                return isDuplicate;
            }
        }
    }
    os_unfair_lock_unlock(&_lock);
    return NO;
}
@end

@implementation MTRAsyncWorkItem

- (instancetype)initWithQueue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _queue = queue;
    }
    return self;
}

// assume lock is held
- (void)_invalidate
{
    // Make sure we don't leak via handlers that close over us, as ours must.
    // This is a bit odd, since these are supposed to be non-nullable
    // properties, but it's the best we can do given our API surface, unless we
    // assume that all consumers consistently use __weak refs to us inside their
    // handlers.
    //
    // Setting the attributes to nil will not compile; set the ivars directly.
    _readyHandler = nil;
    _cancelHandler = nil;
}

- (void)invalidate
{
    os_unfair_lock_lock(&_lock);
    [self _invalidate];
    os_unfair_lock_unlock(&_lock);
}

- (void)markedEnqueued
{
    os_unfair_lock_lock(&_lock);
    _enqueued = YES;
    os_unfair_lock_unlock(&_lock);
}

- (void)setReadyHandler:(MTRAsyncWorkReadyHandler)readyHandler
{
    os_unfair_lock_lock(&_lock);
    if (!_enqueued) {
        _readyHandler = readyHandler;
    }
    os_unfair_lock_unlock(&_lock);
}

- (void)setCancelHandler:(dispatch_block_t)cancelHandler
{
    os_unfair_lock_lock(&_lock);
    if (!_enqueued) {
        _cancelHandler = cancelHandler;
    }
    os_unfair_lock_unlock(&_lock);
}

- (void)endWork
{
    [self.workQueue endWork:self];
    [self invalidate];
}

- (void)retryWork
{
    [self.workQueue retryWork:self];
}

// Called by the work queue
- (void)callReadyHandlerWithContext:(id)context
{
    dispatch_async(self.queue, ^{
        os_unfair_lock_lock(&self->_lock);
        MTRAsyncWorkReadyHandler readyHandler = self->_readyHandler;
        NSUInteger retryCount = self->_retryCount;
        if (readyHandler) {
            self->_retryCount++;
        }
        os_unfair_lock_unlock(&self->_lock);

        if (readyHandler == nil) {
            // Nothing to do here.
            [self endWork];
        } else {
            readyHandler(context, retryCount);
        }
    });
}

// Called by the work queue
- (void)cancel
{
    os_unfair_lock_lock(&self->_lock);
    dispatch_block_t cancelHandler = self->_cancelHandler;
    [self _invalidate];
    os_unfair_lock_unlock(&self->_lock);

    if (cancelHandler) {
        dispatch_async(self.queue, ^{
            cancelHandler();
        });
    }
}

- (void)setBatchingID:(NSUInteger)opaqueBatchingID data:(id)opaqueBatchableData handler:(MTRAsyncWorkBatchingHandler)batchingHandler
{
    os_unfair_lock_lock(&self->_lock);
    _batchable = YES;
    _batchingID = opaqueBatchingID;
    _batchableData = opaqueBatchableData;
    _batchingHandler = batchingHandler;
    os_unfair_lock_unlock(&self->_lock);
}

- (void)setDuplicateTypeID:(NSUInteger)opaqueDuplicateTypeID handler:(MTRAsyncWorkDuplicateCheckHandler)duplicateCheckHandler
{
    _supportsDuplicateCheck = YES;
    _duplicateTypeID = opaqueDuplicateTypeID;
    _duplicateCheckHandler = duplicateCheckHandler;
}

@end
