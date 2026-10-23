/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <ble/Ble.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BlePlatformDelegateImpl : public Ble::BlePlatformDelegate
{
public:
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const Ble::ChipBleUUID * svcId,
                                       const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const Ble::ChipBleUUID * svcId,
                                         const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT connObj) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const override;
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT connObj, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                              System::PacketBufferHandle pBuf) override;
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                System::PacketBufferHandle pBuf) override;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
