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
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"641286075300001000016" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 10);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 12345670);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesUnknown);
}

- (void)testOnboardingPayloadParser_QRCode_NoError
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J00000" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testOnboardingPayloadParser_QRCode_WrongVersion
{
    // Same as testOnboardingPayloadParser_QRCode_NoError, but with version set to 5.
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:R5L90MP500K64J00000" error:&error];

    XCTAssertNil(payload);
    XCTAssertNotNil(error);
}

- (void)testOnboardingPayloadParser_NFC_NoError
{
    NSError * error;
    MTRSetupPayload * payload =
        [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
                                                     error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testManualParser
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"641286075300001000016" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 10);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 12345670);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowCustom);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesUnknown);
}

- (void)testManualParser_Error
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"" error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidStringLength);
}

- (void)testQRCodeParser_Error
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J0000." error:&error];

    XCTAssertNil(payload);
    XCTAssertEqual(error.code, MTRErrorCodeInvalidArgument);
}

- (void)testQRCodeParser
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J00000" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesSoftAP);
}

- (void)testQRCodeParserWithOptionalData
{
    NSError * error;
    MTRSetupPayload * payload =
        [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
                                                     error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesSoftAP);
    XCTAssertTrue([payload.serialNumber isEqualToString:@"123456789"]);

    NSArray<MTROptionalQRCodeInfo *> * vendorOptionalInfo = [payload getAllOptionalVendorData:&error];
    XCTAssertNil(error);
    XCTAssertEqual([vendorOptionalInfo count], 2);
    for (MTROptionalQRCodeInfo * info in vendorOptionalInfo) {
        if (info.tag.intValue == 130) {
            XCTAssertEqual(info.type, MTROptionalQRCodeInfoTypeString);
            XCTAssertEqual([info.infoType unsignedIntValue], MTROptionalQRCodeInfoTypeString);
            XCTAssertTrue([info.stringValue isEqualToString:@"myData"]);
        } else if (info.tag.intValue == 131) {
            XCTAssertEqual(info.type, MTROptionalQRCodeInfoTypeInt32);
            XCTAssertEqual([info.infoType unsignedIntValue], MTROptionalQRCodeInfoTypeInt32);
            XCTAssertEqual(info.integerValue.intValue, 12);
        }
    }
}

- (void)testQRCodeWithNoCapabilities
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L9000000K64J00000" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertFalse(payload.hasShortDiscriminator);
    XCTAssertEqual(payload.discriminator.unsignedIntegerValue, 128);
    XCTAssertEqual(payload.setupPasscode.unsignedIntegerValue, 2048);
    XCTAssertEqual(payload.vendorID.unsignedIntegerValue, 12);
    XCTAssertEqual(payload.productID.unsignedIntegerValue, 1);
    XCTAssertEqual(payload.commissioningFlow, MTRCommissioningFlowStandard);
    XCTAssertEqual(payload.version.unsignedIntegerValue, 0);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesOnNetwork);
}

- (void)testQRCodePropertyAliases
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L9000000K64J00000" error:&error];

    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedIntValue], MTRDiscoveryCapabilitiesOnNetwork);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesOnNetwork);

    // Check that writing to rendezvousInformation is reflected in
    // discoveryCapabilities.
    payload.rendezvousInformation = nil;
    XCTAssertNil(payload.rendezvousInformation);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesUnknown);

    payload.rendezvousInformation = @(MTRDiscoveryCapabilitiesSoftAP);
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedIntValue], MTRDiscoveryCapabilitiesSoftAP);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesSoftAP);

    // Check that writing to discoveryCapabilities is reflected in
    // rendezvousInformation.
    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesBLE;
    XCTAssertNotNil(payload.rendezvousInformation);
    XCTAssertEqual([payload.rendezvousInformation unsignedIntValue], MTRDiscoveryCapabilitiesBLE);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesBLE);

    payload.discoveryCapabilities = MTRDiscoveryCapabilitiesUnknown;
    XCTAssertNil(payload.rendezvousInformation);
    XCTAssertEqual(payload.discoveryCapabilities, MTRDiscoveryCapabilitiesUnknown);

    // Check that setupPasscode and setUpPINCode alias each other.
    payload.setupPasscode = @(1);
    XCTAssertEqualObjects(payload.setupPasscode, @(1));
    XCTAssertEqualObjects(payload.setUpPINCode, @(1));

    payload.setUpPINCode = @(2);
    XCTAssertEqualObjects(payload.setupPasscode, @(2));
    XCTAssertEqualObjects(payload.setUpPINCode, @(2));
}

- (void)testSerialNumberRoundTrip
{
    NSError * error;
    MTRSetupPayload * payload =
        [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
                                                     error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(payload);

    XCTAssertEqualObjects(payload.serialNumber, @"123456789");

    NSString * serialNumber = @"12345";
    payload.serialNumber = serialNumber;

    NSString * qrCode = [payload qrCodeString:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(qrCode);

    MTRSetupPayload * newPayload = [MTRSetupPayload setupPayloadWithOnboardingPayload:qrCode error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(newPayload);

    XCTAssertEqualObjects(newPayload.serialNumber, serialNumber);
}

@end
