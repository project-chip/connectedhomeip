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

#include <platform/Linux/PosixConfig.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.ipp>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t & vendorId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_VendorId, vendorId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_ProductId, productId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    // Read from Posix config.
    size_t outLen;
    auto err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_ProductName, buf, bufSize, outLen);

    // If not found, get from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize > sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorName(char * buf, size_t bufSize)
{
    // Read from Posix config.
    size_t outLen;
    auto err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_VendorName, buf, bufSize, outLen);

    // If not found, get from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize > sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char * buf, size_t bufSize)
{
    // Read from Posix config.
    size_t outLen;
    auto err =
        Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_SerialNum, buf, bufSize, outLen);

    // If not found, get from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize > sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    // Read from Posix config.
    size_t outLen;
    auto err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_HardwareVersionString, buf, bufSize, outLen);

    // If not found, get from preprocessor variable.
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize > sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);

    // Null terminate buf.
    VerifyOrReturnError(outLen < bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
