/*
 *
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          for platforms based on the NXP SDK.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/K32W/GroupKeyStoreImpl.h>

using namespace ::nl;
using namespace ::nl::Weave;
using namespace ::nl::Weave::Profiles::Security::AppKeys;

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

WEAVE_ERROR GroupKeyStoreImpl::RetrieveGroupKey(uint32_t keyId, WeaveGroupKey &key)
{
    WEAVE_ERROR err;

    // Iterate over all the GroupKey PDM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyId, &key](const Key &nvmIdKey, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;
                            size_t      keyLen;
                            uint8_t     buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t    curKeyId;

                            // Read the PDM obj binary data data into the buffer.
                            err2 = ReadConfigValueBin(nvmIdKey, buf, sizeof(buf), keyLen);

                            // Decode the Weave key id for the current key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If it matches the key we're looking for...
                            if (curKeyId == keyId)
                            {
                                // Decode the associated key data.
                                err2 = DecodeGroupKey(buf, keyLen, key);
                                SuccessOrExit(err2);

                                // End the iteration by returning a WEAVE_END_OF_INPUT result.
                                ExitNow(err2 = WEAVE_END_OF_INPUT);
                            }

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case WEAVE_END_OF_INPUT:
        err = WEAVE_NO_ERROR; // Match found.
        break;
    case WEAVE_NO_ERROR:
        err = WEAVE_ERROR_KEY_NOT_FOUND; // Match not found.
        break;
    default:
        break;
    }

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::StoreGroupKey(const WeaveGroupKey &key)
{
    WEAVE_ERROR err;

    // Delete any existing group key with the same id (this may or may not exit).
    DeleteGroupKey(key.KeyId); // no error checking here.

    // Iterate over all the GroupKey PDM records looking for the first
    // empty PDM key where we can store the data. (Note- use arg addNewrecord=true)
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, true,
                        [&key](const Key &pdmKey, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;
                            size_t      keyLen;
                            uint8_t     buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)

                            // Encode the key for storage in an PDM record.
                            err2 = EncodeGroupKey(key, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Write the encoded binary data into the PDM object.
                            err2 = WriteConfigValueBin(pdmKey, buf, keyLen);
                            SuccessOrExit(err2);

                            // End the iteration by returning a WEAVE_END_OF_INPUT result.
                            ExitNow(err2 = WEAVE_END_OF_INPUT);

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case WEAVE_END_OF_INPUT:
        err = WEAVE_NO_ERROR; // Key entry was stored.
        break;
    case WEAVE_NO_ERROR:
        err = WEAVE_ERROR_KEY_NOT_FOUND; // Key entry was not stored.
        break;
    default:
        break;
    }

    if (err == WEAVE_NO_ERROR)
    {
#if WEAVE_PROGRESS_LOGGING
        {
            char extraKeyInfo[32];
            if (WeaveKeyId::IsAppEpochKey(key.KeyId))
            {
                snprintf(extraKeyInfo, sizeof(extraKeyInfo), ", start time %" PRId32, key.StartTime);
            }
            else if (WeaveKeyId::IsAppGroupMasterKey(key.KeyId))
            {
                snprintf(extraKeyInfo, sizeof(extraKeyInfo), ", global id 0x%08" PRIX32, key.GlobalId);
            }
            else
            {
                extraKeyInfo[0] = 0;
            }

#if WEAVE_CONFIG_SECURITY_TEST_MODE
            WeaveLogProgress(SecurityManager,
                             "GroupKeyStore: storing key 0x%08" PRIX32 " (%s), len %" PRId8 ", data 0x%02" PRIX8
                             "...%s",
                             key.KeyId, WeaveKeyId::DescribeKey(key.KeyId), key.KeyLen, key.Key[0], extraKeyInfo);
#else
            WeaveLogProgress(SecurityManager, "GroupKeyStore: storing key 0x%08" PRIX32 " (%s), len %" PRId8 "%s",
                             key.KeyId, WeaveKeyId::DescribeKey(key.KeyId), key.KeyLen, extraKeyInfo);
#endif
        }

#endif // WEAVE_PROGRESS_LOGGING
    }

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::DeleteGroupKey(uint32_t keyId)
{
    WEAVE_ERROR err;

    // Iterate over all the GroupKey PDM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyId](const Key &pdmKey, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;
                            size_t      keyLen;
                            uint8_t     buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t    curKeyId;

                            // Read the PDM obj binary data data into the buffer.
                            err2 = ReadConfigValueBin(pdmKey, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Decode the Weave key id for the current group key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If it matches the key we are looking for, delete the PDM record.
                            if (curKeyId == keyId)
                            {
                                err2 = ClearConfigValue(pdmKey);
                                WeaveLogProgress(DeviceLayer, "GroupKeyStore: deleting key 0x%08" PRIX32, curKeyId);

                                // End the iteration by returning a WEAVE_END_OF_INPUT result.
                                ExitNow(err2 = WEAVE_END_OF_INPUT);
                            }

                        exit:
                            return err2;
                        });

    // Modify error code for return.
    switch (err)
    {
    case WEAVE_END_OF_INPUT:
        err = WEAVE_NO_ERROR; // Key entry was deleted.
        break;
    case WEAVE_NO_ERROR:
        err = WEAVE_ERROR_KEY_NOT_FOUND; // Key entry was not deleted.
        break;
    default:
        break;
    }

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::DeleteGroupKeysOfAType(uint32_t keyType)
{
    WEAVE_ERROR err;

    // Iterate over all the GroupKey PDM records looking for a matching key...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [keyType](const Key &pdmKey, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;
                            size_t      keyLen;
                            uint8_t     buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
                            uint32_t    curKeyId;

                            // Read the PDM obj binary data data into the buffer.
                            err2 = ReadConfigValueBin(pdmKey, buf, sizeof(buf), keyLen);
                            SuccessOrExit(err2);

                            // Decode the Weave key id for the current group key.
                            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
                            SuccessOrExit(err2);

                            // If the current key matches the type we are looking for, delete the PDM record.
                            if (WeaveKeyId::GetType(curKeyId) == keyType)
                            {
                                err2 = ClearConfigValue(pdmKey);
                                WeaveLogProgress(DeviceLayer, "GroupKeyStore: deleting key 0x%08" PRIX32, curKeyId);
                            }

                        exit:
                            return err2;
                        });

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::EnumerateGroupKeys(uint32_t  keyType,
                                                  uint32_t *keyIds,
                                                  uint8_t   keyIdsArraySize,
                                                  uint8_t & keyCount)
{
    WEAVE_ERROR err;

    keyCount = 0;

    // Iterate over all the GroupKey records looking for keys of the specified type...
    err = ForEachRecord(
        kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
        [keyType, keyIds, keyIdsArraySize, &keyCount](const Key &pdmKey, const size_t &length) -> WEAVE_ERROR {
            WEAVE_ERROR err2;
            size_t      keyLen;
            uint8_t     buf[kMaxEncodedKeySize]; // (buf length == 45 bytes)
            uint32_t    curKeyId;

            // Read the PDM obj binary data data into the buffer.
            err2 = ReadConfigValueBin(pdmKey, buf, sizeof(buf), keyLen);
            SuccessOrExit(err2);

            // Decode the Weave key id for the current group key.
            err2 = DecodeGroupKeyId(buf, keyLen, curKeyId);
            SuccessOrExit(err2);

            // If the current key matches the type we're looking for, add it to the keyIds array.
            if ((keyType == WeaveKeyId::kType_None) || (WeaveKeyId::GetType(curKeyId) == keyType))
            {
                keyIds[keyCount++] = curKeyId;

                // Stop iterating if there's no more room in the keyIds array.
                VerifyOrExit(keyCount < keyIdsArraySize, err2 = WEAVE_ERROR_BUFFER_TOO_SMALL);
            }

        exit:
            return err2;
        });

    // Simply return a truncated list if there are more matching keys than will fit in the array.
    if (err == WEAVE_ERROR_BUFFER_TOO_SMALL)
    {
        err = WEAVE_NO_ERROR;
    }

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::Clear(void)
{
    WEAVE_ERROR err;

    // Iterate over all the GroupKey PDM records deleting each one...
    err = ForEachRecord(kConfigKey_GroupKeyBase, kConfigKey_GroupKeyMax, false,
                        [](const Key &pdmKey, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;

                            err2 = ClearConfigValue(pdmKey);
                            SuccessOrExit(err2);

                        exit:
                            return err2;
                        });

    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::RetrieveLastUsedEpochKeyId(void)
{
    WEAVE_ERROR err;

    err = ReadConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
    if (err == WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        LastUsedEpochKeyId = WeaveKeyId::kNone;
        err                = WEAVE_NO_ERROR;
    }
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::StoreLastUsedEpochKeyId(void)
{
    return WriteConfigValue(kConfigKey_LastUsedEpochKeyId, LastUsedEpochKeyId);
}

WEAVE_ERROR GroupKeyStoreImpl::Init()
{
    // Nothing to do
    return WEAVE_NO_ERROR;
}

WEAVE_ERROR GroupKeyStoreImpl::EncodeGroupKey(const WeaveGroupKey &key,
                                              uint8_t *            buf,
                                              size_t               bufSize,
                                              size_t &             encodedKeyLen)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    uint8_t *   p   = buf;

    VerifyOrExit(bufSize >= kFixedEncodedKeySize + key.KeyLen, err = WEAVE_ERROR_BUFFER_TOO_SMALL);

    Encoding::LittleEndian::Write32(p, key.KeyId);
    Encoding::LittleEndian::Write32(p, key.StartTime);
    Encoding::Write8(p, key.KeyLen);
    memcpy(p, key.Key, key.KeyLen);
    p += key.KeyLen;

    encodedKeyLen = p - buf;

exit:
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::DecodeGroupKeyId(const uint8_t *encodedKey, size_t encodedKeyLen, uint32_t &keyId)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize, err = WEAVE_ERROR_INVALID_ARGUMENT);

    keyId = Encoding::LittleEndian::Get32(encodedKey);

exit:
    return err;
}

WEAVE_ERROR GroupKeyStoreImpl::DecodeGroupKey(const uint8_t *encodedKey, size_t encodedKeyLen, WeaveGroupKey &key)
{
    WEAVE_ERROR    err = WEAVE_NO_ERROR;
    const uint8_t *p   = encodedKey;

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize, err = WEAVE_ERROR_INVALID_ARGUMENT);

    key.KeyId     = Encoding::LittleEndian::Read32(p);
    key.StartTime = Encoding::LittleEndian::Read32(p);
    key.KeyLen    = Encoding::Read8(p);

    VerifyOrExit(encodedKeyLen >= kFixedEncodedKeySize + key.KeyLen, err = WEAVE_ERROR_INVALID_ARGUMENT);

    memcpy(key.Key, p, key.KeyLen);

exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
