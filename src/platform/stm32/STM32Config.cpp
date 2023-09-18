/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "flash_wb.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/stm32/STM32Config.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR STM32Config::Init()
{
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR STM32Config::ReadConfigValue(Key key, T & val)
{
    uint8_t * buffer_key[35] = { 0 };
    size_t * read_by_size    = NULL;

    sprintf((char *) buffer_key, "Config%i", key);
    NM_GetKeyValue((void *) &val, (char *) buffer_key, sizeof(val), read_by_size, SECTOR_NO_SECURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR STM32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{

    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), bufSize, outLen);
}

CHIP_ERROR STM32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template CHIP_ERROR STM32Config::ReadConfigValue(Key key, bool & val);
template CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint32_t & val);
template CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint64_t & val);

CHIP_ERROR STM32Config::WriteConfigValue(Key key, uint32_t val)
{
    uint8_t * buffer_key[35] = { 0 };
    size_t * read_by_size    = NULL;

    sprintf((char *) buffer_key, "Config%i", key);
    NM_SetKeyValue((char *) &val, (char *) buffer_key, sizeof(val), SECTOR_NO_SECURE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR STM32Config::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueStr(key, str, (str != NULL) ? strlen(str) : 0);
}

CHIP_ERROR STM32Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR STM32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    uint8_t * buffer_key[35] = { 0 };
    size_t * read_by_size    = NULL;

    sprintf((char *) buffer_key, "Config%i", key);
    NM_SetKeyValue((char *) data, (char *) buffer_key, dataLen, SECTOR_NO_SECURE);
    return CHIP_NO_ERROR;
}

bool STM32Config::ConfigValueExists(Key key)
{
    return false;
}

CHIP_ERROR STM32Config::FactoryResetConfig(void)
{

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

void STM32Config::RunConfigUnitTest(void) {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
