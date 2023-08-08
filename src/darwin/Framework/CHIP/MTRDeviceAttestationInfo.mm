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
                          basicInformationVendorID:(NSNumber *)basicInformationVendorID
                         basicInformationProductID:(NSNumber *)basicInformationProductID
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
        _basicInformationVendorID = basicInformationVendorID;
        _basicInformationProductID = basicInformationProductID;
    }
    return self;
}

- (instancetype)initWithDeviceAttestationChallenge:(NSData *)challenge
                                             nonce:(NSData *)nonce
                                       elementsTLV:(MTRTLVBytes)elementsTLV
                                 elementsSignature:(NSData *)elementsSignature
                      deviceAttestationCertificate:(MTRCertificateDERBytes)deviceAttestationCertificate
         productAttestationIntermediateCertificate:(MTRCertificateDERBytes)productAttestationIntermediateCertificate
                          certificationDeclaration:(NSData *)certificationDeclaration
                                      firmwareInfo:(NSData *)firmwareInfo
{
    return [self initWithDeviceAttestationChallenge:challenge
                                              nonce:nonce
                                        elementsTLV:elementsTLV
                                  elementsSignature:elementsSignature
                       deviceAttestationCertificate:deviceAttestationCertificate
          productAttestationIntermediateCertificate:productAttestationIntermediateCertificate
                           certificationDeclaration:certificationDeclaration
                                       firmwareInfo:firmwareInfo
                           basicInformationVendorID:@(0)
                          basicInformationProductID:@(0)];
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
