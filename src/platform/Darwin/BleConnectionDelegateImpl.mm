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

#include <ble/Ble.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/BleConnectionDelegate.h>
#include <platform/Darwin/BleScannerDelegate.h>
#include <platform/LockTracker.h>
#include <setup_payload/SetupPayload.h>
#include <tracing/metric_event.h>

#import "MTRUUIDHelper.h"
#import "PlatformMetricKeys.h"

using namespace chip::Ble;
using namespace chip::DeviceLayer;
using namespace chip::Tracing::DarwinPlatform;

constexpr uint64_t kScanningWithDiscriminatorTimeoutInSeconds = 60;
constexpr uint64_t kPreWarmScanTimeoutInSeconds = 120;
constexpr uint64_t kCachePeripheralTimeoutInSeconds
    = static_cast<uint64_t>(CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX / 1000.0 * 8.0 * 0.625);
constexpr char kBleWorkQueueName[] = "org.csa-iot.matter.framework.ble.workqueue";

typedef NS_ENUM(uint8_t, BleConnectionMode) {
    kUndefined = 0,
    kScanning,
    kScanningWithTimeout,
    kConnecting,
};

@interface BleConnection : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) dispatch_queue_t chipWorkQueue;
@property (strong, nonatomic) dispatch_queue_t workQueue;
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (strong, nonatomic) CBPeripheral * peripheral;
@property (strong, nonatomic) CBUUID * shortServiceUUID;
@property (nonatomic, readonly, nullable) dispatch_source_t timer;
@property (nonatomic, readonly) BleConnectionMode currentMode;
@property (strong, nonatomic) NSMutableDictionary<CBPeripheral *, NSDictionary *> * cachedPeripherals;
@property (unsafe_unretained, nonatomic) bool found;
@property (unsafe_unretained, nonatomic) chip::SetupDiscriminator deviceDiscriminator;
@property (unsafe_unretained, nonatomic) void * appState;
@property (unsafe_unretained, nonatomic) BleConnectionDelegate::OnConnectionCompleteFunct onConnectionComplete;
@property (unsafe_unretained, nonatomic) BleConnectionDelegate::OnConnectionErrorFunct onConnectionError;
@property (unsafe_unretained, nonatomic) chip::DeviceLayer::BleScannerDelegate * scannerDelegate;
@property (unsafe_unretained, nonatomic) chip::Ble::BleLayer * mBleLayer;

- (id)initWithQueue:(dispatch_queue_t)queue;
- (id)initWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm queue:(dispatch_queue_t)queue;
- (id)initWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator queue:(dispatch_queue_t)queue;
- (void)setBleLayer:(chip::Ble::BleLayer *)bleLayer;
- (void)start;
- (void)stop;
- (void)updateWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm;
- (void)updateWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator;
- (void)updateWithPeripheral:(CBPeripheral *)peripheral;
- (BOOL)isConnecting;
- (void)addPeripheralToCache:(CBPeripheral *)peripheral data:(NSData *)data;
- (void)removePeripheralFromCache:(CBPeripheral *)peripheral;
- (void)removePeripheralsFromCache;

@end

namespace chip {
namespace DeviceLayer {
    namespace Internal {
        BleConnection * ble;
        dispatch_queue_t bleWorkQueue;

