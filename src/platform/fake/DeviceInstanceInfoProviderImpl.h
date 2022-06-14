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

#pragma once

#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProviderImpl : public Internal::GenericDeviceInstanceInfoProvider<>
{
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetProductId(uint16_t & productId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

private:
    friend DeviceInstanceInfoProviderImpl & DeviceInstanceInfoProviderMgrImpl();
    static DeviceInstanceInfoProviderImpl sInstance;
};

inline DeviceInstanceInfoProviderImpl & DeviceInstanceInfoProviderMgrImpl()
{
    return DeviceInstanceInfoProviderImpl::sInstance;
}
} // namespace DeviceLayer
} // namespace chip
