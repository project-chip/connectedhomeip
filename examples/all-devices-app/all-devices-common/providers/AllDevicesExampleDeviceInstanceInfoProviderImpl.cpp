/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AllDevicesExampleDeviceInstanceInfoProviderImpl.h"

#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {

AllDevicesExampleDeviceInstanceInfoProviderImpl::AllDevicesExampleDeviceInstanceInfoProviderImpl(
    DeviceInstanceInfoProvider * delegate, std::optional<uint16_t> vendorId, std::optional<uint16_t> productId) :
    mDelegate(delegate),
    mVendorId(vendorId), mProductId(productId)
{}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetVendorName(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetVendorName(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetVendorId(uint16_t & vendorId)
{
    if (mVendorId.has_value())
    {
        vendorId = mVendorId.value();
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetVendorId(vendorId);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductName(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    if (mProductId.has_value())
    {
        productId = mProductId.value();
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductId(productId);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetPartNumber(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetPartNumber(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetProductURL(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductURL(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetProductLabel(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetProductLabel(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetSerialNumber(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetSerialNumber(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetManufacturingDate(year, month, day);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetHardwareVersion(uint16_t & hardwareVersion)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetHardwareVersion(hardwareVersion);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetHardwareVersionString(buf, bufSize);
}

CHIP_ERROR AllDevicesExampleDeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
}

} // namespace DeviceLayer
} // namespace chip