        void BleConnectionDelegateImpl::NewConnection(
            Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & inDeviceDiscriminator)
        {
            assertChipStackLockedByCurrentThread();

            // Make a copy of the device discriminator for the block to capture.
            SetupDiscriminator deviceDiscriminator = inDeviceDiscriminator;

            ChipLogProgress(Ble, "ConnectionDelegate NewConnection with discriminator");
            if (!bleWorkQueue) {
                bleWorkQueue = dispatch_queue_create(kBleWorkQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            }

            dispatch_async(bleWorkQueue, ^{
                // If the previous connection delegate was not a try to connect to something, just reuse it instead of
                // creating a brand new connection but update the discriminator and the ble layer members.
                if (ble and ![ble isConnecting]) {
                    [ble setBleLayer:bleLayer];
                    ble.appState = appState;
                    ble.onConnectionComplete = OnConnectionComplete;
                    ble.onConnectionError = OnConnectionError;
                    [ble updateWithDiscriminator:deviceDiscriminator];
                    return;
                }

                [ble stop];
                ble = [[BleConnection alloc] initWithDiscriminator:deviceDiscriminator queue:bleWorkQueue];
                [ble setBleLayer:bleLayer];
                ble.appState = appState;
                ble.onConnectionComplete = OnConnectionComplete;
                ble.onConnectionError = OnConnectionError;
                ble.centralManager = [ble.centralManager initWithDelegate:ble queue:bleWorkQueue];
            });
        }

        void BleConnectionDelegateImpl::NewConnection(Ble::BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj)
        {
            assertChipStackLockedByCurrentThread();

            ChipLogProgress(Ble, "ConnectionDelegate NewConnection with conn obj: %p", connObj);

            if (!bleWorkQueue) {
                bleWorkQueue = dispatch_queue_create(kBleWorkQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            }

            CBPeripheral * peripheral = (__bridge CBPeripheral *) connObj; // bridge (and retain) before dispatching
            dispatch_async(bleWorkQueue, ^{
                // The BLE_CONNECTION_OBJECT represent a CBPeripheral object. In order for it to be valid the central
                // manager needs to still be running.
                if (!ble || [ble isConnecting]) {
                    if (OnConnectionError) {
                        auto workQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();
                        dispatch_async(workQueue, ^{
                            OnConnectionError(appState, CHIP_ERROR_INCORRECT_STATE);
                        });
                    }
                    return;
                }

                [ble setBleLayer:bleLayer];
                ble.appState = appState;
                ble.onConnectionComplete = OnConnectionComplete;
                ble.onConnectionError = OnConnectionError;
                [ble updateWithPeripheral:peripheral];
            });
        }

        void BleConnectionDelegateImpl::StartScan(BleScannerDelegate * delegate, BleScanMode mode)
        {
            assertChipStackLockedByCurrentThread();

            bool prewarm = (mode == BleScanMode::kPreWarm);
            ChipLogProgress(Ble, "ConnectionDelegate StartScan (%s)", (prewarm ? "pre-warm" : "default"));

            if (!bleWorkQueue) {
                bleWorkQueue = dispatch_queue_create(kBleWorkQueueName, DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            }

            dispatch_async(bleWorkQueue, ^{
                // Pre-warming is best-effort, don't cancel an ongoing scan or connection attempt
                if (prewarm && ble) {
                    // TODO: Once we get rid of the separate BLE queue we can just return CHIP_ERROR_BUSY.
                    // That will also allow these cases to be distinguished in our metric.
                    ChipLogProgress(Ble, "Not starting pre-warm scan, an operation is already in progress");
                    if (delegate) {
                        dispatch_async(PlatformMgrImpl().GetWorkQueue(), ^{
                            delegate->OnBleScanStopped();
                        });
                    }
                    return;
                }

                // If the previous connection delegate was not a try to connect to something, just reuse it instead of
                // creating a brand new connection but update the discriminator and the ble layer members.
                if (ble and ![ble isConnecting]) {
                    [ble updateWithDelegate:delegate prewarm:prewarm];
                    return;
                }

                [ble stop];
                ble = [[BleConnection alloc] initWithDelegate:delegate prewarm:prewarm queue:bleWorkQueue];
                // Do _not_ set onConnectionComplete and onConnectionError
                // here.  The connection callbacks we have expect an appState
                // that we do not have here, and in any case connection
                // complete/error make no sense for a scan.
                ble.centralManager = [ble.centralManager initWithDelegate:ble queue:bleWorkQueue];
            });
        }

        void BleConnectionDelegateImpl::StopScan()
        {
            ChipLogProgress(Ble, "ConnectionDelegate StopScan");
            DoCancel();
        }

        CHIP_ERROR BleConnectionDelegateImpl::CancelConnection()
        {
            ChipLogProgress(Ble, "ConnectionDelegate CancelConnection");
            return DoCancel();
        }

        CHIP_ERROR BleConnectionDelegateImpl::DoCancel()
        {
            assertChipStackLockedByCurrentThread();
            if (bleWorkQueue == nil) {
                return CHIP_NO_ERROR;
            }

            dispatch_async(bleWorkQueue, ^{
                [ble stop];
                ble = nil;
            });

            bleWorkQueue = nil;
            return CHIP_NO_ERROR;
        }
    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip

@interface BleConnection ()
@property (nonatomic, readonly) int32_t totalDevicesAdded;
@property (nonatomic, readonly) int32_t totalDevicesRemoved;
@end

@implementation BleConnection

- (id)initWithQueue:(dispatch_queue_t)queue
{
    self = [super init];
    if (self) {
        self.shortServiceUUID = [MTRUUIDHelper GetShortestServiceUUID:&chip::Ble::CHIP_BLE_SVC_ID];
        _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();
        _workQueue = queue;
        _centralManager = [CBCentralManager alloc];
        _found = false;
        _cachedPeripherals = [[NSMutableDictionary alloc] init];
        _currentMode = kUndefined;
        [self _resetCounters];
    }

    return self;
}

- (id)initWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm queue:(dispatch_queue_t)queue
{
    self = [self initWithQueue:queue];
    if (self) {
        _scannerDelegate = delegate;
        if (prewarm) {
            _currentMode = kScanningWithTimeout;
            [self setupTimer:kPreWarmScanTimeoutInSeconds];
        } else {
            _currentMode = kScanning;
        }
    }

    return self;
}

- (id)initWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator queue:(dispatch_queue_t)queue
{
    self = [self initWithQueue:queue];
    if (self) {
        _deviceDiscriminator = deviceDiscriminator;
        _currentMode = kConnecting;
        [self setupTimer:kScanningWithDiscriminatorTimeoutInSeconds];
    }

    return self;
}

- (BOOL)isConnecting
{
    return _currentMode == kConnecting;
}

- (void)setupTimer:(uint64_t)timeout
{
    [self clearTimer];

    _timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _workQueue);
    dispatch_source_set_event_handler(_timer, ^{
        ChipLogProgress(Ble, "ConnectionDelegate timeout");
        [self stop];
        [self dispatchConnectionError:BLE_ERROR_APP_CLOSED_CONNECTION];
    });

    auto value = static_cast<int64_t>(timeout * NSEC_PER_SEC);
    dispatch_source_set_timer(_timer, dispatch_walltime(nullptr, value), DISPATCH_TIME_FOREVER, 5 * NSEC_PER_SEC);
    dispatch_resume(_timer);
}

- (void)clearTimer
{
    if (_timer) {
        dispatch_source_cancel(_timer);
        _timer = nil;
    }
}

// All our callback dispatch must happen on _chipWorkQueue
- (void)dispatchConnectionError:(CHIP_ERROR)error
{
    dispatch_async(_chipWorkQueue, ^{
        if (self.onConnectionError != nil) {
            self.onConnectionError(self.appState, error);
        }
    });
}

- (void)dispatchConnectionComplete:(CBPeripheral *)peripheral
{
    dispatch_async(_chipWorkQueue, ^{
        if (self.onConnectionComplete != nil) {
            self.onConnectionComplete(self.appState, (__bridge void *) peripheral);
        }
    });
}

// Start CBCentralManagerDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    MATTER_LOG_METRIC(kMetricBLECentralManagerState, static_cast<uint32_t>(central.state));

    switch (central.state) {
    case CBManagerStatePoweredOn:
        ChipLogDetail(Ble, "CBManagerState: ON");
        [self start];
        break;
    case CBManagerStatePoweredOff:
        ChipLogDetail(Ble, "CBManagerState: OFF");
        [self stop];
        [self dispatchConnectionError:BLE_ERROR_APP_CLOSED_CONNECTION];
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
    NSDictionary * servicesData = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
    NSData * serviceData;
    for (CBUUID * serviceUUID in servicesData) {
        if ([serviceUUID.data isEqualToData:_shortServiceUUID.data]) {
            serviceData = [servicesData objectForKey:serviceUUID];
            break;
        }
    }

    if (!serviceData) {
        return;
    }

    NSNumber * isConnectable = [advertisementData objectForKey:CBAdvertisementDataIsConnectable];
    MATTER_LOG_METRIC_END(kMetricBLEDiscoveredPeripheral, [isConnectable boolValue]);

    if ([isConnectable boolValue] == NO) {
        ChipLogError(Ble, "A device (%p) with a matching Matter UUID has been discovered but it is not connectable.", peripheral);
        return;
    }

    const uint8_t * bytes = (const uint8_t *) [serviceData bytes];
    if ([serviceData length] != sizeof(ChipBLEDeviceIdentificationInfo)) {
        NSMutableString * hexString = [NSMutableString stringWithCapacity:([serviceData length] * 2)];
        for (NSUInteger i = 0; i < [serviceData length]; i++) {
            [hexString appendString:[NSString stringWithFormat:@"%02lx", (unsigned long) bytes[i]]];
        }
        ChipLogError(Ble,
            "A device (%p) with a matching Matter UUID has been discovered but the service data len does not match our expectation "
            "(serviceData = %s)",
            peripheral, [hexString UTF8String]);
        MATTER_LOG_METRIC(kMetricBLEBadServiceDataLength, static_cast<uint32_t>([serviceData length]));
        return;
    }

    uint8_t opCode = bytes[0];
    if (opCode != 0 && opCode != 1) {
        ChipLogError(Ble,
            "A device (%p) with a matching Matter UUID has been discovered but the service data opCode not match our expectation "
            "(opCode = %u).",
            peripheral, opCode);
        MATTER_LOG_METRIC(kMetricBLEBadOpCode, opCode);
        return;
    }

    uint16_t discriminator = (bytes[1] | (bytes[2] << 8)) & 0xfff;

    if ([self isConnecting]) {
        if (![self checkDiscriminator:discriminator]) {
            ChipLogError(Ble,
                "A device (%p) with a matching Matter UUID has been discovered but the service data discriminator not match our "
                "expectation (discriminator = %u).",
                peripheral, discriminator);
            MATTER_LOG_METRIC(kMetricBLEMismatchedDiscriminator);
            return;
        }

        MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredMatchingPeripheral);
        ChipLogProgress(Ble, "Connecting to device %p with discriminator: %d", peripheral, discriminator);
        [self connect:peripheral];
        [self stopScanning];
    } else {
        [self addPeripheralToCache:peripheral data:serviceData];
    }
}

- (BOOL)checkDiscriminator:(uint16_t)discriminator
{
    return _deviceDiscriminator.MatchesLongDiscriminator(discriminator);
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
    MATTER_LOG_METRIC_END(kMetricBLEConnectPeripheral);
    MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredServices);
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

    MATTER_LOG_METRIC_END(kMetricBLEDiscoveredServices, CHIP_ERROR(chip::ChipError::Range::kOS, static_cast<uint32_t>(error.code)));

    for (CBService * service in peripheral.services) {
        if ([service.UUID.data isEqualToData:_shortServiceUUID.data] && !self.found) {
            MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredCharacteristics);
            [peripheral discoverCharacteristics:nil forService:service];
            self.found = true;
            break;
        }
    }

