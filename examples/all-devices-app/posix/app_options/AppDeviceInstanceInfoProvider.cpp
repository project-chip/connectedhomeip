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

#include <app_options/AppDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

AppDeviceInstanceInfoProvider::AppDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * delegate,
                                                             chip::Optional<uint16_t> vendorId,
                                                             chip::Optional<uint16_t> productId) :
    mDelegate(delegate), mVendorId(vendorId), mProductId(productId)
{}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetVendorName(char * buf, size_t bufSize)
{
    return mDelegate->GetVendorName(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetVendorId(uint16_t & vendorId)
{
    if (mVendorId.HasValue())
    {
        vendorId = mVendorId.Value();
        return CHIP_NO_ERROR;
    }
    return mDelegate->GetVendorId(vendorId);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetProductName(char * buf, size_t bufSize)
{
    return mDelegate->GetProductName(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetProductId(uint16_t & productId)
{
    if (mProductId.HasValue())
    {
        productId = mProductId.Value();
        return CHIP_NO_ERROR;
    }
    return mDelegate->GetProductId(productId);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return mDelegate->GetPartNumber(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetProductURL(char * buf, size_t bufSize)
{
    return mDelegate->GetProductURL(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return mDelegate->GetProductLabel(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    return mDelegate->GetSerialNumber(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return mDelegate->GetManufacturingDate(year, month, day);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    return mDelegate->GetHardwareVersion(hardwareVersion);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return mDelegate->GetHardwareVersionString(buf, bufSize);
}

CHIP_ERROR AppDeviceInstanceInfoProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    return mDelegate->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
}

} // namespace DeviceLayer
} // namespace chip
