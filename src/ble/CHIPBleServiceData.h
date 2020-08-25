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

#ifndef CHIP_BLE_SERVICE_DATA_H
#define CHIP_BLE_SERVICE_DATA_H

#include <core/CHIPEncoding.h>

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
    enum
    {
        kPairingStatus_Unpaired = 0,
        kPairingStatus_Paired   = 1,
    };

    uint8_t PairingStatus;
    uint8_t DeviceDiscriminator[2];
    uint8_t DeviceVendorId[2];
    uint8_t DeviceProductId[2];

    void Init() { memset(this, 0, sizeof(*this)); }

    uint16_t GetVendorId(void) { return chip::Encoding::LittleEndian::Get16(DeviceVendorId); }

    void SetVendorId(uint16_t vendorId) { chip::Encoding::LittleEndian::Put16(DeviceVendorId, vendorId); }

    uint16_t GetProductId(void) { return chip::Encoding::LittleEndian::Get16(DeviceProductId); }

    void SetProductId(uint16_t productId) { chip::Encoding::LittleEndian::Put16(DeviceProductId, productId); }

    uint16_t GetDeviceDiscriminator(void)
    {
        uint16_t discriminator                = chip::Encoding::LittleEndian::Get16(DeviceDiscriminator);
        constexpr uint16_t kDiscriminatorMask = 0x7f;

        return discriminator & kDiscriminatorMask;
    }

    void SetDeviceDiscriminator(uint16_t deviceDiscriminator)
    {
        chip::Encoding::LittleEndian::Put16(DeviceDiscriminator, deviceDiscriminator);
        DeviceDiscriminator[1] &= 0x0f;
    }
} __attribute__((packed));

} /* namespace Ble */
} /* namespace chip */

#endif // CHIP_BLE_SERVICE_DATA_H
