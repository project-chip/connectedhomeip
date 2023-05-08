/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents information relating to a certificate signing request for a Matter
 * operational certificate.
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTROperationalCSRInfo : NSObject

/**
 * DER-encoded certificate signing request.
 */
@property (nonatomic, copy, readonly) MTRCSRDERBytes csr;

/**
 * The nonce associated with this CSR.
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

/**
 * Initialize an MTROperationalCSRInfo by providing all the fields.  It's the
 * caller's responsibility to ensure that csr and csrNonce match the csrElementsTLV.
 */
- (instancetype)initWithCSR:(MTRCSRDERBytes)csr
                   csrNonce:(NSData *)csrNonce
             csrElementsTLV:(MTRTLVBytes)csrElementsTLV
       attestationSignature:(NSData *)attestationSignature
    MTR_NEWLY_DEPRECATED("Please use one of the initializers that validates the input");

/**
 * Initialize an MTROperationalCSRInfo by providing the csrNonce (for example,
 * the nonce the client initially supplied), and the csrElementsTLV and
 * attestationSignature that the server returned.  This will ensure that
 * csrNonce matches the data in csrElementsTLV, returning nil if it does not,
 * and extract the csr from csrElementsTLV.
 */
- (nullable instancetype)initWithCSRNonce:(NSData *)csrNonce
                           csrElementsTLV:(MTRTLVBytes)csrElementsTLV
                     attestationSignature:(NSData *)attestationSignature MTR_NEWLY_AVAILABLE;

/**
 * Initialize an MTROperationalCSRInfo by providing just the csrElementsTLV and
 * attestationSignature (which can come from an
 * MTROperationalCredentialsClusterCSRResponseParams).  This will extract the
 * csr and csrNonce from the csrElementsTLV, if possible, and return nil if that
 * fails.
 */
- (nullable instancetype)initWithCSRElementsTLV:(MTRTLVBytes)csrElementsTLV
                           attestationSignature:(NSData *)attestationSignature MTR_NEWLY_AVAILABLE;

/**
 * Initialize an MTROperationalCSRInfo by providing an
 * MTROperationalCredentialsClusterCSRResponseParams.  This will extract the
 * relevant fields from the response data.
 */
- (nullable instancetype)initWithCSRResponseParams:(MTROperationalCredentialsClusterCSRResponseParams *)responseParams
    MTR_NEWLY_AVAILABLE;
@end

MTR_DEPRECATED("Please use MTROperationalCSRInfo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
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
