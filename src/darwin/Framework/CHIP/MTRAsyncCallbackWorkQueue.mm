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

#import "MTRAsyncCallbackWorkQueue_Internal.h"
#import "MTRLogging.h"

#pragma mark - Class extensions

@interface MTRAsyncCallbackWorkQueue ()
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic, strong, readonly) id context;
@property (nonatomic, strong, readonly) dispatch_queue_t queue;
@property (nonatomic, strong, readonly) NSMutableArray<MTRAsyncCallbackQueueWorkItem *> * items;
@property (nonatomic, readwrite) NSUInteger runningWorkItemCount;

// For WorkItem's use only - the parameter is for sanity check
- (void)endWork:(MTRAsyncCallbackQueueWorkItem *)workItem;
- (void)retryWork:(MTRAsyncCallbackQueueWorkItem *)workItem;
@end

@interface MTRAsyncCallbackQueueWorkItem ()
@property (nonatomic, strong, readonly) dispatch_queue_t queue;
@property (nonatomic, readwrite) NSUInteger retryCount;
@property (nonatomic, strong) MTRAsyncCallbackWorkQueue * workQueue;
// Called by the queue
- (void)callReadyHandlerWithContext:(id)context;
- (void)cancel;
@end

#pragma mark - Class implementations

@implementation MTRAsyncCallbackWorkQueue
- (instancetype)initWithContext:(id)context queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _context = context;
        _queue = queue;
        _items = [NSMutableArray array];
    }
    return self;
}

- (void)enqueueWorkItem:(MTRAsyncCallbackQueueWorkItem *)item
{
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

    for (MTRAsyncCallbackQueueWorkItem * item in invalidateItems) {
        [item cancel];
    }
    [invalidateItems removeAllObjects];
}

- (void)endWork:(MTRAsyncCallbackQueueWorkItem *)workItem
{
    os_unfair_lock_lock(&_lock);
    // sanity check if running
    if (!self.runningWorkItemCount) {
        // something is wrong with state - nothing is currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("endWork: no work is running on work queue");
        return;
    }

    // sanity check the same work item is running
    // when "concurrency width" is implemented need to check first N items
    MTRAsyncCallbackQueueWorkItem * firstWorkItem = self.items.firstObject;
    if (firstWorkItem != workItem) {
        // something is wrong with this work item - should not be currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("endWork: work item is not first on work queue");
        return;
    }

    // since work is done, remove from queue and call ready on the next item
    [self.items removeObjectAtIndex:0];

    // when "concurrency width" is implemented this will be decremented instead
    self.runningWorkItemCount = 0;
    [self _callNextReadyWorkItem];
    os_unfair_lock_unlock(&_lock);
}

- (void)retryWork:(MTRAsyncCallbackQueueWorkItem *)workItem
{
    // reset BOOL and call again
    os_unfair_lock_lock(&_lock);
    // sanity check if running
    if (!self.runningWorkItemCount) {
        // something is wrong with state - nothing is currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("retryWork: no work is running on work queue");
        return;
    }

    // sanity check the same work item is running
    // when "concurrency width" is implemented need to check first N items
    MTRAsyncCallbackQueueWorkItem * firstWorkItem = self.items.firstObject;
    if (firstWorkItem != workItem) {
        // something is wrong with this work item - should not be currently running
        os_unfair_lock_unlock(&_lock);
        MTR_LOG_ERROR("retryWork: work item is not first on work queue");
        return;
    }

    // when "concurrency width" is implemented this will be decremented instead
    self.runningWorkItemCount = 0;
    [self _callNextReadyWorkItem];
    os_unfair_lock_unlock(&_lock);
}

// assume lock is held while calling this
- (void)_callNextReadyWorkItem
{
    // when "concurrency width" is implemented this will be checked against the width
    if (self.runningWorkItemCount) {
        // can't run next work item until the current one is done
        return;
    }

    // when "concurrency width" is implemented this will be incremented instead
    self.runningWorkItemCount = 1;

    MTRAsyncCallbackQueueWorkItem * workItem = self.items.firstObject;
    [workItem callReadyHandlerWithContext:self.context];
}
@end

@implementation MTRAsyncCallbackQueueWorkItem

- (instancetype)initWithQueue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _queue = queue;
    }
    return self;
}

// Called by Cluster object's after async work is done
- (void)endWork
{
    [self.workQueue endWork:self];
}

// Called by Cluster object's after async work is done
- (void)retryWork
{
    [self.workQueue retryWork:self];
}

// Called by the work queue
- (void)callReadyHandlerWithContext:(id)context
{
    dispatch_async(self.queue, ^{
        self.readyHandler(context, self.retryCount);
        self.retryCount++;
    });
}

// Called by the work queue
- (void)cancel
{
    dispatch_async(self.queue, ^{
        self.cancelHandler();
    });
}
@end
