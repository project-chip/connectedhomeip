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

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_OPTIONS(NSUInteger, MTRDiscoveryCapabilities) {
    MTRDiscoveryCapabilitiesUnknown = 0, // Device capabilities are not known (e.g. we parsed a Manual Pairing Code).
    MTRDiscoveryCapabilitiesNone MTR_DEPRECATED_WITH_REPLACEMENT(
        "MTRDiscoveryCapabilitiesUnknown", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0,
    MTRDiscoveryCapabilitiesSoftAP = 1 << 0, // Device supports WiFi softAP
    MTRDiscoveryCapabilitiesBLE = 1 << 1, // Device supports BLE
    MTRDiscoveryCapabilitiesOnNetwork = 1 << 2, // Device supports On Network setup

    // Note: New values added here need to be included in MTRDiscoveryCapabilitiesAsString()

    MTRDiscoveryCapabilitiesAllMask
    = MTRDiscoveryCapabilitiesSoftAP | MTRDiscoveryCapabilitiesBLE | MTRDiscoveryCapabilitiesOnNetwork,
};

typedef NS_ENUM(NSUInteger, MTRCommissioningFlow) {
    MTRCommissioningFlowStandard = 0, // Device automatically enters commissioning mode upon power-up
    MTRCommissioningFlowUserActionRequired = 1, // Device requires a user interaction to enter commissioning mode
    MTRCommissioningFlowCustom = 2, // Commissioning steps should be retrieved from the distributed compliance ledger
    MTRCommissioningFlowInvalid MTR_DEPRECATED("Not a valid MTRCommissioningFlow value", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6)) = 3,
} MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1));

typedef NS_ENUM(NSUInteger, MTROptionalQRCodeInfoType) {
    MTROptionalQRCodeInfoTypeUnknown MTR_DEPRECATED(
        "The type is never actually unknown", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)),
    MTROptionalQRCodeInfoTypeString,
    MTROptionalQRCodeInfoTypeInt32,
};

/**
 * An optional information item present in the setup payload.
 *
 * Note that while the Matter specification allows elements containing
 * arbitrary TLV data types, this implementation currently only supports
 * String and Int32 values.
 *
 * Objects of this type are immutable; calling any deprecated property
 * setters has no effect.
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTROptionalQRCodeInfo : NSObject /* <NSCopying> (see below) */

/**
 * Initializes the object with a tag and string value.
 * The tag must be in the range 0x80 - 0xFF.
 */
- (instancetype)initWithTag:(NSNumber *)tag stringValue:(NSString *)value MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Initializes the object with a tag and int32 value.
 * The tag must be in the range 0x80 - 0xFF.
 */
- (instancetype)initWithTag:(NSNumber *)tag int32Value:(int32_t)value MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@property (nonatomic, readonly, assign) MTROptionalQRCodeInfoType type MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The vendor-specific TLV tag number for this information item.
 *
 * Vendor-specific elements have tags in the range 0x80 - 0xFF.
 */
@property (nonatomic, readonly, copy) NSNumber * tag;

/**
 * The value held in this extension element,
 * if `type` is an integer type, or nil otherwise.
 */
@property (nonatomic, readonly, copy, nullable) NSNumber * integerValue;

/**
 * The value held in this extension element,
 * if `type` is `MTROptionalQRCodeInfoTypeString`, or nil otherwise.
 */
@property (nonatomic, readonly, copy, nullable) NSString * stringValue;

@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTROptionalQRCodeInfo () <NSCopying>
@end

/**
 * A Matter Onboarding Payload.
 *
 * It can be represented as a numeric Manual Pairing Code or as QR Code.
 * The QR Code format contains more information though, so creating a
 * QR Code from a payload that was initialized from a Manual Pairing Code
 * will not work, because some required information will be missing.
 *
 * This class can also be used to create an onboarding payload directly
 * from the underlying values (passcode, discriminator, etc).
 */
MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRSetupPayload : NSObject <NSSecureCoding> /* also <NSCopying> (see below) */

/**
 * Initializes the payload object from the provide QR Code or Manual Pairing Code string.
 * Returns nil if the payload is not valid.
 */
- (nullable instancetype)initWithPayload:(NSString *)payload MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@property (nonatomic, copy) NSNumber * version;
@property (nonatomic, copy) NSNumber * vendorID;
@property (nonatomic, copy) NSNumber * productID;
@property (nonatomic, assign) MTRCommissioningFlow commissioningFlow;

/**
 * The value of discoveryCapabilities is made up of the various
 * MTRDiscoveryCapabilities flags.  If the discovery capabilities are not known,
 * this will be set to MTRDiscoveryCapabilitiesUnknown.
 */
@property (nonatomic, assign)
    MTRDiscoveryCapabilities discoveryCapabilities MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy) NSNumber * discriminator;

/**
 * If hasShortDiscriminator is true, the discriminator value contains just the
 * high 4 bits of the full discriminator.  For example, if
 * hasShortDiscriminator is true and discriminator is 0xA, then the full
 * discriminator can be anything in the range 0xA00 to 0xAFF.
 */
