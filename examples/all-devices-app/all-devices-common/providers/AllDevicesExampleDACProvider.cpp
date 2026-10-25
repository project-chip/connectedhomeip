/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "AllDevicesExampleDACProvider.h"

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CodeUtils.h>

#include <fstream>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR AllDevicesExampleDACProvider::Init(const std::optional<std::string> & filePath)
{
    if (filePath.has_value() && !filePath.value().empty())
    {
        std::ifstream jsonFile(filePath.value().c_str(), std::ifstream::binary);
        VerifyOrReturnError(jsonFile.is_open(), CHIP_ERROR_INVALID_ARGUMENT);

        auto fileProvider = std::make_unique<Credentials::Examples::TestHarnessDACProvider>();
        fileProvider->Init(filePath.value().c_str());
        mDynamicProvider = std::move(fileProvider);
        mDelegate        = mDynamicProvider.get();
    }
    else
    {
        mDynamicProvider.reset();
        mDelegate = Credentials::Examples::GetExampleDACProvider();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AllDevicesExampleDACProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetCertificationDeclaration(out_cd_buffer);
}

CHIP_ERROR AllDevicesExampleDACProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetFirmwareInformation(out_firmware_info_buffer);
}

CHIP_ERROR AllDevicesExampleDACProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetDeviceAttestationCert(out_dac_buffer);
}

CHIP_ERROR AllDevicesExampleDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductAttestationIntermediateCert(out_pai_buffer);
}

CHIP_ERROR AllDevicesExampleDACProvider::GetDeviceAttestationCertForProfile(Credentials::DeviceAttestationCertProfile profile,
                                                                            MutableByteSpan & out_dac_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetDeviceAttestationCertForProfile(profile, out_dac_buffer);
}

CHIP_ERROR
AllDevicesExampleDACProvider::GetProductAttestationIntermediateCertForProfile(Credentials::DeviceAttestationCertProfile profile,
                                                                              MutableByteSpan & out_pai_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductAttestationIntermediateCertForProfile(profile, out_pai_buffer);
}

Credentials::DeviceAttestationProfileSupport AllDevicesExampleDACProvider::GetDeviceAttestationProfileSupport() const
{
    return mDelegate != nullptr ? mDelegate->GetDeviceAttestationProfileSupport()
                                : DeviceAttestationCredentialsProvider::GetDeviceAttestationProfileSupport();
}

Credentials::DeviceAttestationCertProfile AllDevicesExampleDACProvider::GetPreferredDeviceAttestationChainProfile() const
{
    return mDelegate != nullptr ? mDelegate->GetPreferredDeviceAttestationChainProfile()
                                : DeviceAttestationCredentialsProvider::GetPreferredDeviceAttestationChainProfile();
}

CHIP_ERROR
AllDevicesExampleDACProvider::GetDeviceAttestationDocumentSegment(Credentials::DeviceAttestationDocumentType documentType,
                                                                  Credentials::DeviceAttestationCertProfile profile, size_t offset,
                                                                  MutableByteSpan & out_document_buffer, size_t & out_document_size)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetDeviceAttestationDocumentSegment(documentType, profile, offset, out_document_buffer, out_document_size);
}

CHIP_ERROR AllDevicesExampleDACProvider::GetDeviceAttestationDocumentSegmentForChain(
    Credentials::DeviceAttestationDocumentType documentType, const Credentials::DeviceAttestationChainProfile & chain,
    size_t offset, MutableByteSpan & out_document_buffer, size_t & out_document_size)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetDeviceAttestationDocumentSegmentForChain(documentType, chain, offset, out_document_buffer,
                                                                  out_document_size);
}

CHIP_ERROR AllDevicesExampleDACProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                                      MutableByteSpan & out_signature_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->SignWithDeviceAttestationKey(message_to_sign, out_signature_buffer);
}

} // namespace DeviceLayer
} // namespace chip
