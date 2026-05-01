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

#include "DeviceInstanceInfoProviderImpl.h"

namespace chip {
namespace DeviceLayer {

DeviceInstanceInfoProviderImpl::DeviceInstanceInfoProviderImpl(DeviceInstanceInfoProvider * delegate,
                                                               chip::Optional<uint16_t> vendorId,
                                                               chip::Optional<uint16_t> productId) :
    mDelegate(delegate),
    mVendorId(vendorId), mProductId(productId)
{}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorName(char * buf, size_t bufSize)
{
    return mDelegate->GetVendorName(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t & vendorId)
{
    if (mVendorId.HasValue())
    {
        vendorId = mVendorId.Value();
        return CHIP_NO_ERROR;
    }
    return mDelegate->GetVendorId(vendorId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductName(char * buf, size_t bufSize)
{
    return mDelegate->GetProductName(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    if (mProductId.HasValue())
    {
        productId = mProductId.Value();
        return CHIP_NO_ERROR;
    }
    return mDelegate->GetProductId(productId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetPartNumber(char * buf, size_t bufSize)
{
    return mDelegate->GetPartNumber(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductURL(char * buf, size_t bufSize)
{
    return mDelegate->GetProductURL(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductLabel(char * buf, size_t bufSize)
{
    return mDelegate->GetProductLabel(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetSerialNumber(char * buf, size_t bufSize)
{
    return mDelegate->GetSerialNumber(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    return mDelegate->GetManufacturingDate(year, month, day);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersion(uint16_t & hardwareVersion)
{
    return mDelegate->GetHardwareVersion(hardwareVersion);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetHardwareVersionString(char * buf, size_t bufSize)
{
    return mDelegate->GetHardwareVersionString(buf, bufSize);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    return mDelegate->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
}

} // namespace DeviceLayer
} // namespace chip
