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

#include <hal/wiced_hal_eflash.h>

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
    size_t read_count;
    ReturnErrorOnFailure(ReadConfigValueBin(key, &val, sizeof(val), read_count));
    VerifyOrReturnError(sizeof(val) == read_count, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CYW30739Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR CYW30739Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, static_cast<void *>(buf), bufSize, outLen);
}

CHIP_ERROR CYW30739Config::ReadConfigValueBin(Key key, void * buf, size_t bufSize, size_t & outLen)
{
    wiced_result_t result;
    uint16_t read_count;
    if (kMinConfigKey_ChipFactory <= key && key <= kMaxConfigKey_ChipFactory)
    {
        read_count = wiced_hal_read_nvram_static(key, bufSize, buf, &result);
    }
    else
    {
        read_count = wiced_hal_read_nvram(key, bufSize, (uint8_t *) buf, &result);
    }
    if (result == WICED_SUCCESS)
    {
        outLen = read_count;
        return CHIP_NO_ERROR;
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, bool & val);
template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, uint32_t & val);
template CHIP_ERROR CYW30739Config::ReadConfigValue(Key key, uint64_t & val);

template <typename T>
CHIP_ERROR CYW30739Config::WriteConfigValue(Key key, T val)
{
    return WriteConfigValueBin(key, &val, sizeof(val));
}

template CHIP_ERROR CYW30739Config::WriteConfigValue(Key key, bool val);
template CHIP_ERROR CYW30739Config::WriteConfigValue(Key key, uint32_t val);
template CHIP_ERROR CYW30739Config::WriteConfigValue(Key key, uint64_t val);

CHIP_ERROR CYW30739Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR CYW30739Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, str, strLen);
}

CHIP_ERROR CYW30739Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return WriteConfigValueBin(key, static_cast<const void *>(data), dataLen);
}

CHIP_ERROR CYW30739Config::WriteConfigValueBin(Key key, const void * data, size_t dataLen)
{
    /* Skip writing because the write API reports error result for zero length data. */
    if (dataLen == 0)
        return CHIP_NO_ERROR;

    if (IsDataFromFlash(data))
    {
        /*
         * Copy data to RAM before calling the platform API
         * which doesn't support writing data from the flash.
         */
        using namespace chip::Platform;
        std::unique_ptr<void, decltype(&MemoryFree)> buffer(MemoryAlloc(dataLen), &MemoryFree);
        if (!buffer)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(buffer.get(), data, dataLen);
        return WriteConfigValueBin(key, buffer.get(), dataLen);
    }

    wiced_result_t result;
    const uint16_t write_count = wiced_hal_write_nvram(key, dataLen, (uint8_t *) data, &result);
    if (result == WICED_SUCCESS && write_count == dataLen)
        return CHIP_NO_ERROR;

    ChipLogError(DeviceLayer, "%s wiced_hal_write_nvram %u", __func__, result);
    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR CYW30739Config::ClearConfigValue(Key key)
{
    wiced_result_t result;
    wiced_hal_delete_nvram(key, &result);
    if (result == WICED_SUCCESS)
    {
        return CHIP_NO_ERROR;
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

bool CYW30739Config::ConfigValueExists(Key key)
{
    uint8_t val;
    return ChipError::IsSuccess(ReadConfigValue(key, val));
}

CHIP_ERROR CYW30739Config::FactoryResetConfig(void)
{
    for (Key key = kMinConfigKey_ChipConfig; key <= kMaxConfigKey_ChipConfig; key++)
        ClearConfigValue(key);
    return CHIP_NO_ERROR;
}

void CYW30739Config::RunConfigUnitTest(void) {}

bool CYW30739Config::IsDataFromFlash(const void * data)
{
    return reinterpret_cast<void *>(FLASH_BASE_ADDRESS) <= data && data < reinterpret_cast<void *>(FLASH_BASE_ADDRESS + FLASH_SIZE);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
