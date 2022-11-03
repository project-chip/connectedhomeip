/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <MTRDeviceAttestationDelegate.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceAttestationDeviceInfo ()
- (instancetype)initWithDACCertificate:(NSData *)dacCertificate
                     dacPAICertificate:(NSData *)dacPAICertificate
                certificateDeclaration:(NSData *)certificateDeclaration;
@end

NS_ASSUME_NONNULL_END