    if (!self.found || error != nil) {
        ChipLogError(Ble, "Service not found on the device.");
        MATTER_LOG_METRIC(kMetricBLEDiscoveredServices, CHIP_ERROR_INCORRECT_STATE);
        [self dispatchConnectionError:CHIP_ERROR_INCORRECT_STATE];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    MATTER_LOG_METRIC_END(kMetricBLEDiscoveredCharacteristics, CHIP_ERROR(chip::ChipError::Range::kOS, static_cast<uint32_t>(error.code)));

    if (nil != error) {
        ChipLogError(
            Ble, "BLE:Error finding Characteristics in Chip service on the device: [%s]", [error.localizedDescription UTF8String]);
    }

    // XXX error ?
    [self dispatchConnectionComplete:peripheral];
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
            MATTER_LOG_METRIC(kMetricBLEWriteChrValueFailed, BLE_ERROR_GATT_WRITE_FAILED);
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
                MATTER_LOG_METRIC(kMetricBLEUpdateNotificationStateForChrFailed, BLE_ERROR_GATT_WRITE_FAILED);
                // we're still notifying, so we must failed the unsubscription
                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_UNSUBSCRIBE_FAILED);
            } else {
                // we're not notifying, so we must failed the subscription
                MATTER_LOG_METRIC(kMetricBLEUpdateNotificationStateForChrFailed, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
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
        auto * value = characteristic.value; // read immediately before dispatching

        dispatch_async(_chipWorkQueue, ^{
            // build a inet buffer from the rxEv and send to blelayer.
            auto msgBuf = chip::System::PacketBufferHandle::NewWithData(value.bytes, value.length);

            if (msgBuf.IsNull()) {
                ChipLogError(Ble, "Failed at allocating buffer for incoming BLE data");
                MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, CHIP_ERROR_NO_MEMORY);
                _mBleLayer->HandleConnectionError((__bridge void *) peripheral, CHIP_ERROR_NO_MEMORY);
            } else if (!_mBleLayer->HandleIndicationReceived((__bridge void *) peripheral, &svcId, &charId, std::move(msgBuf))) {
                // since this error comes from device manager core
                // we assume it would do the right thing, like closing the connection
                ChipLogError(Ble, "Failed at handling incoming BLE data");
                MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, CHIP_ERROR_INCORRECT_STATE);
            }
        });
    } else {
        ChipLogError(
            Ble, "BLE:Error receiving indication of Characteristics on the device: [%s]", [error.localizedDescription UTF8String]);
        dispatch_async(_chipWorkQueue, ^{
            MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, BLE_ERROR_GATT_INDICATE_FAILED);
            _mBleLayer->HandleConnectionError((__bridge void *) peripheral, BLE_ERROR_GATT_INDICATE_FAILED);
        });
    }
}

