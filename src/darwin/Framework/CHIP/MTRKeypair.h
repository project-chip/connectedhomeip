/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
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
@protocol MTRKeypair <NSObject>
@required
/**
 * @brief Return public key for the keypair.
 */
- (SecKeyRef)publicKey;

@optional
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
