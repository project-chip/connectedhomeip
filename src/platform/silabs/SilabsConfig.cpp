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
#include <platform/silabs/SilabsConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <platform/silabs/CHIPDevicePlatformConfig.h>

#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include <nvm3_lock.h>

#include <FreeRTOS.h>
#include <semphr.h>
// Substitute the GSDK weak nvm3_lockBegin and nvm3_lockEnd
// for an application controlled re-entrance protection
static SemaphoreHandle_t nvm3_Sem;
static StaticSemaphore_t nvm3_SemStruct;

void nvm3_lockBegin(void)
{
    if (nvm3_Sem == NULL)
    {
        nvm3_Sem = xSemaphoreCreateBinaryStatic(&nvm3_SemStruct);
        xSemaphoreGive(nvm3_Sem);
    }

    VerifyOrDie(nvm3_Sem != NULL);
    xSemaphoreTake(nvm3_Sem, portMAX_DELAY);
}

void nvm3_lockEnd(void)
{
    VerifyOrDie(nvm3_Sem != NULL);
    xSemaphoreGive(nvm3_Sem);
}

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Matter NVM3 space is placed in the silabs default nvm3 section shared with other stack.
// 'kMatterNvm3KeyDomain' identify the matter nvm3 domain.
// The NVM3 default section is placed at end of Flash minus 1 page byt the linker file
// See examples/platform/efr32/ldscripts/efr32mgXX.ld

CHIP_ERROR SilabsConfig::Init()
{
    // nvm3_Sem is created in nvm3_lockBegin()

    return MapNvm3Error(nvm3_open(nvm3_defaultHandle, nvm3_defaultInit));
}

void SilabsConfig::DeInit()
{
    vSemaphoreDelete(nvm3_Sem);
    nvm3_close(nvm3_defaultHandle);
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    bool tmpVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint16_t & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    uint16_t tmpVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    uint32_t tmpVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;
    uint64_t tmpVal = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    // Read nvm3 bytes into tmp.
    err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, &tmpVal, dataLen));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = CHIP_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into the output buffer- check buffer is
        // long enough to take the string (nvm3 string does not include the
        // terminator char).
        VerifyOrExit((bufSize > dataLen), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, buf, dataLen));
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
            err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, &firstByte, 1));
            SuccessOrExit(err);

            outLen = (firstByte == 0) ? 0 : dataLen;
        }
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);
    VerifyOrExit(dataLen > 0, err = CHIP_ERROR_INVALID_STRING_LENGTH);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into output buffer- check buffer is long
        // enough to take the data.
        VerifyOrExit((bufSize >= dataLen), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        err = MapNvm3Error(nvm3_readData(nvm3_defaultHandle, key, buf, dataLen));
        SuccessOrExit(err);

        outLen = dataLen;
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset)
{
    CHIP_ERROR err;
    uint32_t objectType;
    size_t dataLen;

    outLen = 0;
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Get nvm3 object info.
    err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    SuccessOrExit(err);

    if (buf != NULL)
    {
        // Read nvm3 bytes directly into output buffer- check buffer is long enough to take the data
        // else read what we can but return CHIP_ERROR_BUFFER_TOO_SMALL.
        size_t maxReadLength = dataLen - offset;
        if (bufSize >= maxReadLength)
        {
            err = MapNvm3Error(nvm3_readPartialData(nvm3_defaultHandle, key, buf, offset, maxReadLength));
            SuccessOrExit(err);
            outLen = maxReadLength;
        }
        else
        {
            err = MapNvm3Error(nvm3_readPartialData(nvm3_defaultHandle, key, buf, offset, bufSize));
            SuccessOrExit(err);
            // read was successful, but we did not read all the data from the object.
            err    = CHIP_ERROR_BUFFER_TOO_SMALL;
            outLen = bufSize;
        }
    }
exit:
    return err;
}

