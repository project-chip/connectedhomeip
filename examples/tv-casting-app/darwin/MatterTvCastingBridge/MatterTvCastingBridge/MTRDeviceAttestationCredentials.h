/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#ifndef MTRDeviceAttestationCredentials_h
#define MTRDeviceAttestationCredentials_h

@interface MTRDeviceAttestationCredentials : NSObject

@property (nonatomic, strong, readonly) NSData * _Nonnull certificationDeclaration;

@property (nonatomic, strong, readonly) NSData * _Nonnull firmwareInformation;

@property (nonatomic, strong, readonly) NSData * _Nonnull deviceAttestationCert;

@property (nonatomic, strong, readonly) NSData * _Nonnull productAttestationIntermediateCert;

- (MTRDeviceAttestationCredentials * _Nonnull)initWithCertificationDeclaration:(NSData * _Nonnull)certificationDeclaration
                                                           firmwareInformation:(NSData * _Nonnull)firmwareInformation
                                                         deviceAttestationCert:(NSData * _Nonnull)deviceAttestationCert
                                            productAttestationIntermediateCert:
                                                (NSData * _Nonnull)productAttestationIntermediateCert;

@end

#endif /* MTRDeviceAttestationCredentials_h */
