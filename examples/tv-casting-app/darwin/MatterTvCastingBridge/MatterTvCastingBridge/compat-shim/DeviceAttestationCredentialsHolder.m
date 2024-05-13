/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
