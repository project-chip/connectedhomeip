/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MCDataSource.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/logging/CHIPLogging.h>

#import <Foundation/Foundation.h>
#include <Security/Security.h>

#ifndef MCDeviceAttestationCredentialsProvider_h
#define MCDeviceAttestationCredentialsProvider_h

namespace matter {
namespace casting {
namespace support {

class MCDeviceAttestationCredentialsProvider : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    CHIP_ERROR Initialize(id<MCDataSource> dataSource);

    CHIP_ERROR GetCertificationDeclaration(chip::MutableByteSpan & outCertificationDeclaration) override;
    CHIP_ERROR GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation) override;
    CHIP_ERROR GetDeviceAttestationCert(chip::MutableByteSpan & outDeviceAttestationCert) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(chip::MutableByteSpan & outProductAttestationIntermediateCert) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const chip::ByteSpan & messageToSign,
                                            chip::MutableByteSpan & outSignatureBuffer) override;

private:
    id<MCDataSource> mDataSource = nullptr;
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MCDeviceAttestationCredentialsProvider_h */
