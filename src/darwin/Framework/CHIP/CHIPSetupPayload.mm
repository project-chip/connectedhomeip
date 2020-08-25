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

#import "CHIPSetupPayload.h"
#import <setup_payload/SetupPayload.h>

@implementation CHIPOptionalQRCodeInfo
@end

@implementation CHIPSetupPayload {
    chip::SetupPayload _chipSetupPayload;
}

- (RendezvousInformationFlags)valueOf:(chip::RendezvousInformationFlags)value
{
    RendezvousInformationFlags rv = kRendezvousInformationNone;
    switch (value) {
    case chip::RendezvousInformationFlags::kNone:
        rv = kRendezvousInformationNone;
        break;
    case chip::RendezvousInformationFlags::kWiFi:
        rv = kRendezvousInformationWiFi;
        break;
    case chip::RendezvousInformationFlags::kBLE:
        rv = kRendezvousInformationBLE;
        break;
    case chip::RendezvousInformationFlags::kThread:
        rv = kRendezvousInformationThread;
        break;
    case chip::RendezvousInformationFlags::kEthernet:
        rv = kRendezvousInformationEthernet;
        break;
    case chip::RendezvousInformationFlags::kAllMask:
        rv = kRendezvousInformationAllMask;
        break;
    }
    return rv;
}

- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload
{
    if (self = [super init]) {
        _chipSetupPayload = setupPayload;
        _version = [NSNumber numberWithUnsignedChar:setupPayload.version];
        _vendorID = [NSNumber numberWithUnsignedShort:setupPayload.vendorID];
        _productID = [NSNumber numberWithUnsignedShort:setupPayload.productID];
        _requiresCustomFlow = setupPayload.requiresCustomFlow == 1;
        _rendezvousInformation = [self valueOf:setupPayload.rendezvousInformation];
        _discriminator = [NSNumber numberWithUnsignedShort:setupPayload.discriminator];
        _setUpPINCode = [NSNumber numberWithUnsignedLong:setupPayload.setUpPINCode];

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
    vector<chip::OptionalQRCodeInfo> chipOptionalData = _chipSetupPayload.getAllOptionalVendorData();
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
