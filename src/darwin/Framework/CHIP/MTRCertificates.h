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
#ifndef MATTER_CERTIFICATES_H
#define MATTER_CERTIFICATES_H

/**
 * Utilities for working with Matter certificates.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol CHIPKeypair;

@interface MTRCertificates : NSObject

/**
 * Generate a root (self-signed) X.509 DER encoded certificate that has the
 * right fields to be a valid Matter root certificate.
 *
 * If issuerId is nil, a random issuer id is generated.  Otherwise the provided
 * issuer id is used.
 *
 * If fabricId is not nil, it will be included in the subject DN of the
 * certificate.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateRootCertificate:(id<CHIPKeypair>)keypair
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
 * certificate.
 *
 * On failure returns nil and if "error" is not null sets *error to the relevant
 * error.
 */
+ (nullable NSData *)generateIntermediateCertificate:(id<CHIPKeypair>)rootKeypair
                                     rootCertificate:(NSData *)rootCertificate
                               intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                            issuerId:(nullable NSNumber *)issuerId
                                            fabricId:(nullable NSNumber *)fabricId
                                               error:(NSError * __autoreleasing _Nullable * _Nullable)error;

/**
 * Check whether the given keypair's public key matches the given certificate's
 * public key.
 *
 * Will return NO on failures to extract public keys from the objects.
 */
+ (BOOL)keypairMatchesCertificate:(NSData *)certificate keypair:(id<CHIPKeypair>)keypair;

@end

NS_ASSUME_NONNULL_END

#endif // MATTER_CERTIFICATES_H
