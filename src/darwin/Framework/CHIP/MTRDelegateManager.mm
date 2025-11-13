/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#import "MTRDelegateManager.h"
#import "MTRLogging_Internal.h"

@interface MTRDelegateInfo ()

#ifdef DEBUG
- (BOOL)callDelegateSynchronouslyWithBlock:(void (^)(id delegate))block;
#endif // DEBUG

@end

@implementation MTRDelegateInfo
- (instancetype)initWithDelegate:(id)delegate queue:(dispatch_queue_t)queue
{
    if (!(self = [super init])) {
        return nil;
    }

    _delegate = delegate;
    _queue = queue;
    _delegatePointerValue = (__bridge void *) delegate;

    return self;
}

- (BOOL)callDelegateWithBlock:(void (^)(id))block
{
    id strongDelegate = _delegate;
    if (strongDelegate == nil) {
        return NO;
    }
    dispatch_async(_queue, ^{
        block(strongDelegate);
    });

    return YES;
}

#ifdef DEBUG
- (BOOL)callDelegateSynchronouslyWithBlock:(void (^)(id delegate))block
{
    id strongDelegate = _delegate;
    if (strongDelegate == nil) {
        return NO;
    }

    block(strongDelegate);

    return YES;
}
#endif // DEBUG
@end

@interface MTRDelegateManager () {
    NSMutableSet<MTRDelegateInfo *> * _delegates;
    __weak id _owner;
}
@end

@implementation MTRDelegateManager

- (instancetype)initWithOwner:(id)owner
{
    if (!(self = [super init])) {
        return nil;
    }

    _delegates = [NSMutableSet set];
    _owner = owner;
    return self;
}

- (void)addDelegateInfo:(MTRDelegateInfo *)delegateInfo
{
    id strongOwner = _owner;

    // Replace delegate info with the same delegate object, and opportunistically remove defunct delegate references
    NSMutableSet<MTRDelegateInfo *> * delegatesToRemove = [NSMutableSet set];
    for (MTRDelegateInfo * existingInfo in _delegates) {
        id strongDelegate = existingInfo.delegate;
        if (!strongDelegate) {
            [delegatesToRemove addObject:existingInfo];
            MTR_LOG("%@ removing delegate info for nil delegate %p", strongOwner, existingInfo.delegatePointerValue);
        } else if (strongDelegate == delegateInfo.delegate) {
            [delegatesToRemove addObject:existingInfo];
            MTR_LOG("%@ replacing delegate info for %p", strongOwner, delegateInfo.delegate);
        }
    }
    if (delegatesToRemove.count) {
        NSUInteger oldDelegatesCount = _delegates.count;
        [_delegates minusSet:delegatesToRemove];
        MTR_LOG("%@ addDelegate: removed %lu", strongOwner, static_cast<unsigned long>(oldDelegatesCount - _delegates.count));
    }

    [_delegates addObject:delegateInfo];
    MTR_LOG("%@ added delegate %p total %lu", strongOwner, delegateInfo.delegatePointerValue, static_cast<unsigned long>(_delegates.count));
}

- (void)removeDelegate:(id)delegate
{
    id strongOwner = _owner;

    MTR_LOG("%@ removeDelegate %p", strongOwner, delegate);

    // Note: _iterateDelegatesWithBlock already removes delegate infos for dead
    // delegates.
    __block MTRDelegateInfo * delegateInfoToRemove = nil;
    [self iterateDelegatesWithBlock:^(MTRDelegateInfo * delegateInfo) {
        if (delegateInfo.delegate == delegate) {
            delegateInfoToRemove = delegateInfo;
        }
    }];

    if (delegateInfoToRemove) {
        [_delegates removeObject:delegateInfoToRemove];
        MTR_LOG("%@ removed %p remaining %lu", strongOwner, delegate, static_cast<unsigned long>(_delegates.count));
    } else {
        MTR_LOG("%@ delegate %p not found in %lu", strongOwner, delegate, static_cast<unsigned long>(_delegates.count));
    }
}

- (void)removeAllDelegates
{
    [_delegates removeAllObjects];
}

