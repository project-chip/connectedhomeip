/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
