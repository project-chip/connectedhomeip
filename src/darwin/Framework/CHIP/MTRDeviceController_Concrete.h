/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "MTRDeviceControllerStartupParams_Internal.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceController_Concrete : MTRDeviceController

/**
 * Init a newly created controller.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (nullable instancetype)initWithFactory:(MTRDeviceControllerFactory *)factory
                                   queue:(dispatch_queue_t)queue
                         storageDelegate:(id<MTRDeviceControllerStorageDelegate> _Nullable)storageDelegate
                    storageDelegateQueue:(dispatch_queue_t _Nullable)storageDelegateQueue
                     otaProviderDelegate:(id<MTROTAProviderDelegate> _Nullable)otaProviderDelegate
                otaProviderDelegateQueue:(dispatch_queue_t _Nullable)otaProviderDelegateQueue
                        uniqueIdentifier:(NSUUID *)uniqueIdentifier
          concurrentSubscriptionPoolSize:(NSUInteger)concurrentSubscriptionPoolSize
            storageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration
                          startSuspended:(BOOL)startSuspended;

/**
 * Start a new controller.  Returns whether startup succeeded.  If this fails,
 * it guarantees that it has called controllerShuttingDown on the
 * MTRDeviceControllerFactory.
 *
 * The return value will always match [controller isRunning] for this
 * controller.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (BOOL)startup:(MTRDeviceControllerStartupParamsInternal *)startupParams;

/**
 * Takes an assertion to keep the controller running. If `-[MTRDeviceController shutdown]` is called while an assertion
 * is held, the shutdown will be honored only after all assertions are released. Invoking this method multiple times increases
 * the number of assertions and needs to be matched with equal amount of '-[MTRDeviceController removeRunAssertion]` to release
 * the assertion.
 */
- (void)addRunAssertion;

/**
 * Removes an assertion to allow the controller to shutdown once all assertions have been released.
 * Invoking this method once all assertions have been released in a noop.
 */
- (void)removeRunAssertion;

@end

NS_ASSUME_NONNULL_END
