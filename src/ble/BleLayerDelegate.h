/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the interface for downcalls from BleLayer
 *      to a BLE transport.
 */

#pragma once

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <system/SystemPacketBuffer.h>

#include "BleConfig.h"
#include "BleError.h"
#include "BleUUID.h"

namespace chip {
namespace Ble {

// BLEEndPoint holds a pointer to BleLayerDelegate for messages,
// while BleLayerDelegate functions also accepts BLEEndPoint.
class BLEEndPoint;

class BleLayerDelegate
{
public:
    virtual ~BleLayerDelegate() = default;

    virtual void OnBleConnectionComplete(BLEEndPoint * endpoint) = 0;
    virtual void OnBleConnectionError(CHIP_ERROR err)            = 0;

    virtual void OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err)                    = 0;
    virtual void OnEndPointMessageReceived(BLEEndPoint * endPoint, System::PacketBufferHandle && msg) = 0;
    virtual void OnEndPointConnectionClosed(BLEEndPoint * endPoint, CHIP_ERROR err)                   = 0;

    virtual CHIP_ERROR SetEndPoint(BLEEndPoint * endPoint) = 0;
};

} // namespace Ble
} // namespace chip
