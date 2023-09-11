/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationCredentials.h"
#import <Foundation/Foundation.h>

@implementation MTRDeviceAttestationCredentials

- (instancetype)initWithCertificationDeclaration:(NSData * _Nonnull)certificationDeclaration
                             firmwareInformation:(NSData * _Nonnull)firmwareInformation
                           deviceAttestationCert:(NSData * _Nonnull)deviceAttestationCert
              productAttestationIntermediateCert:(NSData * _Nonnull)productAttestationIntermediateCert;
{
    if (self = [super init]) {
        _certificationDeclaration = certificationDeclaration;
        _firmwareInformation = firmwareInformation;
        _deviceAttestationCert = deviceAttestationCert;
        _productAttestationIntermediateCert = productAttestationIntermediateCert;
    }
    return self;
}

@end
