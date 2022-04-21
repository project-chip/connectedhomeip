/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <platform/CommissionableDataProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief This class provides Commissionable data and Device Attestation Credentials.
 */

class ESP32FactoryDataProvider : public CommissionableDataProvider, public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;

    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;

    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;

    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;

    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // ===== Members functions that implement the DeviceAttestationCredentialsProvider
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer) override;
};

} // namespace DeviceLayer
} // namespace chip
