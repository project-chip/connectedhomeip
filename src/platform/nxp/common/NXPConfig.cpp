/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2023 NXP
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

#include "NXPConfig.h"

#include "FreeRTOS.h"
#include "FunctionLib.h"
#include "board.h"
#include <lib/core/CHIPEncoding.h>
#include <platform/CHIPDeviceError.h>
#include <platform/internal/testing/ConfigUnitTest.h>

/* FS Writes in Idle task only - LittleFS only , already enabled by default on NVM */
#ifndef CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE
#define CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE 1
#endif

#define BUFFER_LOG_SIZE 256

/*
 * If the developer has specified a size for integer keys RAM buffer
 * partition, use it. Othewise use the default.
 */
#ifndef CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT
#define CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT (4 * 2048)
#endif

/*
 * If the developer has specified a size for string keys RAM buffer
 * partition, use it. Othewise use the default.
 */
#ifndef CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING
#define CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING (4 * 5000)
#endif

#ifndef NVM_ID_CHIP_CONFIG_DATA_KEY_INT
#define NVM_ID_CHIP_CONFIG_DATA_KEY_INT 0xf104
#endif

#ifndef NVM_ID_CHIP_CONFIG_DATA_KEY_STRING
#define NVM_ID_CHIP_CONFIG_DATA_KEY_STRING 0xf105
#endif

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

#define FREERTOS_TIMER_TICKS_2_MS(ticks) (((uint64_t) ticks) * portTICK_PERIOD_MS)

typedef struct
{
    uint16_t chipConfigRamBufferLen;
    uint16_t padding;
    uint8_t chipConfigRamBuffer[CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT] __attribute__((aligned(4)));
} ChipConfigRamStructKeyInt;

typedef struct
{
    uint16_t chipConfigRamBufferLen;
    uint16_t padding;
    uint8_t chipConfigRamBuffer[CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING] __attribute__((aligned(4)));
} ChipConfigRamStructKeyString;

/* File system containing only integer keys */
static ChipConfigRamStructKeyInt chipConfigRamStructKeyInt;
static ramBufferDescriptor ramDescrKeyInt = {
    .ramBufferLen    = &chipConfigRamStructKeyInt.chipConfigRamBufferLen,
    .ramBufferMaxLen = CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT,
    .pRamBuffer      = &chipConfigRamStructKeyInt.chipConfigRamBuffer[0],
};

/* File system containing only string keys */
static ChipConfigRamStructKeyString chipConfigRamStructKeyString;
static ramBufferDescriptor ramDescrKeyString = {
    .ramBufferLen    = &chipConfigRamStructKeyString.chipConfigRamBufferLen,
    .ramBufferMaxLen = CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING,
    .pRamBuffer      = &chipConfigRamStructKeyString.chipConfigRamBuffer[0],
};

static bool isInitialized = false;

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
NVM_RegisterDataSet((void *) &chipConfigRamStructKeyInt, 1, sizeof(chipConfigRamStructKeyInt), NVM_ID_CHIP_CONFIG_DATA_KEY_INT,
                    gNVM_MirroredInRam_c);
NVM_RegisterDataSet((void *) &chipConfigRamStructKeyString, 1, sizeof(chipConfigRamStructKeyString),
                    NVM_ID_CHIP_CONFIG_DATA_KEY_STRING, gNVM_MirroredInRam_c);

#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
const char * mt_key_int_file_name    = "mt_key_int";
const char * mt_key_str_file_name    = "mt_key_str";
#if CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE
static bool mt_key_int_save_in_flash = false;
static bool mt_key_str_save_in_flash = false;
#endif

#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

int NXPConfig::SaveIntKeysToFS(void)
{
    int err_len;
#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
    err_len = -1;
    NvSaveOnIdle(&chipConfigRamStructKeyInt, false);

#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
    err_len                  = -2;

#if CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE
    mt_key_int_save_in_flash = true;
#else

    /* Save it in flash now */
    err_len = ramStorageSavetoFlash(mt_key_int_file_name, &chipConfigRamStructKeyInt.chipConfigRamBuffer[0],
                                    chipConfigRamStructKeyInt.chipConfigRamBufferLen);

    assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT);
    assert(err_len >= 0);
#endif

#else

    err_len = -3;
#endif
    return err_len;
}

int NXPConfig::SaveStringKeysToFS(void)
{
    int err_len;
#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
    err_len = -1;
    NvSaveOnIdle(&chipConfigRamStructKeyString, false);

#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
    err_len                  = -2;
#if CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE
    mt_key_str_save_in_flash = true;
#else

    /* Save it in flash now */
    err_len = ramStorageSavetoFlash(mt_key_str_file_name, &chipConfigRamStructKeyString.chipConfigRamBuffer[0],
                                    chipConfigRamStructKeyString.chipConfigRamBufferLen);

    assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING);
    assert(err_len >= 0);
#endif

#else
    err_len = -3;
#endif

    return err_len;
}

