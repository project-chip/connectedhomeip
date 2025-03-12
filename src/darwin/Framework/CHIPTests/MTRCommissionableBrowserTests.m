/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Matter/Matter.h>

#import "MTRMockCB.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

// Fixture 1: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1

static const uint16_t kTestVendorId = 0xFFF1u;
static const __auto_type kTestProductIds = @[ @(0x8000u), @(0x8001u), @(0x8002u) ];
static const __auto_type kTestDiscriminators = @[ @(2000), @(3839u), @(3840u), @(0xb1e) ];
static const uint16_t kDiscoverDeviceTimeoutInSeconds = 10;
static const uint16_t kExpectedDiscoveredDevicesCount = 4;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

static NSString * const kInstanceNameKey = @"instanceName";
static NSString * const kVendorIDKey = @"vendorID";
static NSString * const kProductIDKey = @"productID";
static NSString * const kDiscriminatorKey = @"discriminator";
static NSString * const kCommissioningModeKey = @"commissioningMode";

static NSDictionary<NSString *, id> * ResultSnapshot(MTRCommissionableBrowserResult * result)
{
    return @{
        kInstanceNameKey : result.instanceName,
        kVendorIDKey : result.vendorID,
        kProductIDKey : result.productID,
        kDiscriminatorKey : result.discriminator,
        kCommissioningModeKey : @(result.commissioningMode),
    };
}

@interface DeviceScannerDelegate : NSObject <MTRCommissionableBrowserDelegate>
@property (nonatomic, nullable) XCTestExpectation * expectation;
@property (nonatomic) NSMutableArray<NSDictionary<NSString *, id> *> * results;
@property (nonatomic) NSMutableArray<NSDictionary<NSString *, id> *> * removedResults;
@property (nonatomic) BOOL expectedResultsCountReached;

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation;
- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device;
- (void)controller:(MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device;
@end

@implementation DeviceScannerDelegate
- (instancetype)initWithExpectation:(XCTestExpectation *)expectation
{
    if (!(self = [super init])) {
        return nil;
    }

    _expectation = expectation;
    _results = [[NSMutableArray alloc] init];
    _removedResults = [[NSMutableArray alloc] init];
    _expectedResultsCountReached = NO;
    return self;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    _expectation = nil;
    _results = [[NSMutableArray alloc] init];
    _removedResults = [[NSMutableArray alloc] init];
    _expectedResultsCountReached = NO;
    return self;
}

- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    if (self.expectation == nil) {
        // We are not actually supposed to be looking at results; don't do it,
        // because we may be starting/stopping browse multiple times and seeing
        // odd numbers of results.
        return;
    }

    __auto_type * snapshot = ResultSnapshot(device);

    XCTAssertFalse([_results containsObject:snapshot], @"Newly discovered device %@ should not be in results already.", snapshot);

    [_results addObject:snapshot];

    XCTAssertLessThanOrEqual(_results.count, kExpectedDiscoveredDevicesCount);

    __auto_type instanceName = device.instanceName;
    __auto_type vendorId = device.vendorID;
    __auto_type productId = device.productID;
    __auto_type discriminator = device.discriminator;
    __auto_type commissioningMode = device.commissioningMode;

    if (![instanceName isEqual:@"BLE"]) {
        XCTAssertEqual(instanceName.length, 16); // The  instance name is random, so just ensure the len is right.
    }
    XCTAssertEqualObjects(vendorId, @(kTestVendorId));
    XCTAssertTrue([kTestProductIds containsObject:productId]);
    XCTAssertTrue([kTestDiscriminators containsObject:discriminator]);
    XCTAssertEqual(commissioningMode, YES);

    NSLog(@"Found device %@", snapshot);

    if (_results.count == kExpectedDiscoveredDevicesCount) {
        // Do some sanity checking on our results and removedResults to make
        // sure we really only saw the relevant set of things.
        NSSet<NSDictionary<NSString *, id> *> * finalResultsSet = [NSSet setWithArray:_results];
        NSSet<NSDictionary<NSString *, id> *> * allResultsSet = [finalResultsSet copy];
        allResultsSet = [allResultsSet setByAddingObjectsFromArray:_removedResults];

        // Ensure that we just saw the same results as our final set popping in and out if things
        // ever got removed here.
        XCTAssertEqualObjects(finalResultsSet, allResultsSet);

        // If we have a remove and re-add after the result count reached the
        // expected one, we can end up in this branch again.  Doing the above
        // checks is fine, but we shouldn't double-fulfill the expectation.
        if (self.expectedResultsCountReached == NO) {
            self.expectedResultsCountReached = YES;
            [self.expectation fulfill];
        }
    }
}

