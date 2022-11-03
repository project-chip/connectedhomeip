/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <MTRDeviceAttestationDelegate_Internal.h>

@implementation MTRDeviceAttestationDeviceInfo
- (instancetype)initWithDACCertificate:(NSData *)dacCertificate
                     dacPAICertificate:(NSData *)dacPAICertificate
                certificateDeclaration:(NSData *)certificateDeclaration
{
    if (self = [super init]) {
        _dacCertificate = [dacCertificate copy];
        _dacPAICertificate = [dacPAICertificate copy];
        _certificateDeclaration = [certificateDeclaration copy];
    }
    return self;
}
@end
