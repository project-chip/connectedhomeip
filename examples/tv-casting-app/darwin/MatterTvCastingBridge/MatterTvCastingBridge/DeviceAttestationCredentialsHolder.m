/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "DeviceAttestationCredentialsHolder.h"

#import <Foundation/Foundation.h>

@interface DeviceAttestationCredentialsHolder ()

@property NSData * certificationDeclaration;

@property NSData * firmwareInformation;

@property NSData * deviceAttestationCert;

@property NSData * productAttestationIntermediateCert;

@property SecKeyRef deviceAttestationCertPrivateKeyRef;

@end

@implementation DeviceAttestationCredentialsHolder

- (DeviceAttestationCredentialsHolder * _Nonnull)
      initWithCertificationDeclaration:(NSData * _Nonnull)certificationDeclaration
                   firmwareInformation:(NSData * _Nonnull)firmwareInformation
                 deviceAttestationCert:(NSData * _Nonnull)deviceAttestationCert
    productAttestationIntermediateCert:(NSData * _Nonnull)productAttestationIntermediateCert
    deviceAttestationCertPrivateKeyRef:(SecKeyRef _Nonnull)deviceAttestationCertPrivateKeyRef
{
    self = [super init];
    if (self) {
        _certificationDeclaration = certificationDeclaration;
        _firmwareInformation = firmwareInformation;
        _deviceAttestationCert = deviceAttestationCert;
        _productAttestationIntermediateCert = productAttestationIntermediateCert;
        _deviceAttestationCertPrivateKeyRef = deviceAttestationCertPrivateKeyRef;
    }
    return self;
}

- (NSData * _Nonnull)getCertificationDeclaration
{
    return _certificationDeclaration;
}

- (NSData * _Nonnull)getFirmwareInformation
{
    return _firmwareInformation;
}

- (NSData * _Nonnull)getDeviceAttestationCert
{
    return _deviceAttestationCert;
}

- (NSData * _Nonnull)getProductAttestationIntermediateCert
{
    return _productAttestationIntermediateCert;
}

- (SecKeyRef)getDeviceAttestationCertPrivateKeyRef
{
    return _deviceAttestationCertPrivateKeyRef;
}

@end
