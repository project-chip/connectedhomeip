/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <ble/BleConfig.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BleUUID.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/BleConnectionDelegate.h>
#include <setup_payload/SetupPayload.h>
#include <support/logging/CHIPLogging.h>

#import "UUIDHelper.h"

using namespace chip::Ble;

constexpr uint64_t kScanningTimeoutInSeconds = 60;

@interface BleConnection : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) dispatch_queue_t workQueue;
@property (strong, nonatomic) dispatch_queue_t chipWorkQueue;
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (strong, nonatomic) CBPeripheral * peripheral;
@property (strong, nonatomic) CBUUID * shortServiceUUID;
@property (nonatomic, readonly, nullable) dispatch_source_t timer;
@property (unsafe_unretained, nonatomic) bool found;
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
        BleConnection * ble;

        void BleConnectionDelegateImpl::NewConnection(Ble::BleLayer * bleLayer, void * appState, const uint16_t deviceDiscriminator)
        {
            ChipLogProgress(Ble, "%s", __FUNCTION__);
            ble = [[BleConnection alloc] initWithDiscriminator:deviceDiscriminator];
            [ble setBleLayer:bleLayer];
            ble.appState = appState;
            ble.onConnectionComplete = OnConnectionComplete;
            ble.onConnectionError = OnConnectionError;
            ble.centralManager = [ble.centralManager initWithDelegate:ble queue:ble.workQueue];
        }

        CHIP_ERROR BleConnectionDelegateImpl::CancelConnection()
        {
            ChipLogProgress(Ble, "%s", __FUNCTION__);
            [ble stop];
            ble = nil;
            return CHIP_NO_ERROR;
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
        self.shortServiceUUID = [UUIDHelper GetShortestServiceUUID:&chip::Ble::CHIP_BLE_SVC_ID];
        _deviceDiscriminator = deviceDiscriminator;
        _workQueue = dispatch_queue_create("com.chip.ble.work_queue", DISPATCH_QUEUE_SERIAL);
        _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();
        _timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _workQueue);
        _centralManager = [CBCentralManager alloc];
        _found = false;

        dispatch_source_set_event_handler(_timer, ^{
            [self stop];
            _onConnectionError(_appState, BLE_ERROR_APP_CLOSED_CONNECTION);
        });
        dispatch_source_set_timer(
            _timer, dispatch_walltime(NULL, kScanningTimeoutInSeconds * NSEC_PER_SEC), DISPATCH_TIME_FOREVER, 5 * NSEC_PER_SEC);
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
        _onConnectionError(_appState, BLE_ERROR_APP_CLOSED_CONNECTION);
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

                NSUInteger length = [serviceData length];
                if (length == 7) {
                    const uint8_t * bytes = (const uint8_t *) [serviceData bytes];
                    uint8_t opCode = bytes[0];
                    uint16_t discriminator = (bytes[1] | (bytes[2] << 8)) & 0xfff;

                    if ((opCode == 0 || opCode == 1) && [self checkDiscriminator:discriminator]) {
                        ChipLogProgress(Ble, "Connecting to device with discriminator: %d", discriminator);
                        [self connect:peripheral];
                        [self stopScanning];
                    }
                }

                break;
            }
        }
    }
}

