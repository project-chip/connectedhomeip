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

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Instantiating this class will intercept any future calls that
/// allocate `CBCentralManager` objects to return mock objects instead.
/// Only one instance of this class should exist at any one time.
@interface MTRMockCB : NSObject

/// Stops mock `CBCentralManager` allocations and disables existing mocks.
/// Failure to call this method may result in leaks or other issues.
- (void)stopMocking;

/// Resets `on*` hooks and mocked peripherals but does not stop mocking.
- (void)reset;

// Adds a mocked peripheral and causes any mocked `CBCentralManager`
// instances to discover this as a `CBPeripheral`, if they are scanning.
// The provided identifier becomes the `identifier` of the `CBPeripheral`.
- (void)addMockPeripheralWithIdentifier:(NSUUID *)identifier
                               services:(NSArray<CBUUID *> *)services
                      advertisementData:(nullable NSDictionary<NSString *, id> *)advertisementData;

// Convenience version of `addMockPeripheralWithIdentifier:...` that
// advertises the relevant service with advertisement data as defined
// in the "Matter BLE Service Data payload format" section of the spec.
- (void)addMockCommissionableMatterDeviceWithIdentifier:(NSUUID *)identifier
                                               vendorID:(NSNumber *)vendorID
                                              productID:(NSNumber *)productID
                                          discriminator:(NSNumber *)discriminator;

- (void)removeMockPeripheralWithIdentifier:(NSUUID *)identifier;

/// Mocked state. Defaults to CBManagerStatePoweredOn.
@property (readwrite, assign) CBManagerState state;

@property (readwrite, strong, nullable) void (^onScanForPeripheralsWithServicesOptions)
    (NSArray<CBUUID *> * _Nullable serviceUUIDs, NSDictionary<NSString *, id> * _Nullable options);

@property (readwrite, strong, nullable) void (^onStopScan)(void);

@end

NS_ASSUME_NONNULL_END
