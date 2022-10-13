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

/**
 * An object that allows creating Matter controllers. There can be only one such
 * object in a given process.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRPersistentStorageDelegate;
@protocol MTROTAProviderDelegate;
@protocol MTRKeypair;

@class MTRDeviceController;
@class MTRDeviceControllerStartupParams;

@interface MTRControllerFactoryParams : NSObject
/*
 * Storage delegate must be provided for correct functioning of Matter
 * controllers.  It is used to store persistent information for the fabrics the
 * controllers ends up interacting with.
 */
@property (nonatomic, strong, readonly) id<MTRPersistentStorageDelegate> storageDelegate;

/*
 * OTA Provider delegate to be called when an OTA Requestor is requesting a software update.
 * Defaults to nil.
 */
@property (nonatomic, strong, nullable) id<MTROTAProviderDelegate> otaProviderDelegate;

/*
 * The Product Attestation Authority certificates that are trusted to sign
 * device attestation information.  Defaults to nil.
 *
 */
@property (nonatomic, copy, nullable) NSArray<NSData *> * paaCerts;
/*
 * The Certificate Declaration certificates that are trusted to sign
 * device attestation information.  Defaults to nil.
 *
 */
@property (nonatomic, copy, nullable) NSArray<NSData *> * cdCerts;
/*
 * The network port to bind to.  If not specified, an ephemeral port will be
 * used.
 */
@property (nonatomic, copy, nullable) NSNumber * port;
/*
 * Whether to run a server capable of accepting incoming CASE
 * connections.  Defaults to NO.
 */
@property (nonatomic, assign) BOOL startServer;

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithStorage:(id<MTRPersistentStorageDelegate>)storageDelegate;
@end

@interface MTRControllerFactory : NSObject

@property (readonly, nonatomic) BOOL isRunning;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single MTRControllerFactory we support existing.  It starts off
 * in a "not started" state.
 */
+ (instancetype)sharedInstance;

/**
 * Start the controller factory. Repeated calls to startup without calls to
 * shutdown in between are NO-OPs. Use the isRunning property to check whether
 * the controller factory needs to be started up.
 *
 * @param[in] startupParams data needed to start up the controller factory.
 *
 * @return Whether startup succeded.
 */
- (BOOL)startup:(MTRControllerFactoryParams *)startupParams;

/**
 * Shut down the controller factory. This will shut down any outstanding
 * controllers as part of the factory shutdown.
 *
 * Repeated calls to shutdown without calls to startup in between are
 * NO-OPs.
 */
- (void)shutdown;

/**
 * Create a MTRDeviceController on an existing fabric.  Returns nil on failure.
 *
 * This method will fail if there is no such fabric or if there is
 * already a controller started for that fabric.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 */
- (MTRDeviceController * _Nullable)startControllerOnExistingFabric:(MTRDeviceControllerStartupParams *)startupParams;

/**
 * Create a MTRDeviceController on a new fabric.  Returns nil on failure.
 *
 * This method will fail if the given fabric already exists.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 */
- (MTRDeviceController * _Nullable)startControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams;

@end

NS_ASSUME_NONNULL_END
