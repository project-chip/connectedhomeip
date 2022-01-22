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
 *          Utilities for accessing persisted device configuration.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CYW30739/CYW30739Config.h>
#include <platform_nvram.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR CYW30739Config::Init()
{
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, T & val)
{
    wiced_result_t result;
    uint16_t read_count = wiced_hal_read_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, sizeof(val), (uint8_t *) &val, &result);
    if (result != WICED_SUCCESS || read_count != sizeof(val))
    {
        read_count = wiced_hal_read_nvram_static(PLATFORM_NVRAM_SSID_MATTER_BASE + key, sizeof(val), &val, &result);
    }
    if (result == WICED_SUCCESS && read_count == sizeof(val))
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR CYW30739Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), bufSize, outLen);
}

CHIP_ERROR CYW30739Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    wiced_result_t result;
    uint16_t read_count = wiced_hal_read_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, bufSize, (uint8_t *) buf, &result);
    if (result != WICED_SUCCESS)
    {
        read_count = wiced_hal_read_nvram_static(PLATFORM_NVRAM_SSID_MATTER_BASE + key, bufSize, buf, &result);
    }
    if (result == WICED_SUCCESS)
    {
        outLen = read_count;
        return CHIP_NO_ERROR;
    }
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, bool & val);
template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, uint32_t & val);
template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, uint64_t & val);

CHIP_ERROR CYW30739Config::WriteConfigValue(Key key, uint32_t val)
{
    wiced_result_t result;
    uint16_t write_count = wiced_hal_write_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, sizeof(val), (uint8_t *) &val, &result);
    if (result == WICED_SUCCESS && write_count == sizeof(val))
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR CYW30739Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR CYW30739Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR CYW30739Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    wiced_result_t result;
    wiced_hal_write_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, dataLen, (uint8_t *) data, &result);
    if (result == WICED_SUCCESS)
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

bool CYW30739Config::ConfigValueExists(Key key)
{
    wiced_result_t result;
    wiced_hal_read_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, 0, NULL, &result);
    if (result != WICED_SUCCESS)
        wiced_hal_read_nvram_static(PLATFORM_NVRAM_SSID_MATTER_BASE + key, 0, NULL, &result);
    return result == WICED_SUCCESS;
}

CHIP_ERROR CYW30739Config::FactoryResetConfig(void)
{
    wiced_result_t result;
    for (Key key = kConfigKey_Base; key <= kConfigKey_Max; key++)
        wiced_hal_delete_nvram(PLATFORM_NVRAM_VSID_MATTER_BASE + key, &result);
    return CHIP_NO_ERROR;
}

void CYW30739Config::RunConfigUnitTest(void) {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
