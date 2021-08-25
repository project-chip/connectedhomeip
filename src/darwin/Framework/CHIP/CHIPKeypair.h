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
 * @brief Initialize the keypair.
 * @return Should return whether or not the keypair was successfully initialized
 **/
- (BOOL)initialize;

/**
 * @brief A function to sign a hash using ECDSA
 * @param hash Hash that needs to be signed
 *
 * @return Returns A signature that consists of: 2 EC elements (r and s), in raw <r,s> point form (see SEC1).
 **/
- (NSData *)ECDSA_sign_hash:(NSData *)hash;

/** @brief Return public key for the keypair.
 **/
- (SecKeyRef)pubkey;

@end

NS_ASSUME_NONNULL_END
