/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationDelegate.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceAttestationDeviceInfo ()

- (instancetype)initWithDACCertificate:(MTRCertificateDERBytes)dacCertificate
                     dacPAICertificate:(MTRCertificateDERBytes)dacPAICertificate
                certificateDeclaration:(NSData *)certificateDeclaration
              basicInformationVendorID:(NSNumber *)basicInformationVendorID
             basicInformationProductID:(NSNumber *)basicInformationProductID;

@end

NS_ASSUME_NONNULL_END
