/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the  NXP SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "NXPConfig.h"

#include "FreeRTOS.h"
#include "FunctionLib.h"
#include "board.h"
#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "fwk_file_cache.h"
#include "fwk_key_storage.h"
#include "fwk_lfs_mflash.h"

#if defined(DEBUG_NVM) && (DEBUG_NVM == 2)
#include "fsl_debug_console.h"
#define DBG_PRINTF PRINTF
#define INFO_PRINTF PRINTF

#elif defined(DEBUG_NVM) && (DEBUG_NVM == 1)
#include "fsl_debug_console.h"
#define DBG_PRINTF PRINTF
#define INFO_PRINTF(...)

#else
#define DBG_PRINTF(...)
#define INFO_PRINTF(...)
#endif

/* Temporary namespace for integer and string keys */
#define NS_INT "_fki"
#define NS_STR "_fks"

/* Set to 1 if you want to see the statistics about the keys and their length */
#define ENABLE_KEYS_STATS 0

/* Size of the ram memory section for the KS configuration */
/* following the study of the size of the Matter key storage files,
we defined that a buffer size equal to 2265 is necessary. We decided to add a mark-up, bringing the buffer size to 5k to be safe */
#ifndef KS_MATTER_SCRATCH_AREA_SIZE_MAX
#define KS_MATTER_SCRATCH_AREA_SIZE_MAX 5 * 1024
#endif

static uint8_t mem_section[KS_MATTER_SCRATCH_AREA_SIZE_MAX];
static ks_config_t ks_config = {
    .size = KS_MATTER_SCRATCH_AREA_SIZE_MAX, .KS_name = "KSconf", .mem_p = mem_section,
    //.mem_p = NULL,
};
static void * ks_handle_p = NULL;

static bool isInitialized = false;

#if (ENABLE_KEYS_STATS == 1)
typedef struct
{
    uint16_t writtenKeys_str;
    uint16_t deletedKeys_str;
    uint16_t writtenKeys_int;
    uint16_t deletedKeys_int;
} ks_keys_stats_t;
static ks_keys_stats_t keys_stats = { 0, 0, 0, 0 };
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR NXPConfig::Init()
{
    if (!isInitialized)
    {
        ks_handle_p   = KS_Init(&ks_config);
        isInitialized = true;
    }

    DBG_PRINTF("Init");
    return CHIP_NO_ERROR;
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int req_len;
    int outLen;
    bool tempVal;

    req_len = sizeof(bool);
    outLen  = 0;
    tempVal = false;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_GetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &tempVal, req_len, &outLen);
    SuccessOrExit(err = MapKeyStorageStatus(status));
    val = tempVal;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValue bool = %u", val);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int req_len;
    int outLen;
    uint32_t tempVal;

    req_len = sizeof(uint32_t);
    outLen  = 0;
    tempVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_GetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &tempVal, req_len, &outLen);
    SuccessOrExit(err = MapKeyStorageStatus(status));
    val = tempVal;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValue uint32_t = %lu", val);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int req_len;
    int outLen;
    uint64_t tempVal;

    req_len = sizeof(uint64_t);
    outLen  = 0;
    tempVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_GetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &tempVal, req_len, &outLen);
    SuccessOrExit(err = MapKeyStorageStatus(status));
    val = tempVal;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValue uint64_t = " ChipLogFormatX64, ChipLogValueX64(val));
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    ks_error_t status;
    uint32_t sizeToRead;

    sizeToRead = bufSize;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_GetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) buf, (int) bufSize, (int *) &sizeToRead);
    SuccessOrExit(err = MapKeyStorageStatus(status));
    outLen = sizeToRead;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValueStr bufSize = %u, lenRead = %u", bufSize, outLen);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueStr(key, (char *) buf, bufSize, outLen);
}

