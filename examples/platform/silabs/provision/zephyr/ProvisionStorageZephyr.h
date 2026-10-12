/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/Zephyr/CHIPDevicePlatformConfig.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <psa/crypto.h>
#include <zephyr/settings/settings.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
namespace ZephyrStorage {

#define DEFINE_CONFIG_KEY(name, suffix)                                                                                            \
    inline constexpr char name[] = CHIP_DEVICE_CONFIG_SETTINGS_KEY "-sl-fct/" suffix;                                              \
    static_assert(sizeof(name) <= SETTINGS_MAX_NAME_LEN, #name " is too long")

DEFINE_CONFIG_KEY(kConfigKeyVendorId, "vendor-id");
DEFINE_CONFIG_KEY(kConfigKeyProductId, "product-id");
DEFINE_CONFIG_KEY(kConfigKeyVendorName, "vendor-name");
DEFINE_CONFIG_KEY(kConfigKeyProductName, "product-name");
DEFINE_CONFIG_KEY(kConfigKeyProductLabel, "product-label");
DEFINE_CONFIG_KEY(kConfigKeyProductUrl, "product-url");
DEFINE_CONFIG_KEY(kConfigKeyPartNumber, "part-number");
DEFINE_CONFIG_KEY(kConfigKeyHardwareVersionString, "hardware-ver-str");
DEFINE_CONFIG_KEY(kConfigKeySetupPayload, "setup-payload");
DEFINE_CONFIG_KEY(kConfigKeyProvisionRequest, "provision-req");
DEFINE_CONFIG_KEY(kConfigKeyProvisionVersion, "provision-ver");
DEFINE_CONFIG_KEY(kConfigKeyDacKeyId, "dac-key-id");

#undef DEFINE_CONFIG_KEY

inline constexpr psa_key_id_t kDacPsaKeyId = 2;

// Stored id when one was provisioned, otherwise the default id.
inline uint32_t GetDacPsaKeyId()
{
    uint32_t keyId = 0;
    if (chip::DeviceLayer::Internal::ZephyrConfig::ReadConfigValue(kConfigKeyDacKeyId, keyId) == CHIP_NO_ERROR && keyId != 0)
    {
        return keyId;
    }
    return kDacPsaKeyId;
}

inline bool DacPsaKeyExists()
{
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    const psa_status_t status       = psa_get_key_attributes(static_cast<psa_key_id_t>(GetDacPsaKeyId()), &attributes);
    psa_reset_key_attributes(&attributes);
    return status == PSA_SUCCESS;
}

} // namespace ZephyrStorage
} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
