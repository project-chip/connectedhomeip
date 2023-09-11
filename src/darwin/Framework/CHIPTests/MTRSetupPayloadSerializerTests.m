/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// module headers
#import "MTRSetupPayload.h"

// additional includes
#import "MTRError.h"

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRSetupPayloadSerializerTests : XCTestCase

@end

@implementation MTRSetupPayloadSerializerTests

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

// Make sure to not add any tests that involve parsing setup payloads to this
// file.  Those should go in MTRSetupPayloadParserTests.m.
@end
