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

#import "MTRSetupPayload_Internal.h"

#import "MTRError_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTRUtilities.h"

#include <lib/support/SafeInt.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>
#include <string>

MTR_DIRECT_MEMBERS
@implementation MTROptionalQRCodeInfo {
    chip::OptionalQRCodeInfo _info;
}

static uint8_t ValidateVendorTag(NSNumber * tag)
{
    auto integerValue = tag.integerValue;
    auto tagValue = static_cast<uint8_t>(integerValue);
    MTRVerifyArgumentOrDie(tag != nil && chip::CanCastTo<uint8_t>(integerValue) && chip::SetupPayload::IsVendorTag(tagValue), @"tag must be a vendor tag (0x80 - 0xFF)");
    return tagValue;
}

- (instancetype)initWithTag:(NSNumber *)tag int32Value:(int32_t)value
{
    self = [super init];
    _info.type = chip::optionalQRCodeInfoTypeInt32;
    _info.tag = ValidateVendorTag(tag);
    _info.int32 = value;
    return self;
}

- (instancetype)initWithTag:(NSNumber *)tag stringValue:(NSString *)value
{
    self = [super init];
    _info.type = chip::optionalQRCodeInfoTypeString;
    _info.tag = ValidateVendorTag(tag);
    MTRVerifyArgumentOrDie(value != nil, @"value");
    _info.data = value.UTF8String;
    return self;
}

- (nullable instancetype)initWithQRCodeInfo:(chip::OptionalQRCodeInfo const &)info
{
    self = [super init];
    _info = info;
    // Don't expose objects with an out-of-range tag or invalid type
    VerifyOrReturnValue(chip::SetupPayload::IsVendorTag(_info.tag), nil);
    VerifyOrReturnValue(self.type != MTROptionalQRCodeInfoTypeUnknown, nil);
    return self;
}

