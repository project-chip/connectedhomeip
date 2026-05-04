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

#include "OverrideDeviceInstanceInfoProvider.h"

namespace chip {
namespace DeviceLayer {

OverrideDeviceInstanceInfoProvider::OverrideDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * delegate,
                                                                       chip::Optional<uint16_t> vendorId,
                                                                       chip::Optional<uint16_t> productId) :
    mDelegate(delegate),
    mVendorId(vendorId), mProductId(productId)
{}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetVendorName(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetVendorName(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetVendorId(uint16_t & vendorId)
{
    if (mVendorId.HasValue())
    {
        vendorId = mVendorId.Value();
        return CHIP_NO_ERROR;
    }
    if (mDelegate)
    {
        return mDelegate->GetVendorId(vendorId);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetProductName(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetProductName(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetProductId(uint16_t & productId)
{
    if (mProductId.HasValue())
    {
        productId = mProductId.Value();
        return CHIP_NO_ERROR;
    }
    if (mDelegate)
    {
        return mDelegate->GetProductId(productId);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetPartNumber(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetPartNumber(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetProductURL(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetProductURL(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetProductLabel(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetProductLabel(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetSerialNumber(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    if (mDelegate)
    {
        return mDelegate->GetManufacturingDate(year, month, day);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    if (mDelegate)
    {
        return mDelegate->GetHardwareVersion(hardwareVersion);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    if (mDelegate)
    {
        return mDelegate->GetHardwareVersionString(buf, bufSize);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR OverrideDeviceInstanceInfoProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    if (mDelegate)
    {
        return mDelegate->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

} // namespace DeviceLayer
} // namespace chip
