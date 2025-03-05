/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
#include <platform/Darwin/BleConnectionDelegateImpl.h>
#include <platform/Darwin/BleScannerDelegate.h>
#include <platform/Darwin/BleUtils.h>
#include <platform/LockTracker.h>
#include <setup_payload/SetupPayload.h>
#include <tracing/metric_event.h>

#import <CoreBluetooth/CoreBluetooth.h>

#import "PlatformMetricKeys.h"

using namespace chip::Ble;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;
using namespace chip::Tracing::DarwinPlatform;

constexpr uint64_t kScanningWithDiscriminatorTimeoutInSeconds = 60;
constexpr uint64_t kPreWarmScanTimeoutInSeconds = 120;
constexpr uint64_t kCachePeripheralTimeoutInSeconds
    = static_cast<uint64_t>(CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX / 1000.0 * 8.0 * 0.625);

typedef NS_ENUM(uint8_t, BleConnectionMode) {
    kUndefined = 0,
    kScanning,
    kScanningWithTimeout,
    kConnecting,
};

@interface BleConnection : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>

@property (strong, nonatomic) dispatch_queue_t workQueue; // the CHIP work queue
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (strong, nonatomic) CBPeripheral * peripheral;
@property (nonatomic, readonly, nullable) dispatch_source_t timer;
@property (nonatomic, readonly) BleConnectionMode currentMode;
@property (strong, nonatomic) NSMutableDictionary<CBPeripheral *, NSDictionary *> * cachedPeripherals;
@property (assign, nonatomic) bool found;
@property (assign, nonatomic) chip::SetupDiscriminator deviceDiscriminator;
@property (assign, nonatomic) void * appState;
@property (assign, nonatomic) BleConnectionDelegate::OnConnectionCompleteFunct onConnectionComplete;
@property (assign, nonatomic) BleConnectionDelegate::OnConnectionErrorFunct onConnectionError;
@property (assign, nonatomic) chip::DeviceLayer::BleScannerDelegate * scannerDelegate;
@property (assign, nonatomic) chip::Ble::BleLayer * mBleLayer;

- (instancetype)initWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm;
- (instancetype)initWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator;

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

        void BleConnectionDelegateImpl::NewConnection(
            Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & inDeviceDiscriminator)
        {
            assertChipStackLockedByCurrentThread();

            // Make a copy of the device discriminator for the block to capture.
            SetupDiscriminator deviceDiscriminator = inDeviceDiscriminator;

            ChipLogProgress(Ble, "ConnectionDelegate NewConnection with discriminator");
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
            ble = [[BleConnection alloc] initWithDiscriminator:deviceDiscriminator];
            [ble setBleLayer:bleLayer];
            ble.appState = appState;
            ble.onConnectionComplete = OnConnectionComplete;
            ble.onConnectionError = OnConnectionError;
            ble.centralManager = [ble.centralManager initWithDelegate:ble queue:ble.workQueue];
        }

        void BleConnectionDelegateImpl::NewConnection(Ble::BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj)
        {
            assertChipStackLockedByCurrentThread();

            ChipLogProgress(Ble, "ConnectionDelegate NewConnection with conn obj: %p", connObj);
            CBPeripheral * peripheral = CBPeripheralFromBleConnObject(connObj);

            // The BLE_CONNECTION_OBJECT represents a CBPeripheral object. In order for it to be valid the central
            // manager needs to still be running.
            if (!ble || [ble isConnecting]) {
                if (OnConnectionError) {
                    // Avoid calling back prior to returning
                    dispatch_async(PlatformMgrImpl().GetWorkQueue(), ^{
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
        }

        void BleConnectionDelegateImpl::StartScan(BleScannerDelegate * delegate, BleScanMode mode)
        {
            assertChipStackLockedByCurrentThread();

            bool prewarm = (mode == BleScanMode::kPreWarm);
            ChipLogProgress(Ble, "ConnectionDelegate StartScan (%s)", (prewarm ? "pre-warm" : "default"));

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
            ble = [[BleConnection alloc] initWithDelegate:delegate prewarm:prewarm];
            // Do _not_ set onConnectionComplete and onConnectionError
            // here.  The connection callbacks we have expect an appState
            // that we do not have here, and in any case connection
            // complete/error make no sense for a scan.
            ble.centralManager = [ble.centralManager initWithDelegate:ble queue:ble.workQueue];
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
            [ble stop];
            ble = nil;
            return CHIP_NO_ERROR;
        }
    } // namespace Internal
} // namespace DeviceLayer
} // namespace chip

@interface BleConnection ()
@property (nonatomic, readonly) int32_t totalDevicesAdded;
@property (nonatomic, readonly) int32_t totalDevicesRemoved;
@end

@implementation BleConnection {
    CBUUID * _chipServiceUUID;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _chipServiceUUID = CBUUIDFromBleUUID(chip::Ble::CHIP_BLE_SVC_ID);
        _workQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();
        _centralManager = [CBCentralManager alloc];
        _found = false;
        _cachedPeripherals = [[NSMutableDictionary alloc] init];
        _currentMode = kUndefined;
        [self _resetCounters];
    }

    return self;
}

