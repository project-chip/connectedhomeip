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

#import <os/lock.h>

#import "MTRBaseDevice.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerDataStore.h"
#import "MTRDeviceControllerDelegate.h"
#import "MTRDeviceStorageBehaviorConfiguration.h"

#import <Matter/MTRP256KeypairBridge.h>

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#import <Matter/MTRDiagnosticLogsType.h>
#import <Matter/MTROTAProviderDelegate.h>

@class MTRDeviceControllerParameters;
@class MTRDeviceControllerFactory;
@class MTRDevice;
@class MTRAsyncWorkQueue;
@protocol MTRDevicePairingDelegate;
@protocol MTRDeviceControllerDelegate;
@class MTRDevice_Concrete;

namespace chip {
class FabricTable;

namespace Controller {
    class DeviceCommissioner;
}
} // namespace chip

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceController ()

@property (nonatomic, readonly) NSMapTable<NSNumber *, MTRDevice *> * nodeIDToDeviceMap;
@property (readonly, assign) os_unfair_lock_t deviceMapLock;

@property (readwrite, nonatomic) NSUUID * uniqueIdentifier;

// queue used to serialize all work performed by the MTRDeviceController
// (moved here so subclasses can initialize differently)
@property (readwrite, retain) dispatch_queue_t chipWorkQueue;

- (instancetype)initForSubclasses:(BOOL)startSuspended;

#pragma mark - MTRDeviceControllerFactory methods

/**
 * Will return chip::kUndefinedFabricIndex if we do not have a fabric index.
 */
@property (readonly) chip::FabricIndex fabricIndex;

/**
 * Will return the compressed fabric id of the fabric if the controller is
 * running, else nil.
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
 * Fabric ID tied to controller
 */
@property (nonatomic, retain, nullable) NSNumber * fabricID;

/**
 * Node ID tied to controller
 */
@property (nonatomic, retain, nullable) NSNumber * nodeID;

/**
 * Root Public Key tied to controller
 */
@property (nonatomic, retain, nullable) NSData * rootPublicKey;

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
/**
 * _setupDeviceForNodeID is a hook expected to be implemented by subclasses to
 * actually allocate a device object of the right type.
 */
- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData;
- (void)removeDevice:(MTRDevice *)device;

@end

/**
 * Shim to allow us to treat an MTRDevicePairingDelegate as an
 * MTRDeviceControllerDelegate.
 */
@interface MTRDevicePairingDelegateShim : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, readonly) id<MTRDevicePairingDelegate> delegate;
- (instancetype)initWithDelegate:(id<MTRDevicePairingDelegate>)delegate;
@end

static NSString * const kDeviceControllerErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kDeviceControllerErrorIPKInit = @"Init failure while initializing IPK";
static NSString * const kDeviceControllerErrorSigningKeypairInit = @"Init failure while creating signing keypair bridge";
static NSString * const kDeviceControllerErrorOperationalCredentialsInit = @"Init failure while creating operational credentials delegate";
static NSString * const kDeviceControllerErrorOperationalKeypairInit = @"Init failure while creating operational keypair bridge";
static NSString * const kDeviceControllerErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kDeviceControllerErrorPartialDacVerifierInit = @"Init failure while creating a partial DAC verifier";
static NSString * const kDeviceControllerErrorPairDevice = @"Failure while pairing the device";
static NSString * const kDeviceControllerErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kDeviceControllerErrorOpenPairingWindow = @"Open Pairing Window failed";
static NSString * const kDeviceControllerErrorNotRunning = @"Controller is not running. Call startup first.";
static NSString * const kDeviceControllerErrorSetupCodeGen = @"Generating Manual Pairing Code failed";
static NSString * const kDeviceControllerErrorGenerateNOC = @"Generating operational certificate failed";
static NSString * const kDeviceControllerErrorKeyAllocation = @"Generating new operational key failed";
static NSString * const kDeviceControllerErrorCSRValidation = @"Extracting public key from CSR failed";
static NSString * const kDeviceControllerErrorGetCommissionee = @"Failure obtaining device being commissioned";
static NSString * const kDeviceControllerErrorGetAttestationChallenge = @"Failure getting attestation challenge";
static NSString * const kDeviceControllerErrorSpake2pVerifierGenerationFailed = @"PASE verifier generation failed";
static NSString * const kDeviceControllerErrorSpake2pVerifierSerializationFailed = @"PASE verifier serialization failed";
static NSString * const kDeviceControllerErrorCDCertStoreInit = @"Init failure while initializing Certificate Declaration Signing Keys store";

NS_ASSUME_NONNULL_END
