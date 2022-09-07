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

/**
 * Utilities for working with Matter certificates.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRKeypair;

@interface MTRCertificates : NSObject

/**
 * Generate a root (self-signed) X.509 DER encoded certificate that has the
 * right fields to be a valid Matter root certificate.
 *
 * If issuerId is nil, a random issuer id is generated.  Otherwise the provided
 * issuer id is used.
 *
 * If fabricId is not nil, it will be included in the subject DN of the
 * certificate.  In this case it must be a valid Matter fabric id.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateRootCertificate:(id<MTRKeypair>)keypair
                                    issuerId:(nullable NSNumber *)issuerId
                                    fabricId:(nullable NSNumber *)fabricId
                                       error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Generate an intermediate X.509 DER encoded certificate that has the
 * right fields to be a valid Matter intermediate certificate.
 *
 * If issuerId is nil, a random issuer id is generated.  Otherwise the provided
 * issuer id is used.
 *
 * If fabricId is not nil, it will be included in the subject DN of the
 * certificate.  In this case it must be a valid Matter fabric id.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                     rootCertificate:(NSData *)rootCertificate
                               intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                            issuerId:(nullable NSNumber *)issuerId
                                            fabricId:(nullable NSNumber *)fabricId
                                               error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Generate an X.509 DER encoded certificate that has the
 * right fields to be a valid Matter operational certificate.
 *
 * signingKeypair and signingCertificate are the root or intermediate that is
 * signing the operational certificate.
 *
 * nodeId and fabricId are expected to be 64-bit unsigned integers.
 *
 * nodeId must be a valid Matter operational node id.
 *
 * fabricId must be a valid Matter fabric id.
 *
 * caseAuthenticatedTags may be nil to indicate no CASE Authenticated Tags
 * should be used.  If caseAuthenticatedTags is not nil, it must have length at
 * most 3 and the values in the array are expected to be 32-bit unsigned Case
 * Authenticated Tag values.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                 signingCertificate:(NSData *)signingCertificate
                               operationalPublicKey:(SecKeyRef)operationalPublicKey
                                           fabricId:(NSNumber *)fabricId
                                             nodeId:(NSNumber *)nodeId
                              caseAuthenticatedTags:(NSArray<NSNumber *> * _Nullable)caseAuthenticatedTags
                                              error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Check whether the given keypair's public key matches the given certificate's
 * public key.  The certificate is expected to be an X.509 DER encoded
 * certificate.
 *
 * Will return NO on failures to extract public keys from the objects.
 */
+ (BOOL)keypair:(id<MTRKeypair>)keypair matchesCertificate:(NSData *)certificate;

/**
 * Check whether two X.509 DER encoded certificates are equivalent, in the sense
 * of having the same public key and the same subject DN.  Returns NO if public
 * keys or subject DNs cannot be extracted from the certificates.
 */
+ (BOOL)isCertificate:(NSData *)certificate1 equalTo:(NSData *)certificate2;

/**
 * Generate a PKCS#10 certificate signing request from a MTRKeypair.  This can
 * then be used to request an operational or ICA certificate from an external
 * certificate authority.
 *
 * The CSR will have the subject OU DN set to 'CSA', because omitting all
 * identifying information altogether often trips up CSR parsing code.  The CA
 * being used should expect this and ignore the request subject, producing a
 * subject that matches the rules for Matter certificates.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                 error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Convert the given X.509v3 DER encoded certificate to the Matter certificate
 * format.
 *
 * Returns nil if the conversion fails (e.g. if the input data cannot be parsed
 * as a DER encoded X.509 certificate, or if the certificate cannot be
 * represented in the Matter certificate format).
 */
+ (nullable NSData *)convertX509Certificate:(NSData *)x509Certificate;

@end

NS_ASSUME_NONNULL_END
