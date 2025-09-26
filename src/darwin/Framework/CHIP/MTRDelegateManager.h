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

#import "Foundation/Foundation.h"

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * MTRDelegateInfo can be subclassed to store additional information, in
 * addition to the delegate and queue.
 */
MTR_TESTABLE
@interface MTRDelegateInfo<__covariant DelegateType> : NSObject
- (instancetype)initWithDelegate:(DelegateType)delegate queue:(dispatch_queue_t)queue;

@property (nonatomic, weak, readonly) DelegateType delegate;
@property (nonatomic, readonly) dispatch_queue_t queue;

// Pointer value for logging purpose only
@property (readonly) void * delegatePointerValue;

// Queues an async call to the provided block on the delegate's queue if the
// delegate still exists.  Returns whether the delegate exists.
- (BOOL)callDelegateWithBlock:(void (^)(id delegate))block;

@end

/**
 * A delegate manager that handles automatic removal of delegates that have gone
 * away and provides helpers for working with delegates.  This interface does
 * not do any synchronization; access to an MTRDelegateManager should be
 * synchronized by its consumer as needed.
 *
 * It sure would be nice if the type bound on our second type parameter could be
 * MTRDelegateInfo<DelegateType>, but that does not seem to be supported.
 */
MTR_TESTABLE
@interface MTRDelegateManager<DelegateType, DelegateInfoType : MTRDelegateInfo *> : NSObject

// Owner is used for logging only, and held weakly.
- (instancetype)initWithOwner:(id)owner;

- (void)addDelegateInfo:(DelegateInfoType)delegateInfo;
- (void)removeDelegate:(DelegateType)delegate;
- (void)removeAllDelegates;

// Iterates the delegates, and removes delegate info if the delegate object has dealloc'ed.
// Returns the number of delegates remaining at the end of the iteration.
//
// The passed-in block must not attempt to add or remove delegates during the
// iteration.  The block is called synchronously during the iteration.
- (NSUInteger)iterateDelegatesWithBlock:(void(NS_NOESCAPE ^ _Nullable)(DelegateInfoType delegateInfo))block;

// Calls the delegates, asynchronously, with the provided block, on the
// appropriate queue for each delegate.  Returns whether any asynchronous
// delegate calls were queued up.
- (BOOL)callDelegatesWithBlock:(void (^_Nullable)(DelegateType delegate))block;
- (BOOL)callDelegatesWithBlock:(void (^_Nullable)(DelegateType delegate))block logString:(const char *)logString;

#ifdef DEBUG

// Returns number of still-alive delegates, without removing entries.
- (NSUInteger)unitTestNonnullDelegateCount;

// Calls the first delegate synchronously with the provided block.
// Only used for unit test purposes - normal delegate should not expect or handle being called back synchronously
- (void)callFirstDelegateSynchronouslyWithBlock:(void(NS_NOESCAPE ^)(DelegateType delegate))block;

// Returns the first non-nil delegate, if any.
- (DelegateType _Nullable)firstDelegate;

#endif // DEBUG
@end

NS_ASSUME_NONNULL_END
