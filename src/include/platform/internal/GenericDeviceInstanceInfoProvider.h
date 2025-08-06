/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ConfigClass>
class GenericConfigurationManagerImpl;

template <class ConfigClass>
class GenericDeviceInstanceInfoProvider : public DeviceInstanceInfoProvider
{

public:
    // GenericConfigurationManagerImpl will own a GenericDeviceInstanceInfoProvider which
    // *refers back to that GenericConfigurationManagerImpl*, due to how CRTP-based
    // storage APIs are defined.
    // This circular dependency is NOT needed by DeviceInstanceInfoProvider, but required
    // to keep generic code running.
    GenericDeviceInstanceInfoProvider(GenericConfigurationManagerImpl<ConfigClass> & configManager) :
        mGenericConfigManager(configManager)
    {}

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;

private:
    GenericConfigurationManagerImpl<ConfigClass> & mGenericConfigManager;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
