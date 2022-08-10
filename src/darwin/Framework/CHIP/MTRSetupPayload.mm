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

#import "MTRError.h"
#import "MTRError_Internal.h"
#import "MTRSetupPayload_Internal.h"
#import <setup_payload/SetupPayload.h>

@implementation MTROptionalQRCodeInfo
@end

@implementation MTRSetupPayload {
    chip::SetupPayload _chipSetupPayload;
}

- (NSNumber *)convertRendezvousFlags:(const chip::Optional<chip::RendezvousInformationFlags> &)value
{
    if (!value.HasValue()) {
        return nil;
    }

    NSUInteger flags = MTRDiscoveryCapabilitiesNone;
    if (value.Value().Has(chip::RendezvousInformationFlag::kBLE)) {
        flags |= MTRDiscoveryCapabilitiesBLE;
    }
    if (value.Value().Has(chip::RendezvousInformationFlag::kSoftAP)) {
        flags |= MTRDiscoveryCapabilitiesSoftAP;
    }
    if (value.Value().Has(chip::RendezvousInformationFlag::kOnNetwork)) {
        flags |= MTRDiscoveryCapabilitiesOnNetwork;
    }
    return [NSNumber numberWithUnsignedLong:flags];
}

- (MTRCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value
{
    if (value == chip::CommissioningFlow::kStandard) {
        return MTRCommissioningFlowStandard;
    }
    if (value == chip::CommissioningFlow::kUserActionRequired) {
        return MTRCommissioningFlowUserActionRequired;
    }
    if (value == chip::CommissioningFlow::kCustom) {
        return MTRCommissioningFlowCustom;
    }
    return MTRCommissioningFlowInvalid;
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
        _hasShortDiscriminator = setupPayload.discriminator.IsShortDiscriminator();
        if (_hasShortDiscriminator) {
            _discriminator = [NSNumber numberWithUnsignedShort:setupPayload.discriminator.GetShortValue()];
        } else {
            _discriminator = [NSNumber numberWithUnsignedShort:setupPayload.discriminator.GetLongValue()];
        }
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

- (NSArray<MTROptionalQRCodeInfo *> *)getAllOptionalVendorData:(NSError * __autoreleasing *)error
{
    NSMutableArray<MTROptionalQRCodeInfo *> * allOptionalData = [NSMutableArray new];
    std::vector<chip::OptionalQRCodeInfo> chipOptionalData = _chipSetupPayload.getAllOptionalVendorData();
    for (chip::OptionalQRCodeInfo chipInfo : chipOptionalData) {
        MTROptionalQRCodeInfo * info = [MTROptionalQRCodeInfo new];
        info.tag = [NSNumber numberWithUnsignedChar:chipInfo.tag];
        switch (chipInfo.type) {
        case chip::optionalQRCodeInfoTypeString:
            info.infoType = [NSNumber numberWithInt:MTROptionalQRCodeInfoTypeString];
            info.stringValue = [NSString stringWithUTF8String:chipInfo.data.c_str()];
            break;
        case chip::optionalQRCodeInfoTypeInt32:
            info.infoType = [NSNumber numberWithInt:MTROptionalQRCodeInfoTypeInt32];
            info.integerValue = [NSNumber numberWithInt:chipInfo.int32];
            break;
        default:
            if (error) {
                *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidArgument userInfo:nil];
            }
            return @[];
        }
        [allOptionalData addObject:info];
    }
    return allOptionalData;
}

+ (NSUInteger)generateRandomPIN
{
    do {
        // Make sure the thing we generate is in the right range.
        uint32_t setupPIN = arc4random_uniform(chip::kSetupPINCodeMaximumValue) + 1;
        if (chip::SetupPayload::IsValidSetupPIN(setupPIN)) {
            return setupPIN;
        }

        // We got pretty unlikely with our random number generation.  Just try
        // again.  The chance that this loop does not terminate in a reasonable
        // amount of time is astronomically low, assuming arc4random_uniform is not
        // broken.
    } while (1);

    // Not reached.
    return chip::kSetupPINCodeUndefinedValue;
}

#pragma mark - NSSecureCoding

static NSString * const MTRSetupPayloadCodingKeyVersion = @"MTRSP.ck.version";
static NSString * const MTRSetupPayloadCodingKeyVendorID = @"MTRSP.ck.vendorID";
static NSString * const MTRSetupPayloadCodingKeyProductID = @"MTRSP.ck.productID";
static NSString * const MTRSetupPayloadCodingKeyCommissioningFlow = @"MTRSP.ck.commissioningFlow";
static NSString * const MTRSetupPayloadCodingKeyRendezvousFlags = @"MTRSP.ck.rendezvousFlags";
static NSString * const MTRSetupPayloadCodingKeyHasShortDiscriminator = @"MTRSP.ck.hasShortDiscriminator";
static NSString * const MTRSetupPayloadCodingKeyDiscriminator = @"MTRSP.ck.discriminator";
static NSString * const MTRSetupPayloadCodingKeySetupPINCode = @"MTRSP.ck.setupPINCode";
static NSString * const MTRSetupPayloadCodingKeySerialNumber = @"MTRSP.ck.serialNumber";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.version forKey:MTRSetupPayloadCodingKeyVersion];
    [coder encodeObject:self.vendorID forKey:MTRSetupPayloadCodingKeyVendorID];
    [coder encodeObject:self.productID forKey:MTRSetupPayloadCodingKeyProductID];
    // Casts are safe because commissioning flow and hasShortDiscriminator
    // values are both pretty small and non-negative.
    [coder encodeInteger:static_cast<NSInteger>(self.commissioningFlow) forKey:MTRSetupPayloadCodingKeyCommissioningFlow];
    [coder encodeObject:self.rendezvousInformation forKey:MTRSetupPayloadCodingKeyRendezvousFlags];
    [coder encodeInteger:static_cast<NSInteger>(self.hasShortDiscriminator) forKey:MTRSetupPayloadCodingKeyHasShortDiscriminator];
    [coder encodeObject:self.discriminator forKey:MTRSetupPayloadCodingKeyDiscriminator];
    [coder encodeObject:self.setUpPINCode forKey:MTRSetupPayloadCodingKeySetupPINCode];
    [coder encodeObject:self.serialNumber forKey:MTRSetupPayloadCodingKeySerialNumber];
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    NSNumber * version = [decoder decodeObjectOfClass:[NSNumber class] forKey:MTRSetupPayloadCodingKeyVersion];
    NSNumber * vendorID = [decoder decodeObjectOfClass:[NSNumber class] forKey:MTRSetupPayloadCodingKeyVendorID];
    NSNumber * productID = [decoder decodeObjectOfClass:[NSNumber class] forKey:MTRSetupPayloadCodingKeyProductID];
    NSInteger commissioningFlow = [decoder decodeIntegerForKey:MTRSetupPayloadCodingKeyCommissioningFlow];
    NSNumber * rendezvousInformation = [decoder decodeObjectOfClass:[NSNumber class]
                                                             forKey:MTRSetupPayloadCodingKeyRendezvousFlags];
    NSInteger hasShortDiscriminator = [decoder decodeIntegerForKey:MTRSetupPayloadCodingKeyHasShortDiscriminator];
    NSNumber * discriminator = [decoder decodeObjectOfClass:[NSNumber class] forKey:MTRSetupPayloadCodingKeyDiscriminator];
    NSNumber * setUpPINCode = [decoder decodeObjectOfClass:[NSNumber class] forKey:MTRSetupPayloadCodingKeySetupPINCode];
    NSString * serialNumber = [decoder decodeObjectOfClass:[NSString class] forKey:MTRSetupPayloadCodingKeySerialNumber];

    MTRSetupPayload * payload = [[MTRSetupPayload alloc] init];
    payload.version = version;
    payload.vendorID = vendorID;
    payload.productID = productID;
    payload.commissioningFlow = static_cast<MTRCommissioningFlow>(commissioningFlow);
    payload.rendezvousInformation = rendezvousInformation;
    payload.hasShortDiscriminator = static_cast<BOOL>(hasShortDiscriminator);
    payload.discriminator = discriminator;
    payload.setUpPINCode = setUpPINCode;
    payload.serialNumber = serialNumber;

    return payload;
}

@end
