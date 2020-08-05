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
 *          Provides an implementation of BleConnectionDelegate for Darwin platforms.
 */

#include <ble/BleConfig.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BleUUID.h>
#include <platform/Darwin/BleConnectionDelegate.h>
#include <support/logging/CHIPLogging.h>

#import <CoreBluetooth/CoreBluetooth.h>

@interface BleConnection : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) dispatch_queue_t workQueue;
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (strong, nonatomic) CBPeripheral * peripheral;
@property (strong, nonatomic) CBUUID * shortServiceUUID;
@property (unsafe_unretained, nonatomic) uint16_t deviceDiscriminator;
@property (unsafe_unretained, nonatomic) void * appState;
@property (unsafe_unretained, nonatomic) BleConnectionDelegate::OnConnectionCompleteFunct onConnectionComplete;
@property (unsafe_unretained, nonatomic) BleConnectionDelegate::OnConnectionErrorFunct onConnectionError;
@property (unsafe_unretained, nonatomic) chip::Ble::BleLayer * mBleLayer;

- (id)initWithDiscriminator:(uint16_t)deviceDiscriminator;
- (void)setBleLayer:(chip::Ble::BleLayer *)bleLayer;
- (void)start;
- (void)stop;

@end

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        void BleConnectionDelegateImpl::NewConnection(Ble::BleLayer * bleLayer, void * appState, const uint16_t deviceDiscriminator)
        {
            ChipLogProgress(Ble, "%s", __FUNCTION__);
            BleConnection * ble = [[BleConnection alloc] initWithDiscriminator:deviceDiscriminator];
            [ble setBleLayer:bleLayer];
            ble.appState = appState;
            ble.onConnectionComplete = OnConnectionComplete;
            ble.onConnectionError = OnConnectionError;
        }
    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip

@interface BleConnection ()
@end

@implementation BleConnection

- (id)initWithDiscriminator:(uint16_t)deviceDiscriminator
{
    self = [super init];
    if (self) {
        _deviceDiscriminator = deviceDiscriminator;
        _workQueue = dispatch_queue_create("com.chip.ble.work_queue", DISPATCH_QUEUE_SERIAL);
        _centralManager = [CBCentralManager alloc];
        _shortServiceUUID = [BleConnection getShortestServiceUUID:&chip::Ble::CHIP_BLE_SVC_ID];
        [_centralManager initWithDelegate:self queue:_workQueue];
    }

    return self;
}

// Start CBCentralManagerDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
    case CBManagerStatePoweredOn:
        ChipLogDetail(Ble, "CBManagerState: ON");
        [self start];
        break;
    case CBManagerStatePoweredOff:
        ChipLogDetail(Ble, "CBManagerState: OFF");
        [self stop];
        break;
    case CBManagerStateUnauthorized:
        ChipLogDetail(Ble, "CBManagerState: Unauthorized");
        break;
    case CBManagerStateResetting:
        ChipLogDetail(Ble, "CBManagerState: RESETTING");
        break;
    case CBManagerStateUnsupported:
        ChipLogDetail(Ble, "CBManagerState: UNSUPPORTED");
        break;
    case CBManagerStateUnknown:
        ChipLogDetail(Ble, "CBManagerState: UNKNOWN");
        break;
    }
}