- (CHIP_ERROR)addAsVendorElementTo:(chip::SetupPayload &)payload
{
    switch (_info.type) {
    case chip::optionalQRCodeInfoTypeString:
        return payload.addOptionalVendorData(_info.tag, _info.data);
    case chip::optionalQRCodeInfoTypeInt32:
        return payload.addOptionalVendorData(_info.tag, _info.int32);
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

- (MTROptionalQRCodeInfoType)type
{
    // The way OptionalQRCodeInfo uses what are effectively C type identifiers (uint32 etc)
    // rather than TLV types is not ideal. If we add support for additional integer types
    // we should consider replacing MTROptionalQRCodeInfoTypeInt32 with
    // MTROptionalQRCodeInfoTypeInteger and hiding the low-level C representation.
    switch (_info.type) {
    case chip::optionalQRCodeInfoTypeString:
        return MTROptionalQRCodeInfoTypeString;
    case chip::optionalQRCodeInfoTypeInt32:
        return MTROptionalQRCodeInfoTypeInt32;
    // No 'default:' so we get a warning if new types are added.
    // Note: isEqual: also switches over these types.
    // OptionalQRCodeInfo does not support these types
    case chip::optionalQRCodeInfoTypeInt64:
    case chip::optionalQRCodeInfoTypeUInt32:
    case chip::optionalQRCodeInfoTypeUInt64:
    // We should never see the unknown type
    case chip::optionalQRCodeInfoTypeUnknown:
        /* fall through */;
    }
    return MTROptionalQRCodeInfoTypeUnknown;
}

- (NSNumber *)tag
{
    return @(_info.tag);
}

- (NSNumber *)integerValue
{
    VerifyOrReturnValue(_info.type == chip::optionalQRCodeInfoTypeInt32, nil);
    return @(_info.int32);
}

- (NSString *)stringValue
{
    VerifyOrReturnValue(_info.type == chip::optionalQRCodeInfoTypeString, nil);
    return [NSString stringWithUTF8String:_info.data.c_str()];
}

- (id)copyWithZone:(NSZone *)zone
{
    return self; // immutable
}

- (NSUInteger)hash
{
    return _info.type << 8 | _info.tag;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTROptionalQRCodeInfo * other = object;
    VerifyOrReturnValue(_info.tag == other->_info.tag, NO);
    VerifyOrReturnValue(_info.type == other->_info.type, NO);
    switch (_info.type) {
    case chip::optionalQRCodeInfoTypeString:
        return _info.data == other->_info.data;
    case chip::optionalQRCodeInfoTypeInt32:
        return _info.int32 == other->_info.int32;
    default:
        return NO; // unreachable, type is checked in init
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"0x%02x=%@", self.tag.unsignedCharValue, self.integerValue ?: self.stringValue];
}

@end

static constexpr uint8_t kShortDiscriminatorMask = (1 << chip::SetupDiscriminator::kShortBits) - 1;
static constexpr uint16_t kLongDiscriminatorMask = (1 << chip::SetupDiscriminator::kLongBits) - 1;

static MTRDiscoveryCapabilities KnownDiscoveryCapabilities(NSUInteger value)
{
    // A known but 0 value cannot be represented by MTRDiscoveryCapabilities
    // since MTRDiscoveryCapabilitiesUnknown represents an absent value.
    // However such a 0 value is not logically valid, because OnNetwork must
    // always be supported. So just return that if we get a value of 0.
    return (value != MTRDiscoveryCapabilitiesUnknown ? value : MTRDiscoveryCapabilitiesOnNetwork);
}

static NSString * MTRDiscoveryCapabilitiesAsString(MTRDiscoveryCapabilities value)
{
    VerifyOrReturnValue(value != MTRDiscoveryCapabilitiesUnknown, @"Unknown");
    NSMutableString * capabilities = [NSMutableString string];
    if (value & MTRDiscoveryCapabilitiesSoftAP) {
        [capabilities appendString:@"|SoftAP"];
        value &= ~MTRDiscoveryCapabilitiesSoftAP;
    }
    if (value & MTRDiscoveryCapabilitiesBLE) {
        [capabilities appendString:@"|BLE"];
        value &= ~MTRDiscoveryCapabilitiesBLE;
    }
    if (value & MTRDiscoveryCapabilitiesOnNetwork) {
        [capabilities appendString:@"|OnNetwork"];
        value &= ~MTRDiscoveryCapabilitiesOnNetwork;
    }
    if (value != 0) {
        [capabilities appendFormat:@"|0x%llx", (unsigned long long) value];
    }
    return [capabilities substringFromIndex:1];
}

MTR_DIRECT_MEMBERS
@implementation MTRSetupPayload {
    // Apart from the additional logic to handle discriminators detailed below,
    // this class is simply a wrapper around this underlying SetupPayload.
    chip::SetupPayload _payload;

    // SetupPayload deals with the discriminator value and its shortness as a composite
    // SetupDiscriminator value, which is arguably a tidier API than letting clients
    // set a value first and only later tell us whether it's long or short. But that is
    // what our API does, so we need to continue to support it. To make this work, we
    // keep track of the potentially-long value that was set by the client.
    NSNumber * _Nullable _shadowDiscriminator;
}

+ (void)initialize
{
    // Some aspects of working with chip::SetupPayload use Platform memory primitives.
    MTRFrameworkInit();
}

- (nullable instancetype)initWithPayload:(NSString *)payload
{
    return ([payload hasPrefix:@"MT:"]) ? [self initWithQRCode:payload] : [self initWithManualPairingCode:payload];
}

- (CHIP_ERROR)initializeFromQRCode:(NSString *)qrCode
{
    std::string string([(qrCode ?: @"") UTF8String]); // handle nil gracefully
    chip::QRCodeSetupPayloadParser parser(string);
    return parser.populatePayload(_payload);
}

- (nullable instancetype)initWithQRCode:(NSString *)qrCodePayload
{
    self = [super init];
    CHIP_ERROR err = [self initializeFromQRCode:qrCodePayload];
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to parse QR Code payload: %" CHIP_ERROR_FORMAT, err.Format());
        return nil;
    }
    if (!_payload.isValidQRCodePayload(chip::PayloadContents::ValidationMode::kConsume)) {
        MTR_LOG_ERROR("Invalid QR Code payload");
        return nil;
    }

    return self;
}

- (nullable instancetype)initWithManualPairingCode:(NSString *)manualCode
{
    self = [super init];

    std::string string([(manualCode ?: @"") UTF8String]); // handle nil gracefully
    chip::ManualSetupPayloadParser parser(string);
    CHIP_ERROR err = parser.populatePayload(_payload);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to parse Manual Pairing Code: %" CHIP_ERROR_FORMAT, err.Format());
        return nil;
    }
    if (!_payload.isValidManualCode(chip::PayloadContents::ValidationMode::kConsume)) {
        MTR_LOG_ERROR("Invalid Manual Pairing Code");
        return nil;
    }

    return self;
}

