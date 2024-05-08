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

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#ifndef DeviceAttestationCredentialsHolder_h
#define DeviceAttestationCredentialsHolder_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface DeviceAttestationCredentialsHolder : NSObject

- (DeviceAttestationCredentialsHolder * _Nonnull)
      initWithCertificationDeclaration:(NSData * _Nonnull)certificationDeclaration
                   firmwareInformation:(NSData * _Nonnull)firmwareInformation
                 deviceAttestationCert:(NSData * _Nonnull)deviceAttestationCert
    productAttestationIntermediateCert:(NSData * _Nonnull)productAttestationIntermediateCert
    deviceAttestationCertPrivateKeyRef:(SecKeyRef _Nonnull)deviceAttestationCertPrivateKeyRef;

- (NSData * _Nonnull)getCertificationDeclaration;

- (NSData * _Nonnull)getFirmwareInformation;

- (NSData * _Nonnull)getDeviceAttestationCert;

- (NSData * _Nonnull)getProductAttestationIntermediateCert;

- (SecKeyRef _Nonnull)getDeviceAttestationCertPrivateKeyRef;

@end

#endif /* DeviceAttestationCredentialsHolder_h */
