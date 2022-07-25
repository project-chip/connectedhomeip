/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          platforms based on the  MW320 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/nxp/mw320/MW320Config.h>

#include <lib/core/CHIPEncoding.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include "FreeRTOS.h"

extern "C" {
#include <network_flash_storage.h>
}

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define kMaxKeyValueBytes 2048
#define keyNameBytes 128

CHIP_ERROR MW320Config::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, bool & val)
{
    uint32_t ret;
    uint32_t read_size = sizeof(bool);
    char keyname[keyNameBytes];
    snprintf(keyname, keyNameBytes, "%08lx", key);

    ret = ::get_saved_wifi_network(keyname, (uint8_t *) (&val), &read_size);
    VerifyOrReturnError(ret == 0, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t ret;
    uint32_t read_size = sizeof(uint32_t);
    char keyname[keyNameBytes];
    snprintf(keyname, keyNameBytes, "%08lx", key);

    ret = ::get_saved_wifi_network(keyname, (uint8_t *) (&val), &read_size);
    VerifyOrReturnError(ret == 0, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, uint64_t & val)
{
    uint32_t ret;
    uint32_t read_size = sizeof(uint64_t);
    char keyname[keyNameBytes];
    snprintf(keyname, keyNameBytes, "%08lx", key);

    ret = ::get_saved_wifi_network(keyname, (uint8_t *) (&val), &read_size);
    VerifyOrReturnError(ret == 0, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t ret;
    uint32_t read_size = bufSize;
    char keyname[keyNameBytes];
    snprintf(keyname, keyNameBytes, "%08lx", key);

    ret = ::get_saved_wifi_network(keyname, (uint8_t *) (buf), &read_size);
    VerifyOrReturnError(ret == 0, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    if (read_size <= bufSize)
    {
        outLen = read_size;
    }

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t ret;
    uint32_t read_size = bufSize;
    char keyname[keyNameBytes];
    snprintf(keyname, keyNameBytes, "%08lx", key);

    ret = ::get_saved_wifi_network(keyname, (uint8_t *) (buf), &read_size);
    VerifyOrReturnError(ret == 0, CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    if (read_size <= bufSize)
    {
        outLen = read_size;
    }

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValueCounter(const char * index, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    return err;
}

CHIP_ERROR MW320Config::WriteConfigValue(Key key, bool val)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::save_wifi_network(keyname, (uint8_t *) &val, sizeof(bool));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValue(Key key, uint32_t val)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::save_wifi_network(keyname, (uint8_t *) &val, sizeof(uint32_t));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValue(Key key, uint64_t val)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::save_wifi_network(keyname, (uint8_t *) &val, sizeof(uint64_t));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR MW320Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::save_wifi_network(keyname, (uint8_t *) str, strLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::save_wifi_network(keyname, (uint8_t *) data, dataLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ClearConfigValue(Key key)
{
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ::reset_saved_wifi_network(keyname);
    return CHIP_NO_ERROR;
}

bool MW320Config::ConfigValueExists(Key key)
{
    uint32_t ret;
    uint32_t read_size = kMaxKeyValueBytes;
    uint8_t buf[kMaxKeyValueBytes];
    char keyname[keyNameBytes];

    snprintf(keyname, keyNameBytes, "%08lx", key);
    ret = ::get_saved_wifi_network(keyname, buf, &read_size);
    if (ret == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

CHIP_ERROR MW320Config::FactoryResetConfig(void)
{
    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
    {
        ClearConfigValue(key);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (Key nvmKey = firstKey; nvmKey <= lastKey; ++nvmKey)
    {

        SuccessOrExit(err);
    }

exit:
    return err;
}

void MW320Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
