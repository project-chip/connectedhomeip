/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#ifndef DeviceAttestationCredentialsHolder_h
#define DeviceAttestationCredentialsHolder_h

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
