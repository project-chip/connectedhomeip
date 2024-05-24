/**
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

/**
 * Parts of MTRDeviceController that are not part of the framework API.  Mostly
 * for use from MTRDeviceControllerFactory.
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRAccessGrant.h>
#import <Matter/MTRBaseDevice.h> // for MTRClusterPath

#import "MTRDeviceConnectionBridge.h" // For MTRInternalDeviceConnectionCallback
#import "MTRDeviceController.h"

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#import "MTRBaseDevice.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerDataStore.h"
#import "MTRDeviceStorageBehaviorConfiguration.h"

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerStartupParams.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#import <Matter/MTRDiagnosticLogsType.h>
#import <Matter/MTROTAProviderDelegate.h>

@class MTRDeviceControllerStartupParamsInternal;
@class MTRDeviceControllerFactory;
@class MTRDevice;
@class MTRAsyncWorkQueue;

namespace chip {
class FabricTable;

namespace Controller {
    class DeviceCommissioner;
}
} // namespace chip

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceController ()

#pragma mark - MTRDeviceControllerFactory methods

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
 * Will return chip::kUndefinedFabricIndex if we do not have a fabric index.
 */
@property (readonly) chip::FabricIndex fabricIndex;

/**
 * Will return the compressed fabric id of the fabric if the controller is
 * running, else nil.
 *
 * This property MUST be gotten from the Matter work queue.
 */
@property (nonatomic, readonly, nullable) NSNumber * compressedFabricID;

/**
 * The per-controller data store this controller was initialized with, if any.
 */
@property (nonatomic, readonly, nullable) MTRDeviceControllerDataStore * controllerDataStore;

/**
 * OTA delegate and its queue, if this controller supports OTA.  Either both
 * will be non-nil or both will be nil.
 */
@property (nonatomic, readonly, nullable) id<MTROTAProviderDelegate> otaProviderDelegate;
@property (nonatomic, readonly, nullable) dispatch_queue_t otaProviderDelegateQueue;

/**
 * A queue with a fixed width that allows a number of MTRDevice objects to perform
 * subscription at the same time.
 */
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDeviceController *> * concurrentSubscriptionPool;

/**
 * Init a newly created controller.
 *
 * Only MTRDeviceControllerFactory should be calling this.
 */
- (instancetype)initWithFactory:(MTRDeviceControllerFactory *)factory
                             queue:(dispatch_queue_t)queue
                   storageDelegate:(id<MTRDeviceControllerStorageDelegate> _Nullable)storageDelegate
              storageDelegateQueue:(dispatch_queue_t _Nullable)storageDelegateQueue
               otaProviderDelegate:(id<MTROTAProviderDelegate> _Nullable)otaProviderDelegate
          otaProviderDelegateQueue:(dispatch_queue_t _Nullable)otaProviderDelegateQueue
                  uniqueIdentifier:(NSUUID *)uniqueIdentifier
    concurrentSubscriptionPoolSize:(NSUInteger)concurrentSubscriptionPoolSize
      storageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;

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
 * Returns the transport used by the current session with the given device,
 * or `MTRTransportTypeUndefined` if no session is currently active.
 */
- (MTRTransportType)sessionTransportTypeForDevice:(MTRBaseDevice *)device;

/**
 * Invalidate the CASE session for the given node ID.  This is a temporary thing
 * just to support MTRBaseDevice's invalidateCASESession.  Must not be called on
 * the Matter event queue.
 */
- (void)invalidateCASESessionForNode:(chip::NodeId)nodeID;

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
 * Try to asynchronously dispatch the given block on the Matter queue.  If the
 * controller is not running either at call time or when the block would be
 * about to run, the provided error handler will be called with an error.  Note
 * that this means the error handler might be called on an arbitrary queue, and
 * might be called before this function returns or after it returns.
 *
 * If the error handler is nil, failure to run the block will be silent.
 */
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;

/**
 * Get an MTRBaseDevice for the given node id.  This exists to allow subclasses
 * of MTRDeviceController (e.g. MTRDeviceControllerOverXPC) to override what
 * sort of MTRBaseDevice they return.
 */
- (MTRBaseDevice *)baseDeviceForNodeID:(NSNumber *)nodeID;

/**
 * Notify the controller that a new operational instance with the given node id
 * and a compressed fabric id that matches this controller has been observed.
 */
- (void)operationalInstanceAdded:(chip::NodeId)nodeID;

/**
 * Download log of the desired type from the device.
 */
- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

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

#pragma mark - Device-specific data and SDK access
// DeviceController will act as a central repository for this opaque dictionary that MTRDevice manages
- (MTRDevice *)deviceForNodeID:(NSNumber *)nodeID;
- (void)removeDevice:(MTRDevice *)device;

- (NSNumber * _Nullable)syncGetCompressedFabricID;

@end

NS_ASSUME_NONNULL_END
