/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MCDeviceInstanceInfo.h"

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <platform/ConfigurationManager.h>
#include <platform/Darwin/DeviceInstanceInfoProviderImpl.h>
#include <platform/DeviceInstanceInfoProvider.h>

#ifndef MCDeviceInstanceInfoProvider_h
#define MCDeviceInstanceInfoProvider_h

namespace matter {
namespace casting {
namespace support {

class MCDeviceInstanceInfoProvider : public chip::DeviceLayer::DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR Initialize(MCDeviceInstanceInfo * _Nonnull deviceInstanceInfo);

    // Overridden methods — return app value if provided, else delegate to default
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;

    // Delegated methods — always use default implementation
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(chip::MutableByteSpan & uniqueIdSpan) override;

private:
    bool mHasVendorName = false;
    char mVendorName[chip::DeviceLayer::ConfigurationManager::kMaxVendorNameLength + 1];

    bool mHasProductName = false;
    char mProductName[chip::DeviceLayer::ConfigurationManager::kMaxProductNameLength + 1];

    bool mHasSerialNumber = false;
    char mSerialNumber[chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];

    bool mHasHardwareVersionString = false;
    char mHardwareVersionString[chip::DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength + 1];

    chip::Optional<uint16_t> mVendorId;
    chip::Optional<uint16_t> mProductId;
    chip::Optional<uint16_t> mHardwareVersion;

    chip::DeviceLayer::DeviceInstanceInfoProviderImpl mDefaultProvider;
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MCDeviceInstanceInfoProvider_h */
