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

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerFactory.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#import <Matter/MTRDeviceStorageBehaviorConfiguration.h>
#import <Matter/MTROTAProviderDelegate.h>

#import "MTRDeviceConnectionBridge.h"
#import "MTRDeviceControllerStartupParams_Internal.h"

#include <credentials/FabricTable.h>
#include <lib/core/DataModelTypes.h>

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
 * Shut down the underlying C++ controller.  Must be called on the Matter work
 * queue or after the Matter work queue has been shut down.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (void)shutDownCppController;

/**
 * Notification that the MTRDeviceControllerFactory has finished shutting down
 * this controller and will not be touching it anymore.  This is guaranteed to
 * be called after initWithFactory succeeds.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (void)deinitFromFactory;

/**
 * Check whether this controller is running on the given fabric, as represented
 * by the provided FabricTable and fabric index.  The provided fabric table may
 * not be the same as the fabric table this controller is using. This method
 * MUST be called from the Matter work queue.
 *
 * Might return failure, in which case we don't know whether it's running on the
 * given fabric.  Otherwise it will set *isRunning to the right boolean value.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (CHIP_ERROR)isRunningOnFabric:(chip::FabricTable *)fabricTable
                    fabricIndex:(chip::FabricIndex)fabricIndex
                      isRunning:(BOOL *)isRunning;

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

/**
 * This method returns TRUE if this controller matches the fabric reference and node ID as listed in the parameters.
 */
- (BOOL)matchesPendingShutdownControllerWithOperationalCertificate:(nullable MTRCertificateDERBytes)operationalCertificate andRootCertificate:(nullable MTRCertificateDERBytes)rootCertificate;

/**
 * Clear any pending shutdown request.
 */
- (void)clearPendingShutdown;

/**
 * Since getSessionForNode now enqueues by the subscription pool for Thread
 * devices, MTRDevice_Concrete needs a direct non-queued access because it already
 * makes use of the subscription pool.
 */
- (void)directlyGetSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion;

/**
 * Notify the controller that a new operational instance with the given node id
 * and a compressed fabric id that matches this controller has been observed.
 */
- (void)operationalInstanceAdded:(NSNumber *)nodeID;

@end

NS_ASSUME_NONNULL_END
