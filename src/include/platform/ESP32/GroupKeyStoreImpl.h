/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
/**
 *    @file
 *          Provides an implementation of the Chip Group Key Store interface
 *          for the ESP32 platform.
 */

#include <core/CHIPKeyIds.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/internal/ChipDeviceLayerInternal.h>

#include "nvs.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * An implementation of the Chip GroupKeyStoreBase API for the ESP32.
 */
class GroupKeyStoreImpl final : public ::chip::Profiles::Security::AppKeys::GroupKeyStoreBase, private ESP32Config
{
    using ChipGroupKey = ::chip::Profiles::Security::AppKeys::ChipGroupKey;

public:
    enum
    {
        kMaxGroupKeys = CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS + // Maximum number of Epoch keys
            CHIP_CONFIG_MAX_APPLICATION_GROUPS +                 // Maximum number of Application Group Master keys
            1 +                                                  // Maximum number of Root keys (1 for Service root key)
            1                                                    // Fabric secret
    };

    CHIP_ERROR Init();

    CHIP_ERROR RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key) override;
    CHIP_ERROR StoreGroupKey(const ChipGroupKey & key) override;
    CHIP_ERROR DeleteGroupKey(uint32_t keyId) override;
    CHIP_ERROR DeleteGroupKeysOfAType(uint32_t keyType) override;
    CHIP_ERROR EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount) override;
    CHIP_ERROR Clear(void) override;
    CHIP_ERROR RetrieveLastUsedEpochKeyId(void) override;
    CHIP_ERROR StoreLastUsedEpochKeyId(void) override;

private:
    uint32_t mKeyIndex[kMaxGroupKeys];
    uint8_t mNumKeys;

    CHIP_ERROR AddKeyToIndex(uint32_t keyId, bool & indexUpdated);
    CHIP_ERROR WriteKeyIndex(nvs_handle handle);
    CHIP_ERROR DeleteKeyOrKeys(uint32_t targetKeyId, uint32_t targetKeyType);

    static CHIP_ERROR FormKeyName(uint32_t keyId, char * buf, size_t bufSize);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
