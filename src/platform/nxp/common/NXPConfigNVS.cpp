/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2024 NXP
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

#include "NXPConfig.h"
#include <lib/core/CHIPEncoding.h>
#include <platform/CHIPDeviceError.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <platform/nxp/common/CHIPDeviceNXPPlatformDefaultConfig.h>

extern "C" {
#include <settings.h>
}

/* Only for flash init, to be move to sdk framework */
#include "nvs_port.h"
#if (CHIP_DEVICE_CONFIG_KVS_WEAR_STATS == 1)
#include "fwk_nvs_stats.h"
#endif /* CHIP_DEVICE_CONFIG_KVS_WEAR_STATS */

// These can be overridden by the application as needed.
#ifndef CHIP_DEVICE_INTEGER_SETTINGS_KEY
/// Key for all integer keys
#define CHIP_DEVICE_INTEGER_SETTINGS_KEY "mt_i"
#endif // CHIP_DEVICE_CONFIG_SETTINGS_KEY
#ifndef CHIP_DEVICE_STRING_SETTINGS_KEY
/// Key for all string keys
#define CHIP_DEVICE_STRING_SETTINGS_KEY "mt_s"
#endif // CHIP_DEVICE_CONFIG_SETTINGS_KEY

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

struct ReadRequest
{
    void * const destination; // NOTE: can be nullptr in which case `configSize` should still be returned
    const size_t bufferSize;  // size of destination buffer
    CHIP_ERROR result;        // [out] read result
    size_t configSize;        // [out] size of configuration value
};

struct DeleteSubtreeEntry
{
    int result;
};

// Callback for settings_load_subtree_direct() function
int ConfigValueCallback(const char * name, size_t configSize, settings_read_cb readCb, void * cbArg, void * param)
{
    // If requested config key X, process just node X and ignore all its descendants: X/*
    if (name != nullptr && *name != '\0')
        return 0;

    ReadRequest & request = *reinterpret_cast<ReadRequest *>(param);

    if (!request.destination || configSize > request.bufferSize)
    {
        request.result     = CHIP_ERROR_BUFFER_TOO_SMALL;
        request.configSize = configSize;
        return 1;
    }

    // Found requested key
    const ssize_t bytesRead = readCb(cbArg, request.destination, request.bufferSize);
    request.result          = bytesRead > 0 ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    request.configSize      = bytesRead > 0 ? bytesRead : 0;

    // Return 1 to stop processing further keys
    return 1;
}

// Read configuration value of maximum size `bufferSize` and store the actual size in `configSize`.
CHIP_ERROR ReadConfigValueImpl(const NXPConfig::Key key, void * const destination, const size_t bufferSize, size_t & configSize)
{
    ReadRequest request{ destination, bufferSize, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND, 0 };
    char key_name[SETTINGS_MAX_NAME_LEN + 1];

    sprintf(key_name, CHIP_DEVICE_INTEGER_SETTINGS_KEY "/%04x", key);
    settings_load_subtree_direct(key_name, ConfigValueCallback, &request);
    configSize = request.configSize;
    return request.result;
}

CHIP_ERROR WriteConfigValueImpl(const NXPConfig::Key key, const void * const source, const size_t length)
{
    char key_name[SETTINGS_MAX_NAME_LEN + 1];
    sprintf(key_name, CHIP_DEVICE_INTEGER_SETTINGS_KEY "/%04x", key);
    if (settings_save_one(key_name, source, length) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif
    return CHIP_NO_ERROR;
}

template <class T>
inline CHIP_ERROR ReadSimpleConfigValue(const NXPConfig::Key key, T & value)
{
    CHIP_ERROR result;
    T tempValue;
    size_t configSize;

    result = ReadConfigValueImpl(key, &tempValue, sizeof(T), configSize);
    SuccessOrExit(result);

    // For simple types require that size of the output variable matches size of the configuration value
    VerifyOrExit(configSize == sizeof(T), result = CHIP_ERROR_INVALID_ARGUMENT);
    value = tempValue;
exit:
    return result;
}

int DeleteSubtreeCallback(const char * name, size_t /* entrySize */, settings_read_cb /* readCb */, void * /* cbArg */,
                          void * param)
{

    DeleteSubtreeEntry & entry = *static_cast<DeleteSubtreeEntry *>(param);
    char fullKey[SETTINGS_MAX_NAME_LEN + 1];
    (void) snprintf(fullKey, sizeof(fullKey), CHIP_DEVICE_STRING_SETTINGS_KEY "/%s", StringOrNullMarker(name));
    const int result = settings_delete(fullKey);

    // Return the first error, but continue removing remaining keys anyway.
    if (entry.result == 0)
    {
        entry.result = result;
    }

    return 0;
}

#if (CHIP_DEVICE_CONFIG_KVS_WEAR_STATS == 1)
void OnFlashSectorWearCountUpdate(uint16_t sector_idx, const nvs_storage_wear_profile_t * flash_wear_profile)
{
    char keyUser[]                       = CHIP_DEVICE_CONFIG_KVS_WEAR_STATS_KEY;
    const size_t flash_wear_profile_size = NVS_STORAGE_WEAR_PROFILE_SIZE(flash_wear_profile->sector_count);

    /* Update the NVS stats key in storage */
    NXPConfig::WriteConfigValueBin((const char *) keyUser, (uint8_t *) flash_wear_profile, flash_wear_profile_size);
}
#endif /* CHIP_DEVICE_CONFIG_KVS_WEAR_STATS */
} // namespace

