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

#include "BleUtils.h"

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

using namespace chip::Ble;

namespace chip {
namespace DeviceLayer {
    namespace Internal {

        CBUUID * CBUUIDFromBleUUID(ChipBleUUID const & uuid)
        {
            return [CBUUID UUIDWithData:[NSData dataWithBytes:uuid.bytes length:sizeof(uuid.bytes)]];
        }

        ChipBleUUID BleUUIDFromCBUUD(CBUUID * uuid)
        {
            // CBUUID handles the expansion to 128 bit automatically internally,
            // but doesn't expose the expanded UUID in the `data` property, so
            // we have to re-implement the expansion here.
            // The Base UUID 00000000-0000-1000-8000-00805F9B34FB is defined in the BLE spec.
            constexpr ChipBleUUID baseUuid = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
            NSData * uuidData = uuid.data;
            switch (uuidData.length) {
            case 2: {
                ChipBleUUID outUuid = baseUuid;
                memcpy(outUuid.bytes + 2, uuidData.bytes, 2);
                return outUuid;
            }
            case 4: {
                ChipBleUUID outUuid = baseUuid;
                memcpy(outUuid.bytes, uuidData.bytes, 4);
                return outUuid;
            }
            case 16: {
                ChipBleUUID outUuid;
                memcpy(outUuid.bytes, uuidData.bytes, 16);
                return outUuid;
            }
            default: {
                NSCAssert(NO, @"Invalid CBUUID.data: %@", uuidData);
                return ChipBleUUID {};
            }
            }
        }

    }
}
}
