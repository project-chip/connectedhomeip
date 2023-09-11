/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTROnboardingPayloadParser.h"
#import "MTRManualSetupPayloadParser.h"
#import "MTRQRCodeSetupPayloadParser.h"
#import "MTRSetupPayload.h"

@implementation MTROnboardingPayloadParser

+ (bool)isQRCode:(NSString *)codeString
{
    return [codeString hasPrefix:@"MT:"];
}

+ (MTRSetupPayload * _Nullable)setupPayloadForOnboardingPayload:(NSString *)onboardingPayload
                                                          error:(NSError * __autoreleasing *)error
{
    MTRSetupPayload * payload;
    // MTROnboardingPayloadTypeNFC is of type QR code and handled same as QR code
    MTROnboardingPayloadType type =
        [self isQRCode:onboardingPayload] ? MTROnboardingPayloadTypeQRCode : MTROnboardingPayloadTypeManualCode;
    switch (type) {
    case MTROnboardingPayloadTypeManualCode:
        payload = [self setupPayloadForManualCodeOnboardingPayload:onboardingPayload error:error];
        break;
    case MTROnboardingPayloadTypeQRCode:
        payload = [self setupPayloadForQRCodeOnboardingPayload:onboardingPayload error:error];
        break;
    default:
        break;
    }
    return payload;
}

+ (MTRSetupPayload * _Nullable)setupPayloadForQRCodeOnboardingPayload:(NSString *)onboardingPayload
                                                                error:(NSError * __autoreleasing *)error
{
    MTRQRCodeSetupPayloadParser * qrCodeParser =
        [[MTRQRCodeSetupPayloadParser alloc] initWithBase38Representation:onboardingPayload];
    return [qrCodeParser populatePayload:error];
}

+ (MTRSetupPayload * _Nullable)setupPayloadForManualCodeOnboardingPayload:(NSString *)onboardingPayload
                                                                    error:(NSError * __autoreleasing *)error
{
    MTRManualSetupPayloadParser * manualParser =
        [[MTRManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:onboardingPayload];
    return [manualParser populatePayload:error];
}
@end
