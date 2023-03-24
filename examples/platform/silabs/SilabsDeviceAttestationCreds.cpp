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
#include "SilabsDeviceAttestationCreds.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/silabs/SilabsConfig.h>
#include <psa/crypto.h>

#include "silabs_creds.h"

using namespace chip::DeviceLayer::Internal;

extern uint8_t linker_nvm_end[];
static uint8_t * _credentials_address = (uint8_t *) linker_nvm_end;

namespace chip {
namespace Credentials {
namespace Silabs {

namespace {

class DeviceAttestationCredsSilabs : public DeviceAttestationCredentialsProvider
{
    // Miss-aligned certificates is a common error, and printing the first few bytes is
    // useful to verify proper alignment. Eight bytes is enough for this purpose.
    static constexpr size_t kDebugLength = 8;

public:
    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_span) override
    {
        uint32_t offset = SILABS_CREDENTIALS_CD_OFFSET;
        uint32_t size   = SILABS_CREDENTIALS_CD_SIZE;

        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_CD_Offset) &&
            SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_CD_Size))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_CD_Offset, offset));
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_CD_Size, size));
        }

        uint8_t * address = _credentials_address + offset;
        ByteSpan cd_span(address, size);
        ChipLogProgress(DeviceLayer, "GetCertificationDeclaration, addr:%p, size:%lu", address, size);
        ChipLogByteSpan(DeviceLayer, ByteSpan(cd_span.data(), kDebugLength > cd_span.size() ? cd_span.size() : kDebugLength));
        return CopySpanToMutableSpan(cd_span, out_span);
    }

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override
    {
        // TODO: We need a real example FirmwareInformation to be populated.
        out_firmware_info_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_span) override
    {
        uint32_t offset = SILABS_CREDENTIALS_DAC_OFFSET;
        uint32_t size   = SILABS_CREDENTIALS_DAC_SIZE;

        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_DAC_Offset) &&
            SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_DAC_Size))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_DAC_Offset, offset));
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_DAC_Size, size));
        }

        uint8_t * address = _credentials_address + offset;
        ByteSpan cert_span(address, size);
        ChipLogProgress(DeviceLayer, "GetDeviceAttestationCert, addr:%p, size:%lu", address, size);
        ChipLogByteSpan(DeviceLayer, ByteSpan(cert_span.data(), kDebugLength > cert_span.size() ? cert_span.size() : kDebugLength));
        return CopySpanToMutableSpan(cert_span, out_span);
    }

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_span) override
    {
        uint32_t offset = SILABS_CREDENTIALS_PAI_OFFSET;
        uint32_t size   = SILABS_CREDENTIALS_PAI_SIZE;

        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_PAI_Offset) &&
            SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_PAI_Size))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_PAI_Offset, offset));
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_PAI_Size, size));
        }

        uint8_t * address = _credentials_address + offset;
        ByteSpan cert_span(address, size);
        ChipLogProgress(DeviceLayer, "GetProductAttestationIntermediateCert, addr:%p, size:%lu", address, size);
        ChipLogByteSpan(DeviceLayer, ByteSpan(cert_span.data(), kDebugLength > cert_span.size() ? cert_span.size() : kDebugLength));
        return CopySpanToMutableSpan(cert_span, out_span);
    }

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_span) override
    {
        uint32_t key_id       = SILABS_CREDENTIALS_DAC_KEY_ID;
        uint8_t signature[64] = { 0 };
        size_t signature_size = sizeof(signature);

        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key_id));
        }

        ChipLogProgress(DeviceLayer, "SignWithDeviceAttestationKey, key:%lu", key_id);

        psa_status_t err =
            psa_sign_message(static_cast<psa_key_id_t>(key_id), PSA_ALG_ECDSA(PSA_ALG_SHA_256), message_to_sign.data(),
                             message_to_sign.size(), signature, signature_size, &signature_size);
        VerifyOrReturnError(!err, CHIP_ERROR_INTERNAL);

        return CopySpanToMutableSpan(ByteSpan(signature, signature_size), out_span);
    }
};

} // namespace

DeviceAttestationCredentialsProvider * GetSilabsDacProvider()
{
    static DeviceAttestationCredsSilabs dac_provider;
    return &dac_provider;
}

} // namespace Silabs
} // namespace Credentials
} // namespace chip
