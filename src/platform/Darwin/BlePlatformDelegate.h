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

#include <ble/BlePlatformDelegate.h>
#include <system/SystemPacketBuffer.h>

using ::chip::Ble::ChipBleUUID;
using ::chip::System::PacketBufferHandle;

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BlePlatformDelegateImpl : public Ble::BlePlatformDelegate
{
public:
    virtual bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    virtual bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    virtual bool CloseConnection(BLE_CONNECTION_OBJECT connObj);
    virtual uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const;
    virtual bool SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                PacketBufferHandle pBuf);
    virtual bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                  PacketBufferHandle pBuf);
    virtual bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                 PacketBufferHandle pBuf);
    virtual bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
                                  const Ble::ChipBleUUID * svcId, const ChipBleUUID * charId);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
