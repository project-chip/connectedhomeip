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

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceControllerDataStore_Test.h"

NS_ASSUME_NONNULL_BEGIN

// Test-only members of MTRDeviceController.  MTRDeviceController.mm adopts the protocol in a class extension, so the compiler
// checks that each one is implemented with a matching signature.  Tests see it through the category below.
@protocol MTRDeviceControllerTestMethods <NSObject>
- (void)removeDevice:(MTRDevice *)device;
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;
@property (nonatomic, readonly, nullable) NSNumber * compressedFabricID;

#ifdef DEBUG
+ (void)forceLocalhostAdvertisingOnly;
- (NSUInteger)unitTestDelegateCount;
#endif
@end

// Test-only members that tests call through an MTRDeviceController, but that only MTRDeviceController_Concrete implements.
// MTRDeviceController_Concrete.mm adopts the protocol in a class extension, so the compiler checks them the same way.
@protocol MTRDeviceControllerConcreteTestMethods <NSObject>
- (void)syncRunOnWorkQueue:(void (^)(void))block error:(NSError * __autoreleasing *)error;
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDeviceController *> * concurrentSubscriptionPool;
// MTRDeviceController_Concrete.h declares this as an MTRDeviceControllerDataStore, which adopts the protocol.
@property (nonatomic, readonly, nullable) id<MTRDeviceControllerDataStoreAttributeStoreMethods> controllerDataStore;

#ifdef DEBUG
- (NSDictionary<NSNumber *, NSNumber *> *)unitTestGetDeviceAttributeCounts;
- (void)unitTestSetConnectivityMonitorWaitSeconds:(NSTimeInterval)seconds;
// Syncs onto the Matter queue: call off that queue, while the controller is running.
- (NSUInteger)unitTestSessionRequestsWaitingOnConnectivityMonitorCount;
#endif
@end

@interface MTRDeviceController (Test) <MTRDeviceControllerTestMethods, MTRDeviceControllerConcreteTestMethods>
// Implemented in the MatterPrivateForInternalDragonsDoNotFeed category in MTRDeviceController.mm, which is where
// MTRDeviceController_Internal.h declares it (and so where the compiler checks it); a protocol requirement cannot be
// satisfied from a category.
- (nullable MTRDevice *)_existingDeviceForNodeID:(NSNumber *)nodeID;
@end

NS_ASSUME_NONNULL_END
