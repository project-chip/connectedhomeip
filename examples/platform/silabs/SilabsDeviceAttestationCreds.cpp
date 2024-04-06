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
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/SilabsConfig.h>
#include <psa/crypto.h>

#include "silabs_creds.h"

using namespace chip::DeviceLayer::Internal;

using chip::DeviceLayer::Internal::SilabsConfig;

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
        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_Base_Addr))
        {
            // Provisioned CD
            return GetFile("GetCertificationDeclaration", SilabsConfig::kConfigKey_Creds_CD_Offset, SILABS_CREDENTIALS_CD_OFFSET,
                           SilabsConfig::kConfigKey_Creds_CD_Size, SILABS_CREDENTIALS_CD_SIZE, out_span);
        }
        else
        {
            // Example CD
            return Examples::GetExampleDACProvider()->GetCertificationDeclaration(out_span);
        }
    }

    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override
    {
        // TODO: We need a real example FirmwareInformation to be populated.
        out_firmware_info_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_span) override
    {
        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_Base_Addr))
        {
            // Provisioned DAC
            return GetFile("GetDeviceAttestationCert", SilabsConfig::kConfigKey_Creds_DAC_Offset, SILABS_CREDENTIALS_DAC_OFFSET,
                           SilabsConfig::kConfigKey_Creds_DAC_Size, SILABS_CREDENTIALS_DAC_SIZE, out_span);
        }
        else
        {
            // Example DAC
            return Examples::GetExampleDACProvider()->GetDeviceAttestationCert(out_span);
        }
    }

    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_span) override
    {
        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_Base_Addr))
        {
            // Provisioned PAI
            return GetFile("GetProductAttestationIntermediateCert", SilabsConfig::kConfigKey_Creds_PAI_Offset,
                           SILABS_CREDENTIALS_PAI_OFFSET, SilabsConfig::kConfigKey_Creds_PAI_Size, SILABS_CREDENTIALS_PAI_SIZE,
                           out_span);
        }
        else
        {
            // Example PAI
            return Examples::GetExampleDACProvider()->GetProductAttestationIntermediateCert(out_span);
        }
    }

    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_span) override
    {
        if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_Creds_KeyId))
        {
            // Provisioned DAC key
#ifdef SLI_SI91X_MCU_INTERFACE
            return CHIP_ERROR_NOT_IMPLEMENTED;
#else
            uint32_t key_id       = SILABS_CREDENTIALS_DAC_KEY_ID;
            uint8_t signature[64] = { 0 };
            size_t signature_size = sizeof(signature);

            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_KeyId, key_id));

            ChipLogProgress(DeviceLayer, "SignWithDeviceAttestationKey, key:%lu", key_id);

            psa_status_t err =
                psa_sign_message(static_cast<psa_key_id_t>(key_id), PSA_ALG_ECDSA(PSA_ALG_SHA_256), message_to_sign.data(),
                                 message_to_sign.size(), signature, signature_size, &signature_size);
            VerifyOrReturnError(!err, CHIP_ERROR_INTERNAL);

            return CopySpanToMutableSpan(ByteSpan(signature, signature_size), out_span);
#endif
        }
        else
        {
            // Example DAC key
            return Examples::GetExampleDACProvider()->SignWithDeviceAttestationKey(message_to_sign, out_span);
        }
    }

private:
    CHIP_ERROR GetFile(const char * description, uint32_t offset_key, uint32_t offset_default, uint32_t size_key,
                       uint32_t size_default, MutableByteSpan & out_span)
    {
        uint32_t base_addr = 0;
        uint8_t * address  = nullptr;
        uint32_t offset    = offset_default;
        uint32_t size      = size_default;

        ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_Creds_Base_Addr, base_addr));
        address = (uint8_t *) (base_addr + offset);

        // Offset
        if (SilabsConfig::ConfigValueExists(offset_key))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(offset_key, offset));
        }

        // Size
        if (SilabsConfig::ConfigValueExists(size_key))
        {
            ReturnErrorOnFailure(SilabsConfig::ReadConfigValue(size_key, size));
        }

        ByteSpan span(address, size);
        ChipLogProgress(DeviceLayer, "%s, addr:%p, size:%lu", description, address, size);
        ChipLogByteSpan(DeviceLayer, ByteSpan(span.data(), kDebugLength > span.size() ? span.size() : kDebugLength));
        return CopySpanToMutableSpan(span, out_span);
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

namespace DeviceLayer {
namespace Silabs {
namespace {

void MigrateUint32(uint32_t old_key, uint32_t new_key)
{
    uint32_t value = 0;
    if (SilabsConfig::ConfigValueExists(old_key) && (CHIP_NO_ERROR == SilabsConfig::ReadConfigValue(old_key, value)))
    {
        SilabsConfig::WriteConfigValue(new_key, value);
    }
}

} // namespace

void MigrateDacProvider(void)
{
    constexpr uint32_t kOldKey_Creds_KeyId      = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x21);
    constexpr uint32_t kOldKey_Creds_Base_Addr  = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x22);
    constexpr uint32_t kOldKey_Creds_DAC_Offset = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x23);
    constexpr uint32_t kOldKey_Creds_DAC_Size   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x24);
    constexpr uint32_t kOldKey_Creds_PAI_Offset = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x25);
    constexpr uint32_t kOldKey_Creds_PAI_Size   = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x26);
    constexpr uint32_t kOldKey_Creds_CD_Offset  = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x27);
    constexpr uint32_t kOldKey_Creds_CD_Size    = SilabsConfigKey(SilabsConfig::kMatterConfig_KeyBase, 0x28);

    MigrateUint32(kOldKey_Creds_KeyId, SilabsConfig::kConfigKey_Creds_KeyId);
    MigrateUint32(kOldKey_Creds_Base_Addr, SilabsConfig::kConfigKey_Creds_Base_Addr);
    MigrateUint32(kOldKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Offset);
    MigrateUint32(kOldKey_Creds_DAC_Size, SilabsConfig::kConfigKey_Creds_DAC_Size);
    MigrateUint32(kOldKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Offset);
    MigrateUint32(kOldKey_Creds_PAI_Size, SilabsConfig::kConfigKey_Creds_PAI_Size);
    MigrateUint32(kOldKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Offset);
    MigrateUint32(kOldKey_Creds_CD_Size, SilabsConfig::kConfigKey_Creds_CD_Size);
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
