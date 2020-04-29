//
//  CHPSetupPayloadParserTests.m
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

#import <XCTest/XCTest.h>
#import "CHPManualSetupPayloadParser.h"
#import "CHPQRCodeSetupPayloadParser.h"

@interface CHPSetupPayloadParserTests : XCTestCase

@end

@implementation CHPSetupPayloadParserTests

- (void)testManualParser {
    NSError *error;
    CHPManualSetupPayloadParser *parser = [[CHPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@"34896656190000100001"];
    CHPSetupPayload *payload = [parser populatePayload:&error];

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

- (void)testManualParser_Error {
    NSError *error;
    CHPManualSetupPayloadParser *parser = [[CHPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@""];
    CHPSetupPayload *payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHPErrorCodeInvalidStringLength);
}

- (void)testQRCodeParser_Error {
    NSError *error;
    CHPManualSetupPayloadParser *parser = [[CHPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@"B20800G.0G8G000"];
    CHPSetupPayload *payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHPErrorCodeInvalidIntegerValue);
}

- (void)testQRCodeParser {
    NSError *error;
    CHPQRCodeSetupPayloadParser *parser = [[CHPQRCodeSetupPayloadParser alloc] initWithBase45Representation:@"B20800G00G8G000"];
    CHPSetupPayload *payload = [parser populatePayload:&error];

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

@end
