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
#import "CHIPOnboardingPayloadParser.h"
#import "CHIPQRCodeSetupPayloadParser.h"
#import "CHIPSetupPayload.h"

// additional includes
#import "CHIPError.h"

// system dependencies
#import <XCTest/XCTest.h>

@interface CHIPSetupPayloadParserTests : XCTestCase

@end

@implementation CHIPSetupPayloadParserTests

- (void)testOnboardingPayloadParser_Manual_NoError
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                        ofType:CHIPOnboardingPayloadTypeManualCode
                                                                                         error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 2560);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationNone);
}

- (void)testOnboardingPayloadParser_Manual_WrongType
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                        ofType:CHIPOnboardingPayloadTypeQRCode
                                                                                         error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidArgument);
}

- (void)testOnboardingPayloadParser_Admin_NoError
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                        ofType:CHIPOnboardingPayloadTypeAdmin
                                                                                         error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 2560);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationNone);
}

- (void)testOnboardingPayloadParser_Admin_WrongType
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                        ofType:CHIPOnboardingPayloadTypeQRCode
                                                                                         error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidArgument);
}

- (void)testOnboardingPayloadParser_QRCode_NoError
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J00000"
                                                                                        ofType:CHIPOnboardingPayloadTypeQRCode
                                                                                         error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationSoftAP);
}

- (void)testOnboardingPayloadParser_QRCode_WrongType
{
    NSError * error;
    CHIPSetupPayload * payload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J00000"
                                                                                        ofType:CHIPOnboardingPayloadTypeAdmin
                                                                                         error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeIntegrityCheckFailed);
}

- (void)testOnboardingPayloadParser_NFC_NoError
{
    NSError * error;
    CHIPSetupPayload * payload =
        [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J0A33P0GQ670.QT52B.E23O6DE0Y3U10O0"
                                                               ofType:CHIPOnboardingPayloadTypeNFC
                                                                error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationSoftAP);
}

- (void)testOnboardingPayloadParser_NFC_WrongType
{
    NSError * error;
    CHIPSetupPayload * payload =
        [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J0A33P0GQ670.QT52B.E23O6DE0Y3U10O0"
                                                               ofType:CHIPOnboardingPayloadTypeManualCode
                                                                error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeIntegrityCheckFailed);
}

- (void)testManualParser
{
    NSError * error;
    CHIPManualSetupPayloadParser * parser =
        [[CHIPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@"636108753500001000015"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 2560);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationNone);
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
        [[CHIPQRCodeSetupPayloadParser alloc] initWithBase38Representation:@"MT:R5L90MP500K64J0000."];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidArgument);
}

- (void)testQRCodeParser
{
    NSError * error;
    CHIPQRCodeSetupPayloadParser * parser =
        [[CHIPQRCodeSetupPayloadParser alloc] initWithBase38Representation:@"MT:R5L90MP500K64J00000"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationSoftAP);
}

- (void)testQRCodeParserWithOptionalData
{
    NSError * error;
    CHIPQRCodeSetupPayloadParser * parser =
        [[CHIPQRCodeSetupPayloadParser alloc] initWithBase38Representation:@"MT:R5L90MP500K64J0A33P0GQ670.QT52B.E23O6DE0Y3U10O0"];
    CHIPSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, kCommissioningFlowStandard);
    XCTAssertEqual(payload.rendezvousInformation, kRendezvousInformationSoftAP);
    XCTAssertTrue([payload.serialNumber isEqualToString:@"1"]);

    NSArray<CHIPOptionalQRCodeInfo *> * vendorOptionalInfo = [payload getAllOptionalVendorData:&error];
    XCTAssertNil(error);
    XCTAssertEqual([vendorOptionalInfo count], 2);
    for (CHIPOptionalQRCodeInfo * info in vendorOptionalInfo) {
        if (info.tag.intValue == 2) {
            XCTAssertEqual(info.infoType.intValue, kOptionalQRCodeInfoTypeString);
            XCTAssertTrue([info.stringValue isEqualToString:@"myData"]);
        } else if (info.tag.intValue == 3) {
            XCTAssertEqual(info.infoType.intValue, kOptionalQRCodeInfoTypeInt32);
            XCTAssertEqual(info.integerValue.intValue, 12);
        }
    }
}

@end
