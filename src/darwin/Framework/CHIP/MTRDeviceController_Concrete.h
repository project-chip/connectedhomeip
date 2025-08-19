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

#import <Matter/MTRAccessGrant.h>
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerFactory.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#import <Matter/MTRDeviceStorageBehaviorConfiguration.h>
#import <Matter/MTROTAProviderDelegate.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRDeviceConnectionBridge.h"
#import "MTRDeviceControllerDataStore.h"
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
 * Ensure we have a CASE session to the given node ID and then call the provided
 * connection callback.  This may be called on any queue (including the Matter
 * event queue) and on success will always call the provided connection callback
 * on the Matter queue, asynchronously.  Consumers must be prepared to run on
 * the Matter queue (an in particular must not use any APIs that will try to do
 * sync dispatch to the Matter queue).
 *
 * If the controller is not running when this function is called, it will
 * synchronously invoke the completion with an error, on whatever queue
 * getSessionForNode was called on.
 *
 * If the controller is not running when the async dispatch on the Matter queue
 * happens, the completion will be invoked with an error on the Matter queue.
 */
- (void)getSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion;

/**
 * Since getSessionForNode now enqueues by the subscription pool for Thread
 * devices, MTRDevice_Concrete needs a direct non-queued access because it already
 * makes use of the subscription pool.
 */
- (void)directlyGetSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion;

/**
 * Get a session for the commissionee device with the given device id.  This may
 * be called on any queue (including the Matter event queue) and on success will
 * always call the provided connection callback on the Matter queue,
 * asynchronously.  Consumers must be prepared to run on the Matter queue (an in
 * particular must not use any APIs that will try to do sync dispatch to the
 * Matter queue).
 *
 * If the controller is not running when this function is called, it will
 * synchronously invoke the completion with an error, on whatever queue
 * getSessionForCommissioneeDevice was called on.
 *
 * If the controller is not running when the async dispatch on the Matter queue
 * happens, the completion will be invoked with an error on the Matter queue.
 */
- (void)getSessionForCommissioneeDevice:(chip::NodeId)deviceID completion:(MTRInternalDeviceConnectionCallback)completion;

/**
 * Notify the controller that a new operational instance with the given node id
 * and a compressed fabric id that matches this controller has been observed.
 */
- (void)operationalInstanceAdded:(NSNumber *)nodeID;

/**
 * Get the access grants that apply for the given cluster path.
 */
- (NSArray<MTRAccessGrant *> *)accessGrantsForClusterPath:(MTRClusterPath *)clusterPath;

/**
 * Get the privilege level needed to read the given attribute.  There's no
 * endpoint provided because the expectation is that this information is the
 * same for all cluster instances.
 *
 * Returns nil if we have no such attribute defined on any endpoint, otherwise
 * one of MTRAccessControlEntry* constants wrapped in NSNumber.
 *
 * Only called on the Matter queue.
 */
- (nullable NSNumber *)neededReadPrivilegeForClusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID;

/**
 * Try to asynchronously dispatch the given block on the Matter queue.  If the
 * controller is not running either at call time or when the block would be
 * about to run, the provided error handler will be called with an error.  Note
 * that this means the error handler might be called on an arbitrary queue, and
 * might be called before this function returns or after it returns.
 *
 * The DeviceCommissioner pointer passed to the callback should only be used
 * synchronously during the callback invocation.
 *
 * If the error handler is nil, failure to run the block will be silent.
 */
- (void)asyncGetCommissionerOnMatterQueue:(void (^)(chip::Controller::DeviceCommissioner *))block
                             errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;

/**
 * Returns the transport used by the current session with the given device,
 * or `MTRTransportTypeUndefined` if no session is currently active.
 */
- (MTRTransportType)sessionTransportTypeForDevice:(MTRBaseDevice *)device;

/**
 * Invalidate the CASE session for the given node ID.  This is a temporary thing
 * just to support MTRBaseDevice's invalidateCASESession.  Must not be called on
 * the Matter event queue.
 */
- (void)invalidateCASESessionForNode:(NSNumber *)nodeID;

/**
 * Download log of the desired type from the device.
 */
- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

/**
 * Returns YES if the MTRDevice corrresponding to the given node ID is known to be a thread device, NO otherwise.
 */
- (BOOL)definitelyUsesThreadForDevice:(chip::NodeId)nodeID;

/**
 * Will return chip::kUndefinedFabricIndex if we do not have a fabric index.
 */
@property (readonly) chip::FabricIndex fabricIndex;

/**
 * A queue with a fixed width that allows a number of MTRDevice objects to perform
 * subscription at the same time.
 */
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDeviceController *> * concurrentSubscriptionPool;

/**
 * The per-controller data store this controller was initialized with, if any.
 */
@property (nonatomic, readonly, nullable) MTRDeviceControllerDataStore * controllerDataStore;

@end

NS_ASSUME_NONNULL_END
