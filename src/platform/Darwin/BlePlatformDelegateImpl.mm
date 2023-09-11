/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of BlePlatformDelegate for Darwin platforms.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <ble/BleConfig.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BleUUID.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/BlePlatformDelegate.h>

#import "UUIDHelper.h"

using namespace ::chip;
using namespace ::chip::Ble;
using ::chip::System::PacketBufferHandle;

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        bool BlePlatformDelegateImpl::SubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
        {
            bool found = false;

            if (nullptr == svcId || nullptr == charId) {
                return found;
            }

            CBUUID * serviceId = [UUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

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
            if (nullptr == svcId || nullptr == charId) {
                return found;
            }

            CBUUID * serviceId = [UUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes
                                                                                               length:sizeof(charId->bytes)]];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

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

        bool BlePlatformDelegateImpl::CloseConnection(BLE_CONNECTION_OBJECT connObj)
        {
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            // CoreBluetooth API requires a CBCentralManager to close a connection which is a property of the peripheral.
            CBCentralManager * manager = (CBCentralManager *) [peripheral valueForKey:@"manager"];
            if (manager != nil)
                [manager cancelPeripheralConnection:peripheral];

            return true;
        }

        uint16_t BlePlatformDelegateImpl::GetMTU(BLE_CONNECTION_OBJECT connObj) const
        {
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

            // The negotiated mtu length is a property of the peripheral.
            uint16_t mtuLength = [[peripheral valueForKey:@"mtuLength"] unsignedShortValue];
            ChipLogProgress(Ble, "ATT MTU = %u", mtuLength);

            return mtuLength;
        }

        bool BlePlatformDelegateImpl::SendIndication(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBufferHandle pBuf)
        {
            return false;
        }

        bool BlePlatformDelegateImpl::SendWriteRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBufferHandle pBuf)
        {
            bool found = false;
            if (nullptr == svcId || nullptr == charId || pBuf.IsNull()) {
                return found;
            }

            CBUUID * serviceId = [UUIDHelper GetShortestServiceUUID:svcId];
            CBUUID * characteristicId = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
            NSData * data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj;

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

            // Going out of scope releases delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip
            // stack free their references to it. We release pBuf's reference here since its payload bytes were copied into a new
            // NSData object
            return found;
        }

        bool BlePlatformDelegateImpl::SendReadRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBufferHandle pBuf)
        {
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
