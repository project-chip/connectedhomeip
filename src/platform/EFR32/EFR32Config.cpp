/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Silicon Labs SDK.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/EFR32/EFR32Config.h>
#include <Weave/Core/WeaveEncoding.h>
#include <Weave/DeviceLayer/internal/testing/ConfigUnitTest.h>

#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "FreeRTOS.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

// Two macros are provided to support the creation of the Silicon Labs NVM3 area and
// initialization data- NVM3_DEFINE_SECTION_STATIC_DATA() and NVM3_DEFINE_SECTION_INIT_DATA().
// A linker section called 'name'_section is defined by NVM3_DEFINE_SECTION_STATIC_DATA().
// The NVM3 area is placed at the top of the device FLASH section by the linker
// script file: openweave-efr32-bringup-MG12P.ld. An error is returned
// by nvm3_open() on alignment or size violation.

// Local version of SDK macro (avoids uninitialized var compile error).
#define WEAVE_NVM3_DEFINE_SECTION_STATIC_DATA(name, nvmSize, cacheSize) \
    static nvm3_CacheEntry_t name##_cache[cacheSize];                   \
    static uint8_t           name##_nvm[nvmSize] SL_ATTRIBUTE_SECTION(STRINGIZE(name##_section))

// Local version of SDK macro (allows Weave to configure the maximum nvm3 object size and headroom).
#define WEAVE_NVM3_DEFINE_SECTION_INIT_DATA(name, maxObjectSize, repackHeadroom) \
    static nvm3_Init_t name = {                                                  \
        (nvm3_HalPtr_t)name##_nvm,                                               \
        sizeof(name##_nvm),                                                      \
        name##_cache,                                                            \
        sizeof(name##_cache) / sizeof(nvm3_CacheEntry_t),                        \
        maxObjectSize,                                                           \
        repackHeadroom,                                                          \
        &nvm3_halFlashHandle,                                                    \
    }

#define WEAVE_NVM3_REPACK_HEADROOM 64 // Threshold for User non-forced nvm3 flash repacking.

static nvm3_Handle_t handle;

// Declare NVM3 data area and cache.

WEAVE_NVM3_DEFINE_SECTION_STATIC_DATA(weaveNvm3,
                                      WEAVE_DEVICE_CONFIG_NVM3_NUM_FLASH_PAGES_FOR_STORAGE *FLASH_PAGE_SIZE,
                                      WEAVE_DEVICE_CONFIG_NVM3_MAX_NUM_OBJECTS);

WEAVE_NVM3_DEFINE_SECTION_INIT_DATA(weaveNvm3, WEAVE_DEVICE_CONFIG_NVM3_MAX_OBJECT_SIZE, WEAVE_NVM3_REPACK_HEADROOM);

WEAVE_ERROR EFR32Config::Init()
{
    WEAVE_ERROR err;
    bool        needClose = false;

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }

    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValue(Key key, bool &val)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;
    bool        tmpVal;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValue(Key key, uint32_t &val)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;
    uint32_t    tmpVal;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValue(Key key, uint64_t &val)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;
    uint64_t    tmpVal;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(&handle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValueStr(Key key, char *buf, size_t bufSize, size_t &outLen)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;

    outLen = 0;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = WEAVE_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into the output buffer- check buffer is
        // long enough to take the string (nvm3 string does not include the
        // terminator char).
        VerifyOrExit((bufSize > dataLen), err = WEAVE_ERROR_BUFFER_TOO_SMALL);

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
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValueBin(Key key, uint8_t *buf, size_t bufSize, size_t &outLen)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;

    outLen = 0;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = WEAVE_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into output buffer- check buffer is long
        // enough to take the data.
        VerifyOrExit((bufSize >= dataLen), err = WEAVE_ERROR_BUFFER_TOO_SMALL);

        err = MapNvm3Error(nvm3_readData(&handle, key, buf, dataLen));
        SuccessOrExit(err);
    }

    outLen = dataLen;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ReadConfigValueCounter(uint8_t counterIdx, uint32_t &val)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    tmpVal;

    Key key = kMinConfigKey_WeaveCounter + counterIdx;
    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Read bytes into tmp.
    err = MapNvm3Error(nvm3_readCounter(&handle, key, &tmpVal));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValue(Key key, bool val)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_ERROR_INVALID_ARGUMENT); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValue(Key key, uint32_t val)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValue(Key key, uint64_t val)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    err = MapNvm3Error(nvm3_writeData(&handle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValueStr(Key key, const char *str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

WEAVE_ERROR EFR32Config::WriteConfigValueStr(Key key, const char *str, size_t strLen)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

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
    if (needClose)
    {
        nvm3_close(&handle);
    }

    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValueBin(Key key, const uint8_t *data, size_t dataLen)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

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
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    Key key = kMinConfigKey_WeaveCounter + counterIdx;
    VerifyOrExit(ValidConfigKey(key), err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    err = MapNvm3Error(nvm3_writeCounter(&handle, key, val));
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

WEAVE_ERROR EFR32Config::ClearConfigValue(Key key)
{
    WEAVE_ERROR err;
    bool        needClose = false;

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Delete the nvm3 object with the given key id.
    err = MapNvm3Error(nvm3_deleteObject(&handle, key));
    SuccessOrExit(err);

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return err;
}

bool EFR32Config::ConfigValueExists(Key key)
{
    WEAVE_ERROR err;
    bool        needClose = false;
    uint32_t    objectType;
    size_t      dataLen;

    err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
    SuccessOrExit(err);
    needClose = true;

    // Find object with key id.
    err = MapNvm3Error(nvm3_getObjectInfo(&handle, key, &objectType, &dataLen));

exit:
    if (needClose)
    {
        nvm3_close(&handle);
    }
    return (err == WEAVE_NO_ERROR);
}

WEAVE_ERROR EFR32Config::FactoryResetConfig(void)
{
    // Deletes all nvm3 'Config' type objects.
    // Note- 'Factory' and 'Counter' type nvm3 objects are NOT deleted.

    WEAVE_ERROR err;

    // Iterate over all the Weave Config nvm3 records and delete each one...
    err = ForEachRecord(kMinConfigKey_WeaveConfig, kMaxConfigKey_WeaveConfig, false,
                        [](const Key &nvm3Key, const size_t &length) -> WEAVE_ERROR {
                            WEAVE_ERROR err2;

                            err2 = ClearConfigValue(nvm3Key);
                            SuccessOrExit(err2);

                        exit:
                            return err2;
                        });

    // Return success at end of iterations.
    if (err == WEAVE_END_OF_INPUT)
    {
        err = WEAVE_NO_ERROR;
    }

    return err;
}

WEAVE_ERROR EFR32Config::MapNvm3Error(Ecode_t nvm3Res)
{
    WEAVE_ERROR err;

    switch (nvm3Res)
    {
    case ECODE_NVM3_OK:
        err = WEAVE_NO_ERROR;
        break;
    case ECODE_NVM3_ERR_KEY_NOT_FOUND:
        err = WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    default:
        err = (nvm3Res & 0xFF) + WEAVE_DEVICE_CONFIG_EFR32_NVM3_ERROR_MIN;
        break;
    }

    return err;
}

WEAVE_ERROR EFR32Config::ForEachRecord(Key firstNvm3Key, Key lastNvm3Key, bool addNewRecord, ForEachRecordFunct funct)
{
    // Iterates through the specified range of nvm3 object key ids.
    // Invokes the callers CB function when appropriate.

    WEAVE_ERROR err = WEAVE_NO_ERROR;

    for (Key nvm3Key = firstNvm3Key; nvm3Key <= lastNvm3Key; ++nvm3Key)
    {
        Ecode_t  nvm3Res;
        uint32_t objectType;
        size_t   dataLen;

        // Open nvm3 handle for reading on each iteration.
        err = MapNvm3Error(nvm3_open(&handle, &weaveNvm3));
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
    // Always close handle.
    nvm3_close(&handle);

    return err;
}

bool EFR32Config::ValidConfigKey(Key key)
{
    // Returns true if the key is in the valid Weave Config nvm3 key range.

    if ((key >= kMinConfigKey_WeaveFactory) && (key <= kMaxConfigKey_WeaveCounter))
    {
        return true;
    }

    return false;
}

void EFR32Config::RunConfigUnitTest()
{
    // Run common unit test.
    ::nl::Weave::DeviceLayer::Internal::RunConfigUnitTest<EFR32Config>();
}

void EFR32Config::RepackNvm3Flash(void)
{
    // Repack nvm3 flash if nvm3 space < headroom threshold.
    // Note- checking periodically during idle periods should prevent
    // forced repack events on any write operation.
    nvm3_repack(&handle);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
