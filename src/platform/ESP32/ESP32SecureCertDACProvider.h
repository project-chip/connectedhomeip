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
     * @brief API to set the Certification Declaration (CD).
     *
     * The GetCertificationDeclaration() API implementation reads the CD from the NVS namespace `chip-factory`.
     * Use this API to set the CD if it is stored at a different location, e.g., embedded in the firmware.
     * Subsequent reads after calling this API will return the set CD.
     *
     * @param[in] cd ByteSpan containing the Certification Declaration.
     *               The underlying data must remain allocated throughout the lifetime of the device,
     *               as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
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