- (instancetype)initWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm
{
    self = [self init];
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

- (id)initWithDiscriminator:(const chip::SetupDiscriminator &)deviceDiscriminator
{
    self = [self init];
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
    if (self.onConnectionError != nil) {
        self.onConnectionError(self.appState, error);
    }
}

- (void)dispatchConnectionComplete:(CBPeripheral *)peripheral
{
    if (self.onConnectionComplete != nil) {
        self.onConnectionComplete(self.appState, BleConnObjectFromCBPeripheral(peripheral));
    }
}

// Start CBCentralManagerDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    assertChipStackLockedByCurrentThread();
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
    assertChipStackLockedByCurrentThread();

    NSData * serviceData = advertisementData[CBAdvertisementDataServiceDataKey][_chipServiceUUID];
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
    assertChipStackLockedByCurrentThread();
    MATTER_LOG_METRIC_END(kMetricBLEConnectPeripheral);
    MATTER_LOG_METRIC_BEGIN(kMetricBLEDiscoveredServices);

    [peripheral setDelegate:self];
    [peripheral discoverServices:nil];
    [self stopScanning];
}

// End CBCentralManagerDelegate

// Start CBPeripheralDelegate

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
    assertChipStackLockedByCurrentThread();

    if (nil != error) {
        ChipLogError(Ble, "BLE:Error finding Chip Service in the device: [%s]", [error.localizedDescription UTF8String]);
    }

    MATTER_LOG_METRIC_END(kMetricBLEDiscoveredServices, CHIP_ERROR(chip::ChipError::Range::kOS, static_cast<uint32_t>(error.code)));

    for (CBService * service in peripheral.services) {
        if ([service.UUID isEqual:_chipServiceUUID] && !self.found) {
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
    assertChipStackLockedByCurrentThread();
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
    assertChipStackLockedByCurrentThread();

    if (nil == error) {
        ChipBleUUID svcId = BleUUIDFromCBUUD(characteristic.service.UUID);
        ChipBleUUID charId = BleUUIDFromCBUUD(characteristic.UUID);
        _mBleLayer->HandleWriteConfirmation(BleConnObjectFromCBPeripheral(peripheral), &svcId, &charId);
    } else {
        ChipLogError(
            Ble, "BLE:Error writing Characteristics in Chip service on the device: [%s]", [error.localizedDescription UTF8String]);
        MATTER_LOG_METRIC(kMetricBLEWriteChrValueFailed, BLE_ERROR_GATT_WRITE_FAILED);
        _mBleLayer->HandleConnectionError(BleConnObjectFromCBPeripheral(peripheral), BLE_ERROR_GATT_WRITE_FAILED);
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
                                          error:(NSError *)error
{
    assertChipStackLockedByCurrentThread();

    bool isNotifying = characteristic.isNotifying;

    if (nil == error) {
        ChipBleUUID svcId = BleUUIDFromCBUUD(characteristic.service.UUID);
        ChipBleUUID charId = BleUUIDFromCBUUD(characteristic.UUID);
        if (isNotifying) {
            _mBleLayer->HandleSubscribeComplete(BleConnObjectFromCBPeripheral(peripheral), &svcId, &charId);
        } else {
            _mBleLayer->HandleUnsubscribeComplete(BleConnObjectFromCBPeripheral(peripheral), &svcId, &charId);
        }
    } else {
        ChipLogError(Ble, "BLE:Error subscribing/unsubcribing some characteristic on the device: [%s]",
            [error.localizedDescription UTF8String]);

        if (isNotifying) {
            MATTER_LOG_METRIC(kMetricBLEUpdateNotificationStateForChrFailed, BLE_ERROR_GATT_WRITE_FAILED);
            // we're still notifying, so we must failed the unsubscription
            _mBleLayer->HandleConnectionError(BleConnObjectFromCBPeripheral(peripheral), BLE_ERROR_GATT_UNSUBSCRIBE_FAILED);
        } else {
            // we're not notifying, so we must failed the subscription
            MATTER_LOG_METRIC(kMetricBLEUpdateNotificationStateForChrFailed, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
            _mBleLayer->HandleConnectionError(BleConnObjectFromCBPeripheral(peripheral), BLE_ERROR_GATT_SUBSCRIBE_FAILED);
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
    didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
                              error:(NSError *)error
{
    assertChipStackLockedByCurrentThread();

    if (nil == error) {
        ChipBleUUID svcId = BleUUIDFromCBUUD(characteristic.service.UUID);
        ChipBleUUID charId = BleUUIDFromCBUUD(characteristic.UUID);
        auto * value = characteristic.value; // read immediately before dispatching

        // build a inet buffer from the rxEv and send to blelayer.
        auto msgBuf = chip::System::PacketBufferHandle::NewWithData(value.bytes, value.length);

        if (msgBuf.IsNull()) {
            ChipLogError(Ble, "Failed at allocating buffer for incoming BLE data");
            MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, CHIP_ERROR_NO_MEMORY);
            _mBleLayer->HandleConnectionError(BleConnObjectFromCBPeripheral(peripheral), CHIP_ERROR_NO_MEMORY);
        } else if (!_mBleLayer->HandleIndicationReceived(BleConnObjectFromCBPeripheral(peripheral), &svcId, &charId, std::move(msgBuf))) {
            // since this error comes from device manager core
            // we assume it would do the right thing, like closing the connection
            ChipLogError(Ble, "Failed at handling incoming BLE data");
            MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, CHIP_ERROR_INCORRECT_STATE);
        }
    } else {
        ChipLogError(
            Ble, "BLE:Error receiving indication of Characteristics on the device: [%s]", [error.localizedDescription UTF8String]);
        MATTER_LOG_METRIC(kMetricBLEUpdateValueForChrFailed, BLE_ERROR_GATT_INDICATE_FAILED);
        _mBleLayer->HandleConnectionError(BleConnObjectFromCBPeripheral(peripheral), BLE_ERROR_GATT_INDICATE_FAILED);
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

    if (_peripheral) {
        // Close all BLE connections before we release CB objects
        _mBleLayer->CloseAllBleConnections();
        _peripheral = nil;
    }

    if (_centralManager) {
        _centralManager.delegate = nil;
        _centralManager = nil;
    }

    if (chip::DeviceLayer::Internal::ble == self) {
        chip::DeviceLayer::Internal::ble = nil;
    }
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
    [_centralManager scanForPeripheralsWithServices:@[ _chipServiceUUID ] options:scanOptions];
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
        existingDelegate->OnBleScanStopped();
    }
}

- (void)updateWithDelegate:(chip::DeviceLayer::BleScannerDelegate *)delegate prewarm:(bool)prewarm
{
    [self detachScannerDelegate];

    if (delegate) {
        for (CBPeripheral * cachedPeripheral in _cachedPeripherals) {
            NSData * serviceData = _cachedPeripherals[cachedPeripheral][@"data"];
            ChipBLEDeviceIdentificationInfo info;
            memcpy(&info, [serviceData bytes], sizeof(info));
            delegate->OnBleScanAdd(BleConnObjectFromCBPeripheral(cachedPeripheral), info);
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
        // The cached peripheral might be obsolete, so continue scanning until didConnectPeripheral is triggered.
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
            ChipBLEDeviceIdentificationInfo info;
            auto bytes = (const uint8_t *) [data bytes];
            memcpy(&info, bytes, sizeof(info));
            delegate->OnBleScanAdd(BleConnObjectFromCBPeripheral(peripheral), info);
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
            delegate->OnBleScanRemove(BleConnObjectFromCBPeripheral(peripheral));
        }
    }
}

- (void)removePeripheralsFromCache
{
    for (CBPeripheral * peripheral in [_cachedPeripherals allKeys]) {
        [self removePeripheralFromCache:peripheral];
    }
}

- (void)setBleLayer:(chip::Ble::BleLayer *)bleLayer
{
    _mBleLayer = bleLayer;
}

@end
