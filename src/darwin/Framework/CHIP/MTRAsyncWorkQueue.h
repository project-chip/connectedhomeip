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

/// A unit of work that can be run on a `MTRAsyncWorkQueue`.
///
/// A work item can be configured with a number of hander blocks called by the
/// async work queue in various situations. Generally work items will have at
/// least a `readyHandler` (though it is technically optional).
///
/// This class is not thread-safe, and once a work item has be submitted to the
/// queue via `enqueueWorkItem` ownership of the work item passes to the queue.
/// No further modifications may be made to it after that point.
///
/// @see -[MTRAsyncWorkQueue enqueueWorkItem:]
MTR_TESTABLE
@interface MTRAsyncWorkItem<__contravariant ContextType> : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/// Creates a work item that will run on the specified dispatch queue.
- (instancetype)initWithQueue:(dispatch_queue_t)queue;

/// Called by the work queue to start this work item
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

/// Called by the work queue to cancel the work item. The work item may or may
/// not have been started already.
@property (nonatomic, strong, nullable) void (^cancelHandler)(void);

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

/// Enqueues the specified work item, making it eligible for execution.
///
/// Once a work item is enqueued, ownership of it passes to the queue and
/// no further modifications may be made to it. Work item objects cannot be
/// re-used.
- (void)enqueueWorkItem:(MTRAsyncWorkItem<ContextType> *)item;

/// Cancels and removes all work items.
- (void)invalidate;
@end

NS_ASSUME_NONNULL_END
