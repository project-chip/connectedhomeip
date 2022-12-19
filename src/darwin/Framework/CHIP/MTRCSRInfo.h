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

#import <Foundation/Foundation.h>

#import <Matter/MTRDefines.h>

typedef NSData * MTRCSRDERBytes;

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents information relating to a certificate signing request for a Matter
 * operational certificate.
 */
MTR_NEWLY_AVAILABLE
@interface MTROperationalCSRInfo : NSObject

/**
 * DER-encoded certificate signing request.
 */
@property (nonatomic, copy, readonly) MTRCSRDERBytes csr;
/**
 * The nonce provided in the original CSRRequest command hat led to this CSR
 * being created.
 */
@property (nonatomic, copy, readonly) NSData * csrNonce;
/**
 * TLV-encoded nocsr-elements structure.  This includes the "csr" and "csrNonce"
 * fields, and can include additional vendor-specific information.
 */
@property (nonatomic, copy, readonly) MTRTLVBytes csrElementsTLV;
/**
 * A signature, using the device attestation private key of the device that
 * created the CSR, over the concatenation of csrElementsTLV and the attestation
 * challenge from the secure session.
 *
 * The attestation challenge is available in MTRAttestionInfo.
 */
@property (nonatomic, copy, readonly) NSData * attestationSignature;

- (instancetype)initWithCSR:(MTRCSRDERBytes)csr
                   csrNonce:(NSData *)csrNonce
             csrElementsTLV:(MTRTLVBytes)csrElementsTLV
       attestationSignature:(NSData *)attestationSignature;

@end

MTR_NEWLY_DEPRECATED("Please use MTROperationalCSRInfo")
@interface CSRInfo : NSObject

@property (nonatomic, copy) NSData * nonce;

@property (nonatomic, copy) NSData * elements;

@property (nonatomic, copy) NSData * elementsSignature;

@property (nonatomic, copy) NSData * csr;

- (instancetype)initWithNonce:(NSData *)nonce
                     elements:(NSData *)elements
            elementsSignature:(NSData *)elementsSignature
                          csr:(NSData *)csr;

@end

NS_ASSUME_NONNULL_END
