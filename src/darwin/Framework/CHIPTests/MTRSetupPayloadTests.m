/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

    XCTAssertEqualObjects(payload.manualEntryCode, @"641286075300001000016");
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

- (void)testManualParser_Error
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"" error:&error];

    XCTAssertNil(payload);
    XCTAssertNotNil(error);
}

- (void)testQRCodeParser_Error
{
    NSError * error;
    MTRSetupPayload * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:@"MT:M5L90MP500K64J0000." error:&error];

    XCTAssertNil(payload);
    XCTAssertNotNil(error);
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

    // Test access by tag
    XCTAssertEqualObjects([payload vendorElementWithTag:@130].stringValue, @"myData");
    XCTAssertEqualObjects([payload vendorElementWithTag:@131].integerValue, @12);
}

- (void)testAddVendorElement
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@314159 discriminator:@555];
    XCTAssertEqual(payload.vendorElements.count, 0);
    [payload addOrReplaceVendorElement:[[MTROptionalQRCodeInfo alloc] initWithTag:@0xff int32Value:42]];
    XCTAssertEqual(payload.vendorElements.count, 1);
    XCTAssertEqualObjects(payload.vendorElements.firstObject.integerValue, @42);
    XCTAssertEqualObjects([payload vendorElementWithTag:@0xff].integerValue, @42);
}

- (void)testVendorElementsEncodedToQRCode
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@314159 discriminator:@555];
    [payload addOrReplaceVendorElement:[[MTROptionalQRCodeInfo alloc] initWithTag:@0x80 stringValue:@"Hello"]];
    MTRSetupPayload * decoded = [[MTRSetupPayload alloc] initWithPayload:payload.qrCodeString];
    XCTAssertNotNil(decoded);
    XCTAssertEqualObjects([decoded vendorElementWithTag:@0x80].stringValue, @"Hello");
}

- (void)testRemoveVendorElements
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithPayload:@"MT:M5L90MP500K64J0A33P0SET70.QT52B.E23-WZE0WISA0DK5N1K8SQ1RYCU1O0"];
    XCTAssertNotNil(payload);
    XCTAssertEqual(payload.vendorElements.count, 2);
    [payload removeVendorElementWithTag:@128]; // no change, no vendor element present with this tag
    XCTAssertEqual(payload.vendorElements.count, 2);
    [payload removeVendorElementWithTag:@130];
    XCTAssertEqual(payload.vendorElements.count, 1);
    [payload removeVendorElementWithTag:@131];
    XCTAssertEqual(payload.vendorElements.count, 0);
}

- (void)testQrCodeInfoCopyAndEquality
{
    MTROptionalQRCodeInfo * a = [[MTROptionalQRCodeInfo alloc] initWithTag:@0x88 stringValue:@"hello"];
    MTROptionalQRCodeInfo * b = [[MTROptionalQRCodeInfo alloc] initWithTag:@0x88 stringValue:@"hello"];
    MTROptionalQRCodeInfo * c = [[MTROptionalQRCodeInfo alloc] initWithTag:@0xff stringValue:@"hello"];
    MTROptionalQRCodeInfo * d = [[MTROptionalQRCodeInfo alloc] initWithTag:@0x88 int32Value:42];
    MTROptionalQRCodeInfo * e = [[MTROptionalQRCodeInfo alloc] initWithTag:@0x88 int32Value:0xbad];
    XCTAssertTrue([a isEqual:a]);
    XCTAssertTrue([a isEqual:[a copy]]);
    XCTAssertTrue([a isEqual:b]);
    XCTAssertTrue([a isEqual:[b copy]]);
    XCTAssertEqual(a.hash, b.hash);
    XCTAssertEqual(a.hash, [[a copy] hash]);
    XCTAssertFalse([a isEqual:nil]);
    XCTAssertFalse([a isEqual:c]);
    XCTAssertFalse([a isEqual:d]);
    XCTAssertFalse([a isEqual:e]);
    XCTAssertFalse([d isEqual:e]);
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

- (void)testDeprecatedPropertyAliases
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
        // Ensure our test data has discriminator values that make for the most meaningful round-trip tests.
        XCTAssert(payload.discriminator.integerValue != 0);
        XCTAssert(payload.hasShortDiscriminator || payload.discriminator.integerValue > 0xf);
        [payloads addObject:payload];
    }

    // Also test some other payloads that don't have a valid QR / MPC representation
    [payloads addObject:[[MTRSetupPayload alloc] init]];
    [payloads addObject:[[MTRSetupPayload alloc] initWithSetupPasscode:@22222222 discriminator:@42]];

    MTRSetupPayload * futureValues = [[MTRSetupPayload alloc] initWithSetupPasscode:@314159 discriminator:@555];
    futureValues.commissioningFlow = 3; // reserved in the spec
    futureValues.discoveryCapabilities = 0x84; // bits 3-7 reserved in the spec
    [payloads addObject:futureValues];

    for (MTRSetupPayload * payload in payloads) {
        NSLog(@"Payload: %@", payload);

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

        XCTAssertEqual(decodedVDs.count, payloadVDs.count);
        for (int i = 0; i < decodedVDs.count; i++) {
            MTROptionalQRCodeInfo * decodedVD = decodedVDs[i];
            MTROptionalQRCodeInfo * payloadVD = payloadVDs[i];
            XCTAssertEqual(decodedVD.type, payloadVD.type);
            XCTAssertEqualObjects(decodedVD.tag, payloadVD.tag);
            XCTAssertEqualObjects(decodedVD.integerValue, payloadVD.integerValue);
            XCTAssertEqualObjects(decodedVD.stringValue, payloadVD.stringValue);
            XCTAssertEqualObjects(decodedVD, payloadVD); // also check with isEqual:
        }

        // Note that we can't necessarily expect the manualEntryCode and qrCode strings
        // we generate here to match the original string, but we should get the same
        // output from the decoded and original objects.
        XCTAssertEqualObjects([decoded qrCodeString:NULL], [payload qrCodeString:NULL]);
        XCTAssertEqualObjects(decoded.manualEntryCode, payload.manualEntryCode);
    }
}

