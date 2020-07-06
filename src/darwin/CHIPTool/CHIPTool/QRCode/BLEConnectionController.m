/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "BLEConnectionController.h"
#import <CHIP/CHIP.h>

@interface BLEConnectionController ()

@end

@implementation BLEConnectionController

- (id)initWithName:(NSString *)peripheralName
{
    self = [super init];
    if (self) {
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        _peripheralName = peripheralName;
    }

    return self;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
    case CBManagerStatePoweredOn:
        NSLog(@"CBManagerState: ON");
        [self start];
        break;
    case CBManagerStatePoweredOff:
        NSLog(@"CBManagerState: OFF");
        [self stop];
        break;
    case CBManagerStateUnauthorized:
        NSLog(@"CBManagerState: Unauthorized");
        break;
    case CBManagerStateResetting:
        NSLog(@"CBManagerState: RESETTING");
        break;
    case CBManagerStateUnsupported:
        NSLog(@"CBManagerState: UNSUPPORTED");
        break;
    case CBManagerStateUnknown:
        NSLog(@"CBManagerState: UNKNOWN");
        break;
    }
}

- (void)centralManager:(CBCentralManager *)central
    didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
                     RSSI:(NSNumber *)RSSI
{
    BOOL isConnectable = [advertisementData objectForKey:CBAdvertisementDataIsConnectable];
    NSString * localNameKey = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    if (isConnectable && [localNameKey isEqual:_peripheralName]) {
        NSLog(@"Connecting to device: %@", _peripheralName);
        [self connect:peripheral];
        [self stopScanning];
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
    NSLog(@"Did connect to peripheral: %@", peripheral);

    [peripheral setDelegate:[[CHIPDeviceController sharedController] mBleDelegate]];
    [[CHIPDeviceController sharedController] connectBle:peripheral];
}

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

    [_centralManager scanForPeripheralsWithServices:nil options:nil];
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

    [_centralManager connectPeripheral:peripheral options:nil];
    _peripheral = peripheral;
}

- (void)disconnect
{
    if (!_centralManager || !_peripheral) {
        return;
    }

    [_centralManager cancelPeripheralConnection:_peripheral];
    _peripheral = nil;
}

@end
