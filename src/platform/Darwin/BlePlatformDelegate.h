/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
