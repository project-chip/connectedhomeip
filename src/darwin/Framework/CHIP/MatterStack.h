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

#ifndef MATTER_STACK_H
#define MATTER_STACK_H

/**
 * An object representing a Matter stack.  There can be only one such object in
 * a given process.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol CHIPPersistentStorageDelegate;
@protocol CHIPKeypair;
@class CHIPDeviceController;

@interface MatterStackStartupParams : NSObject
/*
 * Storage delegate must be provided for correct functioning of the stack.  It
 * is used to store persistent information for the fabrics the stack ends up
 * interacting with.
 */
@property (strong, nonatomic, readonly) id<CHIPPersistentStorageDelegate> storageDelegate;
/*
 * The Product Attestation Authority certificates that are trusted to sign
 * device attestation information.  Defaults to nil.
 *
 * TODO: We may wabt to support different PAA sets per controller, but
 * the underlying SDK code does not handle that right now.
 */
@property (strong, nonatomic, nullable) NSArray<NSData *> * paaCerts;
/*
 * The network port to bind to.  If not specified, an ephemeral port will be
 * used.
 */
@property (strong, nonatomic, nullable) NSNumber * port;
/*
 * Whether to run a server capable of accepting incoming CASE
 * connections.  Defaults to NO.
 */
@property (nonatomic) BOOL startServer;
/*
 * Path to a file to use for backing our KVS storage.  This should not
 * be used, generally; it will be removed soon.  Defaults to nullptr;
 */
@property (nonatomic) const char * kvsPath;

- (instancetype)init NS_UNAVAILABLE;
// TODO The delegate should not be nullable, but we need to sort out
// CHIPClustersTests use of these APIs first.
- (instancetype)initWithStorage:(_Nullable id<CHIPPersistentStorageDelegate>)storageDelegate;
@end

@interface CHIPDeviceControllerStartupParams : NSObject
/*
 * Vendor ID (allocated by the Connectivity Standards Alliance) for
 * this controller.  Must be set by consumer.
 */
@property (nonatomic) uint16_t vendorId;
/*
 * Root CA keypair to use for signing certificates.  Nullable for now,
 * but that will change.
 */
@property (strong, nonatomic, nullable) id<CHIPKeypair> rootCAKeypair;
/*
 * Fabric id for the controller.  Must be set to a nonzero value.
 */
@property (nonatomic) uint64_t fabricId;
/*
 * IPK to use for the controller's fabric.  Allowed to be null when
 * starting a controller on an existing fabric.
 */
@property (strong, nonatomic, nullable) NSData * ipk;

- (instancetype)init;
@end

@interface MatterStack : NSObject

@property (readonly, nonatomic) BOOL isRunning;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single MatterStack we support existing.  It starts off
 * in a "not started" state.
 */
+ (MatterStack *)singletonStack;

/**
 * Start the Matter Stack. Repeated calls to startup without calls to shutdown
 * in between are NO-OPs. Use the isRunning property to check whether the stack
 * needs to be started up.
 *
 * @param[in] startupParams data needed to start up the stack.
 *
 * @return Whether startup succeded.
 */
- (BOOL)startup:(MatterStackStartupParams *)startupParams;

/**
 * Shut down the Matter Stack. This will shut down any outstanding
 * controllers as part of the stack shutdown.
 *
 * Repeated calls to shutdown without calls to startup in between are
 * NO-OPs.
 */
- (void)shutdown;

/**
 * Create a CHIPDeviceController on an existing fabric.  Returns nil on failure.
 *
 * This method will fail if there is no such fabric or if there is
 * already a controller started for that fabric.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 */
- (CHIPDeviceController * _Nullable)startControllerOnExistingFabric:(CHIPDeviceControllerStartupParams *)startupParams;

/**
 * Create a CHIPDeviceController on a new fabric.  Returns nil on failure.
 *
 * This method will fail if the given fabric already exists.
 *
 * The fabric is identified by the root public key and fabric id in
 * the startupParams.
 */
- (CHIPDeviceController * _Nullable)startControllerOnNewFabric:(CHIPDeviceControllerStartupParams *)startupParams;

- (void)dealloc;

@end

NS_ASSUME_NONNULL_END

#endif // MATTER_STACK_H