- (void)testCopyingAndEquality
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithPayload:@"MT:M5L9000000K64J00000"];
    XCTAssertFalse(payload.hasShortDiscriminator); // came from a QR code
    XCTAssert(payload.discriminator.integerValue > 0xf); // can't "accidentally" round-trip through a short discriminator

    MTRSetupPayload * copy = [payload copy];
    XCTAssertNotIdentical(payload, copy); // MTRSetupPayload is mutable, must be a new object

    XCTAssertTrue([payload isEqual:copy]);
    XCTAssertTrue([copy isEqual:payload]);
    XCTAssertEqual(payload.hash, copy.hash);

    copy.hasShortDiscriminator = YES;
    XCTAssertFalse([copy isEqual:payload]);
    copy.hasShortDiscriminator = NO;
    XCTAssertTrue([copy isEqual:payload]);
    XCTAssertEqual(payload.hash, copy.hash);

    MTROptionalQRCodeInfo * element = [[MTROptionalQRCodeInfo alloc] initWithTag:@0x80 stringValue:@"To infinity and beyond!"];
    [copy addOrReplaceVendorElement:element];
    XCTAssertFalse([copy isEqual:payload]);
    [payload addOrReplaceVendorElement:element];
    XCTAssertTrue([copy isEqual:payload]);
    XCTAssertEqual(payload.hash, copy.hash);

    copy.serialNumber = @"555-123";
    XCTAssertFalse([copy isEqual:payload]);
    payload.serialNumber = @"555-123";
    XCTAssertTrue([copy isEqual:payload]);
    XCTAssertEqual(payload.hash, copy.hash);
}

- (void)testCanParseFutureDiscoveryMethod
{
    // We must be able to process QR codes that include discovery methods we don't understand yet
    XCTAssertEqual([[MTRSetupPayload alloc] initWithPayload:@"MT:000002VDK3VHMR49000"].discoveryCapabilities, 0x84);
    XCTAssertEqual([[MTRSetupPayload alloc] initWithPayload:@"MT:-24J0Q.C.0KA0648G00"].discoveryCapabilities, 0xfa);
}

- (void)testDescriptionShowsUnknownDiscoveryMethods
{
    MTRSetupPayload * a = [[MTRSetupPayload alloc] initWithSetupPasscode:@888 discriminator:@555];
    MTRSetupPayload * b = [a copy];
    b.discoveryCapabilities |= 0x80;
    XCTAssertNotEqualObjects(a.description, b.description);
}

- (uint32_t)generateRepeatedDigitPasscode:(uint8_t)digit
{
    // "digit" is expected to be a single digit.  Generates a number that has
    // that digit repeated 8 times.
    uint32_t passcode = 0;
    for (int i = 0; i < 8; ++i) {
        passcode = passcode * 10 + digit;
    }
    return passcode;
}

- (void)testValidSetupPasscode
{
    // First, check the repeated-digit cases.
    for (uint8_t digit = 0; digit <= 9; ++digit) {
        XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@([self generateRepeatedDigitPasscode:digit])]);
    }

    // Then the sequential special cases.
    XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@(12345678)]);
    XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@(87654321)]);

    // Then the "too big" cases:
    XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@(100000000)]);
    XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@(1lu << 27)]);
    XCTAssertFalse([MTRSetupPayload isValidSetupPasscode:@((1llu << 32) + 1)]);

    // Now some tests for known-valid passcodes:
    XCTAssertTrue([MTRSetupPayload isValidSetupPasscode:@(1)]);
    XCTAssertTrue([MTRSetupPayload isValidSetupPasscode:@(78654321)]);

    // And we should only generate valid ones.
    XCTAssertTrue([MTRSetupPayload isValidSetupPasscode:[MTRSetupPayload generateRandomSetupPasscode]]);
}

@end
