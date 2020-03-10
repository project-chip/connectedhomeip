/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Definitions for Weave BLE service advertisement data.
 */

#ifndef WEAVE_BLE_SERVICE_DATA_H
#define WEAVE_BLE_SERVICE_DATA_H

namespace nl {
namespace Ble {

/**
 * Weave data block types that may appear with Weave BLE service advertisement data.
 */
enum WeaveBLEServiceDataType
{
    kWeaveBLEServiceDataType_DeviceIdentificationInfo       = 0x01,
    kWeaveBLEServiceDataType_TokenIdentificationInfo        = 0x02,
};

/**
 * Weave BLE Device Identification Information Block
 *
 * Defines the over-the-air encoded format of the device identification information block that appears
 * within Weave BLE service advertisement data.
 */
struct WeaveBLEDeviceIdentificationInfo
{
    enum
    {
        kMajorVersion           = 0,
        kMinorVersion           = 1,
    };

    enum
    {
        kPairingStatus_Unpaired = 0,
        kPairingStatus_Paired   = 1,
    };

    uint8_t BlockLen;
    uint8_t BlockType;
    uint8_t MajorVersion;
    uint8_t MinorVersion;
    uint8_t DeviceVendorId[2];
    uint8_t DeviceProductId[2];
    uint8_t DeviceId[8];
    uint8_t PairingStatus;

    void Init()
    {
        memset(this, 0, sizeof(*this));
        BlockLen = sizeof(*this) - sizeof(BlockLen); // size of all fields EXCEPT BlockLen
        BlockType = kWeaveBLEServiceDataType_DeviceIdentificationInfo;
        MajorVersion = kMajorVersion;
        MinorVersion = kMinorVersion;
    }

    uint16_t GetVendorId(void)
    {
        return nl::Weave::Encoding::LittleEndian::Get16(DeviceVendorId);
    }

    void SetVendorId(uint16_t vendorId)
    {
        nl::Weave::Encoding::LittleEndian::Put16(DeviceVendorId, vendorId);
    }

    uint16_t GetProductId(void)
    {
        return nl::Weave::Encoding::LittleEndian::Get16(DeviceProductId);
    }

    void SetProductId(uint16_t productId)
    {
        nl::Weave::Encoding::LittleEndian::Put16(DeviceProductId, productId);
    }

    uint64_t GetDeviceId(void)
    {
        return nl::Weave::Encoding::LittleEndian::Get64(DeviceId);
    }

    void SetDeviceId(uint64_t deviceId)
    {
        nl::Weave::Encoding::LittleEndian::Put64(DeviceId, deviceId);
    }
} __attribute__((packed));

} /* namespace Ble */
} /* namespace nl */

#endif // WEAVE_BLE_SERVICE_DATA_H
