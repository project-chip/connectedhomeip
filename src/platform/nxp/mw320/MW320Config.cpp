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

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR MW320Config::Init()
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;

    err = CHIP_NO_ERROR;
    err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;

    return err;
}

CHIP_ERROR MW320Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (buf == NULL)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
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

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValue(Key key, uint32_t val)
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValue(Key key, uint64_t val)
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR MW320Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR MW320Config::ClearConfigValue(Key key)
{

    return CHIP_NO_ERROR;
}

bool MW320Config::ConfigValueExists(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Return true if the record was found.
    return (err == CHIP_NO_ERROR);
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