- (void)centralManager:(CBCentralManager *)central
    didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
                     RSSI:(NSNumber *)RSSI
{
    NSNumber * isConnectable = [advertisementData objectForKey:CBAdvertisementDataIsConnectable];
    if ([isConnectable boolValue]) {
        NSDictionary * servicesData = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
        for (CBUUID * serviceUUID in servicesData) {
            if ([serviceUUID.data isEqualToData:_shortServiceUUID.data]) {
                NSData * serviceData = [servicesData objectForKey:serviceUUID];

                int length = [serviceData length];
                if (length == 3) {
                    const uint8_t * bytes = (const uint8_t *) [serviceData bytes];
                    uint8_t opCode = bytes[0];
                    uint16_t discriminator = ((bytes[1] & 0x0F) << 8) | bytes[2];
                    if (opCode == 0 && discriminator == _deviceDiscriminator) {
                        ChipLogProgress(Ble, "Connecting to device: %@", peripheral);
                        [self connect:peripheral];
                        [self stopScanning];
                    }
                }

                break;
            }
        }
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
    [peripheral setDelegate:self];
    [peripheral discoverServices:nil];
}

// End CBCentralManagerDelegate

// Start CBPeripheralDelegate

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
    if (nil != error) {
        ChipLogError(Ble, "BLE:Error finding Chip Service in the device: [%@]", error.localizedDescription);
    }

    bool found;

    CBUUID * chipServiceUUID = [CBUUID UUIDWithData:[NSData dataWithBytes:&chip::Ble::CHIP_BLE_SVC_ID.bytes length:16]];
    for (CBService * service in peripheral.services) {
        if ([service.UUID.data isEqualToData:chipServiceUUID.data]) {
            found = true;
            [peripheral discoverCharacteristics:nil forService:service];
            break;
        }
    }

    if (!found || error != nil) {
        _onConnectionError(_appState, BLE_ERROR_INCORRECT_STATE);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    if (nil != error) {
        ChipLogError(Ble, "BLE:Error finding Characteristics in Chip service on the device: [%@]", error.localizedDescription);
    }

    // XXX error ?
    _onConnectionComplete(_appState, (__bridge void *) peripheral);
}

- (void)peripheral:(CBPeripheral *)peripheral
    didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
                             error:(NSError *)error
{
    if (nil == error) {
        chip::Ble::ChipBleUUID svcId;
        chip::Ble::ChipBleUUID charId;
        [BleConnection fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];
        _mBleLayer->HandleWriteConfirmation((__bridge void *) peripheral, &svcId, &charId);
    } else {
        ChipLogError(Ble, "BLE:Error writing Characteristics in Chip service on the device: [%@]", error.localizedDescription);
        _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_WRITE_FAILED);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
                                          error:(NSError *)error
{
    bool isNotifying = characteristic.isNotifying;

    if (nil == error) {
        chip::Ble::ChipBleUUID svcId;
        chip::Ble::ChipBleUUID charId;
        [BleConnection fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];

        if (isNotifying) {
            _mBleLayer->HandleSubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
        } else {
            _mBleLayer->HandleUnsubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
        }
    } else {
        ChipLogError(Ble, "BLE:Error subscribing/unsubcribing some characteristic on the device: [%@]", error.localizedDescription);
        if (isNotifying) {
            // we're still notifying, so we must failed the unsubscription
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_UNSUBSCRIBE_FAILED);
        } else {
            // we're not notifying, so we must failed the subscription
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
                              error:(NSError *)error
{
    if (nil == error) {
        chip::Ble::ChipBleUUID svcId;
        chip::Ble::ChipBleUUID charId;
        [BleConnection fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];

        // build a inet buffer from the rxEv and send to blelayer.
        chip::System::PacketBuffer * msgBuf = chip::System::PacketBuffer::New();

        if (NULL != msgBuf) {
            memcpy(msgBuf->Start(), characteristic.value.bytes, characteristic.value.length);
            msgBuf->SetDataLength(characteristic.value.length);

            if (!_mBleLayer->HandleIndicationReceived((__bridge void *) peripheral, &svcId, &charId, msgBuf)) {
                // since this error comes from device manager core
                // we assume it would do the right thing, like closing the connection
                ChipLogError(Ble, "Failed at handling incoming BLE data");
            }
        } else {
            ChipLogError(Ble, "Failed at allocating buffer for incoming BLE data");
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_NO_MEMORY);
        }
    } else {
        ChipLogError(Ble, "BLE:Error receiving indication of Characteristics on the device: [%@]", error.localizedDescription);
        _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_INDICATE_FAILED);
    }
}

// End CBPeripheralDelegate

- (void)start
{
    [self startScanning];
}

- (void)stop
{
    [self stopScanning];
    [self disconnect];
    _centralManager = nil;
    _peripheral = nil;
}

- (void)startScanning
{
    if (!_centralManager) {
        return;
    }

    [_centralManager scanForPeripheralsWithServices:@[ _shortServiceUUID ] options:nil];
}

- (void)stopScanning
{
    if (!_centralManager) {
        return;
    }

    [_centralManager stopScan];
}

- (void)connect:(CBPeripheral *)peripheral
{
    if (!_centralManager || !peripheral) {
        return;
    }

    _peripheral = peripheral;
    [_peripheral retain];
    [_centralManager connectPeripheral:peripheral options:nil];
}

- (void)disconnect
{
    if (!_centralManager || !_peripheral) {
        return;
    }

    [_centralManager cancelPeripheralConnection:_peripheral];
    _peripheral = nil;
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
        ChipLogError(Ble, "UUID of characteristic is incompatible");
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
        ChipLogError(Ble, "Service UUIDs are incompatible");
    }
    memcpy(svcId->bytes, serviceFullUUID, sizeof(svcId->bytes));
}

+ (CBUUID *)getShortestServiceUUID:(const chip::Ble::ChipBleUUID *)svcId
{
    // shorten the 16-byte UUID reported by BLE Layer to shortest possible, 2 or 4 bytes
    // this is the BLE Service UUID Base. If a 16-byte service UUID partially matches with this 12 bytes,
    // it could be shorten to 2 or 4 bytes.
    static const uint8_t bleBaseUUID[12] = { 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
    if (0 == memcmp(svcId->bytes + 4, bleBaseUUID, sizeof(bleBaseUUID))) {
        // okay, let's try to shorten it
        if ((0 == svcId->bytes[0]) && (0 == svcId->bytes[1])) {
            // the highest 2 bytes are both 0, so we just need 2 bytes
            return [CBUUID UUIDWithData:[NSData dataWithBytes:(svcId->bytes + 2) length:2]];
        } else {
            // we need to use 4 bytes
            return [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:4]];
        }
    } else {
        // it cannot be shorten as it doesn't match with the BLE Service UUID Base
        return [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
    }
}

- (void)setBleLayer:(chip::Ble::BleLayer *)bleLayer
{
    _mBleLayer = bleLayer;
}

@end