- (instancetype)initWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator
{
    self = [super init];

    // A default-constructed SetupPayload already has most of the expected
    // default values. Set discoveryCapabilities to "all known methods" so
    // we're representing a valid QR Code payload.
    self.setupPasscode = setupPasscode;
    self.discriminator = discriminator;
    self.discoveryCapabilities = MTRDiscoveryCapabilitiesAllMask;

    return self;
}

- (instancetype)initWithSetupPayload:(chip::SetupPayload)setupPayload
{
    self = [super init];
    _payload = setupPayload;
    return self;
}

#pragma mark - Mutable properties

- (NSNumber *)version
{
    return @(_payload.version);
}

- (void)setVersion:(NSNumber *)version
{
    _payload.version = static_cast<decltype(_payload.version)>(version.unsignedIntegerValue);
}

- (NSNumber *)vendorID
{
    return @(_payload.vendorID);
}

- (void)setVendorID:(NSNumber *)vendorID
{
    _payload.vendorID = static_cast<decltype(_payload.vendorID)>(vendorID.unsignedIntegerValue);
}

- (NSNumber *)productID
{
    return @(_payload.productID);
}

- (void)setProductID:(NSNumber *)productID
{
    _payload.productID = static_cast<decltype(_payload.productID)>(productID.unsignedIntegerValue);
}

- (MTRCommissioningFlow)commissioningFlow
{
    // To avoid ending up with special logic to handle MTRCommissioningFlowInvalid,
    // we simply cast between MTRCommissioningFlow and chip::CommissioningFlow.
    // Both types represent the same set of values defined in the Matter spec.
    using chip::CommissioningFlow;
    static_assert(static_cast<CommissioningFlow>(MTRCommissioningFlowStandard) == CommissioningFlow::kStandard);
    static_assert(static_cast<CommissioningFlow>(MTRCommissioningFlowUserActionRequired) == CommissioningFlow::kUserActionRequired);
    static_assert(static_cast<CommissioningFlow>(MTRCommissioningFlowCustom) == CommissioningFlow::kCustom);
    return static_cast<MTRCommissioningFlow>(_payload.commissioningFlow);
}

- (void)setCommissioningFlow:(MTRCommissioningFlow)commissioningFlow
{
    _payload.commissioningFlow = static_cast<chip::CommissioningFlow>(commissioningFlow);
}

- (MTRDiscoveryCapabilities)discoveryCapabilities
{
    VerifyOrReturnValue(_payload.rendezvousInformation.HasValue(), MTRDiscoveryCapabilitiesUnknown);

    // MTRDiscoveryCapabilities and chip::RendezvousInformationFlag represent
    // the same set of bit flags from the Matter spec, so we can simply cast.
    using RendezvousFlag = chip::RendezvousInformationFlag;
    static_assert(static_cast<RendezvousFlag>(MTRDiscoveryCapabilitiesSoftAP) == RendezvousFlag::kSoftAP);
    static_assert(static_cast<RendezvousFlag>(MTRDiscoveryCapabilitiesBLE) == RendezvousFlag::kBLE);
    static_assert(static_cast<RendezvousFlag>(MTRDiscoveryCapabilitiesOnNetwork) == RendezvousFlag::kOnNetwork);
    auto value = static_cast<MTRDiscoveryCapabilities>(_payload.rendezvousInformation.Value().Raw());

    // Ensure a known (HasValue()) but 0 value does not map to MTRDiscoveryCapabilitiesUnknown.
    return KnownDiscoveryCapabilities(value);
}

- (void)setDiscoveryCapabilities:(MTRDiscoveryCapabilities)discoveryCapabilities
{
    if (discoveryCapabilities == MTRDiscoveryCapabilitiesUnknown) {
        _payload.rendezvousInformation.ClearValue();
    } else {
        auto flags = static_cast<chip::RendezvousInformationFlags::IntegerType>(discoveryCapabilities);
        _payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlags(flags));
    }
}

- (NSNumber *)discriminator
{
    VerifyOrReturnValue(_shadowDiscriminator == nil, _shadowDiscriminator);
    auto & pd = _payload.discriminator;
    return @(pd.IsShortDiscriminator() ? pd.GetShortValue() : pd.GetLongValue());
}

