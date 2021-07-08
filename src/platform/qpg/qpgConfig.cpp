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
 *          platforms based on the Qorvo QPG platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/qpg/qpgConfig.h>

#include <core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <platform/internal/testing/ConfigUnitTest.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "FreeRTOS.h"
#include "qvCHIP.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR QPGConfig::Init()
{
    qvCHIP_Nvm_Init();

    return CHIP_NO_ERROR;
}

uint16_t QPGConfig::GetSettingsMaxValueLength(Key key)
{
    uint16_t keyLen;

    return (qvCHIP_Nvm_GetMaxKeyLen(key, &keyLen) == QV_STATUS_NO_ERROR) ? keyLen : 0;
}

CHIP_ERROR QPGConfig::ReadConfigValue(Key key, bool & val)
{
    uint16_t length;

    length = sizeof(bool);

    return MapNVMError(qvCHIP_Nvm_Restore(key, reinterpret_cast<uint8_t *>(&val), &length));
}

CHIP_ERROR QPGConfig::ReadConfigValue(Key key, uint32_t & val)
{
    uint16_t length;

    length = sizeof(uint32_t);

    return MapNVMError(qvCHIP_Nvm_Restore(key, reinterpret_cast<uint8_t *>(&val), &length));
}

CHIP_ERROR QPGConfig::ReadConfigValue(Key key, uint64_t & val)
{
    uint16_t length;

    length = sizeof(uint64_t);

    return MapNVMError(qvCHIP_Nvm_Restore(key, reinterpret_cast<uint8_t *>(&val), &length));
}

CHIP_ERROR QPGConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    qvStatus_t res;
    uint16_t length;

    if ((buf == NULL) || (bufSize > UINT16_MAX))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    length = static_cast<uint16_t>(bufSize);
    res    = qvCHIP_Nvm_Restore(key, Uint8::from_char(buf), &length);
    if (length > bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    if (res == QV_STATUS_NO_ERROR)
    {
        outLen      = length;
        buf[outLen] = 0;
    }

    return MapNVMError(res);
}

CHIP_ERROR QPGConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    uint8_t buffer[255];
    uint16_t length;
    qvStatus_t res;

    if ((buf == NULL) || (bufSize > UINT16_MAX))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    length = static_cast<uint16_t>(bufSize);

    res = qvCHIP_Nvm_Restore(key, buffer, &length);
    if (res == QV_STATUS_NO_ERROR)
    {
        outLen = length;
        if (outLen > bufSize)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(buf, buffer, outLen);
        buf[outLen] = 0;
    }

    return MapNVMError(res);
}

CHIP_ERROR QPGConfig::WriteConfigValue(Key key, bool val)
{
    uint16_t length;

    length = sizeof(bool);

    return MapNVMError(qvCHIP_Nvm_Backup(key, reinterpret_cast<uint8_t *>(&val), length));
}

CHIP_ERROR QPGConfig::WriteConfigValue(Key key, uint32_t val)
{
    uint16_t length;

    length = sizeof(uint32_t);

    return MapNVMError(qvCHIP_Nvm_Backup(key, reinterpret_cast<uint8_t *>(&val), length));
}

CHIP_ERROR QPGConfig::WriteConfigValue(Key key, uint64_t val)
{
    uint16_t length;

    length = sizeof(uint64_t);

    return MapNVMError(qvCHIP_Nvm_Backup(key, reinterpret_cast<uint8_t *>(&val), length));
}

CHIP_ERROR QPGConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR QPGConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    if (strLen > UINT16_MAX)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (str == NULL)
    {
        qvCHIP_Nvm_ClearValue(key);
    }
    else
    {
        return MapNVMError(qvCHIP_Nvm_Backup(key, Uint8::from_const_char(str), static_cast<uint16_t>(strLen)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPGConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    if (dataLen > UINT16_MAX)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    if (data == NULL)
    {
        qvCHIP_Nvm_ClearValue(key);
    }
    else
    {
        return MapNVMError(qvCHIP_Nvm_Backup(key, data, static_cast<uint16_t>(dataLen)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPGConfig::ClearConfigValue(Key key)
{
    qvCHIP_Nvm_ClearValue(key);
    return CHIP_NO_ERROR;
}

bool QPGConfig::ConfigValueExists(Key key)
{
    return qvCHIP_Nvm_ValueExists(key, NULL);
}

CHIP_ERROR QPGConfig::FactoryResetConfig(void)
{
    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        ClearConfigValue(key);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR QPGConfig::ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct)
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

CHIP_ERROR QPGConfig::MapNVMError(qvStatus_t aStatus)
{
    switch (aStatus)
    {
    case QV_STATUS_NO_ERROR:
        return CHIP_NO_ERROR;
    case QV_STATUS_BUFFER_TOO_SMALL:
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case QV_STATUS_INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case QV_STATUS_KEY_LEN_TOO_SMALL:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    case QV_STATUS_INVALID_DATA:
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL;
}

void QPGConfig::RunConfigUnitTest()
{
    // Run common unit test
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<QPGConfig>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