@property (nonatomic, assign) BOOL hasShortDiscriminator;

@property (nonatomic, copy) NSNumber * setupPasscode MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The value of the Serial Number extension element, if any.
 */
@property (nonatomic, copy, nullable) NSString * serialNumber;

/**
 * The list of Manufacturer-specific extension elements contained in the setup code. May be empty.
 */
@property (nonatomic, readonly, copy) NSArray<MTROptionalQRCodeInfo *> * vendorElements MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Returns the Manufacturer-specific extension element with the specified tag, if any.
 * The tag must be in the range 0x80 - 0xFF.
 */
- (nullable MTROptionalQRCodeInfo *)vendorElementWithTag:(NSNumber *)tag MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Removes the extension element with the specified tag, if any.
 * The tag must be in the range 0x80 - 0xFF.
 */
- (void)removeVendorElementWithTag:(NSNumber *)tag MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Adds or replaces a Manufacturer-specific extension element.
 */
- (void)addOrReplaceVendorElement:(MTROptionalQRCodeInfo *)element MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Generate a random Matter-valid setup PIN.
 */
+ (NSUInteger)generateRandomPIN;

/**
 * Generate a random Matter-valid setup passcode.
 */
+ (NSNumber *)generateRandomSetupPasscode MTR_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Initialize an MTRSetupPayload with the given passcode and discriminator.
 * This will pre-set version, product id, and vendor id to 0.
 */
- (instancetype)initWithSetupPasscode:(NSNumber *)setupPasscode
                        discriminator:(NSNumber *)discriminator MTR_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Creates a Manual Pairing Code from this setup payload.
 * Returns nil if this payload cannot be represented as a valid Manual Pairing Code.
 *
 * The following properties must be populated for a valid Manual Pairing Code:
 *  - setupPasscode
 *  - discriminator (short or long)
 *
 * In most cases the pairing code will be 11 digits long. If the payload indicates
 * a `commissioningFlow` other than `MTRCommissioningFlowStandard`, a 21 digit code
 * will be produced that includes the vendorID and productID values.
 */
- (nullable NSString *)manualEntryCode;

/**
 * Creates a QR Code payload from this setup payload.
 * Returns nil if this payload cannot be represented as a valid QR Code.
 *
 * The following properties must be populated for a valid QR Code:
 * - setupPasscode
 * - discriminator (must be long)
 * - discoveryCapabilities (not MTRDiscoveryCapabilitiesUnknown)
 */
- (NSString * _Nullable)qrCodeString MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

@end

MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRSetupPayload () <NSCopying>
@end

@interface MTROptionalQRCodeInfo (Deprecated)

- (instancetype)init MTR_DEPRECATED("Please use -initWithTag:...value:", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));

@property (nonatomic, copy) NSNumber * infoType
    MTR_DEPRECATED("Please use type", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)setType:(MTROptionalQRCodeInfoType)type MTR_DEPRECATED("MTROptionalQRCodeInfo is immutable", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));
- (void)setTag:(NSNumber *)tag MTR_DEPRECATED("MTROptionalQRCodeInfo is immutable", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));
- (void)setIntegerValue:(NSNumber *)integerValue MTR_DEPRECATED("MTROptionalQRCodeInfo is immutable", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));
- (void)setStringValue:(NSString *)stringValue MTR_DEPRECATED("MTROptionalQRCodeInfo is immutable", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));
@end

@interface MTRSetupPayload (Deprecated)

@property (nonatomic, copy, nullable) NSNumber * rendezvousInformation MTR_DEPRECATED(
    "Please use discoveryCapabilities", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@property (nonatomic, copy) NSNumber * setUpPINCode MTR_DEPRECATED(
    "Please use setupPasscode", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (instancetype)init MTR_DEPRECATED("Please use -initWithSetupPasscode:discriminator: or -initWithPayload:", ios(16.1, 16.4),
    macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

+ (instancetype)new MTR_DEPRECATED("Please use -initWithSetupPasscode:discriminator: or -initWithPayload:", ios(16.1, 16.4),
    macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

+ (MTRSetupPayload * _Nullable)setupPayloadWithOnboardingPayload:(NSString *)onboardingPayload
                                                           error:(NSError * __autoreleasing *)error
    MTR_DEPRECATED("Please use -initWithPayload:", ios(16.2, 17.6), macos(13.1, 14.6), watchos(9.2, 10.6), tvos(16.2, 17.6));

- (NSString * _Nullable)qrCodeString:(NSError * __autoreleasing *)error
    MTR_DEPRECATED("Please use -qrCodeString", ios(16.2, 17.6), macos(13.1, 14.6), watchos(9.2, 10.6), tvos(16.2, 17.6));

- (NSArray<MTROptionalQRCodeInfo *> * _Nullable)getAllOptionalVendorData:(NSError * __autoreleasing *)error
    MTR_DEPRECATED("Please use -vendorElements", ios(16.1, 17.6), macos(13.0, 14.6), watchos(9.1, 10.6), tvos(16.1, 17.6));

@end

NS_ASSUME_NONNULL_END