CHIP_ERROR NXPConfig::Init()
{
    if (!isInitialized)
    {
        /*
         * Make sure to check that read buffers are always 4 bytes aligned,
         * as NXP flash drivers may mandate the alignment of dst read buffer to 4 bytes
         */
        static_assert(alignof(chipConfigRamStructKeyInt.chipConfigRamBuffer) == 4,
                      "Wrong buffer alignment, it must be 4 bytes aligned");
        static_assert(alignof(chipConfigRamStructKeyString.chipConfigRamBuffer) == 4,
                      "Wrong buffer alignment, it must be 4 bytes aligned");
        ramStorageInit();

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
        /* Init the NVM module */
        NvModuleInit();
#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
        int err_len;

        /* Init the NVM module */
        err_len = FS_Init();
        assert(err_len >= 0);
#endif

        FLib_MemSet((void *) &chipConfigRamStructKeyInt, 0, sizeof(chipConfigRamStructKeyInt));
        FLib_MemSet((void *) &chipConfigRamStructKeyString, 0, sizeof(chipConfigRamStructKeyString));

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
        /* Try to load the ot dataset in RAM */
        NvRestoreDataSet((void *) &chipConfigRamStructKeyInt, 0);
        NvRestoreDataSet((void *) &chipConfigRamStructKeyString, 0);

#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
        /* Try to load the ot dataset in RAM */
        err_len = ramStorageReadFromFlash(mt_key_int_file_name, &chipConfigRamStructKeyInt.chipConfigRamBuffer[0],
                                          CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT);
        assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT);
        assert(err_len >= 0);
        chipConfigRamStructKeyInt.chipConfigRamBufferLen = (uint16_t) err_len;

        err_len = ramStorageReadFromFlash(mt_key_str_file_name, &chipConfigRamStructKeyString.chipConfigRamBuffer[0],
                                          CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING);
        assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING);
        assert(err_len >= 0);
        chipConfigRamStructKeyString.chipConfigRamBufferLen = (uint16_t) err_len;

#endif
        isInitialized = true;
    }

    DBG_PRINTF("mt read %d / %d\r\n", chipConfigRamStructKeyInt.chipConfigRamBufferLen,
               chipConfigRamStructKeyString.chipConfigRamBufferLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NXPConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    bool tempVal;
    rsError status;
    uint16_t sizeToRead = sizeof(tempVal);

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageGet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), 0, (uint8_t *) &tempVal, &sizeToRead);
    SuccessOrExit(err = MapRamStorageStatus(status));
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
    uint32_t tempVal;
    rsError status;
    uint16_t sizeToRead = sizeof(tempVal);

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageGet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), 0, (uint8_t *) &tempVal, &sizeToRead);
    SuccessOrExit(err = MapRamStorageStatus(status));
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
    uint32_t tempVal;
    rsError status;
    uint16_t sizeToRead = sizeof(tempVal);

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageGet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), 0, (uint8_t *) &tempVal, &sizeToRead);
    SuccessOrExit(err = MapRamStorageStatus(status));
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
    rsError status;
    uint16_t sizeToRead = bufSize;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageGet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), 0, (uint8_t *) buf, &sizeToRead);
    SuccessOrExit(err = MapRamStorageStatus(status));
    outLen = sizeToRead;
#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "ReadConfigValueStr lenRead = %u", outLen);
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
    rsError status;
    uint16_t sizeToRead = bufSize;

    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageGet(&ramDescrKeyString, (const uint8_t *) keyString, strlen(keyString), 0, (uint8_t *) buf, &sizeToRead);
    SuccessOrExit(err = MapRamStorageStatus(status));
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
    rsError status;
    int err_len;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageSet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), (uint8_t *) &val, sizeof(bool));
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = SaveIntKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValue: MT write %d\r\n", err_len);

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;
    rsError status;
    int err_len;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageSet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), (uint8_t *) &val, sizeof(uint32_t));
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveIntKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValue: MT write %d\r\n", err_len);

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
#endif

exit:
    return err;
}

CHIP_ERROR NXPConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;
    rsError status;
    int err_len;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageSet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), (uint8_t *) &val, sizeof(uint64_t));
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveIntKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValue64: MT write %d\r\n", err_len);

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
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
    rsError status;
    int err_len;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageSet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), (uint8_t *) str, strLen);
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveIntKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValueStr: MT write %d\r\n", err_len);

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
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
    rsError status;
    int err_len;

    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageSet(&ramDescrKeyString, (const uint8_t *) keyString, strlen(keyString), (uint8_t *) data, dataLen);
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveStringKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValueBin: MT write %d\r\n", err_len);

#if (DEBUG_NVM > 0)
    ChipLogProgress(DeviceLayer, "WriteConfigValue done");
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
    rsError status;
    int err_len;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageDelete(&ramDescrKeyInt, (uint8_t *) &key, sizeof(key), 0);
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveIntKeysToFS();
    (void) err_len;
    DBG_PRINTF("ClearConfigValue: MT write %d\r\n", err_len);

