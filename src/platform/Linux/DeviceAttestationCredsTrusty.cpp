/*
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright 2023 NXP
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
#include "DeviceAttestationCredsTrusty.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <trusty_matter.h>

namespace chip {
namespace Credentials {
namespace Trusty {

CHIP_ERROR TrustyDACProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    size_t out_size = 0;
    int rc;

    rc = trusty_matter.ExportDACCert(out_dac_buffer.data(), out_dac_buffer.size(), out_size);
    if (rc == 0)
    {
        out_dac_buffer.reduce_size(out_size);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CERT_LOAD_FAILED;
}

CHIP_ERROR TrustyDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    size_t out_size = 0;
    int rc;

    rc = trusty_matter.ExportPAICert(out_pai_buffer.data(), out_pai_buffer.size(), out_size);
    if (rc == 0)
    {
        out_pai_buffer.reduce_size(out_size);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CERT_LOAD_FAILED;
}

CHIP_ERROR TrustyDACProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    size_t out_size = 0;
    int rc;

    rc = trusty_matter.ExportCDCert(out_cd_buffer.data(), out_cd_buffer.size(), out_size);
    if (rc == 0)
    {
        out_cd_buffer.reduce_size(out_size);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CERT_LOAD_FAILED;
}

CHIP_ERROR TrustyDACProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TrustyDACProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer)
{
    int rc          = 0;
    size_t out_size = 0;

    VerifyOrReturnError(IsSpanUsable(out_signature_buffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(message_to_sign), CHIP_ERROR_INVALID_ARGUMENT);

    rc = trusty_matter.SignWithDACKey(message_to_sign.data(), message_to_sign.size(), out_signature_buffer.data(),
                                      out_signature_buffer.size(), out_size);
    if (rc == 0)
    {
        out_signature_buffer.reduce_size(out_size);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CERT_LOAD_FAILED;
}

TrustyDACProvider & TrustyDACProvider::GetTrustyDACProvider()
{
    static TrustyDACProvider trusty_dac_provider;

    return trusty_dac_provider;
}

} // namespace Trusty
} // namespace Credentials
} // namespace chip
