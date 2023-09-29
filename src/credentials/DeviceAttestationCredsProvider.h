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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

class DeviceAttestationCredentialsProvider
{
public:
    DeviceAttestationCredentialsProvider()          = default;
    virtual ~DeviceAttestationCredentialsProvider() = default;

    // Not copyable
    DeviceAttestationCredentialsProvider(const DeviceAttestationCredentialsProvider &)             = delete;
    DeviceAttestationCredentialsProvider & operator=(const DeviceAttestationCredentialsProvider &) = delete;

    /**
     * @brief Get the Certification Declaration body. Updates `out_cd_buffer`'s size on success
     *        to match the data size. If no Certification Declaration is available, sets
     *        `out_cd_buffer` to empty.
     *
     * @param[in,out] out_cd_buffer Buffer to receive the Certification Declaration body.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_cd_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) = 0;

    /**
     * @brief Get the Firmware Information body. Updates `out_firmware_info_buffer`'s size
     *        on success to match the data size. If no Firmware Information is available,
     *        sets `out_firmware_info_buffer` to empty.
     *
     * @param[in,out] out_firmware_info_buffer Buffer to receive the Firmware Information body.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_firmware_info_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if access fails.
     */
    virtual CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) = 0;

    /**
     * @brief Get the Device Attestation Certificate in DER format. Updates `out_dac_buffer`'s
     *        size on success to match the data size. If no Device Attestation Certificate
     *        is available, sets `out_dac_buffer` to empty.
     *
     * @param[in,out] out_dac_buffer Buffer to receive the Device Attestation Certificate.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_dac_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if
     *          access fails.
     */
    virtual CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) = 0;

    /**
     * @brief Get the PAI Certificate in DER format. Updates `out_pai_buffer`'s
     *        size on success to match the data size. If no PAI certificate
     *        is available, sets `out_pai_buffer` to empty.
     *
     * @param[in,out] out_pai_buffer Buffer to receive the PAI certificate.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_pai_buffer`
     *          is too small, or another CHIP_ERROR from the underlying implementation if
     *          access fails.
     */
    virtual CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) = 0;

    /**
     * @brief Signs a message using the device attestation private key
     *
     * @param[in] message_to_sign The message to sign using the attestation private key.
     * @param[in,out] out_signature_buffer Buffer to receive the signature in raw <r,s> format.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if `out_signature_buffer` is too small,
     *          or another CHIP_ERROR from the underlying implementation if signature fails.
     */
    virtual CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) = 0;
};

/**
 * Instance getter for the global DeviceAttestationCredentialsProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation credentials provider. Assume never null.
 */
DeviceAttestationCredentialsProvider * GetDeviceAttestationCredentialsProvider();

/**
 * Instance setter for the global DeviceAttestationCredentialsProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the DeviceAttestationCredentialsProvider to start returning with the getter
 */
void SetDeviceAttestationCredentialsProvider(DeviceAttestationCredentialsProvider * provider);

/**
 * Check if Instance is prepared
 */
bool IsDeviceAttestationCredentialsProviderSet();

} // namespace Credentials
} // namespace chip