// The compiler complains that that the type-erased version of this method has
// conflicting parameter types with the header-declared version.  Which sort of
// makes sense: the headers says you can only pass in
// MTRDelegateInfo<DelegateType> but the implementation says you can pass in any
// MTRDelegateInfo (and the same warning happens if this implementation is
// changed to allow passing any id).
//
// In practice, we only have delegate infos of the right type, so this is not a
// problem.  Just suppress the relevant warning.
//
// Why does the compiler not complain about callDelegateWithBlock?  Looks like
// because of the type bound on DelegateInfoType.  Removing that makes this
// problem go away.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmismatched-parameter-types"
- (NSUInteger)iterateDelegatesWithBlock:(void(NS_NOESCAPE ^ _Nullable)(id delegateInfo))block
#pragma clang diagnostic pop
{
    id strongOwner = _owner;

    if (!_delegates.count) {
        MTR_LOG_DEBUG("%@ no delegates to iterate", strongOwner);
        return 0;
    }

    // Opportunistically remove defunct delegate references on every iteration
    NSMutableSet<MTRDelegateInfo *> * delegatesToRemove = nil;
    for (MTRDelegateInfo * delegateInfo in _delegates) {
        id strongDelegate = delegateInfo.delegate;
        if (strongDelegate) {
            if (block) {
                @autoreleasepool {
                    block(delegateInfo);
                }
            }
            (void) strongDelegate; // ensure it stays alive
        } else {
            if (!delegatesToRemove) {
                delegatesToRemove = [NSMutableSet set];
            }
            [delegatesToRemove addObject:delegateInfo];
        }
    }

    if (delegatesToRemove.count) {
        [_delegates minusSet:delegatesToRemove];
        MTR_LOG("%@ iterateDelegatesWithBlock: removed %lu remaining %lu", strongOwner, static_cast<unsigned long>(delegatesToRemove.count), static_cast<unsigned long>(_delegates.count));
    }

    return _delegates.count;
}

- (BOOL)callDelegatesWithBlock:(void (^_Nullable)(id delegate))block
{
    return [self _callDelegatesWithBlock:block logString:nullptr];
}

- (BOOL)callDelegatesWithBlock:(void (^_Nullable)(id delegate))block logString:(const char *)logString
{
    return [self _callDelegatesWithBlock:block logString:logString];
}

- (BOOL)_callDelegatesWithBlock:(void (^_Nullable)(id delegate))block logString:(const char * _Nullable)logString
{
    id strongOwner = _owner;

    __block NSUInteger delegatesCalled = 0;
    [self iterateDelegatesWithBlock:^(MTRDelegateInfo * delegateInfo) {
        if ([delegateInfo callDelegateWithBlock:block]) {
            ++delegatesCalled;
        }
    }];

    if (logString != nullptr) {
        MTR_LOG("%@ %lu delegates called for %s", strongOwner, static_cast<unsigned long>(delegatesCalled), logString);
    }

    return delegatesCalled > 0;
}

#ifdef DEBUG

- (NSUInteger)unitTestNonnullDelegateCount
{
    NSUInteger nonnullDelegateCount = 0;
    for (MTRDelegateInfo * delegateInfo in _delegates) {
        if (delegateInfo.delegate) {
            nonnullDelegateCount++;
        }
    }

    return nonnullDelegateCount;
}

- (void)callFirstDelegateSynchronouslyWithBlock:(void(NS_NOESCAPE ^)(id delegate))block
{
    // It's a bit weird to implement a "call first" API on top of NSMutableSet,
    // but that's what MTRDevice used to do, and in any case in the cases when
    // this is used there's only one delegate around.
    id strongOwner = _owner;

    for (MTRDelegateInfo * delegateInfo in _delegates) {
        if ([delegateInfo callDelegateSynchronouslyWithBlock:block]) {
            MTR_LOG("%@ callFirstDelegateSynchronouslyWithBlock: successfully called %@", strongOwner, delegateInfo);
            return;
        }
    }
}

- (id _Nullable)firstDelegate
{
    for (MTRDelegateInfo * delegateInfo in _delegates) {
        id strongDelegate = delegateInfo.delegate;
        if (strongDelegate != nil) {
            return strongDelegate;
        }
    }

    return nil;
}

#endif // DEBUG

@end
