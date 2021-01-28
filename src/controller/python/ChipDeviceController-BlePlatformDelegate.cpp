/*
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

#include "ChipDeviceController-BlePlatformDelegate.h"
#include <ble/BlePlatformDelegate.h>

using namespace chip::Ble;

DeviceController_BlePlatformDelegate::DeviceController_BlePlatformDelegate(BleLayer * ble)
{
    Ble         = ble;
    writeCB     = NULL;
    subscribeCB = NULL;
    closeCB     = NULL;
}

bool DeviceController_BlePlatformDelegate::SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj,
                                                                   const chip::Ble::ChipBleUUID * svcId,
                                                                   const chip::Ble::ChipBleUUID * charId)
{
    const bool subscribe = true;

    if (subscribeCB && svcId && charId)
    {
        return subscribeCB(connObj, static_cast<const void *>(svcId->bytes), static_cast<const void *>(charId->bytes), subscribe);
    }

    return false;
}

bool DeviceController_BlePlatformDelegate::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj,
                                                                     const chip::Ble::ChipBleUUID * svcId,
                                                                     const chip::Ble::ChipBleUUID * charId)
{
    const bool subscribe = true;

    if (subscribeCB && svcId && charId)
    {
        return subscribeCB(connObj, static_cast<const void *>(svcId->bytes), static_cast<const void *>(charId->bytes), !subscribe);
    }

    return false;
}

bool DeviceController_BlePlatformDelegate::CloseConnection(BLE_CONNECTION_OBJECT connObj)
{
    if (closeCB)
    {
        return closeCB(connObj);
    }

    return false;
}

uint16_t DeviceController_BlePlatformDelegate::GetMTU(BLE_CONNECTION_OBJECT connObj) const
{
    // TODO Python queue-based implementation
    return 0;
}

bool DeviceController_BlePlatformDelegate::SendIndication(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                                                          const chip::Ble::ChipBleUUID * charId,
                                                          chip::System::PacketBufferHandle pBuf)
{
    // TODO Python queue-based implementation

    // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free
    // their references to it.
    return false;
}

bool DeviceController_BlePlatformDelegate::SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                                                            const chip::Ble::ChipBleUUID * charId,
                                                            chip::System::PacketBufferHandle pBuf)
{
    bool ret = false;

    if (writeCB && svcId && charId && !pBuf.IsNull())
    {
        ret = writeCB(connObj, static_cast<const void *>(svcId->bytes), static_cast<const void *>(charId->bytes),
                      static_cast<void *>(pBuf->Start()), pBuf->DataLength());
    }

    // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free
    // their references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object by
    // ChipBleMgr.py's writeCB, and in both the error and succees cases this code has no further use for it.
    return ret;
}

bool DeviceController_BlePlatformDelegate::SendReadRequest(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBleUUID * svcId,
                                                           const chip::Ble::ChipBleUUID * charId,
                                                           chip::System::PacketBufferHandle pBuf)
{
    // TODO Python queue-based implementation
    return false;
}

bool DeviceController_BlePlatformDelegate::SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
                                                            const chip::Ble::ChipBleUUID * svcId,
                                                            const chip::Ble::ChipBleUUID * charId)
{
    // TODO Python queue-based implementation
    return false;
}
