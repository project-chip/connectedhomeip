/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <ble/BlePlatformDelegate.h>
#include <system/SystemPacketBuffer.h>

typedef bool (*SendWriteRequestCallback)(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId,
                                         const uint8_t * data, uint32_t dataLen);
typedef bool (*SubscribeCharacteristicCallback)(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId);
typedef bool (*UnsubscribeCharacteristicCallback)(BLE_CONNECTION_OBJECT connObj, const uint8_t * svcId, const uint8_t * charId);
typedef bool (*CloseConnectionCallback)(BLE_CONNECTION_OBJECT connObj);
typedef uint16_t (*GetMTUCallback)(BLE_CONNECTION_OBJECT connObj);

class AndroidBlePlatformDelegate : public chip::Ble::BlePlatformDelegate
{
private:
    SendWriteRequestCallback SendWriteRequestCb;
    SubscribeCharacteristicCallback SubscribeCharacteristicCb;
    UnsubscribeCharacteristicCallback UnsubscribeCharacteristicCb;
    CloseConnectionCallback CloseConnectionCb;
    GetMTUCallback GetMTUCb;

public:
    AndroidBlePlatformDelegate();

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                                 const chip::Ble::ChipBleUUID * charId);
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                                   const chip::Ble::ChipBleUUID * charId);
    bool CloseConnection(BLE_CONNECTION_OBJECT connObj);
    uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const;
    bool SendIndication(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId, const chip::Ble::ChipBleUUID * charId,
                        chip::System::PacketBufferHandle pBuf);
    bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                          const chip::Ble::ChipBleUUID * charId, chip::System::PacketBufferHandle pBuf);
    bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId, const chip::Ble::ChipBleUUID * charId,
                         chip::System::PacketBufferHandle pBuf);
    bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
                          const chip::Ble::ChipBleUUID * svcId, const chip::Ble::ChipBleUUID * charId);

    void SetSendWriteRequestCallback(SendWriteRequestCallback cb);
    void SetSubscribeCharacteristicCallback(SubscribeCharacteristicCallback cb);
    void SetUnsubscribeCharacteristicCallback(UnsubscribeCharacteristicCallback cb);
    void SetCloseConnectionCallback(CloseConnectionCallback cb);
    void SetGetMTUCallback(GetMTUCallback cb);
};
