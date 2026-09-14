/**
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

// module headers
#import <Matter/Matter.h>

#import "MTRDefines_Internal.h"
#import "MTRErrorTestUtils.h"
#import "MTRSecureCodingTestHelpers.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const uint16_t kPairingTimeoutInSeconds = 30;
static const uint64_t sDeviceId = 100000000;
static NSString * kOnboardingPayload = @"MT:Y.K90SO527JA0648G00";
static const uint16_t kDiscriminator = 3839; // Matches kOnboardingPayload
static const uint32_t kPasscode = 20202021; // Matches kOnboardingPayload

// Using the deprecated delegate API on purpose
@interface MTRPairingBackwardsCompatTestControllerDelegate : NSObject <MTRDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTRPairingBackwardsCompatTestControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)onPairingComplete:(NSError * _Nullable)error
{
    XCTAssertNil(error);

    [_expectation fulfill];
    _expectation = nil;
}
@end

@interface MTRPairingBackwardsCompatTests : MTRTestCase
@end

@implementation MTRPairingBackwardsCompatTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)startServerApp
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);
}

- (void)test001_PairDeviceWithNumbers
{
    [self startServerApp];
    __auto_type * controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(controller);

    __auto_type * expectation = [self expectationWithDescription:@"PASE session established"];
    __auto_type * delegate = [[MTRPairingBackwardsCompatTestControllerDelegate alloc] initWithExpectation:expectation];

    [controller setPairingDelegate:delegate queue:dispatch_get_main_queue()];

    NSError * error;
    BOOL ok = [controller pairDevice:sDeviceId
                       discriminator:kDiscriminator
                        setupPINCode:kPasscode
                               error:&error];
    XCTAssertNil(error);
    XCTAssertTrue(ok);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
}

- (void)test002_PairDeviceWithString
{
    [self startServerApp];
    __auto_type * controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(controller);

    __auto_type * expectation = [self expectationWithDescription:@"PASE session established"];
    __auto_type * delegate = [[MTRPairingBackwardsCompatTestControllerDelegate alloc] initWithExpectation:expectation];

    [controller setPairingDelegate:delegate queue:dispatch_get_main_queue()];

    NSError * error;
    BOOL ok = [controller pairDevice:sDeviceId
                   onboardingPayload:kOnboardingPayload
                               error:&error];
    XCTAssertNil(error);
    XCTAssertTrue(ok);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
}

- (void)test003_PairDeviceWithBadManualPairingCode_SurfacesIntegrityCheckFailed
{
    // Integration regression pin: pairing through MTRDeviceController with a
    // manual pairing code whose Verhoeff check digit is wrong must surface
    // MTRErrorCodeIntegrityCheckFailed end-to-end, not the pre-fix generic
    // MTRErrorCodeInvalidArgument.
    //
    // Deliberately does NOT call -startServerApp (unlike test001/test002): the
    // bad check digit is rejected synchronously in
    // -[MTRDeviceController pairDevice:onboardingPayload:error:] when it parses
    // the payload via -[MTRSetupPayload initWithPayload:error:], which fails
    // before any commissionable-node discovery or PASE handshake is attempted.
    // The call returns NO inline, so no server, network, or paired device is
    // needed and there is no asynchronous delegate callback to await. Adding a
    // server app here would be incorrect -- it would never be contacted.
    __auto_type * controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(controller);

    NSError * error;
    BOOL ok = [controller pairDevice:sDeviceId
                   onboardingPayload:@"02684354589"
                               error:&error];
    XCTAssertFalse(ok);
    XCTAssertNotNil(error);
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeIntegrityCheckFailed);
    XCTAssertNotEqual(error.code, MTRErrorCodeInvalidArgument);
}

@end
