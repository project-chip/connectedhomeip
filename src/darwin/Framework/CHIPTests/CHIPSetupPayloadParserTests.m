//
//  CHIPSetupPayloadParserTests.m
//  CHIPQRCodeReaderTests
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
// module headers
#import "CHIPManualSetupPayloadParser.h"
#import "CHIPQRCodeSetupPayloadParser.h"

// additional includes
#import "CHIPError.h"

// system dependencies
#import <XCTest/XCTest.h>

@interface CHIPSetupPayloadParserTests : XCTestCase

@end

@implementation CHIPSetupPayloadParserTests

- (void)testManualParser
{
    NSError * error;
    CHIPManualSetupPayloadParser * parser =
        [[CHIPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@"348966561900001000017"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 13);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2345);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertFalse(payload.requiresCustomFlow);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.rendezvousInformation.unsignedIntegerValue, 0);
}

- (void)testManualParser_Error
{
    NSError * error;
    CHIPManualSetupPayloadParser * parser = [[CHIPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@""];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidStringLength);
}

- (void)testQRCodeParser_Error
{
    NSError * error;
    CHIPQRCodeSetupPayloadParser * parser =
        [[CHIPQRCodeSetupPayloadParser alloc] initWithBase41Representation:@"CH:B20800G.0G8G000"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidArgument);
}

- (void)testQRCodeParser
{
    NSError * error;
    CHIPQRCodeSetupPayloadParser * parser = [[CHIPQRCodeSetupPayloadParser alloc] initWithBase41Representation:@"CH:J20800G008008000"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertFalse(payload.requiresCustomFlow);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.rendezvousInformation.unsignedIntegerValue, 1);
}

- (void)testQRCodeParserWithOptionalData
{
    NSError * error;
    CHIPQRCodeSetupPayloadParser * parser = [[CHIPQRCodeSetupPayloadParser alloc] initWithBase41Representation:@"CH:H00O0048C7.1000-HW000200100ND0UOGMHARTH7+40Y1CLJJJ7RW7848UG8BT88CD90BP*BVOD 6BI9CO"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 13);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 2);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 3);
    XCTAssertTrue(payload.requiresCustomFlow);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.rendezvousInformation.unsignedIntegerValue, 1);
    XCTAssertTrue([payload.serialNumber isEqualToString:@"123456789QWDHANTYUIOP"]);
    
    NSArray<CHIPOptionalQRCodeInfo *> *vendorOptionalInfo = [payload getAllVendorOptionalData:&error];
    XCTAssertNil(error);
    XCTAssertEqual([vendorOptionalInfo count], 2);
    for (CHIPOptionalQRCodeInfo *info in vendorOptionalInfo) {
        if (info.tag.intValue == 2) {
            XCTAssertEqual(info.infoType.intValue, kOptionalQRCodeInfoTypeString);
            XCTAssertTrue([info.stringValue isEqualToString:@"myData"]);
        } else if (info.tag.intValue == 3) {
            XCTAssertEqual(info.infoType.intValue, kOptionalQRCodeInfoTypeInt);\
            XCTAssertEqual(info.integerValue.intValue, 12);
        }
    }
}

@end
