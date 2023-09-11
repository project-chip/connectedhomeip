/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceAttestationDelegate_Internal.h"

#import "MTRConversion.h"

#include <crypto/CHIPCryptoPAL.h>

using namespace chip::Crypto;

@implementation MTRDeviceAttestationDeviceInfo

- (instancetype)initWithDACCertificate:(MTRCertificateDERBytes)dacCertificate
                     dacPAICertificate:(MTRCertificateDERBytes)dacPAICertificate
                certificateDeclaration:(NSData *)certificateDeclaration
              basicInformationVendorID:(NSNumber *)basicInformationVendorID
             basicInformationProductID:(NSNumber *)basicInformationProductID
{
    if (self = [super init]) {
        _dacCertificate = [dacCertificate copy];
        _dacPAICertificate = [dacPAICertificate copy];
        _certificateDeclaration = [certificateDeclaration copy];
        _basicInformationVendorID = [basicInformationVendorID copy];
        _basicInformationProductID = [basicInformationProductID copy];

        struct AttestationCertVidPid dacVidPid;
        if (ExtractVIDPIDFromX509Cert(AsByteSpan(_dacCertificate), dacVidPid) == CHIP_NO_ERROR) {
            _vendorID = AsNumber(dacVidPid.mVendorId);
            _productID = AsNumber(dacVidPid.mProductId);
        }
    }
    return self;
}

@end