- (void)setDiscriminator:(uint16_t)discriminator isShort:(BOOL)isShort
{
    if (isShort) {
        _payload.discriminator.SetShortValue(discriminator & kShortDiscriminatorMask);
        _shadowDiscriminator = @(discriminator); // keep as a shadow value
    } else {
        _payload.discriminator.SetLongValue(discriminator);
        _shadowDiscriminator = nil; // no need to keep a shadow copy
    }
}

- (void)setDiscriminator:(NSNumber *)discriminator
{
    // Truncate down to the range of a long discriminator. Then update
    // our SetupPayload with the value, but keep the current shortness.
    uint16_t value = discriminator.unsignedIntegerValue & kLongDiscriminatorMask;
    [self setDiscriminator:value isShort:self.hasShortDiscriminator];
}

- (BOOL)hasShortDiscriminator
{
    return _payload.discriminator.IsShortDiscriminator();
}

- (void)setHasShortDiscriminator:(BOOL)hasShortDiscriminator
{
    VerifyOrReturn(hasShortDiscriminator != self.hasShortDiscriminator);
    [self setDiscriminator:self.discriminator.unsignedShortValue isShort:hasShortDiscriminator];
}

- (NSNumber *)setupPasscode
{
    return @(_payload.setUpPINCode);
}

- (void)setSetupPasscode:(NSNumber *)setupPasscode
{
    _payload.setUpPINCode = static_cast<decltype(_payload.setUpPINCode)>(setupPasscode.unsignedIntegerValue);
}

- (nullable NSString *)serialNumber
{
    std::string value;
    VerifyOrReturnValue(_payload.getSerialNumber(value) == CHIP_NO_ERROR, nil);
    return [NSString stringWithUTF8String:value.c_str()];
}

- (void)setSerialNumber:(nullable NSString *)serialNumber
{
    if (serialNumber) {
        NSString * existing = self.serialNumber;
        if (existing) {
            // The underlying TLV tag can be encoded as either a string or an integer,
            // avoid changing it if the represented serial number is not changing.
            VerifyOrReturn(![existing isEqualToString:serialNumber]);
            _payload.removeSerialNumber();
        }
        CHIP_ERROR err = _payload.addSerialNumber(serialNumber.UTF8String);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Ignoring unexpected error in SetupPayload::addSerialNumber: %" CHIP_ERROR_FORMAT, err.Format());
        }
    } else {
        _payload.removeSerialNumber();
    }
}

- (NSArray<MTROptionalQRCodeInfo *> *)vendorElements
{
    std::vector<chip::OptionalQRCodeInfo> elements = _payload.getAllOptionalVendorData();
    VerifyOrReturnValue(!elements.empty(), @[]);
    NSMutableArray<MTROptionalQRCodeInfo *> * infos = [[NSMutableArray alloc] initWithCapacity:elements.size()];
    for (auto const & element : elements) {
        MTROptionalQRCodeInfo * info = [[MTROptionalQRCodeInfo alloc] initWithQRCodeInfo:element];
        if (info != nil) { // ignore invalid elements (there shouldn't be any)
            [infos addObject:info];
        }
    }
    return infos;
}

- (MTROptionalQRCodeInfo *)vendorElementWithTag:(NSNumber *)tag
{
    chip::OptionalQRCodeInfo element;
    VerifyOrReturnValue(_payload.getOptionalVendorData(ValidateVendorTag(tag), element) == CHIP_NO_ERROR, nil);
    return [[MTROptionalQRCodeInfo alloc] initWithQRCodeInfo:element];
}

- (void)removeVendorElementWithTag:(NSNumber *)tag
{
    _payload.removeOptionalVendorData(ValidateVendorTag(tag));
}

- (void)addOrReplaceVendorElement:(MTROptionalQRCodeInfo *)element
{
    MTRVerifyArgumentOrDie(element != nil, @"element");
    CHIP_ERROR err = [element addAsVendorElementTo:_payload];
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, NotSpecified, "Internal error: %" CHIP_ERROR_FORMAT, err.Format());
}

#pragma mark - Export methods

- (nullable NSString *)manualEntryCode
{
    chip::ManualSetupPayloadGenerator generator(_payload);
    std::string result;
    VerifyOrReturnValue(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR, nil);
    return [NSString stringWithUTF8String:result.c_str()];
}

- (nullable NSString *)qrCodeString
{
    return [self qrCodeStringSkippingValidation:NO];
}

