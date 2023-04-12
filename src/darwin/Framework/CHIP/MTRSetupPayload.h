/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
    MTRDiscoveryCapabilitiesUnknown = 0, // Device capabilities are not known (e.g. all we have is a numeric code).
    MTRDiscoveryCapabilitiesNone MTR_DEPRECATED_WITH_REPLACEMENT(
        "MTRDiscoveryCapabilitiesUnknown", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0,
    MTRDiscoveryCapabilitiesSoftAP = 1 << 0, // Device supports WiFi softAP
    MTRDiscoveryCapabilitiesBLE = 1 << 1, // Device supports BLE
    MTRDiscoveryCapabilitiesOnNetwork = 1 << 2, // Device supports On Network setup

    MTRDiscoveryCapabilitiesAllMask
    = MTRDiscoveryCapabilitiesSoftAP | MTRDiscoveryCapabilitiesBLE | MTRDiscoveryCapabilitiesOnNetwork,
};

typedef NS_ENUM(NSUInteger, MTRCommissioningFlow) {
    MTRCommissioningFlowStandard = 0, // Device automatically enters commissioning mode upon power-up
    MTRCommissioningFlowUserActionRequired = 1, // Device requires a user interaction to enter commissioning mode
    MTRCommissioningFlowCustom = 2, // Commissioning steps should be retrieved from the distributed compliance ledger
    MTRCommissioningFlowInvalid = 3,
};

typedef NS_ENUM(NSUInteger, MTROptionalQRCodeInfoType) {
    MTROptionalQRCodeInfoTypeUnknown MTR_DEPRECATED(
        "The type is never actually unknown", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)),
    MTROptionalQRCodeInfoTypeString,
    MTROptionalQRCodeInfoTypeInt32,
};

/**
 * An optional information item present in the QR code the setup payload was
 * initialized from.
 */
@interface MTROptionalQRCodeInfo : NSObject

@property (nonatomic, assign) MTROptionalQRCodeInfoType type API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The numeric value of the TLV tag for this information item.
 */
@property (nonatomic, copy) NSNumber * tag;

/**
 * Exactly one of integerValue and stringValue will be non-nil, depending on the
 * the value of "type".
 */
@property (nonatomic, copy, nullable) NSNumber * integerValue;
@property (nonatomic, copy, nullable) NSString * stringValue;

@end

/**
 * A setup payload that can be created from a numeric code or QR code and
 * serialized to a numeric code or QR code, though serializing to QR code after
 * creating from numeric code will not work, because some required information
 * will be missing.
 */
@interface MTRSetupPayload : NSObject <NSSecureCoding>

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
    MTRDiscoveryCapabilities discoveryCapabilities API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));
@property (nonatomic, copy) NSNumber * discriminator;

/**
 * If hasShortDiscriminator is true, the discriminator value contains just the
 * high 4 bits of the full discriminator.  For example, if
 * hasShortDiscriminator is true and discriminator is 0xA, then the full
 * discriminator can be anything in the range 0xA00 t0 0xAFF.
 */
@property (nonatomic, assign) BOOL hasShortDiscriminator;
@property (nonatomic, copy) NSNumber * setupPasscode API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@property (nonatomic, copy, nullable) NSString * serialNumber;
- (NSArray<MTROptionalQRCodeInfo *> * _Nullable)getAllOptionalVendorData:(NSError * __autoreleasing *)error;

/**
 * Generate a random Matter-valid setup PIN.
 */
+ (NSUInteger)generateRandomPIN;

/**
 * Generate a random Matter-valid setup passcode.
 */
+ (NSNumber *)generateRandomSetupPasscode API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Create an MTRSetupPayload with the given onboarding payload.
 *
 * Will return nil on errors (e.g. if the onboarding payload cannot be parsed).
 */
+ (MTRSetupPayload * _Nullable)setupPayloadWithOnboardingPayload:(NSString *)onboardingPayload
                                                           error:(NSError * __autoreleasing *)error
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Initialize an MTRSetupPayload with the given passcode and discriminator.
 * This will pre-set version, product id, and vendor id to 0.
 */
- (instancetype)initWithSetupPasscode:(NSNumber *)setupPasscode
                        discriminator:(NSNumber *)discriminator API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/** Get 11 digit manual entry code from the setup payload. */
- (NSString * _Nullable)manualEntryCode;

/**
 * Get a QR code from the setup payload.
 *
 * Returns nil on failure (e.g. if the setup payload does not have all the
 * information a QR code needs).
 */
- (NSString * _Nullable)qrCodeString:(NSError * __autoreleasing *)error
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

@end

@interface MTROptionalQRCodeInfo (Deprecated)

@property (nonatomic, copy)
    NSNumber * infoType MTR_DEPRECATED("Please use type", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

@interface MTRSetupPayload (Deprecated)

@property (nonatomic, copy, nullable) NSNumber * rendezvousInformation MTR_DEPRECATED(
    "Please use discoveryCapabilities", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy) NSNumber * setUpPINCode MTR_DEPRECATED(
    "Please use setupPasscode", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (instancetype)init MTR_DEPRECATED("Please use initWithSetupPasscode or setupPayloadWithOnboardingPayload", ios(16.1, 16.4),
    macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
+ (instancetype)new MTR_DEPRECATED("Please use initWithSetupPasscode or setupPayloadWithOnboardingPayload", ios(16.1, 16.4),
    macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
