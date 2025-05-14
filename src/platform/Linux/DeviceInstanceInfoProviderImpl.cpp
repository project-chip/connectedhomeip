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

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorName(char * buf, size_t bufSize)
{
    // First check if it was set from the command line.
    if (mpVendorName)
    {
        VerifyOrReturnError(bufSize > strlen(mpVendorName), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, mpVendorName);
        return CHIP_NO_ERROR;
    }

    // If not found, get from preprocessor variable.
    return Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>::GetVendorName(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    // First check if it was set from the command line.
    if (mpProductName)
    {
        VerifyOrReturnError(bufSize > strlen(mpProductName), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, mpProductName);
        return CHIP_NO_ERROR;
    }

    // If not found, get from preprocessor variable.
    return Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>::GetProductName(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char * buf, size_t bufSize)
{
    // First check if it was set from the command line.
    if (mpSerialNumber)
    {
        VerifyOrReturnError(bufSize > strlen(mpSerialNumber), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, mpSerialNumber);
        return CHIP_NO_ERROR;
    }

    // If not found, get from preprocessor variable.
    return Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>::GetSerialNumber(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    // First check if it was set from the command line.
    if (mpHardwareVersionString)
    {
        VerifyOrReturnError(bufSize > strlen(mpHardwareVersionString), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, mpHardwareVersionString);
        return CHIP_NO_ERROR;
    }

    // If not found, get from preprocessor variable.
    return Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>::GetHardwareVersionString(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    // First check if it was set from the command line.
    if (mpSoftwareVersionString)
    {
        VerifyOrReturnError(bufSize > strlen(mpSoftwareVersionString), CHIP_ERROR_BUFFER_TOO_SMALL);
        strcpy(buf, mpSoftwareVersionString);
        return CHIP_NO_ERROR;
    }

    // If not found, get from configuration manager, which gets it from a preprocessor variable.
    return Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>::GetSoftwareVersionString(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::SetVendorName(const char * buf)
{
    mpVendorName = buf;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::SetProductName(const char * buf)
{
    mpProductName = buf;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::SetSerialNumber(const char * buf)
{
    mpSerialNumber = buf;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::SetHardwareVersionString(const char * buf)
{
    mpHardwareVersionString = buf;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::SetSoftwareVersionString(const char * buf)
{
    mpSoftwareVersionString = buf;
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
