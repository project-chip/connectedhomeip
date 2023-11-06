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

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRAsyncCallbackQueueWorkItem;

typedef void (^MTRAsyncCallbackReadyHandler)(id context, NSUInteger retryCount);

MTR_DEPRECATED("This class was not intended to be part of the public Matter API", ios(16.1, 17.2), macos(13.0, 14.2), watchos(9.1, 10.2), tvos(16.1, 17.2))
@interface MTRAsyncCallbackWorkQueue : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithContext:(id _Nullable)context queue:(dispatch_queue_t)queue;
- (void)invalidate;
- (void)enqueueWorkItem:(MTRAsyncCallbackQueueWorkItem *)item;
@end

MTR_DEPRECATED("This class was not intended to be part of the public Matter API", ios(16.1, 17.2), macos(13.0, 14.2), watchos(9.1, 10.2), tvos(16.1, 17.2))
@interface MTRAsyncCallbackQueueWorkItem : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithQueue:(dispatch_queue_t)queue;

@property (nonatomic, strong) MTRAsyncCallbackReadyHandler readyHandler;
@property (nonatomic, strong) dispatch_block_t cancelHandler;

- (void)endWork;
- (void)retryWork;
@end

NS_ASSUME_NONNULL_END
