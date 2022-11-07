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

typedef NSData * MTRCertificateDERBytes;
typedef NSData * MTRCertificateTLVBytes;

NS_ASSUME_NONNULL_BEGIN

@protocol MTRKeypair;

@interface MTRCertificates : NSObject

/**
 * Create a root (self-signed) X.509 DER encoded certificate that has the
 * right fields to be a valid Matter root certificate.
 *
 * If issuerID is nil, a random issuer id is generated.  Otherwise the provided
 * issuer id is used.
 *
 * If fabricID is not nil, it will be included in the subject DN of the
 * certificate.  In this case it must be a valid Matter fabric id.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (MTRCertificateDERBytes _Nullable)createRootCertificate:(id<MTRKeypair>)keypair
                                                 issuerID:(NSNumber * _Nullable)issuerID
                                                 fabricID:(NSNumber * _Nullable)fabricID
                                                    error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_AVAILABLE;

/**
 * Create an intermediate X.509 DER encoded certificate that has the
 * right fields to be a valid Matter intermediate certificate.
 *
 * If issuerID is nil, a random issuer id is generated.  Otherwise the provided
 * issuer id is used.
 *
 * If fabricID is not nil, it will be included in the subject DN of the
 * certificate.  In this case it must be a valid Matter fabric id.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (MTRCertificateDERBytes _Nullable)createIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                                  rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                            intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                                         issuerID:(NSNumber * _Nullable)issuerID
                                                         fabricID:(NSNumber * _Nullable)fabricID
                                                            error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_AVAILABLE;

/**
 * Create an X.509 DER encoded certificate that has the
 * right fields to be a valid Matter operational certificate.
 *
 * signingKeypair and signingCertificate are the root or intermediate that is
 * signing the operational certificate.
 *
 * nodeID and fabricID are expected to be 64-bit unsigned integers.
 *
 * nodeID must be a valid Matter operational node id.
 *
 * fabricID must be a valid Matter fabric id.
 *
 * caseAuthenticatedTags may be nil to indicate no CASE Authenticated Tags
 * should be used.  If caseAuthenticatedTags is not nil, it must have length at
 * most 3 and the values in the array are expected to be 32-bit unsigned Case
 * Authenticated Tag values.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (MTRCertificateDERBytes _Nullable)createOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                              signingCertificate:(MTRCertificateDERBytes)signingCertificate
                                            operationalPublicKey:(SecKeyRef)operationalPublicKey
                                                        fabricID:(NSNumber *)fabricID
                                                          nodeID:(NSNumber *)nodeID
                                           caseAuthenticatedTags:(NSArray<NSNumber *> * _Nullable)caseAuthenticatedTags
                                                           error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_AVAILABLE;

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
+ (BOOL)isCertificate:(MTRCertificateDERBytes)certificate1 equalTo:(MTRCertificateDERBytes)certificate2;

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
+ (NSData * _Nullable)createCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Convert the given X.509v3 DER encoded certificate to the Matter certificate
 * format.
 *
 * Returns nil if the conversion fails (e.g. if the input data cannot be parsed
 * as a DER encoded X.509 certificate, or if the certificate cannot be
 * represented in the Matter certificate format).
 */
+ (MTRCertificateTLVBytes _Nullable)convertX509Certificate:(MTRCertificateDERBytes)x509Certificate;

@end

@interface MTRCertificates (Deprecated)

+ (nullable NSData *)generateRootCertificate:(id<MTRKeypair>)keypair
                                    issuerId:(nullable NSNumber *)issuerId
                                    fabricId:(nullable NSNumber *)fabricId
                                       error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_DEPRECATED("Please use createRootCertificate:issuerID:fabricID:error:");

+ (nullable NSData *)generateIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                     rootCertificate:(NSData *)rootCertificate
                               intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                            issuerId:(nullable NSNumber *)issuerId
                                            fabricId:(nullable NSNumber *)fabricId
                                               error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_DEPRECATED("Please use createIntermediateCertificate:rootCertificate:intermediatePublicKey:issuerID:fabricID:error:");

+ (nullable NSData *)generateOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                 signingCertificate:(NSData *)signingCertificate
                               operationalPublicKey:(SecKeyRef)operationalPublicKey
                                           fabricId:(NSNumber *)fabricId
                                             nodeId:(NSNumber *)nodeId
                              caseAuthenticatedTags:(NSArray<NSNumber *> * _Nullable)caseAuthenticatedTags
                                              error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_DEPRECATED(
        "Plase use "
        "createOperationalCertificate:signingCertificate:operationalPublicKey:fabricID:nodeID:caseAuthenticatedTags:error:");

+ (nullable NSData *)generateCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                 error:(NSError * __autoreleasing _Nullable * _Nullable)error
    MTR_NEWLY_DEPRECATED("Please use createCertificateSigningRequest:error:");

@end

NS_ASSUME_NONNULL_END
