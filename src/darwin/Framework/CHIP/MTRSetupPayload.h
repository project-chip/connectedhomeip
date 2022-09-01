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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MTRDiscoveryCapabilities) {
    MTRDiscoveryCapabilitiesNone = 0, // Device does not support any method for rendezvous
    MTRDiscoveryCapabilitiesSoftAP = 1 << 0, // Device supports WiFi softAP
    MTRDiscoveryCapabilitiesBLE = 1 << 1, // Device supports BLE
    MTRDiscoveryCapabilitiesOnNetwork = 1 << 2, // Device supports On Network setup

    MTRDiscoveryCapabilitiesAllMask
    = MTRDiscoveryCapabilitiesSoftAP | MTRDiscoveryCapabilitiesBLE | MTRDiscoveryCapabilitiesOnNetwork,
};

typedef NS_ENUM(NSUInteger, MTRCommissioningFlow) {
    MTRCommissioningFlowStandard = 0, // Device automatically enters pairing mode upon power-up
    MTRCommissioningFlowUserActionRequired = 1, // Device requires a user interaction to enter pairing mode
    MTRCommissioningFlowCustom = 2, // Commissioning steps should be retrieved from the distributed compliance ledger
    MTRCommissioningFlowInvalid = 3,
};

typedef NS_ENUM(NSUInteger, MTROptionalQRCodeInfoType) {
    MTROptionalQRCodeInfoTypeUnknown,
    MTROptionalQRCodeInfoTypeString,
    MTROptionalQRCodeInfoTypeInt32
};

@interface MTROptionalQRCodeInfo : NSObject
@property (nonatomic, copy) NSNumber * infoType;
@property (nonatomic, copy) NSNumber * tag;
@property (nonatomic, copy) NSNumber * integerValue;
@property (nonatomic, copy) NSString * stringValue;
@end

@interface MTRSetupPayload : NSObject <NSSecureCoding>

@property (nonatomic, copy) NSNumber * version;
@property (nonatomic, copy) NSNumber * vendorID;
@property (nonatomic, copy) NSNumber * productID;
@property (nonatomic, assign) MTRCommissioningFlow commissioningFlow;
/**
 * rendezvousInformation is nil when the discovery capabilities bitmask is
 * unknown.
 *
 * Otherwise its value is made up of the MTRDiscoveryCapabilities flags.
 */
@property (nonatomic, copy, nullable) NSNumber * rendezvousInformation;
@property (nonatomic, copy) NSNumber * discriminator;
@property (nonatomic, assign) BOOL hasShortDiscriminator;
@property (nonatomic, copy) NSNumber * setUpPINCode;

@property (nonatomic, copy) NSString * serialNumber;
- (nullable NSArray<MTROptionalQRCodeInfo *> *)getAllOptionalVendorData:(NSError * __autoreleasing *)error;

/**
 * Generate a random Matter-valid setup PIN.
 */
+ (NSUInteger)generateRandomPIN;

/** Get 11 digit manual entry code from the setup payload. */
- (nullable NSString *)manualEntryCode;

@end

NS_ASSUME_NONNULL_END
