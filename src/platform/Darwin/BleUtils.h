/**
 *    Copyright (c) 2025 Project CHIP Authors
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

@class CBPeripheral;
@class CBUUID;

namespace chip {
namespace DeviceLayer {
    namespace Internal {

        inline CBPeripheral * CBPeripheralFromBleConnObject(BLE_CONNECTION_OBJECT connObj)
        {
            return (__bridge CBPeripheral *) connObj;
        }

        inline BLE_CONNECTION_OBJECT BleConnObjectFromCBPeripheral(CBPeripheral * peripheral)
        {
            return (__bridge void *) peripheral;
        }

        // Creates a CBUUID from a ChipBleUUID
        CBUUID * CBUUIDFromBleUUID(Ble::ChipBleUUID const & uuid);

        // Creates a ChipBleUUID from a CBUUID, expanding 16 or 32 bit UUIDs if necessary.
        Ble::ChipBleUUID BleUUIDFromCBUUD(CBUUID * uuid);

    }
}
}
