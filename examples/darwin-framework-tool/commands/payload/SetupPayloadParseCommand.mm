/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "SetupPayloadParseCommand.h"
#include "MTRError_Utils.h"
#import <Matter/Matter.h>

using namespace ::chip;

namespace {

#if CHIP_PROGRESS_LOGGING

NSString * CustomFlowString(MTRCommissioningFlow flow)
{
    switch (flow) {
    case MTRCommissioningFlowStandard:
        return @"STANDARD";
    case MTRCommissioningFlowUserActionRequired:
        return @"USER ACTION REQUIRED";
    case MTRCommissioningFlowCustom:
        return @"CUSTOM";
    case MTRCommissioningFlowInvalid:
        return @"INVALID";
    }

    return @"???";
}

#endif // CHIP_PROGRESS_LOGGING

} // namespace

void SetupPayloadParseCommand::LogNSError(const char * logString, NSError * error)
{
    CHIP_ERROR err = MTRErrorToCHIPErrorCode(error);
    if (err == CHIP_NO_ERROR) {
        ChipLogProgress(chipTool, "%s: %s", logString, chip::ErrorStr(err));
    } else {
        ChipLogError(chipTool, "%s: %s", logString, chip::ErrorStr(err));
    }
}

CHIP_ERROR SetupPayloadParseCommand::Run()
{
    NSString * codeString = [NSString stringWithCString:mCode encoding:NSASCIIStringEncoding];
    NSError * error;
    MTRSetupPayload * payload;
    MTROnboardingPayloadType codeType;
    if (IsQRCode(codeString)) {
        codeType = MTROnboardingPayloadTypeQRCode;
    } else {
        codeType = MTROnboardingPayloadTypeManualCode;
    }
    payload = [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:codeString ofType:codeType error:&error];
    if (error) {
        LogNSError("Error: ", error);
        return CHIP_ERROR_INTERNAL;
    }
    ReturnErrorOnFailure(Print(payload));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadParseCommand::Print(MTRSetupPayload * payload)
{
    NSLog(@"Version:       %@", payload.version);
    NSLog(@"VendorID:      %@", payload.vendorID);
    NSLog(@"ProductID:     %@", payload.productID);
    NSLog(@"Custom flow:   %lu    (%@)", payload.commissioningFlow, CustomFlowString(payload.commissioningFlow));
    {
        NSMutableString * humanFlags = [[NSMutableString alloc] init];

        if (payload.rendezvousInformation) {
            if (payload.rendezvousInformation & MTRRendezvousInformationNone) {
                [humanFlags appendString:@"NONE"];
            } else {
                if (payload.rendezvousInformation & MTRRendezvousInformationSoftAP) {
                    [humanFlags appendString:@"SoftAP"];
                }
                if (payload.rendezvousInformation & MTRRendezvousInformationBLE) {
                    if (!humanFlags) {
                        [humanFlags appendString:@", "];
                    }
                    [humanFlags appendString:@"BLE"];
                }
                if (payload.rendezvousInformation & MTRRendezvousInformationOnNetwork) {
                    if (!humanFlags) {
                        [humanFlags appendString:@", "];
                    }
                    [humanFlags appendString:@"ON NETWORK"];
                }
            }
        } else {
            [humanFlags appendString:@"NONE"];
        }

        NSLog(@"Capabilities:  0x%02lX (%@)", payload.rendezvousInformation, humanFlags);
    }
    NSLog(@"Discriminator: %@", payload.discriminator);
    NSLog(@"Passcode:      %@", payload.setUpPINCode);

    if (payload.serialNumber) {
        NSLog(@"SerialNumber: %@", payload.serialNumber);
    }
    NSError * error;
    NSArray<MTROptionalQRCodeInfo *> * optionalVendorData = [payload getAllOptionalVendorData:&error];
    if (error) {
        LogNSError("Error: ", error);
        return CHIP_ERROR_INTERNAL;
    }
    for (const MTROptionalQRCodeInfo * info : optionalVendorData) {
        bool isTypeString = [info.infoType isEqual:@(MTROptionalQRCodeInfoTypeString)];
        bool isTypeInt32 = [info.infoType isEqual:@(MTROptionalQRCodeInfoTypeInt32)];
        VerifyOrReturnError(isTypeString || isTypeInt32, CHIP_ERROR_INVALID_ARGUMENT);

        if (isTypeString) {
            NSLog(@"OptionalQRCodeInfo: tag=%@,string value=%@", info.tag, info.stringValue);
        } else {
            NSLog(@"OptionalQRCodeInfo: tag=%@,int value=%@", info.tag, info.integerValue);
        }
    }

    return CHIP_NO_ERROR;
}

bool SetupPayloadParseCommand::IsQRCode(NSString * codeString) { return [codeString hasPrefix:@"MT:"]; }
