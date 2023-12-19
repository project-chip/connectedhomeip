/*
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
#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>

namespace chip {
namespace DeviceLayer {

class ESP32SecureCertDACProvider : public Credentials::DeviceAttestationCredentialsProvider
{
public:
    ESP32SecureCertDACProvider() : Credentials::DeviceAttestationCredentialsProvider() {}
    ~ESP32SecureCertDACProvider() {}
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

#ifdef CONFIG_ENABLE_SET_CERT_DECLARATION_API
    /**
     * API to set the CD.
     *
     * GetCertificationDeclaration() API impl reads the CD from the NVS namespace `chip-factory`.
     * Use this API to set the CD if it is stored at a different place, eg: embedded in the firmware.
     * Subsequent reads after calling this API will return the set CD.
     */
    CHIP_ERROR SetCertificationDeclaration(const ByteSpan & cd)
    {
        VerifyOrReturnError(!cd.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mCD = cd;
        return CHIP_NO_ERROR;
    }
#endif // CONFIG_ENABLE_SET_CERT_DECLARATION_API

private:
#ifdef CONFIG_ENABLE_SET_CERT_DECLARATION_API
    ByteSpan mCD;
#endif // CONFIG_ENABLE_SET_CERT_DECLARATION_API
};
} // namespace DeviceLayer
} // namespace chip
