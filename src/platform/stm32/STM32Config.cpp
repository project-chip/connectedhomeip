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

CHIP_ERROR STM32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{

    uint8_t buffer_key[35] = { 0 };

    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(NM_GetKeyValue(buf, (char *) buffer_key, bufSize, &outLen, SECTOR_SECURE));
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, bool & val)
{
    uint8_t buffer_key[35] = { 0 };
    size_t Out_Length;

    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(
        NM_GetKeyValue(reinterpret_cast<uint8_t *>(&val), (char *) buffer_key, sizeof(bool), &Out_Length, SECTOR_SECURE));
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint32_t & val)
{
    uint8_t buffer_key[35] = { 0 };
    size_t Out_Length;

    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(
        NM_GetKeyValue(reinterpret_cast<uint8_t *>(&val), (char *) buffer_key, sizeof(uint32_t), &Out_Length, SECTOR_SECURE));
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint64_t & val)
{
    uint8_t buffer_key[35] = { 0 };
    size_t Out_Length;

    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(
        NM_GetKeyValue(reinterpret_cast<uint8_t *>(&val), (char *) buffer_key, sizeof(uint64_t), &Out_Length, SECTOR_SECURE));
}

CHIP_ERROR STM32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{

    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), bufSize, outLen);
}

CHIP_ERROR STM32Config::WriteConfigValue(Key key, uint32_t val)
{
    uint8_t buffer_key[35] = { 0 };
    uint8_t buffer_convert[4];
    buffer_convert[0] = val;
    buffer_convert[1] = val >> 8;
    buffer_convert[2] = val >> 16;
    buffer_convert[3] = val >> 24;
    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(NM_SetKeyValue((char *) buffer_convert, (char *) buffer_key, sizeof(uint32_t), SECTOR_SECURE));
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
    uint8_t buffer_key[35] = { 0 };

    sprintf((char *) buffer_key, "Config%i", key);
    return PrintError(NM_SetKeyValue((char *) data, (char *) buffer_key, dataLen, SECTOR_SECURE));
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

CHIP_ERROR STM32Config::PrintError(NVM_StatusTypeDef err)
{
    switch (err)
    {
    case NVM_OK:
        ChipLogDetail(DataManagement, "NVM_OK");
        return CHIP_NO_ERROR;

    case NVM_KEY_NOT_FOUND:
        ChipLogDetail(DataManagement, "CHIP_ERROR_PERSISTED_STORAGE_NOT_FOUND");
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    case NVM_WRITE_FAILED:
        ChipLogDetail(DataManagement, "NVM_WRITE_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_READ_FAILED:
        ChipLogDetail(DataManagement, "NVM_READ_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_DELETE_FAILED:
        ChipLogDetail(DataManagement, "NVM_DELETE_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_SIZE_FULL:
        ChipLogDetail(DataManagement, "NVM_SIZE_FULL");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_BLOCK_SIZE_OVERFLOW:
        ChipLogDetail(DataManagement, "NVM_BLOCK_SIZE_OVERFLOW");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_ERROR_BLOCK_ALIGN:
        ChipLogDetail(DataManagement, "NVM_ERROR_BLOCK_ALIGN");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_BUFFER_TOO_SMALL:
        ChipLogDetail(DataManagement, "NVM_BUFFER_TOO_SMALL");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_PARAM_ERROR:
        ChipLogDetail(DataManagement, "NVM_BUFFER_TOO_SMALL");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    default:
        ChipLogDetail(DataManagement, "NVM_UNKNOWN_ERROR ");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
}
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
