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

#import <Matter/MTRCertificates.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents information relating to product attestation.
 *
 */
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDeviceAttestationInfo : NSObject

/**
 * The attestation challenge from the secure session.
 */
@property (nonatomic, copy, readonly) NSData * challenge;
/**
 * The attestation nonce from the AttestationRequest command.
 */
@property (nonatomic, copy, readonly) NSData * nonce;
/**
 * The TLV-encoded attestation_elements_message that was used to find the
 * certificationDeclaration and firmwareInfo.
 */
@property (nonatomic, copy, readonly) MTRTLVBytes elementsTLV;
/**
 * A signature, using the device attestation private key of the device that sent
 * the attestation information, over the concatenation of elementsTLV and the
 * attestation challenge from the secure session.
 */
@property (nonatomic, copy, readonly) NSData * elementsSignature;
/**
 * The device attestation certificate for the device.  This can be used to
 * verify signatures created with the device attestation private key.
 */
@property (nonatomic, copy, readonly) MTRCertificateDERBytes deviceAttestationCertificate;
/**
 * The product attestation intermediate certificate that can be used to verify
 * the authenticity of the device attestation certificate.
 */
@property (nonatomic, copy, readonly) MTRCertificateDERBytes productAttestationIntermediateCertificate;
/**
 * The certification declaration of the device.  This is a DER-encoded string
 * representing a CMS-formatted certification declaration.
 */
@property (nonatomic, copy, readonly) NSData * certificationDeclaration;
/*
 * Firmware information, if any, provided in the elementsTLV.  The encoding of
 * this is not currently specified, but if present this must match the
 * Distributed Compliance Ledger entry for the device.
 */
@property (nonatomic, copy, readonly, nullable) NSData * firmwareInfo;

- (instancetype)initWithDeviceAttestationChallenge:(NSData *)challenge
                                             nonce:(NSData *)nonce
                                       elementsTLV:(MTRTLVBytes)elementsTLV
                                 elementsSignature:(NSData *)elementsSignature
                      deviceAttestationCertificate:(MTRCertificateDERBytes)deviceAttestationCertificate
         productAttestationIntermediateCertificate:(MTRCertificateDERBytes)processAttestationIntermediateCertificate
                          certificationDeclaration:(NSData *)certificationDeclaration
                                      firmwareInfo:(NSData *)firmwareInfo;

@end

MTR_DEPRECATED("Please use MTRDeviceAttestationInfo", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface AttestationInfo : NSObject

@property (nonatomic, copy) NSData * challenge;

@property (nonatomic, copy) NSData * nonce;

@property (nonatomic, copy) NSData * elements;

@property (nonatomic, copy) NSData * elementsSignature;

@property (nonatomic, copy) NSData * dac;

@property (nonatomic, copy) NSData * pai;

@property (nonatomic, copy) NSData * certificationDeclaration;

@property (nonatomic, copy) NSData * firmwareInfo;

- (instancetype)initWithChallenge:(NSData *)challenge
                            nonce:(NSData *)nonce
                         elements:(NSData *)elements
                elementsSignature:(NSData *)elementsSignature
                              dac:(NSData *)dac
                              pai:(NSData *)pai
         certificationDeclaration:(NSData *)certificationDeclaration
                     firmwareInfo:(NSData *)firmwareInfo;

@end

NS_ASSUME_NONNULL_END
