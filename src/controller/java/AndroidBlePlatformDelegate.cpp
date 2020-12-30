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

#include <ble/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include "AndroidBlePlatformDelegate.h"

#include <stddef.h>

using namespace chip::Ble;

AndroidBlePlatformDelegate::AndroidBlePlatformDelegate() :
    SendWriteRequestCb(NULL), SubscribeCharacteristicCb(NULL), UnsubscribeCharacteristicCb(NULL), CloseConnectionCb(NULL),
    GetMTUCb(NULL)
{}

bool AndroidBlePlatformDelegate::SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                         const ChipBleUUID * charId)
{
    bool rc = true;
    if (SubscribeCharacteristicCb)
    {
        rc = SubscribeCharacteristicCb(connObj, static_cast<const uint8_t *>(svcId->bytes),
                                       static_cast<const uint8_t *>(charId->bytes));
    }
    return rc;
}

bool AndroidBlePlatformDelegate::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                           const ChipBleUUID * charId)
{
    bool rc = true;
    if (UnsubscribeCharacteristicCb)
    {
        rc = UnsubscribeCharacteristicCb(connObj, static_cast<const uint8_t *>(svcId->bytes),
                                         static_cast<const uint8_t *>(charId->bytes));
    }
    return rc;
}

uint16_t AndroidBlePlatformDelegate::GetMTU(BLE_CONNECTION_OBJECT connObj) const
{
    uint16_t mtu = 0;
    if (GetMTUCb)
    {
        mtu = GetMTUCb(connObj);
    }
    return mtu;
}

bool AndroidBlePlatformDelegate::CloseConnection(BLE_CONNECTION_OBJECT connObj)
{
    bool rc = true;
    if (CloseConnectionCb)
    {
        rc = CloseConnectionCb(connObj);
    }
    return rc;
}

bool AndroidBlePlatformDelegate::SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                const ChipBleUUID * charId, PacketBufferHandle pBuf)
{
    // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both
    // platform delegate and Chip stack free their references to it.
    return false;
}

bool AndroidBlePlatformDelegate::SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                  const ChipBleUUID * charId, PacketBufferHandle pBuf)
{
    bool rc = true;
    if (SendWriteRequestCb)
    {
        rc = SendWriteRequestCb(connObj, static_cast<const uint8_t *>(svcId->bytes), static_cast<const uint8_t *>(charId->bytes),
                                pBuf->Start(), pBuf->DataLength());
    }

    // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both
    // platform delegate and Chip stack free their references to it.
    // We release pBuf's reference here since its payload bytes were copied
    // onto the Java heap by SendWriteRequestCb.
    return rc;
}

bool AndroidBlePlatformDelegate::SendReadRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                 const ChipBleUUID * charId, PacketBufferHandle pBuf)
{
    return true;
}

bool AndroidBlePlatformDelegate::SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
                                                  const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return true;
}

void AndroidBlePlatformDelegate::SetSendWriteRequestCallback(SendWriteRequestCallback cb)
{
    SendWriteRequestCb = cb;
}

void AndroidBlePlatformDelegate::SetSubscribeCharacteristicCallback(SubscribeCharacteristicCallback cb)
{
    SubscribeCharacteristicCb = cb;
}

void AndroidBlePlatformDelegate::SetUnsubscribeCharacteristicCallback(UnsubscribeCharacteristicCallback cb)
{
    UnsubscribeCharacteristicCb = cb;
}

void AndroidBlePlatformDelegate::SetCloseConnectionCallback(CloseConnectionCallback cb)
{
    CloseConnectionCb = cb;
}

void AndroidBlePlatformDelegate::SetGetMTUCallback(GetMTUCallback cb)
{
    GetMTUCb = cb;
}
#endif /* CONFIG_NETWORK_LAYER_BLE */