- (nullable NSString *)qrCodeStringSkippingValidation:(BOOL)allowInvalid
{
    chip::QRCodeSetupPayloadGenerator generator(_payload);
    generator.SetAllowInvalidPayload(allowInvalid);
    std::string result;
    CHIP_ERROR err = generator.payloadBase38RepresentationWithAutoTLVBuffer(result);
    if (allowInvalid) {
        // Encoding should always work if invalid payloads are allowed
        VerifyOrDieWithMsg(err == CHIP_NO_ERROR, NotSpecified, "Internal error: %" CHIP_ERROR_FORMAT, err.Format());
    } else {
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nil);
    }
    return [NSString stringWithUTF8String:result.c_str()];
}

#pragma mark - Miscellaneous

- (id)copyWithZone:(NSZone *)zone
{
    MTRSetupPayload * copy = [[MTRSetupPayload alloc] initWithSetupPayload:_payload];
    copy->_shadowDiscriminator = _shadowDiscriminator;
    return copy;
}

- (NSUInteger)hash
{
    return self.discriminator.unsignedIntegerValue;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRSetupPayload * other = object;
    VerifyOrReturnValue(_payload == other->_payload, NO);
    VerifyOrReturnValue(MTREqualObjects(_shadowDiscriminator, other->_shadowDiscriminator), NO);
    return YES;
}

- (NSString *)description
{
    // Note: The description does not include the passcode for security reasons!

    NSMutableString * result = [NSMutableString stringWithFormat:@"<MTRSetupPayload: discriminator=0x%0*x",
                                                (self.hasShortDiscriminator ? 1 : 3), self.discriminator.unsignedIntValue];

    auto capabilities = self.discoveryCapabilities;
    if (capabilities != MTRDiscoveryCapabilitiesUnknown) {
        [result appendFormat:@" discoveryCapabilities=%@", MTRDiscoveryCapabilitiesAsString(capabilities)];
    }

    auto flow = self.commissioningFlow;
    if (flow != MTRCommissioningFlowStandard) {
        [result appendFormat:@" commissioningFlow=0x%llx", (unsigned long long) flow];
    }

    [result appendString:@">"];
    return result;
}

#pragma mark - NSSecureCoding

static NSString * const MTRSetupPayloadCodingKeyVersion = @"MTRSP.ck.version";
static NSString * const MTRSetupPayloadCodingKeyVendorID = @"MTRSP.ck.vendorID";
static NSString * const MTRSetupPayloadCodingKeyProductID = @"MTRSP.ck.productID";
static NSString * const MTRSetupPayloadCodingKeyCommissioningFlow = @"MTRSP.ck.commissioningFlow";
static NSString * const MTRSetupPayloadCodingKeyRendevouzInformation = @"MTRSP.ck.rendezvousFlags";
static NSString * const MTRSetupPayloadCodingKeyHasShortDiscriminator = @"MTRSP.ck.hasShortDiscriminator";
static NSString * const MTRSetupPayloadCodingKeyDiscriminator = @"MTRSP.ck.discriminator";
static NSString * const MTRSetupPayloadCodingKeySetupPasscode = @"MTRSP.ck.setupPINCode";
static NSString * const MTRSetupPayloadCodingKeySerialNumber = @"MTRSP.ck.serialNumber";
static NSString * const MTRSetupPayloadCodingKeyQRCode = @"qr";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:[self qrCodeStringSkippingValidation:YES] forKey:MTRSetupPayloadCodingKeyQRCode];
    [coder encodeObject:self.version forKey:MTRSetupPayloadCodingKeyVersion];
    [coder encodeObject:self.vendorID forKey:MTRSetupPayloadCodingKeyVendorID];
    [coder encodeObject:self.productID forKey:MTRSetupPayloadCodingKeyProductID];
    [coder encodeInteger:self.commissioningFlow forKey:MTRSetupPayloadCodingKeyCommissioningFlow];
    // For compatibility reasons, keep encoding rendevouzInformation instead of discoveryCapabilities
    [coder encodeObject:self.rendezvousInformation forKey:MTRSetupPayloadCodingKeyRendevouzInformation];
    [coder encodeInteger:(self.hasShortDiscriminator ? 1 : 0) forKey:MTRSetupPayloadCodingKeyHasShortDiscriminator];
    [coder encodeObject:self.discriminator forKey:MTRSetupPayloadCodingKeyDiscriminator];
    [coder encodeObject:self.setupPasscode forKey:MTRSetupPayloadCodingKeySetupPasscode];
    [coder encodeObject:self.serialNumber forKey:MTRSetupPayloadCodingKeySerialNumber];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];

    // We can't rely on the QR code to be present because older versions of this class
    // did not encode it. When present, it carries almost the entire state of the object.
    NSString * qrCode = [coder decodeObjectOfClass:NSString.class forKey:MTRSetupPayloadCodingKeyQRCode];
    if (qrCode != nil) {
        [self initializeFromQRCode:qrCode];
    } else {
        self.version = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeyVersion];
        self.vendorID = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeyVendorID];
        self.productID = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeyProductID];
        self.commissioningFlow = static_cast<MTRCommissioningFlow>([coder decodeIntegerForKey:MTRSetupPayloadCodingKeyCommissioningFlow]);
        self.setupPasscode = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeySetupPasscode];
        self.serialNumber = [coder decodeObjectOfClass:NSString.class forKey:MTRSetupPayloadCodingKeySerialNumber];
    }

    // The QR code cannot represent short discriminators or the absence of rendevouz
    //  information, so always decode the state of those properties separately.
    self.hasShortDiscriminator = ([coder decodeIntegerForKey:MTRSetupPayloadCodingKeyHasShortDiscriminator] != 0);
    self.discriminator = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeyDiscriminator];
    // For compatibility reasons, keep decoding rendevouzInformation instead of discoveryCapabilities
    self.rendezvousInformation = [coder decodeObjectOfClass:NSNumber.class forKey:MTRSetupPayloadCodingKeyRendevouzInformation];

    return self;
}

