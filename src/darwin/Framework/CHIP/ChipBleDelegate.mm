/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file provides base implementation for ChipBleDelegate interface
 *
 */

#import <Foundation/Foundation.h>

#import "CHIPDeviceController.h"
#import "CHIPLogging.h"
#import "ChipBleDelegate_Protected.h"

#include <ble/BleApplicationDelegate.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BlePlatformDelegate.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Ble {
    class BleDelegateTrampoline;
}
}

using chip::System::PacketBuffer;

@interface ChipBleDelegate () {
    chip::Ble::BleDelegateTrampoline * _mTrampoline;
    dispatch_queue_t _mCbWorkQueue;
    NSMapTable * _mMapFromPeripheralToDc;

    // cached static Chip service UUID
    CBUUID * _mChipServiceUUID;

    chip::Ble::BleLayer * _mBleLayer;
}

+ (CBUUID *)GetShortestServiceUUID:(const chip::Ble::ChipBleUUID *)svcId;

@end

namespace chip {
namespace Ble {

    class BleDelegateTrampoline : public BleApplicationDelegate, public BlePlatformDelegate {
    public:
        BleDelegateTrampoline(ChipBleDelegate * pBleDelegate);
        virtual bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId);
        virtual bool UnsubscribeCharacteristic(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId);
        virtual bool CloseConnection(BLE_CONNECTION_OBJECT connObj);
        virtual uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const;
        virtual bool SendIndication(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf);
        virtual bool SendWriteRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf);
        virtual bool SendReadRequest(
            BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf);
        virtual bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
            const Ble::ChipBleUUID * svcId, const ChipBleUUID * charId);

        virtual void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj);

    private:
        /**
            An weak reference to the parenting delegate object, can be nil.
            @note
              It has to be weak to avoid circular references
         */
        __weak ChipBleDelegate * mBleDelegate;
    };

    BleDelegateTrampoline::BleDelegateTrampoline(ChipBleDelegate * pBleDelegate)
        : mBleDelegate(pBleDelegate)
    {
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::SubscribeCharacteristic(
        BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }

        result = [mBleDelegate SubscribeCharacteristic:(__bridge id) connObj service:service characteristic:characteristic];

    exit:
        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::UnsubscribeCharacteristic(
        BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }

        result = [mBleDelegate UnsubscribeCharacteristic:(__bridge id) connObj service:service characteristic:characteristic];

    exit:
        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::CloseConnection(BLE_CONNECTION_OBJECT connObj)
    {
        bool result = false;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        result = [mBleDelegate CloseConnection:(__bridge id) connObj];

    exit:
        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    uint16_t BleDelegateTrampoline::GetMTU(BLE_CONNECTION_OBJECT connObj) const
    {
        bool result = false;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        result = [mBleDelegate GetMTU:(__bridge id) connObj];

    exit:
        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::SendIndication(
        BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;
        NSData * data = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }
        if (NULL != pBuf) {
            data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
        }

        result = [mBleDelegate SendIndication:(__bridge id) connObj service:service characteristic:characteristic data:data];

    exit:
        // Release delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free their
        // references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object
        PacketBuffer::Free(pBuf);

        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::SendWriteRequest(
        BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;
        NSData * data = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }
        if (NULL != pBuf) {
            data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
        }

        result = [mBleDelegate SendWriteRequest:(__bridge id) connObj service:service characteristic:characteristic data:data];

    exit:
        // Release delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free their
        // references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object
        PacketBuffer::Free(pBuf);

        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::SendReadRequest(
        BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId, PacketBuffer * pBuf)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;
        NSData * data = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }
        if (NULL != pBuf) {
            data = [NSData dataWithBytes:pBuf->Start() length:pBuf->DataLength()];
        }

        // Release delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free their
        // references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object
        PacketBuffer::Free(pBuf);

        result = [mBleDelegate SendReadRequest:(__bridge id) connObj service:service characteristic:characteristic data:data];

    exit:
        // Release delegate's reference to pBuf. pBuf will be freed when both platform delegate and Chip stack free their
        // references to it. We release pBuf's reference here since its payload bytes were copied into a new NSData object
        PacketBuffer::Free(pBuf);

        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    bool BleDelegateTrampoline::SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext,
        const ChipBleUUID * svcId, const ChipBleUUID * charId)
    {
        bool result = false;
        CBUUID * service = nil;
        CBUUID * characteristic = nil;

        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        if (NULL != svcId) {
            service = [ChipBleDelegate GetShortestServiceUUID:svcId];
        }
        if (NULL != charId) {
            characteristic = [CBUUID UUIDWithData:[NSData dataWithBytes:charId->bytes length:sizeof(charId->bytes)]];
        }

        result = [mBleDelegate SendReadResponse:(__bridge id) connObj
                                 requestContext:(__bridge id) requestContext
                                        service:service
                                 characteristic:characteristic];

    exit:
        return result;
    }

    /**
        @note
          This method is only called from BleLayer
     */
    void BleDelegateTrampoline::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj)
    {
        VerifyOrExit(mBleDelegate, CHIP_LOG_ERROR("BLE not configured properly\n"));

        [mBleDelegate NotifyChipConnectionClosed:(__bridge id) connObj];

    exit:;
    }

} /* namespace Ble */
} /* namespace chip*/

