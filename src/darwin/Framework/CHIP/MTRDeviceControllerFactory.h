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

@protocol MTRStorage;
@protocol MTROTAProviderDelegate;
@protocol MTRKeypair;

@class MTRDeviceController;
@class MTRDeviceControllerStartupParams;

@interface MTRDeviceControllerFactoryParams : NSObject
/*
 * Storage delegate must be provided for correct functioning of Matter
 * controllers.  It is used to store persistent information for the fabrics the
 * controllers ends up interacting with.
 */
@property (nonatomic, strong, readonly) id<MTRStorage> storage;

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
@property (nonatomic, assign) BOOL shouldStartServer;

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithStorage:(id<MTRStorage>)storage;
@end

@interface MTRDeviceControllerFactory : NSObject

/**
 * If true, the factory is in a state where it can create controllers:
 * startControllerFactory has been called, but stopControllerFactory has not been called
 * since then.
 */
@property (readonly, nonatomic, getter=isRunning) BOOL running;

/**
 * Return the single MTRDeviceControllerFactory we support existing.  It starts off
 * in a "not started" state.
 */
+ (instancetype)sharedInstance;

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
 */
- (MTRDeviceController * _Nullable)createControllerOnNewFabric:(MTRDeviceControllerStartupParams *)startupParams
                                                         error:(NSError * __autoreleasing *)error;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