// End CBPeripheralDelegate

- (void)start
{
    // If a peripheral has already been found, try to connect to it once BLE starts,
    // otherwise start scanning to find the peripheral to connect to.
    if (_peripheral != nil) {
        MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredMatchingPeripheral);
        [self connect:_peripheral];
    } else {
        [self startScanning];
    }
}

- (void)stop
{
    [self detachScannerDelegate];
    _found = false;
    [self stopScanning];
    [self removePeripheralsFromCache];

    if (!_centralManager && !_peripheral) {
        return;
    }

    // Properly closing the underlying ble connections needs to happens
    // on the chip work queue. At the same time the SDK is trying to
    // properly unsubscribe and shutdown the connection, so if we nullify
    // the centralManager and the peripheral members too early it won't be
    // able to reach those.
    // This is why closing connections happens as 2 async steps.
    dispatch_async(_chipWorkQueue, ^{
        if (_peripheral) {
            _mBleLayer->CloseAllBleConnections();
        }

        dispatch_async(_workQueue, ^{
            _centralManager.delegate = nil;
            _centralManager = nil;
            _peripheral = nil;
            if (chip::DeviceLayer::Internal::ble == self) {
                chip::DeviceLayer::Internal::ble = nil;
            }
        });
    });
}

- (void)_resetCounters
{
    _totalDevicesAdded = 0;
    _totalDevicesRemoved = 0;
}

