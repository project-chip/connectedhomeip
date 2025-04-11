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

#include <platform/Darwin/PosixConfig.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorName(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t vendorNameLen = 0; // without counting null-terminator
    err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_VendorName, buf, bufSize,
                                                      vendorNameLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t & vendorId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_VendorId, vendorId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t productNameLen = 0; // without counting null-terminator
    err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_ProductName, buf, bufSize,
                                                      productNameLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_ProductId, productId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t serialNumLen = 0; // without counting null-terminator
    err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_SerialNum, buf, bufSize,
                                                      serialNumLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        //+++x Should this be CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER or CHIP_DEVICE_CONFIG_DEVICE_SERIAL_NUMBER?  The former is already defined in src/include/platform/CHIPDeviceConig.h but the latter is not defined.  The word "TEST" makes me question if this is the right constant to use.
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t hardwareVersionLen = 0; // without counting null-terminator
    err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_HardwareVersionString, buf, bufSize,
                                                      hardwareVersionLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING);
    }

    return err;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t softwareVersionLen = 0; // without counting null-terminator
    err = Internal::PosixConfig::ReadConfigValueStr(Internal::PosixConfig::kConfigKey_SoftwareVersionString, buf, bufSize,
                                                      softwareVersionLen);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        VerifyOrReturnError(bufSize >= sizeof(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING),
                            CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
