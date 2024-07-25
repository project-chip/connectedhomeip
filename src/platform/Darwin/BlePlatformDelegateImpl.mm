/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *          Provides an implementation of BlePlatformDelegate for Darwin platforms.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <ble/Ble.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/BlePlatformDelegate.h>

#import "MTRUUIDHelper.h"

using namespace ::chip;
using namespace ::chip::Ble;
using ::chip::System::PacketBufferHandle;

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        CHIP_ERROR BlePlatformDelegateImpl::SubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            CHIP_ERROR err = BLE_ERROR_GATT_SUBSCRIBE_FAILED;

            if (nullptr == svcId || nullptr == charId) {
                return err;
            }

            CBUUID * serviceId = [MTRUUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            err = CHIP_NO_ERROR;
                            [peripheral setNotifyValue:true forCharacteristic:characteristic];
                            break;
                        }
                    }
                }
            }

            return err;
        }

        CHIP_ERROR BlePlatformDelegateImpl::UnsubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            CHIP_ERROR err = BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;
            if (nullptr == svcId || nullptr == charId) {
                return err;
            }

            CBUUID * serviceId = [MTRUUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes
                                                                                               length:sizeof(charId->bytes)]];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            err = CHIP_NO_ERROR;
                            [peripheral setNotifyValue:false forCharacteristic:characteristic];
                            break;
                        }
                    }
                }
            }

            return err;
        }

        CHIP_ERROR BlePlatformDelegateImpl::CloseConnection(BLE_CONNECTION_OBJECT connObj)
        {
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            // CoreBluetooth API requires a CBCentralManager to close a connection which is a property of the peripheral.
            CBCentralManager * manager = (CBCentralManager *) [peripheral valueForKey:@"manager"];
            if (manager != nil)
                [manager cancelPeripheralConnection:peripheral];

            return CHIP_NO_ERROR;
        }

        uint16_t BlePlatformDelegateImpl::GetMTU(BLE_CONNECTION_OBJECT connObj) const
        {
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            // The negotiated mtu length is a property of the peripheral.
            uint16_t mtuLength = [[peripheral valueForKey:@"mtuLength"] unsignedShortValue];
            ChipLogProgress(Ble, "ATT MTU = %u", mtuLength);

            return mtuLength;
        }

        CHIP_ERROR BlePlatformDelegateImpl::SendIndication(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBufferHandle pBuf)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        CHIP_ERROR BlePlatformDelegateImpl::SendWriteRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBufferHandle pBuf)
        {
            CHIP_ERROR err = BLE_ERROR_GATT_WRITE_FAILED;
            if (nullptr == svcId || nullptr == charId || pBuf.IsNull()) {
                return err;
            }

            CBUUID * serviceId = [MTRUUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            NSData * data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            err = CHIP_NO_ERROR;
                            [peripheral writeValue:data forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];
                            break;
                        }
                    }
                }
            }

            // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip
            // stack free their references to it. We release pBuf's reference here since its payload bytes were copied into a new
            // NSData object
            return err;
        }

    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip
