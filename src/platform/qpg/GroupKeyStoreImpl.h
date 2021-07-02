/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Provides an implementation of the chip Group Key Store interface
 *          for Qorvo QPG platforms.
 */

#include <core/CHIPKeyIds.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/qpg/qpgConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ChipGroupKey
{
public:
    enum
    {
        MaxKeySize = 36
    };
    uint32_t KeyId;          /**< The key ID. */
    uint8_t KeyLen;          /**< The key length. */
    uint8_t Key[MaxKeySize]; /**< The secret key material. */
    union
    {
        uint32_t StartTime; /**< The epoch key start time. */
        uint32_t GlobalId;  /**< The application group key global ID. */
    };
};

/**
 * An implementation of the chip GroupKeyStoreBase API for this platform.
 */
class GroupKeyStoreImpl final : public ::chip::Profiles::Security::AppKeys::GroupKeyStoreBase, private QPGConfig
{
    using ChipGroupKey = ::chip::Profiles::Security::AppKeys::ChipGroupKey;

public:
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
    static constexpr size_t kFixedEncodedKeySize = 4U + // key id
        4U +                                            // start time / global id
        1U;                                             // key data length

    static constexpr size_t kMaxEncodedKeySize = kFixedEncodedKeySize + ChipGroupKey::MaxKeySize;

    static constexpr uint16_t kGroupKeyFileId    = GetFileId(kConfigKey_GroupKey);
    static constexpr uint16_t kGroupKeyRecordKey = GetRecordKey(kConfigKey_GroupKey);

    static CHIP_ERROR EncodeGroupKey(const ChipGroupKey & key, uint8_t * buf, size_t bufSize, size_t & encodedKeyLen);
    static CHIP_ERROR DecodeGroupKey(const uint8_t * encodedKey, size_t encodedKeyLen, ChipGroupKey & key);
    static CHIP_ERROR DecodeGroupKeyId(const uint8_t * encodedKey, size_t encodedKeyLen, uint32_t & keyId);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