CHIP_ERROR NXPConfig::ReadConfigValueBin(const char * keyString, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    ks_error_t status;
    uint32_t sizeToRead;

    sizeToRead = bufSize;
    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_GetKeyString(ks_handle_p, (char *) keyString, strlen(keyString) + 1, (char *) NS_STR, (void *) buf, (int) bufSize,
                             (int *) &sizeToRead); // +1 to add end \0 char
    SuccessOrExit(err = MapKeyStorageStatus(status));
    outLen = sizeToRead;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValueStr lenRead = %u", outLen);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return ReadConfigValue(key, val);
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int valSize;

    valSize = sizeof(bool);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_SetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &val, valSize);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValue: MT write \r\n");

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.writtenKeys_int++;
    ChipLogProgress(DeviceLayer, "Data len: %u. Integer keys written until now: %u.", valSize, keys_stats.writtenKeys_int);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int valSize;

    valSize = sizeof(uint32_t);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_SetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &val, valSize);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValue: MT write \r\n");

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.writtenKeys_int++;
    ChipLogProgress(DeviceLayer, "Data len: %u. Integer keys written until now: %u.", valSize, keys_stats.writtenKeys_int);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;
    ks_error_t status;
    int valSize;

    valSize = sizeof(uint64_t);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_SetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) &val, valSize);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValue64: MT write \r\n");

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.writtenKeys_int++;
    ChipLogProgress(DeviceLayer, "Data len: %u. Integer keys written until now: %u.", valSize, keys_stats.writtenKeys_int);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR NXPConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    ks_error_t status;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_SetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, (void *) str, strLen);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValueStr: MT write \r\n");

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.writtenKeys_int++;
    ChipLogProgress(DeviceLayer, "Data len: %u. Integer keys written until now: %u.", strLen, keys_stats.writtenKeys_int);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueStr(key, (char *) data, dataLen);
}

CHIP_ERROR NXPConfig::WriteConfigValueBin(const char * keyString, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;
    ks_error_t status;
    int keyLen;

    keyLen = (int) strlen(keyString) + 1;                                      // +1 to add end \0 char
    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_SetKeyString(ks_handle_p, (char *) keyString, keyLen, (char *) NS_STR, (void *) data, (int) dataLen);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValueBin: MT write \r\n");

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.writtenKeys_str++;
    ChipLogProgress(DeviceLayer, "Data len: %u. Key len: %u. String keys written until now: %u.", dataLen, keyLen,
                    keys_stats.writtenKeys_str);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    Key key = kMinConfigKey_ChipCounter + counterIdx;
    return WriteConfigValue(key, val);
}

CHIP_ERROR NXPConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ks_error_t status;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_DeleteKeyInt(ks_handle_p, (int) key, (char *) NS_INT);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("ClearConfigValue: MT write \r\n");

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.deletedKeys_int++;
    ChipLogProgress(DeviceLayer, "Integer keys deleted until now: %u.", keys_stats.deletedKeys_int);
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::ClearConfigValue(const char * keyString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ks_error_t status;
    int keyLen;

    keyLen = strlen(keyString) + 1;                                            // +1 to add end \0 char
    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = KS_DeleteKeyString(ks_handle_p, (char *) keyString, keyLen, (char *) NS_STR);
    SuccessOrExit(err = MapKeyStorageStatus(status));

    DBG_PRINTF("WriteConfigValueBin: MT write \r\n");

#if (ENABLE_KEYS_STATS == 1)
    keys_stats.deletedKeys_str++;
    ChipLogProgress(DeviceLayer, "String keys deleted until now: %u.", keys_stats.deletedKeys_str);
#endif

exit:
    return err;
}

bool NXPConfig::ConfigValueExists(Key key)
{
    ks_error_t status;
    bool found;
    void * readValue_p;
    int outLen;
    int bufSize;

    found       = false;
    readValue_p = NULL;
    outLen      = 0;
    /* Max number of bytes read when getting a value */
    bufSize = 256;

    if (ValidConfigKey(key))
    {
        /* Get the first occurence */
        status = KS_GetKeyInt(ks_handle_p, (int) key, (char *) NS_INT, readValue_p, bufSize, &outLen);
        found  = (status == KS_ERROR_NONE && outLen != 0);
    }
    return found;
}

CHIP_ERROR NXPConfig::FactoryResetConfig(void)
{
    /*for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        ClearConfigValue(key);
    }*/

    KS_Reset(ks_handle_p);

    DBG_PRINTF("FactoryResetConfig done\r\n");

    return CHIP_NO_ERROR;
}

bool NXPConfig::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid CHIP Config PDM key range.

    if ((key >= kMinConfigKey_ChipFactory) && (key <= kMaxConfigKey_KVS))
    {
        return true;
    }

    return false;
}

CHIP_ERROR NXPConfig::MapKeyStorageStatus(ks_error_t ksStatus)
{
    CHIP_ERROR err;

    switch (ksStatus)
    {
    case KS_ERROR_NONE:
        err = CHIP_NO_ERROR;
        break;
    case KS_ERROR_BUF_TOO_SMALL:
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    default: /* KS_ERROR_KEY_NOT_FOUND */
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    }

    return err;
}

void NXPConfig::RunConfigUnitTest(void) {}

void NXPConfig::RunSystemIdleTask(void)
{
    if (isInitialized)
    {
        FC_Process();
        INFO_PRINTF("str mt write  \r\n");
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
