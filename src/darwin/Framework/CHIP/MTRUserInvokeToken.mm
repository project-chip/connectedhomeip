/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import "MTRUnfairLock.h"
#import "MTRUserInvokeToken_Internal.h"

#import <os/lock.h>

@implementation MTRUserInvokeToken {
    os_unfair_lock _lock;
    BOOL _used;
    BOOL _cancelledBeforeUse;
    // Weak: the queue owns the item.  Once the item is gone (sent and finished, or cancelled), cancel returns NO.
    __weak MTRAsyncWorkItem * _workItem;
    __weak MTRAsyncWorkQueue * _queue;
}

- (instancetype)initWithTimeout:(NSNumber * _Nullable)timeoutMs
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _timeoutMs = [timeoutMs copy];
    }
    return self;
}

- (BOOL)attachToWorkItem:(MTRAsyncWorkItem *)workItem queue:(MTRAsyncWorkQueue *)queue
{
    std::lock_guard lock(_lock);
    if (_used || _cancelledBeforeUse) {
        return NO;
    }
    _used = YES;
    _workItem = workItem;
    _queue = queue;
    return YES;
}

- (BOOL)cancelledBeforeUse
{
    std::lock_guard lock(_lock);
    return _cancelledBeforeUse;
}

- (BOOL)cancel
{
    MTRAsyncWorkItem * workItem;
    MTRAsyncWorkQueue * queue;
    {
        std::lock_guard lock(_lock);
        if (!_used) {
            // Not tied to an invoke yet: the invoke it is passed to is never queued.
            BOOL wasCancelled = _cancelledBeforeUse;
            _cancelledBeforeUse = YES;
            return !wasCancelled;
        }
        workItem = _workItem;
        queue = _queue;
    }
    if (!workItem || !queue) {
        return NO;
    }
    // Only a command that has sent nothing is cancelled; one already sent completes, and its caller drops a late
    // result if it no longer wants it.
    return [queue cancelWorkItemIfUnsent:workItem];
}

@end
