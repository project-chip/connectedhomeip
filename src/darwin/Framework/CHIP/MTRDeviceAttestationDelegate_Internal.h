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

#import "MTRDeviceAttestationDelegate.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceAttestationDeviceInfo ()

- (instancetype)initWithAttestationChallenge:(NSData *)attestationChallenge
                            attestationNonce:(NSData *)attestationNonce
                                 elementsTLV:(MTRTLVBytes)elementsTLV
                           elementsSignature:(NSData *)elementsSignature
                              dacCertificate:(MTRCertificateDERBytes)dacCertificate
                           dacPAICertificate:(MTRCertificateDERBytes)dacPAICertificate
                    certificationDeclaration:(nullable NSData *)certificationDeclaration
                    basicInformationVendorID:(NSNumber *)basicInformationVendorID
                   basicInformationProductID:(NSNumber *)basicInformationProductID;

@end

NS_ASSUME_NONNULL_END
