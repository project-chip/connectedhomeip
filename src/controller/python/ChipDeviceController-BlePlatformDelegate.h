/*
 *
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#ifndef DEVICEMANAGER_BLEPLATFORMDELEGATE_H_
#define DEVICEMANAGER_BLEPLATFORMDELEGATE_H_

#include <ble/BleLayer.h>
#include <ble/BlePlatformDelegate.h>

using namespace chip::Ble;

extern "C" {
typedef bool (*WriteBleCharacteristicCBFunct)(void *connObj, void *svcId, void *charId, void *buffer, uint16_t length);
typedef bool (*SubscribeBleCharacteristicCBFunct)(void *connObj, void *svcId, void *charId, bool subscribe);
typedef bool (*CloseBleCBFunct)(void *connObj);
}

class DeviceController_BlePlatformDelegate :
    public chip::Ble::BlePlatformDelegate
{
public:
    BleLayer *Ble;
    WriteBleCharacteristicCBFunct writeCB;
    SubscribeBleCharacteristicCBFunct subscribeCB;
    CloseBleCBFunct closeCB;

    // ctor
    DeviceController_BlePlatformDelegate(BleLayer *ble);

    // Set callback used to send GATT write request
    inline void SetWriteCharCB(WriteBleCharacteristicCBFunct cb) { writeCB = cb; };
    inline void SetSubscribeCharCB(SubscribeBleCharacteristicCBFunct cb) { subscribeCB = cb; };
    inline void SetCloseCB(CloseBleCBFunct cb) { closeCB = cb; };

    // Virtuals from BlePlatformDelegate:
    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId);
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId);
    bool CloseConnection(BLE_CONNECTION_OBJECT connObj);
    uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const;
    bool SendIndication(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId, chip::System::PacketBuffer *pBuf);
    bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId, chip::System::PacketBuffer *pBuf);
    bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId, chip::System::PacketBuffer *pBuf);
    bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext, const chip::Ble::ChipBleUUID *svcId, const chip::Ble::ChipBleUUID *charId);
};

#endif /* DEVICEMANAGER_BLEPLATFORMDELEGATE_H_ */
