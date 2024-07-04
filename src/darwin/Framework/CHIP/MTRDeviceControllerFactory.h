/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
 * An object that allows creating Matter controllers. There can be only one such
 * object in a given process.
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRCertificates.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRStorage;
@protocol MTRPersistentStorageDelegate;
@protocol MTROTAProviderDelegate;
@protocol MTRKeypair;

@class MTRDeviceController;
@class MTRDeviceControllerStartupParams;
@class MTRFabricInfo;

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDeviceControllerFactoryParams : NSObject

- (instancetype)init NS_UNAVAILABLE;

/*
 * Storage used to store persistent information for the fabrics the
 * controllers ends up interacting with.  This is only used if "initWithStorage"
 * is used to initialize the MTRDeviceControllerFactoryParams.  If "init" is
 * used, this property will contain a dummy storage that will not be used for
 * anything.
 */
@property (nonatomic, strong, readonly) id<MTRStorage> storage;

/*
 * OTA Provider delegate to be called when an OTA Requestor is requesting a software update.
 * Defaults to nil.
 *
 * Calls to this delegate can happen on an arbitrary thread, but will not happen
 * concurrently.
 */
@property (nonatomic, strong, nullable) id<MTROTAProviderDelegate> otaProviderDelegate;

/*
 * The Product Attestation Authority certificates that are trusted to sign
 * device attestation information (and in particular to sign Product Attestation
 * Intermediate certificates, which then sign Device Attestation Certificates).
 *
 * Defaults to nil.
 */
@property (nonatomic, copy, nullable) NSArray<MTRCertificateDERBytes> * productAttestationAuthorityCertificates;

/*
 * The Certification Declaration certificates whose public keys correspond to
 * private keys that are trusted to sign certification declarations.  Defaults
 * to nil.
 *
 * These certificates are used in addition to, not replacing, the default set of
 * well-known certification declaration signing keys.
 */
@property (nonatomic, copy, nullable) NSArray<MTRCertificateDERBytes> * certificationDeclarationCertificates;

/*
 * The network port to bind to.  If not specified, an ephemeral port will be
 * used.
 */
@property (nonatomic, copy, nullable) NSNumber * port;

/*
 * Whether to run a server capable of accepting incoming CASE
 * connections.  Defaults to NO.
 */
@property (nonatomic, assign) BOOL shouldStartServer;

/*
 * Initialize the device controller factory with storage.  In this mode, the
 * storage will be used to store various information needed by the Matter
 * framework.
 */
- (instancetype)initWithStorage:(id<MTRStorage>)storage;

@end

MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDeviceControllerFactory : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single MTRDeviceControllerFactory we support existing.  It starts off
 * in a "not started" state.
 */
+ (MTRDeviceControllerFactory *)sharedInstance;

/**
 * If true, the factory is in a state where it can create controllers:
 * startControllerFactory has been called, but stopControllerFactory has not been called
 * since then.
 */
@property (readonly, nonatomic, getter=isRunning) BOOL running;

/**
 * Returns the list of MTRFabricInfo representing the fabrics the
 * MTRDeviceControllerFactory knows about and the corresponding node identities
 * of the controller factory on those fabrics.  Returns nil if the factory is
 * not running or if there is an error reading fabric information.
 *
 * All entries in this list will have a non-nil rootCertificate.
 */
@property (readonly, nonatomic, nullable) NSArray<MTRFabricInfo *> * knownFabrics;

/**
 * Start the controller factory. Repeated calls to startControllerFactory
 * without calls to stopControllerFactory in between are NO-OPs. Use the
 * isRunning property to check whether the controller factory needs to be
 * started up.
 *
 * @param[in] startupParams data needed to start up the controller factory.
 *
 * @return Whether startup succeded.
 */
- (BOOL)startControllerFactory:(MTRDeviceControllerFactoryParams *)startupParams error:(NSError * __autoreleasing *)error;

/**
 * Stop the controller factory. This will shut down any outstanding
 * controllers as part of the factory stopping.
 *
 * Repeated calls to stopControllerFactory without calls to
 * startControllerFactory in between are NO-OPs.
 */
