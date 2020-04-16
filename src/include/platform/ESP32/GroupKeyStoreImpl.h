/*
 *
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
 *          Provides an implementation of the Weave Group Key Store interface
 *          for the ESP32 platform.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>
#include <Weave/DeviceLayer/ESP32/ESP32Config.h>

#include "nvs.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * An implementation of the Weave GroupKeyStoreBase API for the ESP32.
 */
class GroupKeyStoreImpl final
        : public ::nl::Weave::Profiles::Security::AppKeys::GroupKeyStoreBase,
          private ESP32Config
{
    using WeaveGroupKey = ::nl::Weave::Profiles::Security::AppKeys::WeaveGroupKey;

public:
    enum
    {
        kMaxGroupKeys = WEAVE_CONFIG_MAX_APPLICATION_EPOCH_KEYS +       // Maximum number of Epoch keys
                        WEAVE_CONFIG_MAX_APPLICATION_GROUPS +           // Maximum number of Application Group Master keys
                        1 +                                             // Maximum number of Root keys (1 for Service root key)
                        1                                               // Fabric secret
    };

    WEAVE_ERROR Init();

    WEAVE_ERROR RetrieveGroupKey(uint32_t keyId, WeaveGroupKey & key) override;
    WEAVE_ERROR StoreGroupKey(const WeaveGroupKey & key) override;
    WEAVE_ERROR DeleteGroupKey(uint32_t keyId) override;
    WEAVE_ERROR DeleteGroupKeysOfAType(uint32_t keyType) override;
    WEAVE_ERROR EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount) override;
    WEAVE_ERROR Clear(void) override;
    WEAVE_ERROR RetrieveLastUsedEpochKeyId(void) override;
    WEAVE_ERROR StoreLastUsedEpochKeyId(void) override;

private:

    uint32_t mKeyIndex[kMaxGroupKeys];
    uint8_t mNumKeys;

    WEAVE_ERROR AddKeyToIndex(uint32_t keyId, bool & indexUpdated);
    WEAVE_ERROR WriteKeyIndex(nvs_handle handle);
    WEAVE_ERROR DeleteKeyOrKeys(uint32_t targetKeyId, uint32_t targetKeyType);

    static WEAVE_ERROR FormKeyName(uint32_t keyId, char * buf, size_t bufSize);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
