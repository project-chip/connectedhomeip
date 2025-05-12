/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRMockCB.h"
#import "MTRTestCase.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>
#import <stdatomic.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRBleTests : MTRTestCase
@end

@interface TestBrowserDelegate : NSObject <MTRCommissionableBrowserDelegate>
@property (strong) void (^onDidFindCommissionableDevice)(MTRDeviceController *, MTRCommissionableBrowserResult *);
@property (strong) void (^onDidRemoveCommissionableDevice)(MTRDeviceController *, MTRCommissionableBrowserResult *);
@end

@implementation TestBrowserDelegate

- (void)controller:(nonnull MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    __auto_type block = self.onDidFindCommissionableDevice;
    if (block) {
        block(controller, device);
    }
}

- (void)controller:(nonnull MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    __auto_type block = self.onDidRemoveCommissionableDevice;
    if (block) {
        block(controller, device);
    }
}

@end

@implementation MTRBleTests {
    MTRDeviceController * _controller;
}

- (void)setUp
{
    [super setUp];

    [self.class.mockCoreBluetooth reset];

    _controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(_controller);
}

- (void)tearDown
{
    _controller = nil; // MTRTestCase handles shutdown
    [super tearDown];
}

- (void)testBleCommissionableBrowserResultAdditionAndRemoval
{
    __block MTRCommissionableBrowserResult * device;
    XCTestExpectation * didFindDevice = [self expectationWithDescription:@"did find device"];
    TestBrowserDelegate * delegate = [[TestBrowserDelegate alloc] init];
    delegate.onDidFindCommissionableDevice = ^(MTRDeviceController * controller, MTRCommissionableBrowserResult * result) {
        if ([result.instanceName isEqualToString:@"BLE"]) { // TODO: This is a lame API
            XCTAssertNil(device);
            XCTAssertEqualObjects(result.vendorID, @0xfff1);
            XCTAssertEqualObjects(result.productID, @0x1234);
            XCTAssertEqualObjects(result.discriminator, @0x444);
            device = result;
            [didFindDevice fulfill];
        }
    };

    XCTestExpectation * didRemoveDevice = [self expectationWithDescription:@"did remove device"];
    delegate.onDidRemoveCommissionableDevice = ^(MTRDeviceController * controller, MTRCommissionableBrowserResult * result) {
        if ([result.instanceName isEqualToString:@"BLE"]) {
            XCTAssertNotNil(device);
            XCTAssertEqual(result, device);
            [didRemoveDevice fulfill];
        }
    };

    XCTAssertTrue([_controller startBrowseForCommissionables:delegate queue:dispatch_get_main_queue()]);

    NSUUID * peripheralID = [NSUUID UUID];
    [self.class.mockCoreBluetooth addMockCommissionableMatterDeviceWithIdentifier:peripheralID vendorID:@0xfff1 productID:@0x1234 discriminator:@0x444];
    [self.class.mockCoreBluetooth removeMockPeripheralWithIdentifier:peripheralID];

    // BleConnectionDelegateImpl kCachePeripheralTimeoutInSeconds is approximately 10 seconds
    [self waitForExpectations:@[ didFindDevice, didRemoveDevice ] timeout:14 enforceOrder:YES];
    XCTAssertTrue([_controller stopBrowseForCommissionables]);
}

- (void)testBleCommissionAfterStopBrowseUAF
{
    __block MTRCommissionableBrowserResult * device;
    XCTestExpectation * didFindDevice = [self expectationWithDescription:@"did find device"];
    TestBrowserDelegate * delegate = [[TestBrowserDelegate alloc] init];
    delegate.onDidFindCommissionableDevice = ^(MTRDeviceController * controller, MTRCommissionableBrowserResult * result) {
        if ([result.instanceName isEqualToString:@"BLE"]) {
            XCTAssertNil(device);
            XCTAssertEqualObjects(result.vendorID, @0xfff1);
            XCTAssertEqualObjects(result.productID, @0x1234);
            XCTAssertEqualObjects(result.discriminator, @0x444);
            device = result;
            [didFindDevice fulfill];
        }
    };

    XCTAssertTrue([_controller startBrowseForCommissionables:delegate queue:dispatch_get_main_queue()]);

    NSUUID * peripheralID = [NSUUID UUID];
    [self.class.mockCoreBluetooth addMockCommissionableMatterDeviceWithIdentifier:peripheralID vendorID:@0xfff1 productID:@0x1234 discriminator:@0x444];
    [self waitForExpectations:@[ didFindDevice ] timeout:2 enforceOrder:YES];

    XCTAssertTrue([_controller stopBrowseForCommissionables]);

    // Attempt to use the MTRCommissionableBrowserResult after we stopped browsing
    // This used to result in a UAF because BLE_CONNECTION_OBJECT is a void*
    // carrying a CBPeripheral without retaining it. When browsing is stopped,
    // BleConnectionDelegateImpl releases all cached CBPeripherals.
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@54321 discriminator:@0x444];
    [_controller setupCommissioningSessionWithDiscoveredDevice:device
                                                       payload:payload
                                                     newNodeID:@999
                                                         error:NULL];
    [_controller cancelCommissioningForNodeID:@999 error:NULL];
}

