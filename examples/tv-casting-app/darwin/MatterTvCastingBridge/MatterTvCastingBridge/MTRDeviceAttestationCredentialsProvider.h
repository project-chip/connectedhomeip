/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDataSource.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/logging/CHIPLogging.h>

#import <Foundation/Foundation.h>
#include <Security/Security.h>

#ifndef MTRDeviceAttestationCredentialsProvider_h
#define MTRDeviceAttestationCredentialsProvider_h

namespace matter {
namespace casting {
namespace support {

class MTRDeviceAttestationCredentialsProvider : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    CHIP_ERROR Initialize(id<MTRDataSource> dataSource);

    CHIP_ERROR GetCertificationDeclaration(chip::MutableByteSpan & outCertificationDeclaration) override;
    CHIP_ERROR GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation) override;
    CHIP_ERROR GetDeviceAttestationCert(chip::MutableByteSpan & outDeviceAttestationCert) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(chip::MutableByteSpan & outProductAttestationIntermediateCert) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const chip::ByteSpan & messageToSign,
                                            chip::MutableByteSpan & outSignatureBuffer) override;

private:
    id<MTRDataSource> mDataSource          = nullptr;
    MTRDeviceAttestationCredentials * mDac = nullptr;
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MTRDeviceAttestationCredentialsProvider_h */