- (void)startScanning
{
    if (!_centralManager) {
        return;
    }

    MATTER_LOG_METRIC_BEGIN(kMetricBLEScan);
    MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredPeripheral);
    MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredMatchingPeripheral);
    [self _resetCounters];

    auto scanOptions = @{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES };
    [_centralManager scanForPeripheralsWithServices:@[ _shortServiceUUID ] options:scanOptions];
}

- (void)stopScanning
{
    if (!_centralManager) {
        return;
    }

    MATTER_LOG_METRIC_END(kMetricBLEScan);
    [self _resetCounters];

    [self clearTimer];
    [_centralManager stopScan];
}

- (void)connect:(CBPeripheral *)peripheral
{
    if (!_centralManager || !peripheral) {
        return;
    }

    MATTER_LOG_METRIC_END(kMetricBLEDiscoveredMatchingPeripheral);
    MATTER_LOG_METRIC_BEGIN(kMetricBLEConnectPeripheral);
    _peripheral = peripheral;
    [_centralManager connectPeripheral:peripheral options:nil];
}

- (void)detachScannerDelegate
{
    auto * existingDelegate = _scannerDelegate;
    if (existingDelegate) {
        _scannerDelegate = nullptr;
        dispatch_async(_chipWorkQueue, ^{
            existingDelegate->OnBleScanStopped();
        });
    }
}

- (void)updateWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm
{
    [self detachScannerDelegate];

    if (delegate) {
        for (CBPeripheral * cachedPeripheral in _cachedPeripherals) {
            NSData * serviceData = _cachedPeripherals[cachedPeripheral][@"data"];
            dispatch_async(_chipWorkQueue, ^{
                ChipBLEDeviceIdentificationInfo info;
                memcpy(&info, [serviceData bytes], sizeof(info));
                delegate->OnBleScanAdd((__bridge void *) cachedPeripheral, info);
            });
        }
        _scannerDelegate = delegate;
    }

    if (prewarm) {
        _currentMode = kScanningWithTimeout;
        [self setupTimer:kPreWarmScanTimeoutInSeconds];
    } else {
        _currentMode = kScanning;
        [self clearTimer];
    }
}

