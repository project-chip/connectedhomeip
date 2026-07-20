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

namespace chip {
namespace DeviceLayer {

struct AllDevicesExampleDACProvider::FileProviderContext
{
    Credentials::Examples::TestHarnessDACProvider provider;
};

AllDevicesExampleDACProvider::AllDevicesExampleDACProvider() = default;
AllDevicesExampleDACProvider::~AllDevicesExampleDACProvider() = default;

CHIP_ERROR AllDevicesExampleDACProvider::Init(const std::optional<std::string> & filePath)
{
    if (filePath.has_value() && !filePath.value().empty())
    {
        mFileContext = std::make_unique<FileProviderContext>();
        mFileContext->provider.Init(filePath.value().c_str());
        mDelegate = &mFileContext->provider;
    }
    else
    {
        mFileContext.reset();
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

CHIP_ERROR AllDevicesExampleDACProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                                     MutableByteSpan & out_signature_buffer)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->SignWithDeviceAttestationKey(message_to_sign, out_signature_buffer);
}

} // namespace DeviceLayer
} // namespace chip
