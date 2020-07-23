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

#include <ble/BleConfig.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BleUUID.h>
#include <platform/Darwin/BlePlatformDelegate.h>
#include <support/logging/CHIPLogging.h>

#import <CoreBluetooth/CoreBluetooth.h>

using namespace ::chip;
using namespace ::chip::Ble;
using ::chip::System::PacketBuffer;

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        bool BlePlatformDelegateImpl::SubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            bool found = false;
            CBUUID * serviceId = nil;
            CBUUID * characteristicId = nil;
            CBPeripheral * peripheral = (CBPeripheral *) connObj;

            if (NULL != svcId) {
                serviceId = [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
            }

            if (NULL != charId) {
                characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            }

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            found = true;
                            [peripheral setNotifyValue:true forCharacteristic:characteristic];
                            break;
                        }
                    }
                }
            }

            return found;
        }

        bool BlePlatformDelegateImpl::UnsubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            bool found = false;
            CBUUID * serviceId = nil;
            CBUUID * characteristicId = nil;
            CBPeripheral * peripheral = (CBPeripheral *) connObj;

            if (NULL != svcId) {
                serviceId = [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
            }
            if (NULL != charId) {
                characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            }

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            found = true;
                            [peripheral setNotifyValue:false forCharacteristic:characteristic];
                            break;
                        }
                    }
                }
            }

            return found;
        }

        bool BlePlatformDelegateImpl::CloseConnection(BLE_CONNECTION_OBJECT connObj) { return true; }

        uint16_t BlePlatformDelegateImpl::GetMTU(BLE_CONNECTION_OBJECT connObj) const { return 0; }

        bool BlePlatformDelegateImpl::SendIndication(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
        {
            if (pBuf) {
                PacketBuffer::Free(pBuf);
            }
            return false;
        }

        bool BlePlatformDelegateImpl::SendWriteRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
        {
            bool found = false;
            CBUUID * serviceId = nil;
            CBUUID * characteristicId = nil;
            NSData * data = nil;
            CBPeripheral * peripheral = (CBPeripheral *) connObj;

            if (NULL != svcId) {
                serviceId = [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
            }
            if (NULL != charId) {
                characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            }
            if (NULL != pBuf) {
                data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
            }

            for (CBService * service in peripheral.services) {
                if ([service.UUID.data isEqualToData:serviceId.data]) {
                    for (CBCharacteristic * characteristic in service.characteristics) {
                        if ([characteristic.UUID.data isEqualToData:characteristicId.data]) {
                            found = true;
                            [peripheral writeValue:data forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];
                            break;
                        }
                    }
                }
            }

            if (pBuf) {
                // Release delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free their
                // references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object
                PacketBuffer::Free(pBuf);
            }
            return found;
        }

        bool BlePlatformDelegateImpl::SendReadRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
        {
            if (pBuf) {
                PacketBuffer::Free(pBuf);
            }
            return false;
        }

        bool BlePlatformDelegateImpl::SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
            const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            return false;
        }

    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip
