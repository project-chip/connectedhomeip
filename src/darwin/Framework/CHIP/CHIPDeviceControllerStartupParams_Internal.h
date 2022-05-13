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

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceControllerStartupParamsInternal : CHIPDeviceControllerStartupParams

/**
 * Operational certificate to use for the controller bringup.  Allowed to be
 * nil to indicate that a new operational certificate should be generated.
 *
 * Meant to be set by MatterControllerFactory.
 */
@property (strong, nonatomic, nullable) NSData * operationalCertificate;

/**
 * Operational keypair to use for the controller bringup.  If nullptr, a new
 * random operational keypair should be generated.
 *
 * If operationalCertificate is not nil, operationalKeypair must not be
 * nullptr.
 *
 * If operationalCertificate is nil, operationalKeypair may be non-nullptr; that
 * corresponds to needing to create a new NOC (e.g. if our signing certificate
 * changed) without changing our operational identity.
 *
 * Meant to be set by MatterControllerFactory.
 *
 * Assumed to be allocated via C++ new and will be deleted via C++ delete.
 */
@property (nonatomic, nullable) chip::Crypto::P256SerializedKeypair * operationalKeypair;

/**
 * Helper method that checks that the nocSigner keypair public key matches the
 * intermediate cert (if there is one) or the root cert (if there is no
 * intermediate cert).  Returns YES if it does, NO if it does not or if we can't
 * even extract public keys from the cert and nocSigner.
 */
- (BOOL)nocSignerMatchesCerts;

@end

NS_ASSUME_NONNULL_END
