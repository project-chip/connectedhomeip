/**
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationInfo.h"

NS_ASSUME_NONNULL_BEGIN

@implementation MTRDeviceAttestationInfo

- (instancetype)initWithDeviceAttestationChallenge:(NSData *)challenge
                                             nonce:(NSData *)nonce
                                       elementsTLV:(MTRTLVBytes)elementsTLV
                                 elementsSignature:(NSData *)elementsSignature
                      deviceAttestationCertificate:(MTRCertificateDERBytes)deviceAttestationCertificate
         productAttestationIntermediateCertificate:(MTRCertificateDERBytes)productAttestationIntermediateCertificate
                          certificationDeclaration:(NSData *)certificationDeclaration
                                      firmwareInfo:(NSData *)firmwareInfo
{
    if (self = [super init]) {
        _challenge = challenge;
        _nonce = nonce;
        _elementsTLV = elementsTLV;
        _elementsSignature = elementsSignature;
        _deviceAttestationCertificate = deviceAttestationCertificate;
        _productAttestationIntermediateCertificate = productAttestationIntermediateCertificate;
        _certificationDeclaration = certificationDeclaration;
        _firmwareInfo = firmwareInfo;
    }
    return self;
}

@end

@implementation AttestationInfo

- (instancetype)initWithChallenge:(NSData *)challenge
                            nonce:(NSData *)nonce
                         elements:(NSData *)elements
                elementsSignature:(NSData *)elementsSignature
                              dac:(NSData *)dac
                              pai:(NSData *)pai
         certificationDeclaration:(NSData *)certificationDeclaration
                     firmwareInfo:(NSData *)firmwareInfo
{
    if (self = [super init]) {
        _challenge = challenge;
        _nonce = nonce;
        _elements = elements;
        _elementsSignature = elementsSignature;
        _dac = dac;
        _pai = pai;
        _certificationDeclaration = certificationDeclaration;
        _firmwareInfo = firmwareInfo;
    }
    return self;
}

@end

NS_ASSUME_NONNULL_END
