/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
};
} // namespace DeviceLayer
} // namespace chip
