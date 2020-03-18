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
 *          for platforms based on the Nordic nRF5 SDK.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>
#include <Weave/DeviceLayer/nRF5/nRF5Config.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * An implementation of the Weave GroupKeyStoreBase API for platforms based
 * on the Nordic nRF5 SDK.
 */
class GroupKeyStoreImpl final
        : public ::nl::Weave::Profiles::Security::AppKeys::GroupKeyStoreBase,
          private NRF5Config
{
    using WeaveGroupKey = ::nl::Weave::Profiles::Security::AppKeys::WeaveGroupKey;

public:

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

    static constexpr size_t kFixedEncodedKeySize =  4U +    // key id
                                                    4U +    // start time / global id
                                                    1U;     // key data length

    static constexpr size_t kMaxEncodedKeySize =    kFixedEncodedKeySize + WeaveGroupKey::MaxKeySize;

    static constexpr uint16_t kGroupKeyFileId =     GetFileId(kConfigKey_GroupKey);
    static constexpr uint16_t kGroupKeyRecordKey =  GetRecordKey(kConfigKey_GroupKey);

    static WEAVE_ERROR EncodeGroupKey(const WeaveGroupKey & key, uint8_t * buf, size_t bufSize, size_t & encodedKeyLen);
    static WEAVE_ERROR DecodeGroupKey(const uint8_t * encodedKey, size_t encodedKeyLen, WeaveGroupKey & key);
    static WEAVE_ERROR DecodeGroupKeyId(const uint8_t * encodedKey, size_t encodedKeyLen, uint32_t & keyId);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
