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

#include <lib/support/CodeUtils.h>

namespace chip {
namespace Credentials {

namespace {

constexpr auto kLegacyDeviceAttestationProfiles =
    BitMask<DeviceAttestationCertProfileBitmap>(DeviceAttestationCertProfileBitmap::kSupportsEcdsaMatterLegacy);

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

CHIP_ERROR DeviceAttestationCredentialsProvider::GetDeviceAttestationCertForProfile(DeviceAttestationCertProfile profile,
                                                                                    MutableByteSpan & out_dac_buffer)
{
    if (profile != DeviceAttestationCertProfile::kEcdsaMatterLegacy)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    return GetDeviceAttestationCert(out_dac_buffer);
}

CHIP_ERROR
DeviceAttestationCredentialsProvider::GetProductAttestationIntermediateCertForProfile(DeviceAttestationCertProfile profile,
                                                                                      MutableByteSpan & out_pai_buffer)
{
    if (profile != DeviceAttestationCertProfile::kEcdsaMatterLegacy)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    return GetProductAttestationIntermediateCert(out_pai_buffer);
}

DeviceAttestationProfileSupport DeviceAttestationCredentialsProvider::GetDeviceAttestationProfileSupport() const
{
    return {
        .paaSupportedProfiles = kLegacyDeviceAttestationProfiles,
        .paiSupportedProfiles = kLegacyDeviceAttestationProfiles,
        .dacSupportedProfiles = kLegacyDeviceAttestationProfiles,
    };
}

CHIP_ERROR DeviceAttestationCredentialsProvider::GetDeviceAttestationDocumentSegment(DeviceAttestationDocumentType documentType,
                                                                                     DeviceAttestationCertProfile profile,
                                                                                     size_t offset,
                                                                                     MutableByteSpan & out_document_buffer,
                                                                                     size_t & out_document_size)
{
    VerifyOrReturnError(profile == DeviceAttestationCertProfile::kEcdsaMatterLegacy, CHIP_ERROR_NOT_IMPLEMENTED);
    VerifyOrReturnError(offset == 0, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err;
    switch (documentType)
    {
    case DeviceAttestationDocumentType::kDACCertificate:
        err = GetDeviceAttestationCert(out_document_buffer);
        break;
    case DeviceAttestationDocumentType::kPAICertificate:
        err = GetProductAttestationIntermediateCert(out_document_buffer);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(err);
    out_document_size = out_document_buffer.size();
    return CHIP_NO_ERROR;
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
