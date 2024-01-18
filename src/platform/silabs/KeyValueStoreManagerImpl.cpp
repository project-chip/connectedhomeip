/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for SILABS
 */

#include "MigrationManager.h"
#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/silabs/SilabsConfig.h>
#include <stdio.h>
#include <string.h>

using namespace ::chip;
using namespace ::chip::Crypto;
using namespace ::chip::DeviceLayer::Internal;

#define CONVERT_KEYMAP_INDEX_TO_NVM3KEY(index) (SilabsConfig::kConfigKey_KvsFirstKeySlot + index)
#define CONVERT_NVM3KEY_TO_KEYMAP_INDEX(nvm3Key) (nvm3Key - SilabsConfig::kConfigKey_KvsFirstKeySlot)

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;
uint16_t mKvsKeyMap[KeyValueStoreManagerImpl::kMaxEntries] = { 0 };

CHIP_ERROR KeyValueStoreManagerImpl::Init(void)
{
    CHIP_ERROR err;
    err = SilabsConfig::Init();
    SuccessOrExit(err);

    Silabs::MigrationManager::GetMigrationInstance().applyMigrations();

    memset(mKvsKeyMap, 0, sizeof(mKvsKeyMap));
    size_t outLen;
    err = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_KvsStringKeyMap, reinterpret_cast<uint8_t *>(mKvsKeyMap),
                                           sizeof(mKvsKeyMap), outLen);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND) // Initial boot
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

bool KeyValueStoreManagerImpl::IsValidKvsNvm3Key(uint32_t nvm3Key) const
{
    return ((SilabsConfig::kConfigKey_KvsFirstKeySlot <= nvm3Key) && (nvm3Key <= SilabsConfig::kConfigKey_KvsLastKeySlot));
}

uint16_t KeyValueStoreManagerImpl::hashKvsKeyString(const char * key) const
{
    uint8_t hash256[Crypto::kSHA256_Hash_Length] = { 0 };
    Crypto::Hash_SHA256(reinterpret_cast<const uint8_t *>(key), strlen(key), hash256);

    uint16_t hash16 = 0, i = 0;

    while (!hash16 && (i < (Crypto::kSHA256_Hash_Length - 1)))
    {
        hash16 = (hash256[i] | (hash256[i + 1] << 8));
        i++;
    }
    return hash16;
}

CHIP_ERROR KeyValueStoreManagerImpl::MapKvsKeyToNvm3(const char * key, uint16_t hash, uint32_t & nvm3Key, bool isSlotNeeded) const
{
    CHIP_ERROR err;
    char * strPrefix          = nullptr;
    uint8_t firstEmptyKeySlot = kMaxEntries;
    for (uint8_t keyIndex = 0; keyIndex < kMaxEntries; keyIndex++)
    {
        if (mKvsKeyMap[keyIndex] == hash)
        {
            uint32_t tempNvm3key = CONVERT_KEYMAP_INDEX_TO_NVM3KEY(keyIndex);
            VerifyOrDie(IsValidKvsNvm3Key(tempNvm3key) == true);

            size_t readCount;
            size_t length = strlen(key);
            if (strPrefix == nullptr)
            {
                // Use a calloc to initialize all bits to 0. alloc +1 for a null char
                strPrefix = static_cast<char *>(Platform::MemoryCalloc(1, length + 1));
                VerifyOrDie(strPrefix != nullptr);
            }

            // Collision prevention
            // Read the data from NVM3 it should be prefixed by the kvsString
            // else we will look for another matching hash in the map
            SilabsConfig::ReadConfigValueBin(tempNvm3key, reinterpret_cast<uint8_t *>(strPrefix), length, readCount, 0);
            if (strcmp(key, strPrefix) == 0)
            {
                // String matches we have confirmed the hash pointed us the right key data
                nvm3Key = tempNvm3key;
                Platform::MemoryFree(strPrefix);
                return CHIP_NO_ERROR;
            }
        }

        if (isSlotNeeded && (firstEmptyKeySlot == kMaxEntries) && (mKvsKeyMap[keyIndex] == 0))
        {
            firstEmptyKeySlot = keyIndex;
        }
    }

    Platform::MemoryFree(strPrefix);

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
    SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_KvsStringKeyMap, reinterpret_cast<const uint8_t *>(mKvsKeyMap),
                                      sizeof(mKvsKeyMap));
}

void KeyValueStoreManagerImpl::ScheduleKeyMapSave(void)
{
    /*
        During commissioning, the key map will be modified multiples times subsequently.
        Commit the key map in nvm once it as stabilized.
    */
#if SIWX_917 && CHIP_CONFIG_ENABLE_ICD_SERVER
    // TODO: Remove this when RTC timer is added MATTER-2705
    SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_KvsStringKeyMap, reinterpret_cast<const uint8_t *>(mKvsKeyMap),
                                      sizeof(mKvsKeyMap));
#else
    SystemLayer().StartTimer(
        std::chrono::duration_cast<System::Clock::Timeout>(System::Clock::Seconds32(SILABS_KVS_SAVE_DELAY_SECONDS)),
        KeyValueStoreManagerImpl::OnScheduledKeyMapSave, NULL);
