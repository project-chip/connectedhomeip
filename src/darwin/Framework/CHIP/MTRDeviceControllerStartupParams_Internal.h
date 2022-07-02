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
@property (strong, nonatomic, nullable) NSData * operationalCertificate;

// Init method that just copies the values of all our ivars.
- (instancetype)initWithParams:(MTRDeviceControllerStartupParams *)params;
@end

@interface MTRDeviceControllerStartupParamsInternal : MTRDeviceControllerStartupParams

// Fabric table we can use to do things like allocate operational keys.
@property (nonatomic, readonly) chip::FabricTable * fabricTable;

// Fabric index we're starting on.  Only has a value when starting on an
// existing fabric.
@property (nonatomic, readonly) chip::Optional<chip::FabricIndex> fabricIndex;

// Key store we're using with our fabric table, for sanity checks.
@property (nonatomic, readonly) chip::Crypto::OperationalKeystore * keystore;

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
                          params:(MTRDeviceControllerStartupParams *)params;

/**
 * Initialize for controller bringup on an existing fabric.
 */
- (instancetype)initForExistingFabric:(chip::FabricTable *)fabricTable
                          fabricIndex:(chip::FabricIndex)fabricIndex
                             keystore:(chip::Crypto::OperationalKeystore *)keystore
                               params:(MTRDeviceControllerStartupParams *)params;

- (instancetype)initWithSigningKeypair:(id<MTRKeypair>)nocSigner fabricId:(uint64_t)fabricId ipk:(NSData *)ipk NS_UNAVAILABLE;
- (instancetype)initWithOperationalKeypair:(id<MTRKeypair>)operationalKeypair
                    operationalCertificate:(NSData *)operationalCertificate
                   intermediateCertificate:(nullable NSData *)intermediateCertificate
                           rootCertificate:(NSData *)rootCertificate
                                       ipk:(NSData *)ipk NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
