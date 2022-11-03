/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "EFR32DeviceAttestationCreds.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include "efr32_creds.h"
#include "psa/crypto.h"

extern uint8_t __attestation_credentials_base[];

namespace chip {
namespace Credentials {
namespace EFR32 {

namespace {

class DeviceAttestationCredsEFR32 : public DeviceAttestationCredentialsProvider
{

public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_buffer) override
    {
        ByteSpan cd_span(((uint8_t *) __attestation_credentials_base) + EFR32_CREDENTIALS_CD_OFFSET, EFR32_CREDENTIALS_CD_SIZE);
        return CopySpanToMutableSpan(cd_span, out_buffer);
    }

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override
    {
        // TODO: We need a real example FirmwareInformation to be populated.
        out_firmware_info_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_buffer) override
    {
        ByteSpan cert_span(((uint8_t *) __attestation_credentials_base) + EFR32_CREDENTIALS_DAC_OFFSET, EFR32_CREDENTIALS_DAC_SIZE);
        return CopySpanToMutableSpan(cert_span, out_buffer);
    }

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override
    {
        ByteSpan cert_span(((uint8_t *) __attestation_credentials_base) + EFR32_CREDENTIALS_PAI_OFFSET, EFR32_CREDENTIALS_PAI_SIZE);
        return CopySpanToMutableSpan(cert_span, out_pai_buffer);
    }

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_buffer) override
    {
        psa_key_id_t key_id   = EFR32_CREDENTIALS_DAC_KEY_ID;
        uint8_t signature[64] = { 0 };
        size_t signature_size = sizeof(signature);

        psa_status_t err = psa_sign_message(key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), message_to_sign.data(), message_to_sign.size(),
                                            signature, signature_size, &signature_size);
        VerifyOrReturnError(!err, CHIP_ERROR_INTERNAL);

        return CopySpanToMutableSpan(ByteSpan(signature, signature_size), out_buffer);
    }
};

} // namespace

DeviceAttestationCredentialsProvider * GetEFR32DacProvider()
{
    static DeviceAttestationCredsEFR32 dac_provider;
    return &dac_provider;
}

} // namespace EFR32
} // namespace Credentials
} // namespace chip