@implementation ChipBleDelegate

- (instancetype)initDummyDelegate
{
    self = [super init];
    VerifyOrExit(self, CHIP_LOG_ERROR("Memory allocation failure\n"));

    _mTrampoline = new chip::Ble::BleDelegateTrampoline(NULL);

exit:
    return self;
}

- (instancetype)init:(dispatch_queue_t)cbWorkQueue
{
    self = [super init];
    VerifyOrExit(self, CHIP_LOG_ERROR("Memory allocation failure\n"));

    _mTrampoline = new chip::Ble::BleDelegateTrampoline(self);

    _mCbWorkQueue = cbWorkQueue;

    _mMapFromPeripheralToDc = [NSMapTable strongToWeakObjectsMapTable];

    _mChipServiceUUID = [ChipBleDelegate GetShortestServiceUUID:&chip::Ble::CHIP_BLE_SVC_ID];

exit:
    return self;
}

- (void)dealloc
{
    delete _mTrampoline;
    _mTrampoline = NULL;
}

/**
 @note
 This method is only called from BleLayer
 */
- (chip::Ble::BlePlatformDelegate *)GetPlatformDelegate
{
    return _mTrampoline;
}

/**
 @note
 This method is only called from BleLayer
 */
- (chip::Ble::BleApplicationDelegate *)GetApplicationDelegate
{
    return _mTrampoline;
}

- (void)SetBleLayer:(chip::Ble::BleLayer *)BleLayer
{
    _mBleLayer = BleLayer;
}

+ (CBUUID *)GetShortestServiceUUID:(const chip::Ble::ChipBleUUID *)svcId
{
    // TODO Implement and validate that all the code paths works with a shorter service uuid
    return [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
}

/**
 * private static method to copy service and characteristic UUIDs from CBCharacteristic to a pair of ChipBleUUID objects.
 * this is used in calls into Chip layer to decouple it from CoreBluetooth
 *
 * @param[in] characteristic the source characteristic
 * @param[in] svcId the destination service UUID
 * @param[in] charId the destination characteristic UUID
 *
 */
+ (void)fillServiceWithCharacteristicUuids:(CBCharacteristic *)characteristic
                                     svcId:(chip::Ble::ChipBleUUID *)svcId
                                    charId:(chip::Ble::ChipBleUUID *)charId
{
    static const size_t FullUUIDLength = 16;
    if ((FullUUIDLength != sizeof(charId->bytes)) || (FullUUIDLength != sizeof(svcId->bytes))
        || (FullUUIDLength != characteristic.UUID.data.length)) {
        // we're dead. we expect the data length to be the same (16-byte) across the board
        CHIP_LOG_ERROR("[%s] UUID of characteristic is incompatible", __PRETTY_FUNCTION__);
        return;
    }

    memcpy(charId->bytes, characteristic.UUID.data.bytes, sizeof(charId->bytes));
    memset(svcId->bytes, 0, sizeof(svcId->bytes));

    // Expand service UUID back to 16-byte long as that's what the BLE Layer expects
    // this is a buffer pre-filled with BLE service UUID Base
    // byte 0 to 3 are reserved for shorter versions of BLE service UUIDs
    // For 4-byte service UUIDs, all bytes from 0 to 3 are used
    // For 2-byte service UUIDs, byte 0 and 1 shall be 0
    uint8_t serviceFullUUID[FullUUIDLength]
        = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };

    switch (characteristic.service.UUID.data.length) {
    case 2:
        // copy the 2-byte service UUID onto the right offset
        memcpy(serviceFullUUID + 2, characteristic.service.UUID.data.bytes, 2);
        break;
    case 4:
        // flow through
    case 16:
        memcpy(serviceFullUUID, characteristic.service.UUID.data.bytes, characteristic.service.UUID.data.length);
        break;
    default:
        // we're dead. we expect the data length to be the same (16-byte) across the board
        CHIP_LOG_ERROR("[%s] Service UUIDs are incompatible", __PRETTY_FUNCTION__);
    }
    memcpy(svcId->bytes, serviceFullUUID, sizeof(svcId->bytes));
}

