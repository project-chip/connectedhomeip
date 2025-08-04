/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      to a platform's BLE framework.
 */

#pragma once

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>

#include "BleConfig.h"
#include "BleUUID.h"

namespace chip {
namespace Ble {

using ::chip::System::PacketBufferHandle;

// Platform-agnostic BLE interface
class DLL_EXPORT BlePlatformDelegate
{
public:
    virtual ~BlePlatformDelegate() {}

    // Following APIs must be implemented by platform:

    // Subscribe to updates and indications on the specfied characteristic
    virtual CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                               const ChipBleUUID * charId) = 0;

    // Unsubscribe from updates and indications on the specified characteristic
    virtual CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                 const ChipBleUUID * charId) = 0;

    // Close the underlying BLE connection.
    virtual CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT connObj) = 0;

    // Get MTU size negotiated for specified BLE connection. Return value of 0 means MTU size could not be determined.
    virtual uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const = 0;

    // Send GATT characteristic indication request
    virtual CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf) = 0;

    // Send GATT characteristic write request
    virtual CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                        PacketBufferHandle pBuf) = 0;
};

} /* namespace Ble */
} /* namespace chip */
