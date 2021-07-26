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
 *          Provides an implementation of the Chip GroupKeyStore interface
 *          for the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ESP32/GroupKeyStoreImpl.h>
#include <platform/ESP32/ScopedNvsHandle.h>

#include "nvs.h"
#include "nvs_flash.h"

using namespace ::chip;
using namespace ::nl::Weave::Profiles::Security::AppKeys;

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR GroupKeyStoreImpl::RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key)
{
    CHIP_ERROR err;
    size_t keyLen;
    char keyName[kMaxConfigKeyNameLength + 1];
    ESP32Config::Key configKey{ kConfigNamespace_ChipConfig, keyName };

    err = FormKeyName(keyId, keyName, sizeof(keyName));
    SuccessOrExit(err);

    err = ReadConfigValueBin(configKey, key.Key, sizeof(key.Key), keyLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_KEY_NOT_FOUND;
    }
    SuccessOrExit(err);

    if (keyId != ChipKeyId::kFabricSecret)
    {
        memcpy(&key.StartTime, key.Key + kChipAppGroupKeySize, sizeof(uint32_t));
        keyLen -= sizeof(uint32_t);
    }

    key.KeyId  = keyId;
    key.KeyLen = keyLen;

exit:
    return err;
}

CHIP_ERROR GroupKeyStoreImpl::StoreGroupKey(const ChipGroupKey & key)
{
    char keyName[kMaxConfigKeyNameLength + 1];
    uint8_t keyData[ChipGroupKey::MaxKeySize];
    bool indexUpdated = false;
    ScopedNvsHandle handle;
    CHIP_ERROR error = CHIP_NO_ERROR;
    esp_err_t err    = ESP_OK;

    error = FormKeyName(key.KeyId, keyName, sizeof(keyName));
    SuccessOrExit(error);

    error = AddKeyToIndex(key.KeyId, indexUpdated);
    SuccessOrExit(error);

    error = handle.Open(kConfigNamespace_ChipConfig, NVS_READWRITE);
    SuccessOrExit(error);

    memcpy(keyData, key.Key, ChipGroupKey::MaxKeySize);
    if (key.KeyId != ChipKeyId::kFabricSecret)
    {
        memcpy(keyData + kChipAppGroupKeySize, (const void *) &key.StartTime, sizeof(uint32_t));
    }

#if CHIP_PROGRESS_LOGGING
    if (ChipKeyId::IsAppEpochKey(key.KeyId))
    {
        ChipLogProgress(DeviceLayer, "GroupKeyStore: storing epoch key %s/%s (key len %" PRId8 ", start time %" PRIu32 ")",
                        kConfigNamespace_ChipConfig, keyName, key.KeyLen, key.StartTime);
    }
    else if (ChipKeyId::IsAppGroupMasterKey(key.KeyId))
    {
        ChipLogProgress(DeviceLayer, "GroupKeyStore: storing app master key %s/%s (key len %" PRId8 ", global id 0x%" PRIX32 ")",
                        kConfigNamespace_ChipConfig, keyName, key.KeyLen, key.GlobalId);
    }
    else
    {
        const char * keyType = (ChipKeyId::IsAppRootKey(key.KeyId)) ? "root" : "general";
        ChipLogProgress(DeviceLayer, "GroupKeyStore: storing %s key %s/%s (key len %" PRId8 ")", keyType,
                        kConfigNamespace_ChipConfig, keyName, key.KeyLen);
    }
#endif // CHIP_PROGRESS_LOGGING

    err = nvs_set_blob(handle, keyName, keyData, ChipGroupKey::MaxKeySize);
    VerifyOrExit(err == ESP_OK, error = ESP32Utils::MapError(err));

    if (indexUpdated)
    {
        error = WriteKeyIndex(handle);
        SuccessOrExit(error);
    }

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    VerifyOrExit(err == ESP_OK, error = ESP32Utils::MapError(err));

exit:
    if (error != CHIP_NO_ERROR && indexUpdated)
    {
        mNumKeys--;
    }
    ClearSecretData(keyData, sizeof(keyData));
    return error;
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKey(uint32_t keyId)
{
    return DeleteKeyOrKeys(keyId, ChipKeyId::kType_None);
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKeysOfAType(uint32_t keyType)
{
    return DeleteKeyOrKeys(ChipKeyId::kNone, keyType);
}

CHIP_ERROR GroupKeyStoreImpl::EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount)
{
    keyCount = 0;

    for (uint8_t i = 0; i < mNumKeys && keyCount < keyIdsArraySize; i++)
    {
        if (keyType == ChipKeyId::kType_None || ChipKeyId::GetType(mKeyIndex[i]) == keyType)
        {
            keyIds[keyCount++] = mKeyIndex[i];
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::Clear(void)
{
    return DeleteKeyOrKeys(ChipKeyId::kNone, ChipKeyId::kType_None);
}

CHIP_ERROR GroupKeyStoreImpl::RetrieveLastUsedEpochKeyId(void)
{
    CHIP_ERROR err;

    err = ReadConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        LastUsedEpochKeyId = ChipKeyId::kNone;
        err                = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR GroupKeyStoreImpl::StoreLastUsedEpochKeyId(void)
{
    return WriteConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
}

CHIP_ERROR GroupKeyStoreImpl::Init()
{
    CHIP_ERROR err;
    size_t indexSizeBytes;

    err = ReadConfigValueBin(kConfigKey_GroupKeyIndex, (uint8_t *) mKeyIndex, sizeof(mKeyIndex), indexSizeBytes);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err            = CHIP_NO_ERROR;
        indexSizeBytes = 0;
    }
    SuccessOrExit(err);

    mNumKeys = indexSizeBytes / sizeof(uint32_t);

exit:
    return err;
}

CHIP_ERROR GroupKeyStoreImpl::AddKeyToIndex(uint32_t keyId, bool & indexUpdated)
{
    indexUpdated = false;

    for (uint8_t i = 0; i < mNumKeys; i++)
    {
        if (mKeyIndex[i] == keyId)
        {
            return CHIP_NO_ERROR;
        }
    }

    VerifyOrReturnError(mNumKeys < kMaxGroupKeys, CHIP_ERROR_TOO_MANY_KEYS);

    mKeyIndex[mNumKeys++] = keyId;
    indexUpdated          = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::WriteKeyIndex(nvs_handle handle)
{
    ChipLogProgress(DeviceLayer, "GroupKeyStore: writing key index %s/%s (num keys %" PRIu8 ")", kConfigKey_GroupKeyIndex.Namespace,
                    kConfigKey_GroupKeyIndex.Name, mNumKeys);
    return nvs_set_blob(handle, kConfigKey_GroupKeyIndex.Name, mKeyIndex, mNumKeys * sizeof(uint32_t));
}

CHIP_ERROR GroupKeyStoreImpl::DeleteKeyOrKeys(uint32_t targetKeyId, uint32_t targetKeyType)
{
    char keyName[kMaxConfigKeyNameLength + 1];
    ScopedNvsHandle handle;

    for (uint8_t i = 0; i < mNumKeys;)
    {
        uint32_t curKeyId = mKeyIndex[i];

        if ((targetKeyId == ChipKeyId::kNone && targetKeyType == ChipKeyId::kType_None) || curKeyId == targetKeyId ||
            ChipKeyId::GetType(curKeyId) == targetKeyType)
        {
            if (!handle.IsOpen())
            {
                ReturnErrorOnFailure(handle.Open(kConfigNamespace_ChipConfig, NVS_READWRITE));
            }

            ReturnErrorOnFailure(FormKeyName(curKeyId, keyName, sizeof(keyName)));

            esp_err_t err = nvs_erase_key(handle, keyName);
#if CHIP_PROGRESS_LOGGING
            if (err == ESP_OK)
            {
                const char * keyType;
                if (ChipKeyId::IsAppRootKey(curKeyId))
                {
                    keyType = "root";
                }
                else if (ChipKeyId::IsAppGroupMasterKey(curKeyId))
                {
                    keyType = "app master";
                }
                else if (ChipKeyId::IsAppEpochKey(curKeyId))
                {
                    keyType = "epoch";
                }
                else
                {
                    keyType = "general";
                }
                ChipLogProgress(DeviceLayer, "GroupKeyStore: erasing %s key %s/%s", keyType, kConfigNamespace_ChipConfig, keyName);
            }
#endif // CHIP_PROGRESS_LOGGING
            if (err == ESP_ERR_NVS_NOT_FOUND)
            {
                err = ESP_OK;
            }
            ReturnMappedErrorOnFailure(err);

            mNumKeys--;

            memmove(&mKeyIndex[i], &mKeyIndex[i + 1], (mNumKeys - i) * sizeof(uint32_t));
        }
        else
        {
            i++;
        }
    }

    if (handle.IsOpen())
    {
        ReturnErrorOnFailure(WriteKeyIndex(handle));

        // Commit to the persistent store.
        ReturnMappedErrorOnFailure(nvs_commit(handle));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::FormKeyName(uint32_t keyId, char * buf, size_t bufSize)
{
    VerifyOrReturnError(bufSize >= kMaxConfigKeyNameLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    if (keyId == ChipKeyId::kFabricSecret)
    {
        strcpy(buf, kConfigKey_FabricSecret.Name);
    }
    else
    {
        snprintf(buf, bufSize, "%s%08" PRIX32, kGroupKeyNamePrefix, keyId);
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