// This is only used by ChipDeviceController
- (void)prepareNewBleConnection:(CHIPDeviceController *)dc
{
    CHIP_LOG_DEBUG("Mapping device manager %@ to peripheral %@", dc, dc.blePeripheral);

    dispatch_async(_mCbWorkQueue, ^{
        [_mMapFromPeripheralToDc setObject:dc forKey:dc.blePeripheral];

        [dc.blePeripheral discoverServices:@[ _mChipServiceUUID ]];
    });
}

- (bool)isPeripheralValid:(CBPeripheral *)peripheral
{
    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    return (dc != nil) ? true : false;
}

- (void)forceBleDisconnect_Sync:(CBPeripheral *)peripheral
{
    // force BleLayer to forget about this connObj
    _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_REMOTE_DEVICE_DISCONNECTED);
}

- (void)notifyBleDisconnected:(CBPeripheral *)peripheral
{
    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
    }

    if (dc == nil || dc.BleConnectionPreparationCompleteHandler == nil) {
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_REMOTE_DEVICE_DISCONNECTED);
        });
    } else {
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            PreparationCompleteHandler handler = dc.BleConnectionPreparationCompleteHandler;
            dc.BleConnectionPreparationCompleteHandler = nil;
            handler(dc, BLE_ERROR_REMOTE_DEVICE_DISCONNECTED);
        });
    }
}

/**
 * part of CBPeripheralDelegate.
 * called after service discovery is done. report failure to attach completed block on error, otherwise
 * proceed with characteristic discovery
 */
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
    // we're in Core Bluetooth work queue

    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
        return;
    }

    bool found = false;

    if (nil != error) {
        CHIP_LOG_ERROR("[%s] BLE:Error finding Chip Service in the device: [%@]", __PRETTY_FUNCTION__, error.localizedDescription);
    } else {
        for (CBService * service in peripheral.services) {
            CHIP_LOG_DEBUG("Found service in device: %@", service.UUID);

            // XXX for some reason one is short the other is not...
            if ([service.UUID.data isEqualToData:_mChipServiceUUID.data]) {
                found = true;

                [peripheral discoverCharacteristics:nil forService:service];

                break;
            }
        }
    }

    if (!found) {
        CHIP_LOG_ERROR("[%s] Cannot find Chip service on peripheral %@", __PRETTY_FUNCTION__, peripheral);
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            PreparationCompleteHandler handler = dc.BleConnectionPreparationCompleteHandler;
            dc.BleConnectionPreparationCompleteHandler = nil;
            handler(dc, BLE_ERROR_NOT_CHIP_DEVICE);
        });
    }
}

/**
 * part of CBPeripheralDelegate.
 * called after characteristic discovery is done. execute attach completed block and report error or success
 */
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    // we're in Core Bluetooth work queue

    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
        return;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (nil != error) {
        CHIP_LOG_ERROR("[%s] BLE:Error finding Characteristics in Chip service on the device: [%@]", __PRETTY_FUNCTION__,
            error.localizedDescription);
        err = BLE_ERROR_NOT_CHIP_DEVICE;
    }

    // we're good to go
    dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
        PreparationCompleteHandler handler = dc.BleConnectionPreparationCompleteHandler;
        dc.BleConnectionPreparationCompleteHandler = nil;
        handler(dc, err);
    });
}

