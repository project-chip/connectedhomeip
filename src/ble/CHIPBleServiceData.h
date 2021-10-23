/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Definitions for chip BLE service advertisement data.
 */

#pragma once

#include <lib/core/CHIPEncoding.h>

namespace chip {
namespace Ble {

/**
 * chip data block types that may appear with chip BLE service advertisement data.
 */
enum chipBLEServiceDataType
{
    kchipBLEServiceDataType_DeviceIdentificationInfo = 0x01,
    kchipBLEServiceDataType_TokenIdentificationInfo  = 0x02,
};

/**
 * chip BLE Device Identification Information Block
 *
 * Defines the over-the-air encoded format of the device identification information block that appears
 * within chip BLE service advertisement data.
 */
struct ChipBLEDeviceIdentificationInfo
{
    constexpr static uint16_t kDiscriminatorMask = 0xfff;

    uint8_t OpCode;
    uint8_t DeviceDiscriminator[2];
    uint8_t DeviceVendorId[2];
    uint8_t DeviceProductId[2];

    void Init() { memset(this, 0, sizeof(*this)); }

    uint16_t GetVendorId() const { return chip::Encoding::LittleEndian::Get16(DeviceVendorId); }

    void SetVendorId(uint16_t vendorId) { chip::Encoding::LittleEndian::Put16(DeviceVendorId, vendorId); }

    uint16_t GetProductId() const { return chip::Encoding::LittleEndian::Get16(DeviceProductId); }

    void SetProductId(uint16_t productId) { chip::Encoding::LittleEndian::Put16(DeviceProductId, productId); }

    uint16_t GetDeviceDiscriminator() const
    {
        return chip::Encoding::LittleEndian::Get16(DeviceDiscriminator) & kDiscriminatorMask;
    }

    void SetDeviceDiscriminator(uint16_t deviceDiscriminator)
    {
        // Discriminator is 12-bit long, so don't overwrite bits 12th through 15th
        deviceDiscriminator &= kDiscriminatorMask;
        deviceDiscriminator |= static_cast<uint16_t>(DeviceDiscriminator[1] << 8u & ~kDiscriminatorMask);
        chip::Encoding::LittleEndian::Put16(DeviceDiscriminator, deviceDiscriminator);
    }
} __attribute__((packed));

} /* namespace Ble */
} /* namespace chip */