- (void)controller:(MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device
{
    __auto_type * snapshot = ResultSnapshot(device);
    XCTAssertTrue([_results containsObject:snapshot], @"Removed device %@ is not something we found before: %@", snapshot, _results);

    [_removedResults addObject:snapshot];
    [_results removeObject:snapshot];

    NSLog(@"Removed device %@", snapshot);
}
@end

@interface MTRCommissionableBrowserTests : MTRTestCase
@end

@implementation MTRCommissionableBrowserTests

+ (void)setUp
{
    [super setUp];

    sController = [MTRTestCase createControllerOnTestFabric];

    // Start the helper apps our tests use. Note these payloads match kTestDiscriminators etc.
    for (NSString * payload in @[
             @"MT:Y.K90SO527JA0648G00",
             @"MT:-24J0AFN00I40648G00",
         ]) {
        BOOL started = [self startAppWithName:@"all-clusters"
                                    arguments:@[]
                                      payload:payload];
        XCTAssertTrue(started);
    }
}

+ (void)tearDown
{
    XCTAssertNotNil(sController);
    [sController shutdown];
    XCTAssertFalse([sController isRunning]);
    sController = nil;

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];

    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    [self.class.mockCoreBluetooth reset];
}

- (void)test001_StartBrowseAndStopBrowse
{
    __auto_type delegate = [[DeviceScannerDelegate alloc] init];
    dispatch_queue_t dispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

    XCTestExpectation * bleScanExpectation = [self expectationWithDescription:@"did start BLE scan"];
    self.class.mockCoreBluetooth.onScanForPeripheralsWithServicesOptions = ^(NSArray<CBUUID *> * _Nullable serviceUUIDs, NSDictionary<NSString *, id> * _Nullable options) {
        XCTAssertEqual(serviceUUIDs.count, 1);
        [bleScanExpectation fulfill];
    };

    // Start browsing
    XCTAssertTrue([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    [self waitForExpectations:@[ bleScanExpectation ] timeout:1];

    XCTestExpectation * bleStopExpectation = [self expectationWithDescription:@"did stop BLE scan"];
    self.class.mockCoreBluetooth.onStopScan = ^{
        [bleStopExpectation fulfill];
    };

    // Stop browsing
    XCTAssertTrue([sController stopBrowseForCommissionables]);

    [self waitForExpectations:@[ bleStopExpectation ] timeout:1];
}

- (void)test002_StartBrowseAndStopBrowseMultipleTimes
{
    __auto_type delegate = [[DeviceScannerDelegate alloc] init];
    dispatch_queue_t dispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

    // Start browsing
    XCTAssertTrue([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    // Stop browsing
    XCTAssertTrue([sController stopBrowseForCommissionables]);

    // Start browsing a second time
    XCTAssertTrue([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    // Stop browsing a second time
    XCTAssertTrue([sController stopBrowseForCommissionables]);
}

- (void)test003_StopBrowseWhileNotBrowsing
{
    // Stop browsing while there is no browse ongoing
    XCTAssertFalse([sController stopBrowseForCommissionables]);
}

- (void)test004_StartBrowseWhileBrowsing
{
    __auto_type delegate = [[DeviceScannerDelegate alloc] init];
    dispatch_queue_t dispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

    // Start browsing
    XCTAssertTrue([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    // Start browsing a second time while a browse is ongoing
    XCTAssertFalse([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    // Properly stop browsing
    XCTAssertTrue([sController stopBrowseForCommissionables]);
}

- (void)test005_StartBrowseGetCommissionableOverMdnsAndBle
{
    __auto_type expectation = [self expectationWithDescription:@"Commissionable devices Found"];
    __auto_type delegate = [[DeviceScannerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t dispatchQueue = dispatch_queue_create("com.chip.discover", DISPATCH_QUEUE_SERIAL);

    // Mock a commissionable device advertising over BLE
    [self.class.mockCoreBluetooth addMockCommissionableMatterDeviceWithIdentifier:[NSUUID UUID]
                                                                         vendorID:@(kTestVendorId)
                                                                        productID:@0x8002
                                                                    discriminator:@0xb1e];

    // Start browsing
    XCTAssertTrue([sController startBrowseForCommissionables:delegate queue:dispatchQueue]);

    [self waitForExpectations:@[ expectation ] timeout:kDiscoverDeviceTimeoutInSeconds];

    // Properly stop browsing
    XCTAssertTrue([sController stopBrowseForCommissionables]);
}

@end