/**
 * part of CBPeripheralDelegate.
 * called after writing completes. call BleLayer for both error and success
 */
- (void)peripheral:(CBPeripheral *)peripheral
    didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
                             error:(NSError *)error
{
    // we're in Core Bluetooth work queue
    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
        return;
    }

    if (nil == error) {
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            chip::Ble::ChipBleUUID svcId;
            chip::Ble::ChipBleUUID charId;
            [ChipBleDelegate fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];
            _mBleLayer->HandleWriteConfirmation((__bridge void *) peripheral, &svcId, &charId);
        });
    } else {
        CHIP_LOG_ERROR("[%s] BLE:Error writing Characteristics in Chip service on the device: [%@]", __PRETTY_FUNCTION__,
            error.localizedDescription);

        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_WRITE_FAILED);
        });
    }
}

/**
 * part of CBPeripheralDelegate.
 * called after characteristic subscription update is done. call BleLayer for both error and success
 */
- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
                                          error:(NSError *)error
{
    // we're in Core Bluetooth work queue

    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
        return;
    }

    bool isNotifying = characteristic.isNotifying;

    if (nil == error) {
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            chip::Ble::ChipBleUUID svcId;
            chip::Ble::ChipBleUUID charId;
            [ChipBleDelegate fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];

            if (isNotifying) {
                _mBleLayer->HandleSubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
            } else {
                _mBleLayer->HandleUnsubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
            }
        });
    } else {
        CHIP_LOG_ERROR("[%s] BLE:Error subscribing/unsubcribing some characteristic on the device: [%@]", __PRETTY_FUNCTION__,
            error.localizedDescription);

        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            if (isNotifying) {
                // we're still notifying, so we must failed the unsubscription
                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_UNSUBSCRIBE_FAILED);
            } else {
                // we're not notifying, so we must failed the subscription
                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
            }
        });
    }
}

/**
 * part of CBPeripheralDelegate.
 * called when indication of some characteristic arrives. call BleLayer for both error and success
 */
- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
                              error:(NSError *)error
{
    // we're in Core Bluetooth work queue

    CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];
    if (dc == nil) {
        CHIP_LOG_ERROR("[%s] Cannot find a matching device manager for peripheral %@", __PRETTY_FUNCTION__, peripheral);
        return;
    }

    if (nil == error) {
        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            chip::Ble::ChipBleUUID svcId;
            chip::Ble::ChipBleUUID charId;
            [ChipBleDelegate fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];

            PacketBuffer * msgBuf;
            // build a inet buffer from the rxEv and send to blelayer.
            msgBuf = PacketBuffer::New();

            if (NULL != msgBuf) {
                memcpy(msgBuf->Start(), characteristic.value.bytes, characteristic.value.length);
                msgBuf->SetDataLength(characteristic.value.length);

                if (!_mBleLayer->HandleIndicationReceived((__bridge void *) peripheral, &svcId, &charId, msgBuf)) {
                    // since this error comes from device manager core
                    // we assume it would do the right thing, like closing the connection
                    CHIP_LOG_ERROR("[%s] Failed at handling incoming BLE data", __PRETTY_FUNCTION__);
                }
            } else {
                CHIP_LOG_ERROR("[%s] Failed at allocating buffer for incoming BLE data", __PRETTY_FUNCTION__);

                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, CHIP_ERROR_NO_MEMORY);
            }
        });
    } else {
        CHIP_LOG_ERROR("[%s] BLE:Error receiving indication of Characteristics on the device: [%@]", __PRETTY_FUNCTION__,
            error.localizedDescription);

        dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_INDICATE_FAILED);
        });
    }
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)SubscribeCharacteristic:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId
{
    dispatch_async(_mCbWorkQueue, ^{
        CBPeripheral * peripheral = (CBPeripheral *) connObj;
        CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];

        bool found = false;

        for (CBService * service in dc.blePeripheral.services) {
            if ([service.UUID.data isEqualToData:svcId.data]) {
                for (CBCharacteristic * characteristic in service.characteristics) {
                    if ([characteristic.UUID.data isEqualToData:charId.data]) {
                        found = true;
                        dispatch_async(_mCbWorkQueue, ^{
                            [dc.blePeripheral setNotifyValue:true forCharacteristic:characteristic];
                        });
                    }
                }
            }
        }

        if (!found) {
            CHIP_LOG_ERROR("[%s] Target peripheral doesn't have the specified service or characteristic", __PRETTY_FUNCTION__);

            dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
                _mBleLayer->HandleConnectionError((__bridge void *) dc.blePeripheral, BLE_ERROR_NOT_CHIP_DEVICE);
            });
        }
    });

    return true;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)UnsubscribeCharacteristic:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId
{
    dispatch_async(_mCbWorkQueue, ^{
        CBPeripheral * peripheral = (CBPeripheral *) connObj;
        CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];

        bool found = false;

        for (CBService * service in dc.blePeripheral.services) {
            if ([service.UUID.data isEqualToData:svcId.data]) {
                for (CBCharacteristic * characteristic in service.characteristics) {
                    if ([characteristic.UUID.data isEqualToData:charId.data]) {
                        found = true;
                        dispatch_async(_mCbWorkQueue, ^{
                            [dc.blePeripheral setNotifyValue:false forCharacteristic:characteristic];
                        });
                    }
                }
            }
        }

        if (!found) {
            CHIP_LOG_ERROR("[%s] Target peripheral doesn't have the specified service or characteristic", __PRETTY_FUNCTION__);

            dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
                _mBleLayer->HandleConnectionError((__bridge void *) dc.blePeripheral, BLE_ERROR_NOT_CHIP_DEVICE);
            });
        }
    });

    return true;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)CloseConnection:(id)connObj
{
    return true;
}

/**
 @note
 This method is only called from BleLayer
 */
- (uint16_t)GetMTU:(id)connObj
{
    return 0;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)SendIndication:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId data:(const NSData *)buf
{
    return false;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)SendWriteRequest:(id)connObj service:(const CBUUID *)svcId characteristic:(CBUUID *)charId data:(NSData *)buf
{
    dispatch_async(_mCbWorkQueue, ^{
        CBPeripheral * peripheral = (CBPeripheral *) connObj;
        CHIPDeviceController * dc = [_mMapFromPeripheralToDc objectForKey:peripheral];

        bool found = false;

        for (CBService * service in dc.blePeripheral.services) {
            if ([service.UUID.data isEqualToData:svcId.data]) {
                for (CBCharacteristic * characteristic in service.characteristics) {
                    if ([characteristic.UUID.data isEqualToData:charId.data]) {
                        found = true;
                        dispatch_async(_mCbWorkQueue, ^{
                            [dc.blePeripheral writeValue:buf
                                       forCharacteristic:characteristic
                                                    type:CBCharacteristicWriteWithResponse];
                        });
                    }
                }
            }
        }

        if (!found) {
            CHIP_LOG_ERROR("[%s] Target peripheral doesn't have the specified service or characteristic", __PRETTY_FUNCTION__);

            dispatch_async([[CHIPDeviceController sharedController] WorkQueue], ^{
                _mBleLayer->HandleConnectionError((__bridge void *) dc.blePeripheral, BLE_ERROR_NOT_CHIP_DEVICE);
            });
        }
    });

    return true;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)SendReadRequest:(id)connObj service:(const CBUUID *)svcId characteristic:(const CBUUID *)charId data:(const NSData *)buf
{
    return false;
}

/**
 @note
 This method is only called from BleLayer
 */
- (bool)SendReadResponse:(id)connObj
          requestContext:(id)readContext
                 service:(const CBUUID *)svcId
          characteristic:(const CBUUID *)charId
{
    return false;
}

/**
 @note
 This method is only called from BleLayer
 */
- (void)NotifyChipConnectionClosed:(id)connObj
{
}

@end
