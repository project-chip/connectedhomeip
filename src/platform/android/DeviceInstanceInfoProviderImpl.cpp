/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "DeviceInstanceInfoProviderImpl.h"

#include <platform/CHIPDeviceConfig.h>
#include <platform/android/AndroidConfig.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t hardwareVersionLen = 0; // without counting null-terminator
    err = Internal::AndroidConfig::ReadConfigValueStr(Internal::AndroidConfig::kConfigKey_HardwareVersionString, buf, bufSize,
                                                      hardwareVersionLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    CHIP_ERROR err;
    uint32_t u32ProductId = 0;
    err                   = Internal::AndroidConfig::ReadConfigValue(Internal::AndroidConfig::kConfigKey_ProductId, u32ProductId);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        productId = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    }
    else
    {
        productId = static_cast<uint16_t>(u32ProductId);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetPartNumber(char * buf, size_t bufSize)
{
    size_t dateLen;
    return Internal::AndroidConfig::ReadConfigValueStr(Internal::AndroidConfig::kConfigKey_PartNumber, buf, bufSize, dateLen);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductURL(char * buf, size_t bufSize)
{
    size_t dateLen;
    return Internal::AndroidConfig::ReadConfigValueStr(Internal::AndroidConfig::kConfigKey_ProductURL, buf, bufSize, dateLen);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductLabel(char * buf, size_t bufSize)
{
    size_t dateLen;
    return Internal::AndroidConfig::ReadConfigValueStr(Internal::AndroidConfig::kConfigKey_ProductLabel, buf, bufSize, dateLen);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t productNameSize = 0; // without counting null-terminator
    err =
        Internal::AndroidConfig::ReadConfigValueStr(Internal::AndroidConfig::kConfigKey_ProductName, buf, bufSize, productNameSize);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ReturnErrorCodeIf(bufSize < sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    }
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