- (void)updateWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator
{
    [self detachScannerDelegate];
    _deviceDiscriminator = deviceDiscriminator;
    _currentMode = kConnecting;

    CBPeripheral * peripheral = nil;
    for (CBPeripheral * cachedPeripheral in _cachedPeripherals) {
        NSData * serviceData = _cachedPeripherals[cachedPeripheral][@"data"];
        ChipBLEDeviceIdentificationInfo info;
        memcpy(&info, [serviceData bytes], sizeof(info));

        if ([self checkDiscriminator:info.GetDeviceDiscriminator()]) {
            peripheral = cachedPeripheral;
            break;
        }
    }

    [self removePeripheralsFromCache];

    if (peripheral) {
        MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredMatchingPeripheral);
        ChipLogProgress(Ble, "Connecting to cached device: %p", peripheral);
        [self connect:peripheral];
        [self stopScanning];
    } else {
        [self setupTimer:kScanningWithDiscriminatorTimeoutInSeconds];
    }
}

- (void)updateWithPeripheral:(CBPeripheral *)peripheral
{
    [self detachScannerDelegate];
    _currentMode = kConnecting;

    MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredMatchingPeripheral);
    ChipLogProgress(Ble, "Connecting to device: %p", peripheral);
    [self connect:peripheral];
    [self stopScanning];
}

- (void)addPeripheralToCache:(CBPeripheral *)peripheral data:(NSData *)data
{
    dispatch_source_t timeoutTimer;

    bool shouldLogData = true;
    if ([_cachedPeripherals objectForKey:peripheral]) {
        shouldLogData = ![data isEqualToData:_cachedPeripherals[peripheral][@"data"]];
        if (shouldLogData) {
            ChipLogProgress(Ble, "Updating peripheral %p from the cache", peripheral);
        }

        timeoutTimer = _cachedPeripherals[peripheral][@"timer"];
    } else {
        ChipLogProgress(Ble, "Adding peripheral %p to the cache", peripheral);
        auto delegate = _scannerDelegate;
        if (delegate) {
            dispatch_async(_chipWorkQueue, ^{
                ChipBLEDeviceIdentificationInfo info;
                auto bytes = (const uint8_t *) [data bytes];
                memcpy(&info, bytes, sizeof(info));
                delegate->OnBleScanAdd((__bridge void *) peripheral, info);
            });
        }

        timeoutTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _workQueue);
        dispatch_source_set_event_handler(timeoutTimer, ^{
            MATTER_LOG_METRIC(kMetricBLEPeripheralRemoved, ++self->_totalDevicesRemoved);
            [self removePeripheralFromCache:peripheral];
        });
        dispatch_resume(timeoutTimer);
    }

    auto timeout = static_cast<int64_t>(kCachePeripheralTimeoutInSeconds * NSEC_PER_SEC);
    dispatch_source_set_timer(timeoutTimer, dispatch_walltime(nullptr, timeout), DISPATCH_TIME_FOREVER, 5 * NSEC_PER_SEC);

    // Add only unique count of devices found
    if (!_cachedPeripherals[peripheral]) {
        MATTER_LOG_METRIC(kMetricBLEPeripheralAdded, ++_totalDevicesAdded);
    }

    _cachedPeripherals[peripheral] = @{
        @"data" : data,
        @"timer" : timeoutTimer,
    };

    if (shouldLogData) {
        ChipBLEDeviceIdentificationInfo info;
        auto bytes = (const uint8_t *) [data bytes];
        memcpy(&info, bytes, sizeof(info));

        ChipLogProgress(Ble, "  - Version: %u", info.GetAdvertisementVersion());
        ChipLogProgress(Ble, "  - Discriminator: %u", info.GetDeviceDiscriminator());
        ChipLogProgress(Ble, "  - VendorId: %u", info.GetVendorId());
        ChipLogProgress(Ble, "  - ProductId: %u", info.GetProductId());
    }
}

- (void)removePeripheralFromCache:(CBPeripheral *)peripheral
{
    auto entry = [_cachedPeripherals objectForKey:peripheral];
    if (entry) {
        ChipLogProgress(Ble, "Removing peripheral %p from the cache", peripheral);

        dispatch_source_cancel(entry[@"timer"]);
        [_cachedPeripherals removeObjectForKey:peripheral];

        auto delegate = _scannerDelegate;
        if (delegate) {
            dispatch_async(_chipWorkQueue, ^{
                delegate->OnBleScanRemove((__bridge void *) peripheral);
            });
        }
    }
}

- (void)removePeripheralsFromCache
{
    for (CBPeripheral * peripheral in [_cachedPeripherals allKeys]) {
        [self removePeripheralFromCache:peripheral];
    }
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
