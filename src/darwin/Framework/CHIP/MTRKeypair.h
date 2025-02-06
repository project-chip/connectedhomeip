/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>
#import <Security/Security.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * This protocol is used by the Matter framework to sign messages with a private
 * key and verify signatures with a public key.
 *
 * The Matter framework may call keypair methods from arbitrary threads and
 * concurrently.
 *
 * Implementations of the keypair methods must not call into any Matter
 * framework APIs.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@protocol MTRKeypair <NSObject>

@optional
/**
 * @brief Returns a copy of the public key for the keypair.
 */
- (SecKeyRef)copyPublicKey CF_RETURNS_RETAINED MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

/**
 * @brief Returns public key for the keypair without adding a reference. DEPRECATED - please use copyPublicKey, otherwise this will leak.
 */
- (SecKeyRef)publicKey MTR_DEPRECATED("Please implement copyPublicKey, this will leak otherwise", ios(16.1, 18.4), macos(13.0, 15.4), watchos(9.1, 11.4), tvos(16.1, 18.4));

/**
 * @brief A function to sign a message using ECDSA
 *
 * @param message Message that needs to be signed
 *
 * @return A signature that consists of: 2 EC elements (r and s), in raw <r,s>
 *         point form (see SEC1).  Sometimes also called RFC 4754 form or P1363
 *         form.
 *
 * Either this selector or signMessageECDSA_DER must be supported by a
 * MTRKeypair.
 */
- (NSData *)signMessageECDSA_RAW:(NSData *)message;

/**
 * @brief A function to sign a message using ECDSA
 *
 * @param message Message that needs to be signed
 *
 * @return An ASN.1 DER-encoded signature (per X9.62).
 *
 * Either this selector or signMessageECDSA_RAW must be supported by a
 * MTRKeypair.
 */
- (NSData *)signMessageECDSA_DER:(NSData *)message;

@end

NS_ASSUME_NONNULL_END
