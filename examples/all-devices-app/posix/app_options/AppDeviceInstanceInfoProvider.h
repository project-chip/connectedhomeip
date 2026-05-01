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

#pragma once

#include <app_options/AppOptions.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/DeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class AppDeviceInstanceInfoProvider : public DeviceInstanceInfoProvider
{
public:
    AppDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * delegate) : mDelegate(delegate) {}

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return mDelegate->GetVendorName(buf, bufSize); }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override
    {
        if (AppOptions::GetConfig().vendorId.HasValue())
        {
            vendorId = AppOptions::GetConfig().vendorId.Value();
            return CHIP_NO_ERROR;
        }
        return mDelegate->GetVendorId(vendorId);
    }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return mDelegate->GetProductName(buf, bufSize); }
    CHIP_ERROR GetProductId(uint16_t & productId) override
    {
        if (AppOptions::GetConfig().productId.HasValue())
        {
            productId = AppOptions::GetConfig().productId.Value();
            return CHIP_NO_ERROR;
        }
        return mDelegate->GetProductId(productId);
    }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return mDelegate->GetPartNumber(buf, bufSize); }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return mDelegate->GetProductURL(buf, bufSize); }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return mDelegate->GetProductLabel(buf, bufSize); }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return mDelegate->GetSerialNumber(buf, bufSize); }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override
    {
        return mDelegate->GetManufacturingDate(year, month, day);
    }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override { return mDelegate->GetHardwareVersion(hardwareVersion); }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override
    {
        return mDelegate->GetHardwareVersionString(buf, bufSize);
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override
    {
        return mDelegate->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
    }

private:
    DeviceInstanceInfoProvider * mDelegate;
};

} // namespace DeviceLayer
} // namespace chip
