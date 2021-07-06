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
 *          Provides an implementation of the CHIP GroupKeyStore interface
 *          for Qorvo QPG platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/qpg/GroupKeyStoreImpl.h>

using namespace ::chip;
using namespace ::chip::Profiles::Security::AppKeys;

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR GroupKeyStoreImpl::RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key)
{
    CHIP_ERROR err;

    // Iterate over all the GroupKey NVM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyId, &key](const Key & nvmKey, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            size_t keyLen;
                            uint8_t buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t curKeyId;

                            // Read the NVM element data into the buffer.
                            err2 = ReadConfigValueBin(nvmKey, buf, sizeof(buf), keyLen);

                            // Decode the CHIP key id for the current key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If it matches the key we're looking for...
                            if (curKeyId == keyId)
                            {
                                // Decode the associated key data.
                                err2 = DecodeGroupKey(buf, keyLen, key);
                                SuccessOrExit(err2);

                                // End the iteration by returning a CHIP_END_OF_INPUT result.
                                ExitNow(err2 = CHIP_END_OF_INPUT);
                            }

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case CHIP_END_OF_INPUT:
        err = CHIP_NO_ERROR; // Match found.
        break;
    case CHIP_NO_ERROR:
        err = CHIP_ERROR_KEY_NOT_FOUND; // Match not found.
        break;
    default:
        break;
    }

    return err;
}

CHIP_ERROR GroupKeyStoreImpl::StoreGroupKey(const ChipGroupKey & key)
{
    CHIP_ERROR err;

    // Delete any existing group key with the same id (this may or may not exit).
    DeleteGroupKey(key.KeyId); // no error checking here.

    // Iterate over all the GroupKey nvm3 records looking for the first
    // empty NVM key where we can store the data. (Note- use arg addNewrecord=true)
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, true,
                        [&key](const Key & nvmKey, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            size_t keyLen;
                            uint8_t buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)

                            // Encode the key for storage in an NVM record.
                            err2 = EncodeGroupKey(key, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Write the encoded binary data into the NVM object.
                            err2 = WriteConfigValueBin(nvmKey, buf, keyLen);
                            SuccessOrExit(err2);

                            // End the iteration by returning a CHIP_END_OF_INPUT result.
                            ExitNow(err2 = CHIP_END_OF_INPUT);

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case CHIP_END_OF_INPUT:
        err = CHIP_NO_ERROR; // Key entry was stored.
        break;
    case CHIP_NO_ERROR:
        err = CHIP_ERROR_KEY_NOT_FOUND; // Key entry was not stored.
        break;
    default:
        break;
    }

    if (err == CHIP_NO_ERROR)
    {
#if CHIP_PROGRESS_LOGGING
        {
            char extraKeyInfo[32];
            if (ChipKeyId::IsAppEpochKey(key.KeyId))
            {
                snprintf(extraKeyInfo, sizeof(extraKeyInfo), ", start time %" PRId32, key.StartTime);
            }
            else if (ChipKeyId::IsAppGroupMasterKey(key.KeyId))
            {
                snprintf(extraKeyInfo, sizeof(extraKeyInfo), ", global id 0x%08" PRIX32, key.GlobalId);
            }
            else
            {
                extraKeyInfo[0] = 0;
            }

#if CHIP_CONFIG_SECURITY_TEST_MODE
            ChipLogProgress(SecurityManager,
                            "GroupKeyStore: storing key 0x%08" PRIX32 " (%s), len %" PRId8 ", data 0x%02" PRIX8 "...%s", key.KeyId,
                            ChipKeyId::DescribeKey(key.KeyId), key.KeyLen, key.Key[0], extraKeyInfo);
#else
            ChipLogProgress(SecurityManager, "GroupKeyStore: storing key 0x%08" PRIX32 " (%s), len %" PRId8 "%s", key.KeyId,
                            ChipKeyId::DescribeKey(key.KeyId), key.KeyLen, extraKeyInfo);
#endif
        }

#endif // CHIP_PROGRESS_LOGGING
    }

    return err;
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKey(uint32_t keyId)
{
    CHIP_ERROR err;

    // Iterate over all the GroupKey NVM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyId](const Key & nvmKey, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            size_t keyLen;
                            uint8_t buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t curKeyId;

                            // Read the NVM element data into the buffer.
                            err2 = ReadConfigValueBin(nvmKey, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Decode the CHIP key id for the current group key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If it matches the key we are looking for, delete the NVM record.
                            if (curKeyId == keyId)
                            {
                                err2 = ClearConfigValue(nvmKey);
                                ChipLogProgress(DeviceLayer, "GroupKeyStore: deleting key 0x%08" PRIX32, curKeyId);

                                // End the iteration by returning a CHIP_END_OF_INPUT result.
                                ExitNow(err2 = CHIP_END_OF_INPUT);
                            }

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case CHIP_END_OF_INPUT:
        err = CHIP_NO_ERROR; // Key entry was deleted.
        break;
    case CHIP_NO_ERROR:
        err = CHIP_ERROR_KEY_NOT_FOUND; // Key entry was not deleted.
        break;
    default:
        break;
    }

    return err;
}

CHIP_ERROR GroupKeyStoreImpl::DeleteGroupKeysOfAType(uint32_t keyType)
{
    CHIP_ERROR err;

    // Iterate over all the GroupKey NVM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyType](const Key & nvmKey, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            size_t keyLen;
                            uint8_t buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t curKeyId;

                            // Read the NVM element data into the buffer.
                            err2 = ReadConfigValueBin(nvmKey, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Decode the CHIP key id for the current group key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If the current key matches the type we are looking for, delete the NVM record.
                            if (ChipKeyId::GetType(curKeyId) == keyType)
                            {
                                err2 = ClearConfigValue(nvmKey);
                                ChipLogProgress(DeviceLayer, "GroupKeyStore: deleting key 0x%08" PRIX32, curKeyId);
                            }

                        exit:
                            return err2;
                        });

    return err;
}

