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

#import "CHIPError.h"
#import "CHIPError_Internal.h"
#import "CHIPSetupPayload_Internal.h"
#import <setup_payload/SetupPayload.h>

@implementation CHIPOptionalQRCodeInfo
@end

@implementation CHIPSetupPayload {
    chip::SetupPayload _chipSetupPayload;
}

- (CHIPRendezvousInformationFlags)convertRendezvousFlags:(chip::RendezvousInformationFlags)value
{
    if (value.Has(chip::RendezvousInformationFlag::kBLE)) {
        return kRendezvousInformationBLE;
    } else if (value.Has(chip::RendezvousInformationFlag::kSoftAP)) {
        return kRendezvousInformationSoftAP;
    } else if (value.Has(chip::RendezvousInformationFlag::kOnNetwork)) {
        return kRendezvousInformationOnNetwork;
    } else {
        return kRendezvousInformationNone;
    }
}

- (CHIPCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value
{
    if (value == chip::CommissioningFlow::kStandard) {
        return kCommissioningFlowStandard;
    } else if (value == chip::CommissioningFlow::kUserActionRequired) {
        return kCommissioningFlowUserActionRequired;
    } else if (value == chip::CommissioningFlow::kCustom) {
        return kCommissioningFlowCustom;
    } else {
        return kCommissioningFlowInvalid;
    }
}

- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload
{
    if (self = [super init]) {
        _chipSetupPayload = setupPayload;
        _version = [NSNumber numberWithUnsignedChar:setupPayload.version];
        _vendorID = [NSNumber numberWithUnsignedShort:setupPayload.vendorID];
        _productID = [NSNumber numberWithUnsignedShort:setupPayload.productID];
        _commissioningFlow = [self convertCommissioningFlow:setupPayload.commissioningFlow];
        _rendezvousInformation = [self convertRendezvousFlags:setupPayload.rendezvousInformation];
        _discriminator = [NSNumber numberWithUnsignedShort:setupPayload.discriminator];
        _setUpPINCode = [NSNumber numberWithUnsignedInt:setupPayload.setUpPINCode];

        [self getSerialNumber:setupPayload];
    }
    return self;
}

- (void)getSerialNumber:(chip::SetupPayload)setupPayload
{
    std::string serialNumberC;
    CHIP_ERROR err = setupPayload.getSerialNumber(serialNumberC);
    if (err == CHIP_NO_ERROR) {
        _serialNumber = [NSString stringWithUTF8String:serialNumberC.c_str()];
    }
}

- (NSArray<CHIPOptionalQRCodeInfo *> *)getAllOptionalVendorData:(NSError * __autoreleasing *)error
{
    NSMutableArray<CHIPOptionalQRCodeInfo *> * allOptionalData = [NSMutableArray new];
    std::vector<chip::OptionalQRCodeInfo> chipOptionalData = _chipSetupPayload.getAllOptionalVendorData();
    for (chip::OptionalQRCodeInfo chipInfo : chipOptionalData) {
        CHIPOptionalQRCodeInfo * info = [CHIPOptionalQRCodeInfo new];
        info.tag = [NSNumber numberWithUnsignedChar:chipInfo.tag];
        switch (chipInfo.type) {
        case chip::optionalQRCodeInfoTypeString:
            info.infoType = [NSNumber numberWithInt:kOptionalQRCodeInfoTypeString];
            info.stringValue = [NSString stringWithUTF8String:chipInfo.data.c_str()];
            break;
        case chip::optionalQRCodeInfoTypeInt32:
            info.infoType = [NSNumber numberWithInt:kOptionalQRCodeInfoTypeInt32];
            info.integerValue = [NSNumber numberWithInt:chipInfo.int32];
            break;
        default:
            if (error) {
                *error = [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeInvalidArgument userInfo:nil];
            }
            return @[];
        }
        [allOptionalData addObject:info];
    }
    return allOptionalData;
}
@end