CHIP_ERROR SilabsConfig::ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val)
{
    CHIP_ERROR err;
    uint32_t tmpVal = 0;
    Key key         = kMinConfigKey_MatterCounter + counterIdx;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Read bytes into tmp.
    err = MapNvm3Error(nvm3_readCounter(nvm3_defaultHandle, key, &tmpVal));
    SuccessOrExit(err);
    val = tmpVal;

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_ERROR_INVALID_ARGUMENT); // Verify key id.

    err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint16_t val)
{
    CHIP_ERROR err;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, &val, sizeof(val)));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR SilabsConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    if (str != NULL)
    {
        // Write the string to nvm3 without the terminator char (apart from
        // empty strings where only the terminator char is stored in nvm3).
        err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, str, (strLen > 0) ? strLen : 1));
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    // Only write NULL pointer if the given size is 0, since in that case, nothing is read at the pointer
    if ((data != NULL) || (dataLen == 0))
    {
        // Write the binary data to nvm3.
        err = MapNvm3Error(nvm3_writeData(nvm3_defaultHandle, key, data, dataLen));
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR SilabsConfig::WriteConfigValueCounter(uint8_t counterIdx, uint32_t val)
{
    CHIP_ERROR err;
    Key key = kMinConfigKey_MatterCounter + counterIdx;

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND); // Verify key id.

    err = MapNvm3Error(nvm3_writeCounter(nvm3_defaultHandle, key, val));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SilabsConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err;

    // Delete the nvm3 object with the given key id.
    err = MapNvm3Error(nvm3_deleteObject(nvm3_defaultHandle, key));
    SuccessOrExit(err);

exit:
    return err;
}

bool SilabsConfig::ConfigValueExists(Key key)
{
    uint32_t objectType;
    size_t dataLen;

    // Find object with key id.
    CHIP_ERROR err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dataLen));
    return (err == CHIP_NO_ERROR);
}

bool SilabsConfig::ConfigValueExists(Key key, size_t & dataLen)
{
    uint32_t objectType;
    size_t dLen;

    // Find object with key id.
    CHIP_ERROR err = MapNvm3Error(nvm3_getObjectInfo(nvm3_defaultHandle, key, &objectType, &dLen));

    if (err == CHIP_NO_ERROR)
    {
        dataLen = dLen;
    }

    return (err == CHIP_NO_ERROR);
}

CHIP_ERROR SilabsConfig::FactoryResetConfig(void)
{
    // Deletes all nvm3 'Config' type objects.
    // Note- 'Factory' and 'Counter' type nvm3 objects are NOT deleted.

    CHIP_ERROR err;

    // Iterate over all the CHIP Config nvm3 records and delete each one...
    err = ForEachRecord(kMinConfigKey_MatterConfig, kMaxConfigKey_MatterConfig, false,
                        [](const Key & nvm3Key, const size_t & length) -> CHIP_ERROR {
                            CHIP_ERROR err2;
                            // Delete the nvm3 object with the given key id.
                            err2 = ClearConfigValue(nvm3Key);
                            SuccessOrExit(err2);

                        exit:
                            return err2;
                        });

    return err;
}

CHIP_ERROR SilabsConfig::MapNvm3Error(Ecode_t nvm3Res)
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
        err = CHIP_ERROR(ChipError::Range::kPlatform, (nvm3Res & 0xFF) + CHIP_DEVICE_CONFIG_SILABS_NVM3_ERROR_MIN);
        break;
    }

    return err;
}

CHIP_ERROR SilabsConfig::ForEachRecord(Key firstNvm3Key, Key lastNvm3Key, bool addNewRecord, ForEachRecordFunct funct)
{
    // Iterates through the specified range of nvm3 object key ids.
    // Invokes the callers CB function when appropriate.

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (Key nvm3Key = firstNvm3Key; nvm3Key <= lastNvm3Key; ++nvm3Key)
    {
        Ecode_t nvm3Res;
        uint32_t objectType;
        size_t dataLen;

        // Find nvm3 object with current nvm3 iteration key.
        nvm3Res = nvm3_getObjectInfo(nvm3_defaultHandle, nvm3Key, &objectType, &dataLen);
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

exit:;
    return err;
}

bool SilabsConfig::ValidConfigKey(Key key)
{
    // Returns true if the key is in the Matter nvm3 reserved key range.
    // or if the key is in the User Domain key range
    // Additional check validates that the user consciously defined the expected key range
    if (((key >= kMatterNvm3KeyLoLimit) && (key <= kMatterNvm3KeyHiLimit) && (key >= kMinConfigKey_MatterFactory) &&
         (key <= kMaxConfigKey_MatterKvs)) ||
        ((key >= kUserNvm3KeyDomainLoLimit) && (key <= kUserNvm3KeyDomainHiLimit)))
    {
        return true;
    }

    return false;
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
void SilabsConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<SilabsConfig>();
}
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

void SilabsConfig::RepackNvm3Flash(void)
{
    // Repack nvm3 flash if nvm3 space < headroom threshold.
    // Note- checking periodically during idle periods should prevent
    // forced repack events on any write operation.
    nvm3_repack(nvm3_defaultHandle);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
