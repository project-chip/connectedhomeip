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
#import <Security/Security.h>

NS_ASSUME_NONNULL_BEGIN

@protocol CHIPKeypair <NSObject>
@required
/**
 * @brief Return public key for the keypair.
 */
- (SecKeyRef)pubkey;

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
 * Either this selector or ECDSA_sign_message_DER must be supported by a
 * CHIPKeypair.
 */
- (NSData *)ECDSA_sign_message_raw:(NSData *)message;

/**
 * @brief A function to sign a message using ECDSA
 *
 * @param message Message that needs to be signed
 *
 * @return An ASN.1 DER-encoded signature (per X9.62).
 *
 * Either this selector or ECDSA_sign_message_raw must be supported by a
 * CHIPKeypair.
 */
- (NSData *)ECDSA_sign_message_DER:(NSData *)message;

@end

NS_ASSUME_NONNULL_END