CHIP_ERROR GroupKeyStoreImpl::EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount)
{
    CHIP_ERROR err;

    keyCount = 0;

    // Iterate over all the GroupKey records looking for keys of the specified type...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyType, keyIds, keyIdsArraySize, &keyCount](const Key & nvm3Key, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            size_t keyLen;
                            uint8_t buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t curKeyId;

                            // Read the NVM element data into the buffer.
                            err2 = ReadConfigValueBin(nvm3Key, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Decode the CHIP key id for the current group key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If the current key matches the type we're looking for, add it to the keyIds array.
                            if ((keyType == ChipKeyId::kType_None) || (ChipKeyId::GetType(curKeyId) == keyType))
                            {
                                keyIds[keyCount++] = curKeyId;

                                // Stop iterating if there's no more room in the keyIds array.
                                VerifyOrExit(keyCount < keyIdsArraySize, err2 = CHIP_ERROR_BUFFER_TOO_SMALL);
                            }

                        exit:
                            return err2;
                        });

    // Simply return a truncated list if there are more matching keys than will fit in the array.
    if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR GroupKeyStoreImpl::Clear(void)
{
    CHIP_ERROR err;

    // Iterate over all the GroupKey NVM records deleting each one...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [](const Key & nvmKey, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;

                            err2 = ClearConfigValue(nvmKey);
                            SuccessOrExit(err2);

                        exit:
                            return err2;
                        });

    return err;
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
    // Nothing to do
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyStoreImpl::EncodeGroupKey(const ChipGroupKey & key, uint8_t * buf, size_t bufSize, size_t & encodedKeyLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t * p    = buf;

    VerifyOrExit(bufSize >= kFixedEncodedKeySize + key.KeyLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    Encoding::LittleEndian::Write32(p, key.KeyId);
    Encoding::LittleEndian::Write32(p, key.StartTime);
    Encoding::Write8(p, key.KeyLen);
    memcpy(p, key.Key, key.KeyLen);
    p += key.KeyLen;

    encodedKeyLen = p - buf;

exit:
    return err;
}

CHIP_ERROR GroupKeyStoreImpl::DecodeGroupKeyId(const uint8_t * encodedKey, size_t encodedKeyLen, uint32_t & keyId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize, err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = Encoding::LittleEndian::Get32(encodedKey);

exit:
    return err;
}

CHIP_ERROR GroupKeyStoreImpl::DecodeGroupKey(const uint8_t * encodedKey, size_t encodedKeyLen, ChipGroupKey & key)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    const uint8_t * p = encodedKey;

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize, err = CHIP_ERROR_INVALID_ARGUMENT);

    key.KeyId     = Encoding::LittleEndian::Read32(p);
    key.StartTime = Encoding::LittleEndian::Read32(p);
    key.KeyLen    = Encoding::Read8(p);

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize + key.KeyLen, err = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(key.Key, p, key.KeyLen);

exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
} // namespace chip
