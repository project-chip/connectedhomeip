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

#import <Matter/Matter.h>

#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestControllerDelegate.h"

static unsigned sAppRunnerIndex = 1;
static const uint16_t kPairingTimeoutInSeconds = 30;

// kBasePort gets the discriminator added to it to figure out the port the app
// should be using.  This ensures that apps with distinct discriminators use
// distinct ports.
static const uint16_t kMinDiscriminator = 1111;

#if HAVE_NSTASK
static const uint16_t kBasePort = 5542 - kMinDiscriminator;

@interface MTRTestCaseServerApp ()
@property (nonatomic, readwrite, strong) NSTask * task;
@end

#endif // HAVE_NSTASK

@implementation MTRTestCaseServerApp
@end

@implementation MTRTestCase (ServerAppRunner)

#if HAVE_NSTASK
+ (NSTask *)doStartAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
    __auto_type uniqueIndex = sAppRunnerIndex++;

    NSError * error;
    __auto_type * parsedPayload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payload error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(parsedPayload);

    XCTAssertFalse(parsedPayload.hasShortDiscriminator);

    NSNumber * discriminator = parsedPayload.discriminator;
    XCTAssertGreaterThanOrEqual(discriminator.unsignedShortValue, kMinDiscriminator);

    NSNumber * passcode = parsedPayload.setupPasscode;

    __auto_type * executable = [NSString stringWithFormat:@"out/debug/%@-app/chip-%@-app", name, name];
    __auto_type * appTask = [self createTaskForPath:executable];

    __auto_type * forcedArguments = @[
        // Make sure we only advertise on the local interface.
        @"--interface-id",
        @"-1",
        @"--secured-device-port",
        [NSString stringWithFormat:@"%u", kBasePort + discriminator.unsignedShortValue],
        @"--discriminator",
        [NSString stringWithFormat:@"%u", discriminator.unsignedShortValue],
        @"--passcode",
        [NSString stringWithFormat:@"%llu", passcode.unsignedLongLongValue],
        @"--KVS",
        [NSString stringWithFormat:@"/tmp/xctest-%d-chip-%@-kvs%u", getpid(), name, uniqueIndex],
        @"--product-id",
        [NSString stringWithFormat:@"%u", parsedPayload.productID.unsignedShortValue],
    ];

    __auto_type * allArguments = [forcedArguments arrayByAddingObjectsFromArray:arguments];
    [appTask setArguments:allArguments];

    NSString * outFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-%u.log", name, uniqueIndex];
    NSString * errorFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/%@-app-err-%u.log", name, uniqueIndex];

    // Make sure the files exist.
    [[NSFileManager defaultManager] createFileAtPath:outFile contents:nil attributes:nil];
    [[NSFileManager defaultManager] createFileAtPath:errorFile contents:nil attributes:nil];

    appTask.standardOutput = [NSFileHandle fileHandleForWritingAtPath:outFile];
    appTask.standardError = [NSFileHandle fileHandleForWritingAtPath:errorFile];

    NSLog(@"Started chip-%@-app (%@) with arguments %@ stdout=%@ and stderr=%@", name, appTask, allArguments, outFile, errorFile);

    return appTask;
}
#endif // HAVE_NSTASK

- (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return NO;
#else
    [self launchTask:[self.class doStartAppWithName:name arguments:arguments payload:payload]];
    return YES;
#endif // HAVE_NSTASK
}

+ (BOOL)startAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return NO;
#else
    [self launchTask:[self doStartAppWithName:name arguments:arguments payload:payload]];
    return YES;
#endif // HAVE_NSTASK
}

+ (void)commissionAppWithController:(MTRDeviceController *)controller payload:(NSString *)payload
                             nodeID:(NSNumber *)nodeID
{
    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for commissioning to complete"];

    MTRTestControllerDelegate * delegate = [[MTRTestControllerDelegate alloc] initWithExpectation:expectation newNodeID:nodeID];

    [controller addDeviceControllerDelegate:delegate queue:dispatch_get_main_queue()];

    NSError * error;
    __auto_type * payloadObj = [MTRSetupPayload setupPayloadWithOnboardingPayload:payload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payloadObj newNodeID:nodeID error:&error];
    XCTAssertNil(error);

    XCTAssertEqual([XCTWaiter waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds], XCTWaiterResultCompleted);
}

- (nullable MTRTestCaseServerApp *)startCommissionedAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments controller:(MTRDeviceController *)controller payload:(NSString *)payload
                                                         nodeID:(NSNumber *)nodeID
{
#if !HAVE_NSTASK
    XCTFail("Unable to start server app when we do not have NSTask");
    return nil;
#else
    NSTask * task = [self.class doStartAppWithName:name arguments:arguments payload:payload];
    [self launchTask:task];

    [self.class commissionAppWithController:controller payload:payload nodeID:nodeID];

    MTRTestCaseServerApp * app = [[MTRTestCaseServerApp alloc] init];
    app.task = task;
    return app;
#endif // HAVE_NSTASK
}

+ (MTRDeviceController *)startCommissionedAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments payload:(NSString *)payload nodeID:(NSNumber *)nodeID
{
    MTRDeviceController * controller = [self createControllerOnTestFabric];

    BOOL started = [self startAppWithName:name arguments:arguments payload:payload];
    XCTAssertTrue(started);

    [self commissionAppWithController:controller payload:payload nodeID:nodeID];

    return controller;
}

+ (MTRDeviceController *)startCommissionedAppWithName:(NSString *)name arguments:(NSArray<NSString *> *)arguments nodeID:(NSNumber *)nodeID
{
    NSNumber * passcode = [MTRSetupPayload generateRandomSetupPasscode];
    XCTAssertNotNil(passcode);

    // Discriminator is 12 bits in general, but we only allow discriminators at
    // least as big as kMinDiscriminator.  The distribution here is not exactly
    // uniform, because (1<<12)-kMinDiscriminator is not a power of 2, but
    // probably OK.
    __auto_type discriminatorRange = (1 << 12) - kMinDiscriminator;
    NSNumber * discriminator = @(kMinDiscriminator + arc4random() % discriminatorRange);

    __auto_type * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:passcode discriminator:discriminator];
    XCTAssertNotNil(payload);

    payload.version = @(0);
    payload.vendorID = @(0xFFF1);
    payload.productID = @(0x8001);
    payload.commissioningFlow = MTRCommissioningFlowStandard;
    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesOnNetwork;
    payload.discriminator = discriminator;
    payload.hasShortDiscriminator = NO;
    payload.setupPasscode = passcode;

    NSString * payloadString = [payload qrCodeString];
    XCTAssertNotNil(payloadString);

    return [self startCommissionedAppWithName:name arguments:arguments payload:payloadString nodeID:nodeID];
}

- (BOOL)restartApp:(MTRTestCaseServerApp *)app additionalArguments:(NSArray<NSString *> *)additionalArguments
{
#if !HAVE_NSTASK
    XCTFail("Unable to restart server app when we do not have NSTask");
    return NO;
#else
    app.task = [self relaunchTask:app.task additionalArguments:additionalArguments];
    return YES;
#endif // HAVE_NSTASK
}

+ (unsigned)nextUniqueIndex
{
    return sAppRunnerIndex;
}

@end