- (void)testShutdownBlePowerOffRaceUAF
{
    // Attempt a PASE connection over BLE, this will call BleConnectionDelegateImpl::NewConnection()
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@54321 discriminator:@0xb1e];
    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesBLE;
    NSError * error;
    XCTAssertTrue([_controller setupCommissioningSessionWithPayload:payload newNodeID:@999 error:&error],
        "setupCommissioningSessionWithPayload failed: %@", error);

    // Create a race between shutdown and a CBManager callback that used to provoke a UAF.
    // Note that on the order of 100 iterations can be necessary to reproduce the crash.
    __block atomic_int tasks = 2;
    dispatch_semaphore_t done = dispatch_semaphore_create(0);

    dispatch_block_t shutdown = ^{
        // Shut down the controller. This causes the SetupCodePairer to call
        // BleConnectionDelegateImpl::CancelConnection(), then the SetupCodePairer
        // is deallocated along with the DeviceCommissioner
        [self->_controller shutdown];
        self->_controller = nil;
        if (atomic_fetch_sub(&tasks, 1) == 1) {
            dispatch_semaphore_signal(done);
        }
    };
    dispatch_block_t powerOff = ^{
        // Cause CBPeripheralManager to signal a state change that
        // triggers a callback to the SetupCodePairer
        self.class.mockCoreBluetooth.state = CBManagerStatePoweredOff;
        if (atomic_fetch_sub(&tasks, 1) == 1) {
            dispatch_semaphore_signal(done);
        }
    };

    dispatch_queue_t pool = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
    dispatch_async(pool, shutdown);
    dispatch_async(pool, powerOff);
    dispatch_wait(done, DISPATCH_TIME_FOREVER);
}

- (void)testShutdownAndBlePowerOffAfterSuccessfulConnect
{
    // Attempt a PASE connection over BLE, this will call BleConnectionDelegateImpl::NewConnection()
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@54321 discriminator:@0xb1e];
    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesBLE;
    NSError * error;
    XCTAssertTrue([_controller setupCommissioningSessionWithPayload:payload newNodeID:@999 error:&error],
        "setupCommissioningSessionWithPayload failed: %@", error);

    // Now make a matching device appear. The commissioner will connect over BLE
    // and attempt to establish a PASE connection by doing a BLE write.
    XCTestExpectation * didConnect = [self expectationWithDescription:@"did connect to peripheral"];
    self.class.mockCoreBluetooth.onConnectPeripheralWithOptions = ^BOOL(CBPeripheral * peripheral, NSDictionary<NSString *, id> * _Nullable options) {
        [didConnect fulfill];
        return YES; // let connection establishment succeed
    };

    XCTestExpectation * didWrite = [self expectationWithDescription:@"did perform write"];
    self.class.mockCoreBluetooth.onWriteValueForCharacteristic = ^(CBPeripheral * peripheral, NSData * value, CBMutableCharacteristic * characteristic, CBCharacteristicWriteType type) {
        [didWrite fulfill];
    };

    [self.class.mockCoreBluetooth addMockCommissionableMatterDeviceWithIdentifier:[NSUUID UUID] vendorID:@0xfff1 productID:@0x1234 discriminator:@0xb1e];

    [self waitForExpectations:@[ didConnect, didWrite ] timeout:5];

    // The BLE connection in the Matter stack is now in an established state,
    // waiting for the commissionee to respond to the BLE write. Shutting down
    // the controller will free the DeviceCommissioner and its SetupCodePairer.
    [_controller shutdown];
    _controller = nil;

    // Now trigger a CBCentralManager state update, attempting to provoke a UAF.
    self.class.mockCoreBluetooth.state = CBManagerStatePoweredOff;
}

@end

NS_ASSUME_NONNULL_END
