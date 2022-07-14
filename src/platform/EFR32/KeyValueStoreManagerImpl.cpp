/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for EFR32
 */

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/EFR32/EFR32Config.h>
#include <platform/KeyValueStoreManager.h>
#include <stdio.h>
#include <string.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

#define CONVERT_KEYMAP_INDEX_TO_NVM3KEY(index) (EFR32Config::kConfigKey_KvsFirstKeySlot + index)
#define CONVERT_NVM3KEY_TO_KEYMAP_INDEX(nvm3Key) (nvm3Key - EFR32Config::kConfigKey_KvsFirstKeySlot)

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;
char mKvsStoredKeyString[KeyValueStoreManagerImpl::kMaxEntries][PersistentStorageDelegate::kKeyLengthMax + 1];

CHIP_ERROR KeyValueStoreManagerImpl::Init(void)
{
    CHIP_ERROR err;
    err = EFR32Config::Init();
    SuccessOrExit(err);

    memset(mKvsStoredKeyString, 0, sizeof(mKvsStoredKeyString));
    size_t outLen;
    err = EFR32Config::ReadConfigValueBin(EFR32Config::kConfigKey_KvsStringKeyMap, reinterpret_cast<uint8_t *>(mKvsStoredKeyString),
                                          sizeof(mKvsStoredKeyString), outLen);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND) // Initial boot
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

bool KeyValueStoreManagerImpl::IsValidKvsNvm3Key(uint32_t nvm3Key) const
{
    return ((EFR32Config::kConfigKey_KvsFirstKeySlot <= nvm3Key) && (nvm3Key <= EFR32Config::kConfigKey_KvsLastKeySlot));
}

CHIP_ERROR KeyValueStoreManagerImpl::MapKvsKeyToNvm3(const char * key, uint32_t & nvm3Key, bool isSlotNeeded) const
{
    CHIP_ERROR err;
    uint8_t firstEmptyKeySlot = kMaxEntries;
    for (uint8_t keyIndex = 0; keyIndex < kMaxEntries; keyIndex++)
    {
        if (strcmp(key, mKvsStoredKeyString[keyIndex]) == 0)
        {
            nvm3Key = CONVERT_KEYMAP_INDEX_TO_NVM3KEY(keyIndex);
            VerifyOrDie(IsValidKvsNvm3Key(nvm3Key) == true);
            return CHIP_NO_ERROR;
        }

        if (isSlotNeeded && (firstEmptyKeySlot == kMaxEntries) && (mKvsStoredKeyString[keyIndex][0] == 0))
        {
            firstEmptyKeySlot = keyIndex;
        }
    }

    if (isSlotNeeded)
    {
        if (firstEmptyKeySlot != kMaxEntries)
        {
            nvm3Key = CONVERT_KEYMAP_INDEX_TO_NVM3KEY(firstEmptyKeySlot);
            VerifyOrDie(IsValidKvsNvm3Key(nvm3Key) == true);
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
    }
    else
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

void KeyValueStoreManagerImpl::ForceKeyMapSave()
{
    OnScheduledKeyMapSave(nullptr, nullptr);
}

void KeyValueStoreManagerImpl::OnScheduledKeyMapSave(System::Layer * systemLayer, void * appState)
{
    EFR32Config::WriteConfigValueBin(EFR32Config::kConfigKey_KvsStringKeyMap,
                                     reinterpret_cast<const uint8_t *>(mKvsStoredKeyString), sizeof(mKvsStoredKeyString));
}

void KeyValueStoreManagerImpl::ScheduleKeyMapSave(void)
{
    /*
        During commissioning, the key map will be modified multiples times subsequently.
        Commit the key map in nvm once it as stabilized.
    */
    SystemLayer().StartTimer(
        std::chrono::duration_cast<System::Clock::Timeout>(System::Clock::Seconds32(EFR32_KVS_SAVE_DELAY_SECONDS)),
        KeyValueStoreManagerImpl::OnScheduledKeyMapSave, NULL);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    CHIP_ERROR err = MapKvsKeyToNvm3(key, nvm3Key);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    size_t outLen;
    err = EFR32Config::ReadConfigValueBin(nvm3Key, reinterpret_cast<uint8_t *>(value), value_size, outLen, offset_bytes);
    if (read_bytes_size)
    {
        *read_bytes_size = outLen;
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    CHIP_ERROR err = MapKvsKeyToNvm3(key, nvm3Key, /* isSlotNeeded */ true);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    err = EFR32Config::WriteConfigValueBin(nvm3Key, reinterpret_cast<const uint8_t *>(value), value_size);
    if (err == CHIP_NO_ERROR)
    {
        uint32_t keyIndex = nvm3Key - EFR32Config::kConfigKey_KvsFirstKeySlot;
        strncpy(mKvsStoredKeyString[keyIndex], key, sizeof(mKvsStoredKeyString[keyIndex]) - 1);
        ScheduleKeyMapSave();
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    CHIP_ERROR err = MapKvsKeyToNvm3(key, nvm3Key);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    err = EFR32Config::ClearConfigValue(nvm3Key);
    if (err == CHIP_NO_ERROR)
    {
        uint32_t keyIndex = CONVERT_NVM3KEY_TO_KEYMAP_INDEX(nvm3Key);
        memset(mKvsStoredKeyString[keyIndex], 0, sizeof(mKvsStoredKeyString[keyIndex]));
        ScheduleKeyMapSave();
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::ErasePartition(void)
{
    // Iterate over all the Matter Kvs nvm3 records and delete each one...
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint32_t nvm3Key = EFR32Config::kMinConfigKey_MatterKvs; nvm3Key < EFR32Config::kMaxConfigKey_MatterKvs; nvm3Key++)
    {
        err = EFR32Config::ClearConfigValue(nvm3Key);

        if (err != CHIP_NO_ERROR)
        {
            break;
        }
    }

    memset(mKvsStoredKeyString, 0, sizeof(mKvsStoredKeyString));
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
