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
#import "MTROnboardingPayloadParser.h"
#import "MTRSetupPayload_Internal.h"
#import "setup_payload/ManualSetupPayloadGenerator.h"
#import "setup_payload/QRCodeSetupPayloadGenerator.h"
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

+ (chip::Optional<chip::RendezvousInformationFlags>)unconvertRendezvousFlags:(nullable NSNumber *)nullableValue
{
    if (nullableValue == nil) {
        return chip::NullOptional;
    }

    MTRDiscoveryCapabilities value = static_cast<MTRDiscoveryCapabilities>([nullableValue unsignedLongValue]);

    chip::RendezvousInformationFlags flags;
    if (value & MTRDiscoveryCapabilitiesBLE) {
        flags.Set(chip::RendezvousInformationFlag::kBLE);
    }
    if (value & MTRDiscoveryCapabilitiesSoftAP) {
        flags.Set(chip::RendezvousInformationFlag::kSoftAP);
    }
    if (value & MTRDiscoveryCapabilitiesOnNetwork) {
        flags.Set(chip::RendezvousInformationFlag::kOnNetwork);
    }
    return chip::MakeOptional(flags);
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

+ (chip::CommissioningFlow)unconvertCommissioningFlow:(MTRCommissioningFlow)value
{
    if (value == MTRCommissioningFlowStandard) {
        return chip::CommissioningFlow::kStandard;
    }
    if (value == MTRCommissioningFlowUserActionRequired) {
        return chip::CommissioningFlow::kUserActionRequired;
    }
    if (value == MTRCommissioningFlowCustom) {
        return chip::CommissioningFlow::kCustom;
    }
    // It's MTRCommissioningFlowInvalid ... now what?  But in practice
    // this is not called when we have MTRCommissioningFlowInvalid.
    return chip::CommissioningFlow::kStandard;
}

- (instancetype)initWithSetupPayload:(chip::SetupPayload)setupPayload
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

- (instancetype)initWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator
{
    if (self = [super init]) {
        _version = @(0); // Only supported Matter version so far.
        _vendorID = @(0); // Not available.
        _productID = @(0); // Not available.
        _commissioningFlow = MTRCommissioningFlowStandard;
        _rendezvousInformation = nil;
        _hasShortDiscriminator = NO;
        _discriminator = discriminator;
        _setUpPINCode = setupPasscode;
        _serialNumber = nil;
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
    return [[MTRSetupPayload generateRandomSetupPasscode] unsignedIntValue];
}

+ (NSNumber *)generateRandomSetupPasscode
{
    do {
        // Make sure the thing we generate is in the right range.
        uint32_t setupPIN = arc4random_uniform(chip::kSetupPINCodeMaximumValue) + 1;
        if (chip::SetupPayload::IsValidSetupPIN(setupPIN)) {
            return @(setupPIN);
        }

        // We got pretty unlikely with our random number generation.  Just try
        // again.  The chance that this loop does not terminate in a reasonable
        // amount of time is astronomically low, assuming arc4random_uniform is not
        // broken.
    } while (1);

    // Not reached.
    return @(chip::kSetupPINCodeUndefinedValue);
}

+ (MTRSetupPayload * _Nullable)setupPayloadWithOnboardingPayload:(NSString *)onboardingPayload
                                                           error:(NSError * __autoreleasing *)error
{
    // TODO: Do we actually need the MTROnboardingPayloadParser abstraction?
    return [MTROnboardingPayloadParser setupPayloadForOnboardingPayload:onboardingPayload error:error];
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

- (nullable NSString *)manualEntryCode
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string outDecimalString;
    chip::SetupPayload payload;

    /// The 11 digit manual pairing code only requires the version, VID_PID present flag,
    /// discriminator, and the setup pincode.
    payload.version = [self.version unsignedCharValue];
    if (self.hasShortDiscriminator) {
        payload.discriminator.SetShortValue([self.discriminator unsignedCharValue]);
    } else {
        payload.discriminator.SetLongValue([self.discriminator unsignedShortValue]);
    }
    payload.setUpPINCode = [self.setUpPINCode unsignedIntValue];

    err = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(outDecimalString);

    if (err != CHIP_NO_ERROR) {
        return nil;
    }

    return [NSString stringWithUTF8String:outDecimalString.c_str()];
}

- (NSString * _Nullable)qrCodeString:(NSError * __autoreleasing *)error
{
    if (self.commissioningFlow == MTRCommissioningFlowInvalid) {
        // No idea how to map this to the standard codes.
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
        return nil;
    }

    if (self.hasShortDiscriminator) {
        // Can't create a QR code with a short discriminator.
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
        return nil;
    }

    if (self.rendezvousInformation == nil) {
        // Can't create a QR code if we don't know the discovery capabilities.
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
        }
        return nil;
    }

    chip::SetupPayload payload;

    payload.version = [self.version unsignedCharValue];
    payload.vendorID = [self.vendorID unsignedShortValue];
    payload.productID = [self.productID unsignedShortValue];
    payload.commissioningFlow = [MTRSetupPayload unconvertCommissioningFlow:self.commissioningFlow];
    payload.rendezvousInformation = [MTRSetupPayload unconvertRendezvousFlags:self.rendezvousInformation];
    payload.discriminator.SetLongValue([self.discriminator unsignedShortValue]);
    payload.setUpPINCode = [self.setUpPINCode unsignedIntValue];

    std::string outDecimalString;
    CHIP_ERROR err = chip::QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(outDecimalString);

    if (err != CHIP_NO_ERROR) {
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    return [NSString stringWithUTF8String:outDecimalString.c_str()];
}

@end
