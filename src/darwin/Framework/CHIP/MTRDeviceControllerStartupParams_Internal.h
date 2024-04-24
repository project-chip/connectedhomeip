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

#pragma once

#import "MTRDeviceControllerStartupParams.h"
#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerParameters.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

namespace chip {
class FabricTable;

namespace Crypto {
    class OperationalKeystore;
} // namespace Crypto
} // namespace chip

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceControllerStartupParams ()
// We want to be able to write to operationalCertificate in
// MTRDeviceControllerStartupParamsInternal.
@property (nonatomic, copy, nullable) MTRCertificateDERBytes operationalCertificate;

// uniqueIdentifier, so that we always have one.
@property (nonatomic, strong, readonly) NSUUID * uniqueIdentifier;

// Init method that just copies the values of all our ivars.
- (instancetype)initWithParams:(MTRDeviceControllerStartupParams *)params;
@end

@interface MTRDeviceControllerAbstractParameters ()
// Allow init from our subclasses.
- (instancetype)_initInternal;
@end

@interface MTRDeviceControllerParameters ()

- (instancetype)initWithStorageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                   storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue
                       uniqueIdentifier:(NSUUID *)uniqueIdentifier
                                    ipk:(NSData *)ipk
                               vendorID:(NSNumber *)vendorID
                     operationalKeypair:(id<MTRKeypair>)operationalKeypair
                 operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                        rootCertificate:(MTRCertificateDERBytes)rootCertificate;

// When we have other subclasses of MTRDeviceControllerParameters, we may
// need to make more things nullable here and/or add more fields.  But for now
// we know exactly what information we have.
@property (nonatomic, copy, readonly) NSData * ipk;
@property (nonatomic, copy, readonly) NSNumber * vendorID;
@property (nonatomic, copy, readonly) MTRCertificateDERBytes rootCertificate;
@property (nonatomic, copy, readonly, nullable) MTRCertificateDERBytes intermediateCertificate;
@property (nonatomic, copy, readonly) MTRCertificateDERBytes operationalCertificate;
@property (nonatomic, strong, readonly) id<MTRKeypair> operationalKeypair;

@property (nonatomic, strong, nullable, readonly) id<MTROperationalCertificateIssuer> operationalCertificateIssuer;
@property (nonatomic, strong, nullable, readonly) dispatch_queue_t operationalCertificateIssuerQueue;

@property (nonatomic, strong, readonly) id<MTRDeviceControllerStorageDelegate> storageDelegate;
@property (nonatomic, strong, readonly) dispatch_queue_t storageDelegateQueue;
@property (nonatomic, strong, readonly) NSUUID * uniqueIdentifier;

@property (nonatomic, strong, readonly, nullable) id<MTROTAProviderDelegate> otaProviderDelegate;
@property (nonatomic, strong, readonly, nullable) dispatch_queue_t otaProviderDelegateQueue;

@end

@interface MTRDeviceControllerStartupParamsInternal : MTRDeviceControllerStartupParams

// Fabric table we can use to do things like allocate operational keys.
@property (nonatomic, assign, readonly) chip::FabricTable * fabricTable;

// Fabric index we're starting on.  Only has a value when starting against an
// existing fabric table entry.
@property (nonatomic, assign, readonly) chip::Optional<chip::FabricIndex> fabricIndex;

// Key store we're using with our fabric table, for sanity checks.
@property (nonatomic, assign, readonly) chip::Crypto::OperationalKeystore * keystore;

@property (nonatomic, assign, readonly) BOOL advertiseOperational;

@property (nonatomic, assign, readonly) BOOL allowMultipleControllersPerFabric;

@property (nonatomic, nullable) NSArray<MTRCertificateDERBytes> * productAttestationAuthorityCertificates;
@property (nonatomic, nullable) NSArray<MTRCertificateDERBytes> * certificationDeclarationCertificates;

/**
 * A storage delegate that can be provided when initializing the startup params.
 * This must be provided if and only if the controller factory was initialized
 * without storage.
 */
@property (nonatomic, strong, nullable, readonly) id<MTRDeviceControllerStorageDelegate> storageDelegate;

/**
 * The queue to use for storageDelegate.  This will be nil if and only if
 * storageDelegate is nil.
 */
@property (nonatomic, strong, nullable, readonly) dispatch_queue_t storageDelegateQueue;

/**
 * Helper method that checks that our keypairs match our certificates.
 * Specifically:
 *
 * 1) If we have a nocSigner keypair, its public key matches the intermediate
 *    cert (if there is one) or the root cert (if there is no intermediate
 *    cert).  Returns YES if we have no nocSigner or if the keys match, NO if
 *    the keys do not match or if we can't even extract public keys from the
 *    certs and nocSigner.
 * 2) If we have both an operationalKeypair and an operationalCertificate,
 *    their public keys match.
 */
- (BOOL)keypairsMatchCertificates;

/**
 * Initialize for controller bringup on a new fabric.
 */
- (instancetype)initForNewFabric:(chip::FabricTable *)fabricTable
                        keystore:(chip::Crypto::OperationalKeystore *)keystore
            advertiseOperational:(BOOL)advertiseOperational
                          params:(MTRDeviceControllerStartupParams *)params;

/**
 * Initialize for controller bringup on an existing fabric.
 */
- (instancetype)initForExistingFabric:(chip::FabricTable *)fabricTable
                          fabricIndex:(chip::FabricIndex)fabricIndex
                             keystore:(chip::Crypto::OperationalKeystore *)keystore
                 advertiseOperational:(BOOL)advertiseOperational
                               params:(MTRDeviceControllerStartupParams *)params;

/**
 * Initialize for controller bringup with per-controller storage.
 */
- (instancetype)initForNewController:(MTRDeviceController *)controller
                         fabricTable:(chip::FabricTable *)fabricTable
                            keystore:(chip::Crypto::OperationalKeystore *)keystore
                advertiseOperational:(BOOL)advertiseOperational
                              params:(MTRDeviceControllerParameters *)params
                               error:(CHIP_ERROR &)error;

/**
 * Should use initForExistingFabric or initForNewFabric or initForController to initialize
 * internally.
 */
- (instancetype)initWithIPK:(NSData *)ipk fabricID:(NSNumber *)fabricID nocSigner:(id<MTRKeypair>)nocSigner NS_UNAVAILABLE;
- (instancetype)initWithIPK:(NSData *)ipk
         operationalKeypair:(id<MTRKeypair>)operationalKeypair
     operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
    intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
            rootCertificate:(MTRCertificateDERBytes)rootCertificate NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