#endif // defined(SIWX_917) && CHIP_CONFIG_ENABLE_ICD_SERVER
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    uint16_t hash  = hashKvsKeyString(key);
    CHIP_ERROR err = MapKvsKeyToNvm3(key, hash, nvm3Key);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    size_t outLen;
    // The user doesn't need the KeyString prefix, Read data after it
    size_t KeyStringLen = strlen(key);
    err                 = SilabsConfig::ReadConfigValueBin(nvm3Key, reinterpret_cast<uint8_t *>(value), value_size, outLen,
                                                           (offset_bytes + KeyStringLen));
    if (read_bytes_size)
    {
        *read_bytes_size = outLen;
    }

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    uint16_t hash  = hashKvsKeyString(key);
    CHIP_ERROR err = MapKvsKeyToNvm3(key, hash, nvm3Key, /* isSlotNeeded */ true);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    // add the string Key as prefix to the stored data as a collision prevention mechanism.
    size_t keyStringLen    = strlen(key);
    uint8_t * prefixedData = static_cast<uint8_t *>(Platform::MemoryAlloc(keyStringLen + value_size));
    VerifyOrDie(prefixedData != nullptr);
    memcpy(prefixedData, key, keyStringLen);
    memcpy(prefixedData + keyStringLen, value, value_size);

    err = SilabsConfig::WriteConfigValueBin(nvm3Key, prefixedData, keyStringLen + value_size);
    if (err == CHIP_NO_ERROR)
    {
        uint32_t keyIndex    = CONVERT_NVM3KEY_TO_KEYMAP_INDEX(nvm3Key);
        mKvsKeyMap[keyIndex] = hash;
        ScheduleKeyMapSave();
    }
    Platform::MemoryFree(prefixedData);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t nvm3Key;
    uint16_t hash  = hashKvsKeyString(key);
    CHIP_ERROR err = MapKvsKeyToNvm3(key, hash, nvm3Key);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    err = SilabsConfig::ClearConfigValue(nvm3Key);
    if (err == CHIP_NO_ERROR)
    {
        uint32_t keyIndex    = CONVERT_NVM3KEY_TO_KEYMAP_INDEX(nvm3Key);
        mKvsKeyMap[keyIndex] = 0;
        ScheduleKeyMapSave();
    }

    return err;
}

void KeyValueStoreManagerImpl::ErasePartition(void)
{
    // Iterate over all the Matter Kvs nvm3 records and delete each one...
    for (uint32_t nvm3Key = SilabsConfig::kMinConfigKey_MatterKvs; nvm3Key <= SilabsConfig::kMaxConfigKey_MatterKvs; nvm3Key++)
    {
        SilabsConfig::ClearConfigValue(nvm3Key);
    }

    memset(mKvsKeyMap, 0, sizeof(mKvsKeyMap));
}

void KeyValueStoreManagerImpl::KvsMapMigration(void)
{
    size_t readlen                                                                        = 0;
    constexpr uint8_t oldMaxEntires                                                       = 120;
    char mKvsStoredKeyString[oldMaxEntires][PersistentStorageDelegate::kKeyLengthMax + 1] = { 0 };
    CHIP_ERROR err =
        SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_KvsStringKeyMap, reinterpret_cast<uint8_t *>(mKvsStoredKeyString),
                                         sizeof(mKvsStoredKeyString), readlen);

    if (err == CHIP_NO_ERROR)
    {
        for (uint8_t i = 0; i < oldMaxEntires; i++)
        {
            if (mKvsStoredKeyString[i][0] != 0)
            {
                size_t dataLen   = 0;
                uint32_t nvm3Key = CONVERT_KEYMAP_INDEX_TO_NVM3KEY(i);

                if (SilabsConfig::ConfigValueExists(nvm3Key, dataLen))
                {
                    // Read old data and prefix it with the string Key for the collision prevention mechanism.
                    size_t keyStringLen    = strlen(mKvsStoredKeyString[i]);
                    uint8_t * prefixedData = static_cast<uint8_t *>(Platform::MemoryAlloc(keyStringLen + dataLen));
                    VerifyOrDie(prefixedData != nullptr);
                    memcpy(prefixedData, mKvsStoredKeyString[i], keyStringLen);

                    SilabsConfig::ReadConfigValueBin(nvm3Key, prefixedData + keyStringLen, dataLen, readlen);
                    SilabsConfig::WriteConfigValueBin(nvm3Key, prefixedData, keyStringLen + dataLen);
                    mKvsKeyMap[i] = KeyValueStoreMgrImpl().hashKvsKeyString(mKvsStoredKeyString[i]);
                    Platform::MemoryFree(prefixedData);
                }
            }
        }
        ForceKeyMapSave();
    }
    else if (err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        // Error reading the old String Keymap. Cannot not resolve stored data for migration.
        ChipLogError(DeviceLayer, "Migration failed ! Kvs Key map could not be recovered %" CHIP_ERROR_FORMAT, err.Format());
        // start with a fresh kvs section.
        KeyValueStoreMgrImpl().ErasePartition();
    }
}

} // namespace PersistedStorage

namespace Silabs {

void MigrateKvsMap(void)
{
    PersistedStorage::KeyValueStoreMgrImpl().KvsMapMigration();
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
