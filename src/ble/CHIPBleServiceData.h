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

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <cstdint>

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
    constexpr static uint16_t kDiscriminatorMask            = 0xfff;
    constexpr static uint8_t kAdditionalDataFlagMask        = 0x1;
    constexpr static uint8_t kExtendedAnnouncementFlagMask  = 0x2;
    constexpr static uint8_t kAdvertisementVersionMask      = 0xf0;
    constexpr static uint8_t kAdvertisementVersionShiftBits = 4u;

    uint8_t OpCode;
    // DeviceDiscriminatorAndAdvVersion[0] contains the low 8 bits of the 12-bit discriminator.
    // DeviceDiscriminatorAndAdvVersion[1] contains the high 8 bits of the 12-bit discriminator in its low 4 bits and
    // the 4 bits of the advertisement version in its high 4 bits.
    uint8_t DeviceDiscriminatorAndAdvVersion[2];
    uint8_t DeviceVendorId[2];
    uint8_t DeviceProductId[2];
    uint8_t AdditionalDataFlag;

    void Init() { memset(this, 0, sizeof(*this)); }

    uint16_t GetVendorId() const { return chip::Encoding::LittleEndian::Get16(DeviceVendorId); }

    void SetVendorId(uint16_t vendorId) { chip::Encoding::LittleEndian::Put16(DeviceVendorId, vendorId); }

    uint16_t GetProductId() const { return chip::Encoding::LittleEndian::Get16(DeviceProductId); }

    void SetProductId(uint16_t productId) { chip::Encoding::LittleEndian::Put16(DeviceProductId, productId); }

    uint8_t GetAdvertisementVersion() const
    {
        uint8_t advertisementVersion = static_cast<uint8_t>((DeviceDiscriminatorAndAdvVersion[1] & kAdvertisementVersionMask) >>
                                                            kAdvertisementVersionShiftBits);
        return advertisementVersion;
    }

    // Use only 4 bits to set advertisement version
    void SetAdvertisementVersion(uint8_t advertisementVersion)
    {
        // Advertisement Version is 4 bit long from 12th to 15th
        advertisementVersion =
            static_cast<uint8_t>((advertisementVersion << kAdvertisementVersionShiftBits) & kAdvertisementVersionMask);
        DeviceDiscriminatorAndAdvVersion[1] =
            static_cast<uint8_t>((DeviceDiscriminatorAndAdvVersion[1] & ~kAdvertisementVersionMask) | advertisementVersion);
    }

    uint16_t GetDeviceDiscriminator() const
    {
        return chip::Encoding::LittleEndian::Get16(DeviceDiscriminatorAndAdvVersion) & kDiscriminatorMask;
    }

    void SetDeviceDiscriminator(uint16_t deviceDiscriminator)
    {
        // Discriminator is 12-bit long, so don't overwrite bits 12th through 15th
        auto advVersion     = static_cast<uint16_t>(DeviceDiscriminatorAndAdvVersion[1] << 8u & ~kDiscriminatorMask);
        deviceDiscriminator = static_cast<uint16_t>(advVersion | (deviceDiscriminator & kDiscriminatorMask));
        chip::Encoding::LittleEndian::Put16(DeviceDiscriminatorAndAdvVersion, deviceDiscriminator);
    }

    uint8_t GetAdditionalDataFlag() const { return (AdditionalDataFlag & kAdditionalDataFlagMask); }

    void SetAdditionalDataFlag(bool flag)
    {
        if (flag)
        {
            AdditionalDataFlag |= kAdditionalDataFlagMask;
        }
        else
        {
            AdditionalDataFlag &= static_cast<uint8_t>(~kAdditionalDataFlagMask);
        }
    }

    void SetExtendedAnnouncementFlag(bool flag)
    {
        if (flag)
        {
            AdditionalDataFlag |= kExtendedAnnouncementFlagMask;
        }
        else
        {
            AdditionalDataFlag &= static_cast<uint8_t>(~kExtendedAnnouncementFlagMask);
        }
    }

} __attribute__((packed));

} /* namespace Ble */
} /* namespace chip */
