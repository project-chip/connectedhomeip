/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "DeviceAttestationCredsProvider.h"

namespace chip {
namespace Credentials {

namespace {

// Version to have a default placeholder so the getter never
// returns `nullptr` by default.
class UnimplementedDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override
    {
        (void) out_cd_buffer;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override
    {
        (void) out_firmware_info_buffer;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override
    {
        (void) out_dac_buffer;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override
    {
        (void) out_pai_buffer;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override
    {
        (void) message_to_sign;
        (void) out_signature_buffer;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

// Default to avoid nullptr on getter and cleanly handle new products/clients before
// they provide their own.
UnimplementedDACProvider gDefaultDACProvider;

DeviceAttestationCredentialsProvider * gDacProvider = &gDefaultDACProvider;

} // namespace

DeviceAttestationCredentialsProvider * GetDeviceAttestationCredentialsProvider()
{
    return gDacProvider;
}

void SetDeviceAttestationCredentialsProvider(DeviceAttestationCredentialsProvider * provider)
{
    if (provider == nullptr)
    {
        return;
    }

    gDacProvider = provider;
}

bool IsDeviceAttestationCredentialsProviderSet()
{
    return (gDacProvider != &gDefaultDACProvider);
}

} // namespace Credentials
} // namespace chip
