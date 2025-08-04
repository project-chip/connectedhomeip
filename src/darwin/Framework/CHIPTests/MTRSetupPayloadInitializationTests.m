/**
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
#import <XCTest/XCTest.h>

@interface MTRSetupPayloadInitializationTests : XCTestCase

@end

@implementation MTRSetupPayloadInitializationTests

- (BOOL)shouldRelaunchBeforeRunningTest
{
    // By having xctest restart the process before each test case we
    // ensure that the relevant MTRSetupPayload code paths correctly
    // call chip::Platform::MemoryInit().
    // Tests that are not specifically designed to test this should
    // be added to MTRSetupPayloadTests to avoid this extra overhead.
    return YES;
}

- (void)testSetupPayloadBasicQRCodeSerialize
{
    __auto_type * payload = [[MTRSetupPayload alloc] init];
    XCTAssertNotNil(payload);

    payload.version = @(0);
    payload.vendorID = @(0xFFF1);
    payload.productID = @(1);
    payload.commissioningFlow = MTRCommissioningFlowStandard;
    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesOnNetwork;
    payload.discriminator = @(0xabc);
    payload.hasShortDiscriminator = NO;
    payload.setupPasscode = @(12121212);

    NSError * error;
    __auto_type * qrCode = [payload qrCodeString:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(qrCode);
    XCTAssertEqualObjects(qrCode, @"MT:-24J06.H14BK9C7R900");
}

@end