exit:
    return err;
}

CHIP_ERROR NXPConfig::ClearConfigValue(const char * keyString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    rsError status;
    int err_len;

    VerifyOrExit(keyString != NULL, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.
    status = ramStorageDelete(&ramDescrKeyString, (const uint8_t *) keyString, strlen(keyString), 0);
    SuccessOrExit(err = MapRamStorageStatus(status));

    err_len = NXPConfig::SaveStringKeysToFS();
    (void) err_len;
    DBG_PRINTF("WriteConfigValueBin: MT write %d\r\n", err_len);

exit:
    return err;
}

bool NXPConfig::ConfigValueExists(Key key)
{
    rsError status;
    uint16_t sizeToRead;
    bool found = false;

    if (ValidConfigKey(key))
    {
        status = ramStorageGet(&ramDescrKeyInt, (uint8_t *) &key, sizeof(Key), 0, NULL, &sizeToRead);
        found  = (status == RS_ERROR_NONE && sizeToRead != 0);
    }
    return found;
}

CHIP_ERROR NXPConfig::FactoryResetConfig(void)
{

    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        ClearConfigValue(key);
    }

    // Clear RebootCount, TotalOperationalHours, UpTime counters during factory reset
    for (Key key = kMinConfigKey_ChipCounter; key <= (kMinConfigKey_ChipCounter + 3); key++)
    {
        ClearConfigValue(key);
    }

    /* Reset the key string file system as it contains on data that needs to be erased when doing a factoryreset */
    FLib_MemSet((void *) &chipConfigRamStructKeyString, 0, sizeof(chipConfigRamStructKeyString));

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
    /*
     * Save to flash now. System is restarting and there is no more time to
     * wait for the idle task to save the data.
     */
    NvSyncSave(&chipConfigRamStructKeyString, false);
    NvSyncSave(&chipConfigRamStructKeyInt, false);
#else
    SaveStringKeysToFS();
    SaveIntKeysToFS();
#endif
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

CHIP_ERROR NXPConfig::MapRamStorageStatus(rsError rsStatus)
{
    CHIP_ERROR err;

    switch (rsStatus)
    {
    case RS_ERROR_NONE:
        err = CHIP_NO_ERROR;
        break;
    case RS_ERROR_NOT_FOUND:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    case RS_ERROR_WRONG_ARG:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        assert(0);
        break;
    default:
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    }

    return err;
}

void NXPConfig::RunConfigUnitTest(void) {}

void NXPConfig::RunSystemIdleTask(void)
{

    if (isInitialized)
    {
#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
        NvIdle();

#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
#if CHIP_PLAT_SAVE_NVM_DATA_ON_IDLE
        if (mt_key_int_save_in_flash)
        {
            int err_len;

            /* Clear the flag first, so there could be an other write
                if the buffer is updated when the write in flash has not completed fully */
            mt_key_int_save_in_flash = false;

            /* Save it in flash now */
            err_len = ramStorageSavetoFlash(mt_key_int_file_name, &chipConfigRamStructKeyInt.chipConfigRamBuffer[0],
                                            chipConfigRamStructKeyInt.chipConfigRamBufferLen);

            assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT);
            assert(err_len >= 0);

            INFO_PRINTF("int mt write %d bytes / %d \r\n", err_len, chipConfigRamStructKeyInt.chipConfigRamBufferLen);
#if 0
            int len = ramStorageReadFromFlash(mt_key_int_file_name,
                &chipConfigRamStructKeyInt.chipConfigRamBuffer[0],
                CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_INT);
            INFO_PRINTF("mt read after write %d\r\n", len);
#endif
        }

        if (mt_key_str_save_in_flash)
        {
            int err_len;

            /* Clear the flag first, so there could be an other write
                if the buffer is updated when the write in flash has not completed fully */
            mt_key_str_save_in_flash = false;

            /* Save it in flash now */
            err_len = ramStorageSavetoFlash(mt_key_str_file_name, &chipConfigRamStructKeyString.chipConfigRamBuffer[0],
                                            chipConfigRamStructKeyString.chipConfigRamBufferLen);

            assert(err_len <= CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING);
            assert(err_len >= 0);

            INFO_PRINTF("str mt write %d bytes / %d \r\n", err_len, chipConfigRamStructKeyString.chipConfigRamBufferLen);
#if 0
            int len = ramStorageReadFromFlash(mt_key_str_file_name,
                &chipConfigRamStructKeyString.chipConfigRamBuffer[0],
                CONFIG_CHIP_NVM_RAMBUFFER_SIZE_KEY_STRING);
            INFO_PRINTF("mt read after write %d\r\n", len);
#endif
        }
#endif
#endif
    }

#if (defined(LOG_ENABLE) && (LOG_ENABLE > 0)) && ((defined LOG_ENABLE_ASYNC_MODE) && (LOG_ENABLE_ASYNC_MODE))
    LOG_Dump(bufferLog, sizeof(bufferLog), NULL);
#endif
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
