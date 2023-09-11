/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace DeviceLayer {

class FactoryDataProvider : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    // ===== Members functions that implement the DeviceAttestationCredentialsProvider
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;

private:
    CHIP_ERROR GetDeviceAttestationCertKey(MutableByteSpan & out_key_buffer);
    CHIP_ERROR LoadKeypairFromDer(const ByteSpan & der_buffer, Crypto::P256Keypair & keypair);
};

} // namespace DeviceLayer
} // namespace chip
