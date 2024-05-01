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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

@interface MTRSetupPayloadTests : XCTestCase

@end

@implementation MTRSetupPayloadTests

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

- (void)test31129 // https://github.com/project-chip/connectedhomeip/issues/31129
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@99999998 discriminator:@3840];
    XCTAssertNotNil(payload);
    // The payload should be representable in at least one of manual or QR format.
    XCTAssert(payload.manualEntryCode != nil || [payload qrCodeString:NULL] != nil);
}

- (void)test23357 // https://github.com/project-chip/connectedhomeip/pull/23357
{
    // Should return nil for invalid payloads (e.g. invalid passcode "@11111111")
    XCTAssertNil([MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:-24J042C00KMSP0Z800" error:NULL]);
    XCTAssertNil([MTRSetupPayload setupPayloadWithOnboardingPayload:@"35191106788" error:NULL]);
}

- (void)testSecureCodingRoundtrip
{
    NSError * error;
    NSMutableArray<MTRSetupPayload *> * payloads = [[NSMutableArray alloc] init];
    for (NSString * string in @[
             @"34970112332",
             @"641286075300001000016",
             @"MT:M5L90MP500K64J00000",
             @"MT:M5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"
         ]) {
        MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:string error:&error];
        XCTAssertNotNil(payload, @"Error: %@", error);
        [payloads addObject:payload];
    }

    // Also test some other payloads that don't have a valid QR / MPC representation
    [payloads addObject:[[MTRSetupPayload alloc] init]];
    [payloads addObject:[[MTRSetupPayload alloc] initWithSetupPasscode:@22222222 discriminator:@42]];

    for (MTRSetupPayload * payload in payloads) {
        NSData * data = [NSKeyedArchiver archivedDataWithRootObject:payload requiringSecureCoding:YES error:&error];
        XCTAssertNotNil(data, @"Error: %@", error);
        MTRSetupPayload * decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRSetupPayload.class fromData:data error:&error];
        XCTAssertNotNil(decoded, @"Error: %@", error);

        XCTAssertEqualObjects(decoded.version, payload.version);
        XCTAssertEqualObjects(decoded.vendorID, payload.vendorID);
        XCTAssertEqualObjects(decoded.productID, payload.productID);
        XCTAssertEqual(decoded.commissioningFlow, payload.commissioningFlow);
        XCTAssertEqual(decoded.discoveryCapabilities, payload.discoveryCapabilities);
        XCTAssertEqual(decoded.hasShortDiscriminator, payload.hasShortDiscriminator);
        XCTAssertEqualObjects(decoded.discriminator, payload.discriminator);
        XCTAssertEqualObjects(decoded.setupPasscode, payload.setupPasscode);
        XCTAssertEqualObjects(decoded.serialNumber, payload.serialNumber);

        NSArray<MTROptionalQRCodeInfo *> * payloadVDs = [payload getAllOptionalVendorData:&error];
        XCTAssertNotNil(payloadVDs, @"Error: %@", error);
        NSArray<MTROptionalQRCodeInfo *> * decodedVDs = [decoded getAllOptionalVendorData:&error];
        XCTAssertNotNil(decodedVDs, @"Error: %@", error);

#if 0 // TODO: Encode / decode optional vendor data
      // MTROptionalQRCodeInfo does not implement isEqual (yet)
        XCTAssertEqual(decodedVDs.count, payloadVDs.count);
        for (int i = 0; i < decodedVDs.count; i++){
            MTROptionalQRCodeInfo * decodedVD = decodedVDs[i];
            MTROptionalQRCodeInfo * payloadVD = payloadVDs[i];
            XCTAssertEqual(decodedVD.type, payloadVD.type);
            XCTAssertEqualObjects(decodedVD.tag, payloadVD.tag);
            XCTAssertEqualObjects(decodedVD.integerValue, payloadVD.integerValue);
            XCTAssertEqualObjects(decodedVD.stringValue, payloadVD.stringValue);
        }
#endif

        // Note that we can't necessarily expect the manualEntryCode and qrCode strings
        // we generate here to match the original string, but we should get the same
        // output from the decoded and original objects.
        XCTAssertEqualObjects([decoded qrCodeString:NULL], [payload qrCodeString:NULL]);
        XCTAssertEqualObjects(decoded.manualEntryCode, payload.manualEntryCode);
    }
}

@end