#pragma mark - Utility class methods

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
    } while (true);
}

+ (NSNumber *)generateRandomSetupPasscode
{
    return @([self generateRandomPIN]);
}

@end

MTR_DIRECT_MEMBERS
@implementation MTROptionalQRCodeInfo (Deprecated)

- (instancetype)init
{
    return [self initWithTag:@0xff stringValue:@""];
}

- (void)setType:(MTROptionalQRCodeInfoType)type
{
    /* ignored */
}

- (void)setTag:(NSNumber *)tag
{
    /* ignored */
}

- (NSNumber *)infoType
{
    return @(self.type);
}

- (void)setInfoType:(NSNumber *)infoType
{
    /* ignored */
}

- (void)setIntegerValue:(NSNumber *)integerValue
{
    /* ignored */
}

- (void)setStringValue:(NSString *)stringValue
{
    /* ignored */
}

@end

MTR_DIRECT_MEMBERS
@implementation MTRSetupPayload (Deprecated)

- (instancetype)init
{
    return [super init]; // a default-constructed SetupPayload is fine here
}

+ (instancetype)new
{
    return [super new];
}

+ (MTRSetupPayload * _Nullable)setupPayloadWithOnboardingPayload:(NSString *)onboardingPayload
                                                           error:(NSError * __autoreleasing *)error
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithPayload:onboardingPayload];
    if (!payload && error) {
        *error = [MTRError errorWithCode:MTRErrorCodeInvalidArgument];
    }
    return payload;
}

- (nullable NSNumber *)rendezvousInformation
{
    auto value = self.discoveryCapabilities;
    return (value != MTRDiscoveryCapabilitiesUnknown) ? @(value) : nil;
}

- (void)setRendezvousInformation:(nullable NSNumber *)rendezvousInformation
{
    if (rendezvousInformation != nil) {
        self.discoveryCapabilities = KnownDiscoveryCapabilities(rendezvousInformation.unsignedIntegerValue);
    } else {
        self.discoveryCapabilities = MTRDiscoveryCapabilitiesUnknown;
    }
}

- (NSNumber *)setUpPINCode
{
    return self.setupPasscode;
}

- (void)setSetUpPINCode:(NSNumber *)setUpPINCode
{
    self.setupPasscode = setUpPINCode;
}

- (nullable NSString *)qrCodeString:(NSError * __autoreleasing _Nullable *)error
{
    NSString * result = [self qrCodeString];
    if (!result && error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }
    return result;
}

- (NSArray<MTROptionalQRCodeInfo *> *)getAllOptionalVendorData:(NSError * __autoreleasing *)error
{
    return self.vendorElements; // never actually fails
}

@end
