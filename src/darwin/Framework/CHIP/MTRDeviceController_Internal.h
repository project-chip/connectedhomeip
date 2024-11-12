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
#import <Matter/MTRBaseDevice.h> // for MTRClusterPath

#import "MTRDeviceConnectionBridge.h" // For MTRInternalDeviceConnectionCallback
#import "MTRDeviceController.h"

#import <os/lock.h>

#import "MTRBaseDevice.h"
#import "MTRDeviceClusterData.h"
#import "MTRDeviceController.h"
#import "MTRDeviceControllerDelegate.h"
#import "MTRDeviceStorageBehaviorConfiguration.h"

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#import <Matter/MTROTAProviderDelegate.h>

@class MTRDeviceControllerParameters;
@class MTRDeviceControllerFactory;
@class MTRDevice;
@protocol MTRDevicePairingDelegate;
@protocol MTRDeviceControllerDelegate;
@class MTRDevice_Concrete;

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceController ()

@property (nonatomic, readonly) NSMapTable<NSNumber *, MTRDevice *> * nodeIDToDeviceMap;
@property (readonly, assign) os_unfair_lock_t deviceMapLock;

@property (readwrite, nonatomic) NSUUID * uniqueIdentifier;
// (moved here so subclasses can initialize differently)

- (instancetype)initForSubclasses:(BOOL)startSuspended;

#pragma mark - MTRDeviceControllerFactory methods

/**
 * Will return the compressed fabric id of the fabric if the controller is
 * running, else nil.
 */
@property (nonatomic, readonly, nullable) NSNumber * compressedFabricID;

/**
 * OTA delegate and its queue, if this controller supports OTA.  Either both
 * will be non-nil or both will be nil.
 */
@property (nonatomic, readonly, nullable) id<MTROTAProviderDelegate> otaProviderDelegate;
@property (nonatomic, readonly, nullable) dispatch_queue_t otaProviderDelegateQueue;

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

#pragma mark - Device-specific data and SDK access
// DeviceController will act as a central repository for this opaque dictionary that MTRDevice manages
- (MTRDevice *)deviceForNodeID:(NSNumber *)nodeID;
// _deviceForNodeID:createIfNeeded: can only return nil if NO is passed for createIfNeeded.
- (MTRDevice * _Nullable)_deviceForNodeID:(NSNumber *)nodeID createIfNeeded:(BOOL)createIfNeeded;
/**
 * _setupDeviceForNodeID is a hook expected to be implemented by subclasses to
 * actually allocate a device object of the right type.
 */
- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(nullable NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData;
- (void)removeDevice:(MTRDevice *)device;

/**
 * Called by MTRDevice object when their dealloc is called, so the controller can notify interested delegate that active devices have changed
 */
- (void)deviceDeallocated;

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
static NSString * const kDeviceControllerErrorCDCertStoreInit = @"Init failure while initializing Certificate Declaration Signing Keys store";

NS_ASSUME_NONNULL_END
