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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Qorvo QPG6100 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/qpg6100/qpg6100Config.h>

#include <core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "FreeRTOS.h"
#include "qvCHIP.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR QPG6100Config::Init()
{
    qvCHIP_Nvm_Init();

    return CHIP_NO_ERROR;
}

uint16_t QPG6100Config::GetSettingsMaxValueLength(Key key)
{
    return qvCHIP_Nvm_GetMaxKeyLen(key);
}

CHIP_ERROR QPG6100Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t length;
    bool res;

    length = sizeof(bool);

    res = qvCHIP_Nvm_Restore(key, (uint8_t *) (&val), &length);
    if (res == false)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR QPG6100Config::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t length;
    bool res;

    length = sizeof(uint32_t);

    res = qvCHIP_Nvm_Restore(key, (uint8_t *) (&val), &length);
    if (res == false)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR QPG6100Config::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t length;
    bool res;

    length = sizeof(uint64_t);

    res = qvCHIP_Nvm_Restore(key, (uint8_t *) (&val), &length);
    if (res == false)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR QPG6100Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool res;
    uint16_t length;

    if (buf == NULL)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    length = (uint16_t) bufSize;
    res    = qvCHIP_Nvm_Restore(key, (uint8_t *) buf, &length);
    if (length > bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    if (res == true)
    {
        outLen      = length;
        buf[outLen] = 0;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR QPG6100Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[255];
    uint16_t length;
    bool res;

    if (buf == NULL)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    length = (uint16_t) outLen;

    res = qvCHIP_Nvm_Restore(key, buffer, &length);
    if (res == true)
    {
        outLen = length;
        if (outLen > bufSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(buf, buffer, outLen);
        buf[outLen] = 0;
    }
    else
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR QPG6100Config::WriteConfigValue(Key key, bool val)
{
    uint16_t length;

    length = sizeof(bool);

    qvCHIP_Nvm_Backup(key, (uint8_t *) (&val), length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::WriteConfigValue(Key key, uint32_t val)
{
    uint16_t length;

    length = sizeof(uint32_t);

    qvCHIP_Nvm_Backup(key, (uint8_t *) (&val), length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::WriteConfigValue(Key key, uint64_t val)
{
    uint16_t length;

    length = sizeof(uint64_t);

    qvCHIP_Nvm_Backup(key, (uint8_t *) (&val), length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR QPG6100Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    if (str == NULL)
    {
        qvCHIP_Nvm_ClearValue(key);
    }
    else
    {
        qvCHIP_Nvm_Backup(key, (uint8_t *) str, (uint16_t) strLen);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    if (data == NULL)
    {
        qvCHIP_Nvm_ClearValue(key);
    }
    else
    {
        qvCHIP_Nvm_Backup(key, (uint8_t *) data, (uint16_t) dataLen);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::ClearConfigValue(Key key)
{
    qvCHIP_Nvm_ClearValue(key);
    return CHIP_NO_ERROR;
}

bool QPG6100Config::ConfigValueExists(Key key)
{
    return qvCHIP_Nvm_ValueExists(key, NULL);
}

CHIP_ERROR QPG6100Config::FactoryResetConfig(void)
{
    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        ClearConfigValue(key);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPG6100Config::ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t length;

    for (Key nvmKey = firstKey; nvmKey <= lastKey; ++nvmKey)
    {
        if (qvCHIP_Nvm_ValueExists(nvmKey, &length))
        {
            if (!addNewRecord)
            {
                // Invoke the caller's function
                // (for retrieve,store,delete,enumerate GroupKey operations).
                err = funct(nvmKey, length);
            }
        }
        else
        {
            if (addNewRecord)
            {
                // Invoke caller's function
                // (for add GroupKey operation).
                err = funct(nvmKey, length);
            }
        }

        SuccessOrExit(err);
    }

exit:
    return err;
}

void QPG6100Config::RunConfigUnitTest()
{
    // Run common unit test
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<QPG6100Config>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
