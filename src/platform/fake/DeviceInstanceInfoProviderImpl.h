/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
