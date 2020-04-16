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
 *          Provides an implementation of the Weave GroupKeyStore interface
 *          for the ESP32 platform.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ESP32/GroupKeyStoreImpl.h>

#include "nvs_flash.h"
#include "nvs.h"

using namespace ::nl;
using namespace ::nl::Weave;
using namespace ::nl::Weave::Profiles::Security::AppKeys;

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

WEAVE_ERROR GroupKeyStoreImpl::RetrieveGroupKey(uint32_t keyId, WeaveGroupKey & key)
{
    WEAVE_ERROR err;
    size_t keyLen;
    char keyName[kMaxConfigKeyNameLength + 1];
    ESP32Config::Key configKey { kConfigNamespace_WeaveConfig, keyName };

    err = FormKeyName(keyId, keyName, sizeof(keyName));
    SuccessOrExit(err);

    err = ReadConfigValueBin(configKey, key.Key, sizeof(key.Key), keyLen);
    if (err == WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = WEAVE_ERROR_KEY_NOT_FOUND;
    }
    SuccessOrExit(err);

    if (keyId != WeaveKeyId::kFabricSecret)
    {
    	memcpy(&key.StartTime, key.Key + kWeaveAppGroupKeySize, sizeof(uint32_t));
    	keyLen -= sizeof(uint32_t);
    }

    key.KeyId = keyId;
    key.KeyLen = keyLen;

exit:
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::StoreGroupKey(const WeaveGroupKey & key)
{
    WEAVE_ERROR err;
    nvs_handle handle;
    char keyName[kMaxConfigKeyNameLength + 1];
    uint8_t keyData[WeaveGroupKey::MaxKeySize];
    bool needClose = false;
    bool indexUpdated = false;

    err = FormKeyName(key.KeyId, keyName, sizeof(keyName));
    SuccessOrExit(err);

    err = AddKeyToIndex(key.KeyId, indexUpdated);
    SuccessOrExit(err);

    err = nvs_open(kConfigNamespace_WeaveConfig, NVS_READWRITE, &handle);
    SuccessOrExit(err);
    needClose = true;

    memcpy(keyData, key.Key, WeaveGroupKey::MaxKeySize);
    if (key.KeyId != WeaveKeyId::kFabricSecret)
    {
        memcpy(keyData + kWeaveAppGroupKeySize, (const void *)&key.StartTime, sizeof(uint32_t));
    }

#if WEAVE_PROGRESS_LOGGING
    if (WeaveKeyId::IsAppEpochKey(key.KeyId))
    {
        WeaveLogProgress(DeviceLayer, "GroupKeyStore: storing epoch key %s/%s (key len %" PRId8 ", start time %" PRIu32 ")",
                kConfigNamespace_WeaveConfig, keyName, key.KeyLen, key.StartTime);
    }
    else if (WeaveKeyId::IsAppGroupMasterKey(key.KeyId))
    {
        WeaveLogProgress(DeviceLayer, "GroupKeyStore: storing app master key %s/%s (key len %" PRId8 ", global id 0x%" PRIX32 ")",
                kConfigNamespace_WeaveConfig, keyName, key.KeyLen, key.GlobalId);
    }
    else
    {
        const char * keyType = (WeaveKeyId::IsAppRootKey(key.KeyId)) ? "root": "general";
        WeaveLogProgress(DeviceLayer, "GroupKeyStore: storing %s key %s/%s (key len %" PRId8 ")", keyType,
                kConfigNamespace_WeaveConfig, keyName, key.KeyLen);
    }
#endif // WEAVE_PROGRESS_LOGGING

    err = nvs_set_blob(handle, keyName, keyData, WeaveGroupKey::MaxKeySize);
    SuccessOrExit(err);

    if (indexUpdated)
    {
        err = WriteKeyIndex(handle);
        SuccessOrExit(err);
    }

    // Commit the value to the persistent store.
    err = nvs_commit(handle);
    SuccessOrExit(err);

exit:
	if (needClose)
	{
		nvs_close(handle);
	}
	if (err != WEAVE_NO_ERROR && indexUpdated)
	{
	    mNumKeys--;
	}
	ClearSecretData(keyData, sizeof(keyData));
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::DeleteGroupKey(uint32_t keyId)
{
    return DeleteKeyOrKeys(keyId, WeaveKeyId::kType_None);
}

WEAVE_ERROR GroupKeyStoreImpl::DeleteGroupKeysOfAType(uint32_t keyType)
{
    return DeleteKeyOrKeys(WeaveKeyId::kNone, keyType);
}

WEAVE_ERROR GroupKeyStoreImpl::EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds,
        uint8_t keyIdsArraySize, uint8_t & keyCount)
{
    keyCount = 0;

    for (uint8_t i = 0; i < mNumKeys && keyCount < keyIdsArraySize; i++)
    {
        if (keyType == WeaveKeyId::kType_None || WeaveKeyId::GetType(mKeyIndex[i]) == keyType)
        {
            keyIds[keyCount++] = mKeyIndex[i];
        }
    }

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR GroupKeyStoreImpl::Clear(void)
{
    return DeleteKeyOrKeys(WeaveKeyId::kNone, WeaveKeyId::kType_None);
}

WEAVE_ERROR GroupKeyStoreImpl::RetrieveLastUsedEpochKeyId(void)
{
    WEAVE_ERROR err;

    err = ReadConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
    if (err == WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        LastUsedEpochKeyId = WeaveKeyId::kNone;
        err = WEAVE_NO_ERROR;
    }
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::StoreLastUsedEpochKeyId(void)
{
    return WriteConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
}

WEAVE_ERROR GroupKeyStoreImpl::Init()
{
    WEAVE_ERROR err;
    size_t indexSizeBytes;

    err = ReadConfigValueBin(kConfigKey_GroupKeyIndex,
            (uint8_t *)mKeyIndex, sizeof(mKeyIndex), indexSizeBytes);
    if (err == WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = WEAVE_NO_ERROR;
        indexSizeBytes = 0;
    }
    SuccessOrExit(err);

    mNumKeys = indexSizeBytes / sizeof(uint32_t);

exit:
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::AddKeyToIndex(uint32_t keyId, bool & indexUpdated)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    indexUpdated = false;

    for (uint8_t i = 0; i < mNumKeys; i++)
    {
        if (mKeyIndex[i] == keyId)
        {
            ExitNow(err = WEAVE_NO_ERROR);
        }
    }

    VerifyOrExit(mNumKeys < kMaxGroupKeys, err = WEAVE_ERROR_TOO_MANY_KEYS);

    mKeyIndex[mNumKeys++] = keyId;
    indexUpdated = true;

exit:
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::WriteKeyIndex(nvs_handle handle)
{
    WeaveLogProgress(DeviceLayer, "GroupKeyStore: writing key index %s/%s (num keys %" PRIu8 ")",
            kConfigKey_GroupKeyIndex.Namespace, kConfigKey_GroupKeyIndex.Name, mNumKeys);
    return nvs_set_blob(handle, kConfigKey_GroupKeyIndex.Name, mKeyIndex, mNumKeys * sizeof(uint32_t));
}

WEAVE_ERROR GroupKeyStoreImpl::DeleteKeyOrKeys(uint32_t targetKeyId, uint32_t targetKeyType)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    nvs_handle handle;
    char keyName[kMaxConfigKeyNameLength + 1];
    bool needClose = false;

    for (uint8_t i = 0; i < mNumKeys; )
    {
        uint32_t curKeyId = mKeyIndex[i];

        if ((targetKeyId == WeaveKeyId::kNone && targetKeyType == WeaveKeyId::kType_None) ||
            curKeyId == targetKeyId ||
            WeaveKeyId::GetType(curKeyId) == targetKeyType)
        {
            if (!needClose)
            {
                err = nvs_open(kConfigNamespace_WeaveConfig, NVS_READWRITE, &handle);
                SuccessOrExit(err);
                needClose = true;
            }

            err = FormKeyName(curKeyId, keyName, sizeof(keyName));
            SuccessOrExit(err);

            err = nvs_erase_key(handle, keyName);
#if WEAVE_PROGRESS_LOGGING
            if (err == ESP_OK)
            {
                const char * keyType;
                if (WeaveKeyId::IsAppRootKey(curKeyId))
                {
                    keyType = "root";
                }
                else if (WeaveKeyId::IsAppGroupMasterKey(curKeyId))
                {
                    keyType = "app master";
                }
                else if (WeaveKeyId::IsAppEpochKey(curKeyId))
                {
                    keyType = "epoch";
                }
                else
                {
                    keyType = "general";
                }
                WeaveLogProgress(DeviceLayer, "GroupKeyStore: erasing %s key %s/%s", keyType,
                        kConfigNamespace_WeaveConfig, keyName);
            }
            else
#endif // WEAVE_PROGRESS_LOGGING
            if (err == ESP_ERR_NVS_NOT_FOUND)
            {
                err = WEAVE_NO_ERROR;
            }
            SuccessOrExit(err);

            mNumKeys--;

            memmove(&mKeyIndex[i], &mKeyIndex[i+1], (mNumKeys - i) * sizeof(uint32_t));
        }
        else
        {
            i++;
        }
    }

    if (needClose)
    {
        err = WriteKeyIndex(handle);
        SuccessOrExit(err);

        // Commit to the persistent store.
        err = nvs_commit(handle);
        SuccessOrExit(err);
    }

exit:
    if (needClose)
    {
        nvs_close(handle);
    }
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::FormKeyName(uint32_t keyId, char * buf, size_t bufSize)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(bufSize >= kMaxConfigKeyNameLength, err = WEAVE_ERROR_BUFFER_TOO_SMALL);

    if (keyId == WeaveKeyId::kFabricSecret)
    {
        strcpy(buf, kConfigKey_FabricSecret.Name);
    }
    else
    {
        snprintf(buf, bufSize, "%s%08" PRIX32, kGroupKeyNamePrefix, keyId);
    }

exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

