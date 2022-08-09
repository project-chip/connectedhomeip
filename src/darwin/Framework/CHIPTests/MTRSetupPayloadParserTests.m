//
//  MTRSetupPayloadParserTests.m
//  MTRQRCodeReaderTests
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
#import "MTRManualSetupPayloadParser.h"
#import "MTROnboardingPayloadParser.h"
#import "MTRQRCodeSetupPayloadParser.h"
#import "MTRSetupPayload.h"

// additional includes
#import "MTRError.h"

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRSetupPayloadParserTests : XCTestCase

@end

@implementation MTRSetupPayloadParserTests

- (void)testOnboardingPayloadParser_Manual_NoError
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                      ofType:MTROnboardingPayloadTypeManualCode
                                                                                       error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 10);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertNil(payload.rendezvousInformation);
}

- (void)testOnboardingPayloadParser_Manual_WrongType
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                      ofType:MTROnboardingPayloadTypeQRCode
                                                                                       error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidArgument);
}

- (void)testOnboardingPayloadParser_Admin_NoError
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                      ofType:MTROnboardingPayloadTypeAdmin
                                                                                       error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 10);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertNil(payload.rendezvousInformation);
}

- (void)testOnboardingPayloadParser_Admin_WrongType
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"636108753500001000015"
                                                                                      ofType:MTROnboardingPayloadTypeQRCode
                                                                                       error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidArgument);
}

- (void)testOnboardingPayloadParser_QRCode_NoError
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J00000"
                                                                                      ofType:MTROnboardingPayloadTypeQRCode
                                                                                       error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedLongValue], MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testOnboardingPayloadParser_QRCode_WrongType
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J00000"
                                                                                      ofType:MTROnboardingPayloadTypeAdmin
                                                                                       error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeIntegrityCheckFailed);
}

- (void)testOnboardingPayloadParser_NFC_NoError
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser
        setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
                                  ofType:MTROnboardingPayloadTypeNFC
                                   error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedLongValue], MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testOnboardingPayloadParser_NFC_WrongType
{
    NSError * error;
    MTRSetupPayload * payload = [MTROnboardingPayloadParser
        setupPayloadForOnboardingPayload:@"MT:R5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
                                  ofType:MTROnboardingPayloadTypeManualCode
                                   error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeIntegrityCheckFailed);
}

- (void)testManualParser
{
    NSError * error;
    MTRManualSetupPayloadParser * parser =
        [[MTRManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@"636108753500001000015"];
    MTRSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 10);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 123456780);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertNil(payload.rendezvousInformation);
}

- (void)testManualParser_Error
{
    NSError * error;
    MTRManualSetupPayloadParser * parser = [[MTRManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:@""];
    MTRSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidStringLength);
}

- (void)testQRCodeParser_Error
{
    NSError * error;
    MTRQRCodeSetupPayloadParser * parser =
        [[MTRQRCodeSetupPayloadParser alloc] initWithBase38Representation:@"MT:R5L90MP500K64J0000."];
    MTRSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidArgument);
}

- (void)testQRCodeParser
{
    NSError * error;
    MTRQRCodeSetupPayloadParser * parser =
        [[MTRQRCodeSetupPayloadParser alloc] initWithBase38Representation:@"MT:R5L90MP500K64J00000"];
    MTRSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedLongValue], MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testQRCodeParserWithOptionalData
{
    NSError * error;
    MTRQRCodeSetupPayloadParser * parser = [[MTRQRCodeSetupPayloadParser alloc]
        initWithBase38Representation:@"MT:R5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"];
    MTRSetupPayload * payload = [parser populatePayload:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.version.unsignedIntegerValue, 5);
    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setUpPINCode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedLongValue], MTRDiscoveryCapabilitiesSoftAP);
    XCTAssertTrue([payload.serialNumber isEqualToString:@"123456789"]);

    NSArray<MTROptionalQRCodeInfo *> * vendorOptionalInfo = [payload getAllOptionalVendorData:&error];
    XCTAssertNil(error);
    XCTAssertEqual([vendorOptionalInfo count], 2);
    for (MTROptionalQRCodeInfo * info in vendorOptionalInfo) {
        if (info.tag.intValue == 130) {
            XCTAssertEqual(info.infoType.intValue, MTROptionalQRCodeInfoTypeString);
            XCTAssertTrue([info.stringValue isEqualToString:@"myData"]);
        } else if (info.tag.intValue == 131) {
            XCTAssertEqual(info.infoType.intValue, MTROptionalQRCodeInfoTypeInt32);
            XCTAssertEqual(info.integerValue.intValue, 12);
        }
    }
}

@end
