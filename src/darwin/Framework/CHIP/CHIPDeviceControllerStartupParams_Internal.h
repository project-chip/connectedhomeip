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

#import "CHIPDeviceControllerStartupParams.h"
#import <Foundation/Foundation.h>

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
class FabricInfo;
}

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceControllerStartupParams ()
// We want to be able to write to operationalCertificate in
// CHIPDeviceControllerStartupParamsInternal.
@property (strong, nonatomic, nullable) NSData * operationalCertificate;

// Init method that just copies the values of all our ivars.
- (instancetype)initWithParams:(CHIPDeviceControllerStartupParams *)params;
@end

@interface CHIPDeviceControllerStartupParamsInternal : CHIPDeviceControllerStartupParams

/**
 * We may have an operational keypair either provided externally, via
 * operationalKeypair, or internally (from the fabric table) via
 * serializedOperationalKeypair.
 *
 * If operationalKeypair is nil and serializedOperationalKeypair is nullptr, a
 * new random operational keypair will be generated.
 *
 * If operationalCertificate is not nil, either operationalKeypair must be not
 * nil or serializedOperationalKeypair must be not nullptr.
 *
 * If operationalCertificate is nil, operationalKeypair may be not nil or
 * serializedOperationalKeypair may be not nullptr; that corresponds to needing
 * to create a new NOC (e.g. if our signing certificate changed) without
 * changing our operational identity.
 *
 * Meant to be set by MatterControllerFactory.
 *
 * Assumed to be allocated via C++ new and will be deleted via C++ delete.
 */
@property (nonatomic, nullable, readonly) chip::Crypto::P256SerializedKeypair * serializedOperationalKeypair;

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
- (instancetype)initForNewFabric:(CHIPDeviceControllerStartupParams *)params;

/**
 * Initialize for controller bringup on an existing fabric.
 */
- (instancetype)initForExistingFabric:(chip::FabricInfo *)fabric params:(CHIPDeviceControllerStartupParams *)params;

- (instancetype)initWithSigningKeypair:(id<CHIPKeypair>)nocSigner fabricId:(uint64_t)fabricId ipk:(NSData *)ipk NS_UNAVAILABLE;
- (instancetype)initWithOperationalKeypair:(id<CHIPKeypair>)operationalKeypair
                    operationalCertificate:(NSData *)operationalCertificate
                   intermediateCertificate:(nullable NSData *)intermediateCertificate
                           rootCertificate:(NSData *)rootCertificate
                                       ipk:(NSData *)ipk NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