CHIP_ERROR NXPConfig::Init()
{
    /* Only for flash init, to be move to sdk framework */
    /* Initialize flash components */
    const struct flash_area * fa;

    VerifyOrReturnError(!flash_area_open(SETTINGS_PARTITION, &fa), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    VerifyOrReturnError(!flash_init(fa->fa_dev), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    /* End flash init */

    VerifyOrReturnError(!settings_subsys_init(), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

#if (CHIP_DEVICE_CONFIG_KVS_WEAR_STATS == 1)
    ReturnErrorOnFailure(InitStorageWearStats());
#endif /* CHIP_DEVICE_CONFIG_KVS_WEAR_STATS */

    return CHIP_NO_ERROR;
}

#if (CHIP_DEVICE_CONFIG_KVS_WEAR_STATS == 1)
CHIP_ERROR NXPConfig::InitStorageWearStats(void)
{
    nvs_storage_wear_profile_t * flash_wear_profile = NULL;
    const size_t flash_wear_profile_size            = NVS_STORAGE_WEAR_PROFILE_SIZE((uint32_t) NV_STORAGE_MAX_SECTORS);
    size_t size;
    char keyUser[] = CHIP_DEVICE_CONFIG_KVS_WEAR_STATS_KEY;

    /* Create an empty flash wear profile */
    flash_wear_profile = (nvs_storage_wear_profile_t *) calloc(1, flash_wear_profile_size);
    VerifyOrReturnError(flash_wear_profile != NULL, CHIP_ERROR_NO_MEMORY);

    /* Try to read the flash wear profile from the User Support diagnostic log key */
    CHIP_ERROR err = ReadConfigValueBin((const char *) keyUser, (uint8_t *) flash_wear_profile, flash_wear_profile_size, size);
    if ((err != CHIP_NO_ERROR) || (size != flash_wear_profile_size) ||
        (flash_wear_profile->sector_count != (uint32_t) NV_STORAGE_MAX_SECTORS))
    {
        /* Either the flash wear stats are not available in the persistent
         * storage or the flash wear statistics that we have read are not
         * compatible with the current persistent storage configuration. In
         * this case - just reset and save the flash wear statistics. */
        flash_wear_profile->sector_count = (uint32_t) NV_STORAGE_MAX_SECTORS;
        memset(flash_wear_profile->erase_count, 0, (uint32_t) NV_STORAGE_MAX_SECTORS * sizeof(uint16_t));
        WriteConfigValueBin((const char *) keyUser, (uint8_t *) flash_wear_profile, flash_wear_profile_size);
    }
    else
    {
        /* Load the flash wear profile into the NVS statistics */
        nvs_stats_load_profile(flash_wear_profile);
    }
    free(flash_wear_profile);

    nvs_stats_config_event_handler(OnFlashSectorWearCountUpdate);

    return CHIP_NO_ERROR;
}
#endif /* CHIP_DEVICE_CONFIG_KVS_WEAR_STATS */

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, bool & val)
{
    VerifyOrReturnError(ValidConfigKey(key), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, uint32_t & val)
{
    VerifyOrReturnError(ValidConfigKey(key), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, uint64_t & val)
{
    VerifyOrReturnError(ValidConfigKey(key), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    return ReadSimpleConfigValue(key, val);
}

CHIP_ERROR NXPConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrReturnError(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    // Pretend that the buffer is smaller by 1 to secure space for null-character
    err = ReadConfigValueImpl(key, buf, bufSize ? bufSize - 1 : 0, outLen);

    if (err == CHIP_NO_ERROR)
    {
        if (buf[outLen - 1]) // CHIP_NO_ERROR implies outLen > 0
            buf[outLen] = 0;
        else
            outLen--;
    }
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueImpl(key, buf, bufSize, outLen);
}

CHIP_ERROR NXPConfig::ReadConfigValueBin(const char * keyString, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    ReadRequest request{ buf, bufSize, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND, 0 };
    char key_name[SETTINGS_MAX_NAME_LEN + 1];
    unsigned key_name_len;

    // to be able to concat CHIP_DEVICE_STRING_SETTINGS_KEY"/" and keyString, + 1 for end char
    key_name_len = strlen(keyString) + strlen(CHIP_DEVICE_STRING_SETTINGS_KEY) + 1;
    VerifyOrReturnError(key_name_len <= (SETTINGS_MAX_NAME_LEN + 1), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    sprintf(key_name, CHIP_DEVICE_STRING_SETTINGS_KEY "/%s", keyString);
    settings_load_subtree_direct(key_name, ConfigValueCallback, &request);
    outLen = request.configSize;
    return request.result;
}

CHIP_ERROR NXPConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return ReadConfigValue(key, val);
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, bool val)
{
    return WriteConfigValueImpl(key, &val, sizeof(bool));
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueImpl(key, &val, sizeof(uint32_t));
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueImpl(key, &val, sizeof(uint64_t));
}

CHIP_ERROR NXPConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, str ? strlen(str) : 0);
}

CHIP_ERROR NXPConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    VerifyOrReturnError(ValidConfigKey(key), CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    return WriteConfigValueImpl(key, str, strLen);
}

CHIP_ERROR NXPConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueStr(key, (char *) data, dataLen);
}

CHIP_ERROR NXPConfig::WriteConfigValueBin(const char * keyString, const uint8_t * data, size_t dataLen)
{
    char key_name[SETTINGS_MAX_NAME_LEN + 1];
    unsigned key_name_len;

    // to be able to concat CHIP_DEVICE_STRING_SETTINGS_KEY"/" and keyString, + 1 for end char
    key_name_len = strlen(keyString) + strlen(CHIP_DEVICE_STRING_SETTINGS_KEY) + 1;
    VerifyOrReturnError(key_name_len <= (SETTINGS_MAX_NAME_LEN + 1), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    sprintf(key_name, CHIP_DEVICE_STRING_SETTINGS_KEY "/%s", keyString);
    if (settings_save_one(key_name, data, dataLen) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR NXPConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return WriteConfigValue(key, val);
}

CHIP_ERROR NXPConfig::ClearConfigValue(Key key)
{
    char key_name[SETTINGS_MAX_NAME_LEN + 1];
    sprintf(key_name, CHIP_DEVICE_INTEGER_SETTINGS_KEY "/%04x", key);
    if (settings_delete(key_name) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    return CHIP_NO_ERROR;
}

CHIP_ERROR NXPConfig::ClearConfigValue(const char * keyString)
{
    char key_name[SETTINGS_MAX_NAME_LEN];
    unsigned key_name_len;

    // to be able to concat CHIP_DEVICE_STRING_SETTINGS_KEY"/" and keyString, + 1 for end char
    key_name_len = strlen(keyString) + strlen(CHIP_DEVICE_STRING_SETTINGS_KEY) + 1;
    VerifyOrReturnError(key_name_len <= (SETTINGS_MAX_NAME_LEN + 1), CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    sprintf(key_name, CHIP_DEVICE_STRING_SETTINGS_KEY "/%s", keyString);
    if (settings_delete(key_name) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

bool NXPConfig::ConfigValueExists(Key key)
{
    bool err = false;
    if (ValidConfigKey(key))
    {
        size_t configSize;
        err = ReadConfigValueImpl(key, nullptr, 0, configSize) == CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return err;
}

CHIP_ERROR NXPConfig::FactoryResetConfig(void)
{
    DeleteSubtreeEntry entry{ /* success */ 0 };
    // Clear CHIP_DEVICE_STRING_SETTINGS_KEY/* keys
    int result = settings_load_subtree_direct(CHIP_DEVICE_STRING_SETTINGS_KEY, DeleteSubtreeCallback, &entry);

    if (result == 0)
    {
        result = entry.result;
    }

    char key_name[SETTINGS_MAX_NAME_LEN + 1];
    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        sprintf(key_name, CHIP_DEVICE_INTEGER_SETTINGS_KEY "/%04x", key);
        if (settings_delete(key_name) != 0)
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    // Clear RebootCount, TotalOperationalHours, UpTime counters during factory reset
    for (Key key = kMinConfigKey_ChipCounter; key <= (kMinConfigKey_ChipCounter + 3); key++)
    {
        sprintf(key_name, CHIP_DEVICE_INTEGER_SETTINGS_KEY "/%04x", key);
        if (settings_delete(key_name) != 0)
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
    return CHIP_NO_ERROR;
}

bool NXPConfig::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid CHIP Config PDM key range.
    return (key >= kMinConfigKey_ChipFactory) && (key <= kMaxConfigKey_KVS);
}

void NXPConfig::RunConfigUnitTest(void) {}

void NXPConfig::RunSystemIdleTask(void) {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