- (void)stopControllerFactory;

/**
 * Create a MTRDeviceController on an existing fabric.  Returns nil on failure.
 *
 * This method will fail if there is no such fabric or if there is
 * already a controller started for that fabric.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 *
 * This method can only be used if the factory was initialized with storage.
 * When using per-controller storage, use [MTRDeviceController initWithParameters:error:].
 */
- (MTRDeviceController * _Nullable)createControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams
                                                              error:(NSError * __autoreleasing *)error;

/**
 * Create a MTRDeviceController on a new fabric.  Returns nil on failure.
 *
 * This method will fail if the given fabric already exists.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 *
 * This method can only be used if the factory was initialized with storage.
 * When using per-controller storage, use [MTRDeviceController initWithParameters:error:].
 */
- (MTRDeviceController * _Nullable)createControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams
                                                         error:(NSError * __autoreleasing *)error;

/**
 * If possible, pre-warm the Matter stack for setting up a commissioning session.
 *
 * This may be called before -[MTRDeviceController setupCommissioningSessionWithPayload:]
 * if it is known that a commissioning attempt will soon take place, but the commissioning
 * payload is not known yet.
 *
 * The controller factory must be running for pre-warming to take place.  Pre-warming can take place
 * before any controllers are started.
 */
- (void)preWarmCommissioningSession MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

/**
 * Set the Message Reliability Protocol parameters for all controllers,
 * including already-running ones.  This allows control over retransmit delays
 * to account for high-latency networks.
 *
 * Since MRP parameters are communicated to peers during session setup, existing
 * sessions will not be affected when this function is called, but all sessions
 * established after the call will be.
 *
 * Setting all arguments to nil will reset to the MRP parameters to their
 * default values.
 *
 * Setting some arguments to non-nil will change just those values, keeping
 * current values for any arguments that are nil (not resetting them to
 * defaults).
 *
 * Non-nil arguments are specified as an integer number of milliseconds.
 *
 * @param idleRetransmitMs the minimal interval between retransmits for someone
 *                         sending messages to us, when they think we are
 *                         "idle" and might have our radio only turned on
 *                         intermittently.
 * @param activeRetransmitMs the minimal interval between retransmits for
 *                           someone sending messages to us, when they think we
 *                           are "active" and have the radio turned on
 *                           consistently.
 * @param activeThresholdMs the amount of time we will stay in "active" mode after
 *                          network activity.
 * @param additionalRetransmitDelayMs additional delay between retransmits for
 *                                    messages we send, on top of whatever delay
 *                                    the other side requests via its MRP
 *                                    parameters.
 */
MTR_EXTERN MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6)) void MTRSetMessageReliabilityParameters(NSNumber * _Nullable idleRetransmitMs,
    NSNumber * _Nullable activeRetransmitMs,
    NSNumber * _Nullable activeThresholdMs,
    NSNumber * _Nullable additionalRetransmitDelayMs);

MTR_DEPRECATED(
    "Please use MTRDeviceControllerFactoryParams", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRControllerFactoryParams : MTRDeviceControllerFactoryParams
@property (nonatomic, strong, readonly) id<MTRPersistentStorageDelegate> storageDelegate MTR_DEPRECATED(
    "Please use the storage property", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, assign) BOOL startServer MTR_DEPRECATED(
    "Please use shouldStartServer", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable) NSArray<NSData *> * paaCerts MTR_DEPRECATED(
    "Please use productAttestationAuthorityCertificates", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable) NSArray<NSData *> * cdCerts MTR_DEPRECATED(
    "Please use certificationDeclarationCertificates", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@end

MTR_DEPRECATED("Please use MTRDeviceControllerFactory", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRControllerFactory : NSObject
@property (readonly, nonatomic) BOOL isRunning;
+ (MTRControllerFactory *)sharedInstance;
- (BOOL)startup:(MTRControllerFactoryParams *)startupParams;
- (void)shutdown;
- (MTRDeviceController * _Nullable)startControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams;
- (MTRDeviceController * _Nullable)startControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
