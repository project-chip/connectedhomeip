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

#import "CHIPOnboardingPayloadParser.h"
#import "CHIPManualSetupPayloadParser.h"
#import "CHIPQRCodeSetupPayloadParser.h"
#import "CHIPSetupPayload.h"

@implementation CHIPOnboardingPayloadParser

+ (nullable CHIPSetupPayload *)setupPayloadForOnboardingPayload:(NSString *)onboardingPayload
                                                         ofType:(CHIPOnboardingPayloadType)type
                                                          error:(NSError * __autoreleasing *)error
{
    CHIPSetupPayload * payload;
    switch (type) {
    case CHIPOnboardingPayloadTypeManualCode:
    case CHIPOnboardingPayloadTypeAdmin:
        payload = [self setupPayloadForManualCodeOnboardingPayload:onboardingPayload error:error];
        break;
    case CHIPOnboardingPayloadTypeQRCode:
    case CHIPOnboardingPayloadTypeNFC:
        payload = [self setupPayloadForQRCodeOnboardingPayload:onboardingPayload error:error];
        break;
    default:
        break;
    }
    return payload;
}

+ (nullable CHIPSetupPayload *)setupPayloadForQRCodeOnboardingPayload:(NSString *)onboardingPayload
                                                                error:(NSError * __autoreleasing *)error
{
    CHIPQRCodeSetupPayloadParser * qrCodeParser =
        [[CHIPQRCodeSetupPayloadParser alloc] initWithBase38Representation:onboardingPayload];
    return [qrCodeParser populatePayload:error];
}

+ (nullable CHIPSetupPayload *)setupPayloadForManualCodeOnboardingPayload:(NSString *)onboardingPayload
                                                                    error:(NSError * __autoreleasing *)error
{
    CHIPManualSetupPayloadParser * manualParser =
        [[CHIPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:onboardingPayload];
    return [manualParser populatePayload:error];
}
@end