- (BOOL)checkDiscriminator:(uint16_t)discriminator
{
    // If the manual setup discriminator was passed in, only match the most significant 4 bits from the BLE advertisement
    constexpr uint16_t manualSetupDiscriminatorOffsetInBits
        = chip::kPayloadDiscriminatorFieldLengthInBits - chip::kManualSetupDiscriminatorFieldLengthInBits;
    constexpr uint16_t maxManualDiscriminatorValue = (1 << chip::kManualSetupDiscriminatorFieldLengthInBits) - 1;
    constexpr uint16_t kManualSetupDiscriminatorFieldBitMask = maxManualDiscriminatorValue << manualSetupDiscriminatorOffsetInBits;
    if (_deviceDiscriminator == (_deviceDiscriminator & kManualSetupDiscriminatorFieldBitMask)) {
        return _deviceDiscriminator == (discriminator & kManualSetupDiscriminatorFieldBitMask);
    } else {
        // else compare the entire thing
        return _deviceDiscriminator == discriminator;
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
        ChipLogError(Ble, "BLE:Error finding Chip Service in the device: [%s]", [error.localizedDescription UTF8String]);
    }

    for (CBService * service in peripheral.services) {
        if ([service.UUID.data isEqualToData:_shortServiceUUID.data] && !self.found) {
            [peripheral discoverCharacteristics:nil forService:service];
            self.found = true;
            break;
        }
    }

    if (!self.found || error != nil) {
        ChipLogError(Ble, "Service not found on the device.");
        _onConnectionError(_appState, CHIP_ERROR_INCORRECT_STATE);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    if (nil != error) {
        ChipLogError(
            Ble, "BLE:Error finding Characteristics in Chip service on the device: [%s]", [error.localizedDescription UTF8String]);
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
        dispatch_async(_chipWorkQueue, ^{
            _mBleLayer->HandleWriteConfirmation((__bridge void *) peripheral, &svcId, &charId);
        });
    } else {
        ChipLogError(
            Ble, "BLE:Error writing Characteristics in Chip service on the device: [%s]", [error.localizedDescription UTF8String]);
        dispatch_async(_chipWorkQueue, ^{
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_WRITE_FAILED);
        });
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

        dispatch_async(_chipWorkQueue, ^{
            if (isNotifying) {
                _mBleLayer->HandleSubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
            } else {
                _mBleLayer->HandleUnsubscribeComplete((__bridge void *) peripheral, &svcId, &charId);
            }
        });
    } else {
        ChipLogError(Ble, "BLE:Error subscribing/unsubcribing some characteristic on the device: [%s]",
            [error.localizedDescription UTF8String]);
        dispatch_async(_chipWorkQueue, ^{
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

- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
                              error:(NSError *)error
{
    if (nil == error) {
        chip::Ble::ChipBleUUID svcId;
        chip::Ble::ChipBleUUID charId;
        [BleConnection fillServiceWithCharacteristicUuids:characteristic svcId:&svcId charId:&charId];

        // build a inet buffer from the rxEv and send to blelayer.
        __block chip::System::PacketBufferHandle msgBuf
            = chip::System::PacketBufferHandle::NewWithData(characteristic.value.bytes, characteristic.value.length);

        if (!msgBuf.IsNull()) {
            dispatch_async(_chipWorkQueue, ^{
                if (!_mBleLayer->HandleIndicationReceived((__bridge void *) peripheral, &svcId, &charId, std::move(msgBuf))) {
                    // since this error comes from device manager core
                    // we assume it would do the right thing, like closing the connection
                    ChipLogError(Ble, "Failed at handling incoming BLE data");
                }
            });
        } else {
            ChipLogError(Ble, "Failed at allocating buffer for incoming BLE data");
            dispatch_async(_chipWorkQueue, ^{
                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, CHIP_ERROR_NO_MEMORY);
            });
        }
    } else {
        ChipLogError(
            Ble, "BLE:Error receiving indication of Characteristics on the device: [%s]", [error.localizedDescription UTF8String]);
        dispatch_async(_chipWorkQueue, ^{
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_INDICATE_FAILED);
        });
    }
}

// End CBPeripheralDelegate

- (void)start
{
    dispatch_resume(_timer);
    [self startScanning];
}

- (void)stop
{
    [self stopScanning];
    [self disconnect];
    _centralManager.delegate = nil;
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
    dispatch_source_cancel(_timer);
    [_centralManager stopScan];
}

- (void)connect:(CBPeripheral *)peripheral
{
    if (!_centralManager || !peripheral) {
        return;
    }

    _peripheral = peripheral;
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

- (void)setBleLayer:(chip::Ble::BleLayer *)bleLayer
{
    _mBleLayer = bleLayer;
}

@end
