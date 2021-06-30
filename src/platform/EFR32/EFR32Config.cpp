/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          platforms based on the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/EFR32/EFR32Config.h>

#include <core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"
#include "nvm3.h"
#include "nvm3_hal_flash.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Two macros are provided to support the creation of the Silicon Labs NVM3 area and
// initialization data- NVM3_DEFINE_SECTION_STATIC_DATA() and NVM3_DEFINE_SECTION_INIT_DATA().
// A linker section called 'name'_section is defined by NVM3_DEFINE_SECTION_STATIC_DATA().
// The NVM3 area is placed at the top of the device FLASH section by the linker
// script file: chip-efr32-bringup-MG12P.ld. An error is returned
// by nvm3_open() on alignment or size violation.

// Local version of SDK macro (avoids uninitialized var compile error).
#define CHIP_NVM3_DEFINE_SECTION_STATIC_DATA(name, nvmSize, cacheSize)                                                             \
    static nvm3_CacheEntry_t name##_cache[cacheSize];                                                                              \
    static uint8_t name##_nvm[nvmSize] SL_ATTRIBUTE_SECTION(STRINGIZE(name##_section))

// Local version of SDK macro (allows CHIP to configure the maximum nvm3 object size and headroom).
#define CHIP_NVM3_DEFINE_SECTION_INIT_DATA(name, maxObjectSize, repackHeadroom)                                                    \
    static nvm3_Init_t name = {                                                                                                    \
        (nvm3_HalPtr_t) name##_nvm,                                                                                                \
        sizeof(name##_nvm),                                                                                                        \
        name##_cache,                                                                                                              \
        sizeof(name##_cache) / sizeof(nvm3_CacheEntry_t),                                                                          \
        maxObjectSize,                                                                                                             \
        repackHeadroom,                                                                                                            \
        &nvm3_halFlashHandle,                                                                                                      \
    }

#define CHIP_NVM3_REPACK_HEADROOM 64 // Threshold for User non-forced nvm3 flash repacking.

#define EFR32_SEM_TIMEOUT_ms 5

static nvm3_Handle_t handle;
static SemaphoreHandle_t nvm3_Sem;
static StaticSemaphore_t nvm3_SemStruct;

// Declare NVM3 data area and cache.

CHIP_NVM3_DEFINE_SECTION_STATIC_DATA(chipNvm3, CHIP_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE * FLASH_PAGE_SIZE,
                                     CHIP_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS);

CHIP_NVM3_DEFINE_SECTION_INIT_DATA(chipNvm3, CHIP_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE, CHIP_NVM3_REPACK_HEADROOM);

CHIP_ERROR EFR32Config::Init()
{
    CHIP_ERROR err;

    nvm3_Sem = xSemaphoreCreateBinaryStatic(&nvm3_SemStruct);

    if (nvm3_Sem == NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    bool tmpVal;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    uint32_t tmpVal;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    uint64_t tmpVal;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = CHIP_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into the output buffer- check buffer is
        // long enough to take the string (nvm3 string does not include the
        // terminator char).
        VerifyOrExit((bufSize > dataLen), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        err = MapNvm3Error(nvm3_readData(&handle, key, buf, dataLen));
        SuccessOrExit(err);

        outLen      = ((dataLen == 1) && (buf[0] == 0)) ? 0 : dataLen;
        buf[outLen] = 0; // Add the terminator char.
    }
    else
    {
        if (dataLen > 1)
        {
            outLen = dataLen;
        }
        else
        {
            // Read the first byte of the nvm3 string into a tmp var.
            char firstByte;
            err = MapNvm3Error(nvm3_readData(&handle, key, &firstByte, 1));
            SuccessOrExit(err);

            outLen = (firstByte == 0) ? 0 : dataLen;
        }
    }

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;
    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = CHIP_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into output buffer- check buffer is long
        // enough to take the data.
        VerifyOrExit((bufSize >= dataLen), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        err = MapNvm3Error(nvm3_readData(&handle, key, buf, dataLen));
        SuccessOrExit(err);
    }

    outLen = dataLen;

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t tmpVal;
    Key key = kMinConfigKey_ChipCounter + counterIdx;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Read bytes into tmp.
    err = MapNvm3Error(nvm3_readCounter(&handle, key, &tmpVal));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_ERROR_INVALID_ARGUMENT); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR EFR32Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    if (str != NULL)
    {
        // Write the string to nvm3 without the terminator char (apart from
        // empty strings where only the terminator char is stored in nvm3).
        err = MapNvm3Error(nvm3_writeData(&handle, key, str, (strLen > 0) ? strLen : 1));
        SuccessOrExit(err);
    }
    else
    {
        nvm3_deleteObject(&handle, key); // no error checking here.
    }

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    if (data != NULL)
    {
        if (dataLen > 0)
        {
            // Write the binary data to nvm3.
            err = MapNvm3Error(nvm3_writeData(&handle, key, data, dataLen));
            SuccessOrExit(err);
        }
    }
    else
    {
        nvm3_deleteObject(&handle, key); // no error checking here.
    }

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    CHIP_ERROR err;
    Key key = kMinConfigKey_ChipCounter + counterIdx;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    err = MapNvm3Error(nvm3_writeCounter(&handle, key, val));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

CHIP_ERROR EFR32Config::ClearConfigValue(Key key)
{
    CHIP_ERROR err;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Delete the nvm3 object with the given key id.
    err = MapNvm3Error(nvm3_deleteObject(&handle, key));
    SuccessOrExit(err);

exit:
    OnExit();
    return err;
}

bool EFR32Config::ConfigValueExists(Key key)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
    SuccessOrExit(err);

    // Find object with key id.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));

exit:
    OnExit();
    return (err == CHIP_NO_ERROR);
}

CHIP_ERROR EFR32Config::FactoryResetConfig(void)
{
    // Deletes all nvm3 'Config' type objects.
    // Note- 'Factory' and 'Counter' type nvm3 objects are NOT deleted.

    CHIP_ERROR err;

    // Iterate over all the CHIP Config nvm3 records and delete each one...
    err = ForEachRecord(kMinConfigKey_ChipConfig, kMaxConfigKey_ChipConfig, false,
                        [](const Key & nvm3Key, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;

                            err2 = ClearConfigValue(nvm3Key);
                            SuccessOrExit(err2);

                        exit:
                            return err2;
                        });

    // Return success at end of iterations.
    if (err == CHIP_END_OF_INPUT)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR EFR32Config::MapNvm3Error(Ecode_t nvm3Res)
{
    CHIP_ERROR err;

    switch (nvm3Res)
    {
    case ECODE_NVM3_OK:
        err = CHIP_NO_ERROR;
        break;
    case ECODE_NVM3_ERR_KEY_NOT_FOUND:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    default:
        err = static_cast<CHIP_ERROR>((nvm3Res & 0xFF) + CHIP_DEVICE_CONFIG_EFR32_NVM3_ERROR_MIN);
        break;
    }

    return err;
}

CHIP_ERROR EFR32Config::ForEachRecord(Key firstNvm3Key, Key lastNvm3Key, bool addNewRecord, ForEachRecordFunct funct)
{
    // Iterates through the specified range of nvm3 object key ids.
    // Invokes the callers CB function when appropriate.

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (pdFALSE == xSemaphoreTake(nvm3_Sem, pdMS_TO_TICKS(EFR32_SEM_TIMEOUT_ms)))
    {
        err = CHIP_ERROR_TIMEOUT;
        SuccessOrExit(err);
    }

    for (Key nvm3Key = firstNvm3Key; nvm3Key <= lastNvm3Key; ++nvm3Key)
    {
        Ecode_t nvm3Res;
        uint32_t objectType;
        size_t dataLen;

        // Open nvm3 handle for reading on each iteration.
        err = MapNvm3Error(nvm3_open(&handle, &chipNvm3));
        SuccessOrExit(err);

        // Find nvm3 object with current nvm3 iteration key.
        nvm3Res = nvm3_getObjectInfo(&handle, nvm3Key, &objectType, &dataLen);
        switch (nvm3Res)
        {
        case ECODE_NVM3_OK:
            if (!addNewRecord)
            {
                // Invoke the caller's function
                // (for retrieve,store,delete,enumerate GroupKey operations).
                err = funct(nvm3Key, dataLen);
            }
            break;
        case ECODE_NVM3_ERR_KEY_NOT_FOUND:
            if (addNewRecord)
            {
                // Invoke caller's function
                // (for add GroupKey operation).
                err = funct(nvm3Key, dataLen);
            }
            break;
        default:
            err = MapNvm3Error(nvm3Res);
            break;
        }

        SuccessOrExit(err);
    }

exit:
    OnExit();
    return err;
}

bool EFR32Config::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid CHIP Config nvm3 key range.

    if ((key >= kMinConfigKey_ChipFactory) && (key <= kMaxConfigKey_ChipCounter))
    {
        return true;
    }

    return false;
}

void EFR32Config::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<EFR32Config>();
}

void EFR32Config::RepackNvm3Flash(void)
{
    // Repack nvm3 flash if nvm3 space < headroom threshold.
    // Note- checking periodically during idle periods should prevent
    // forced repack events on any write operation.
    nvm3_repack(&handle);
}

void EFR32Config::OnExit()
{
    xSemaphoreGive(nvm3_Sem);
    nvm3_close(&handle);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
