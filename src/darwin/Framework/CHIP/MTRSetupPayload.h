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

typedef NS_ENUM(NSUInteger, MTRRendezvousInformationFlags) {
    MTRRendezvousInformationNone = 0, // Device does not support any method for rendezvous
    MTRRendezvousInformationSoftAP = 1 << 0, // Device supports WiFi softAP
    MTRRendezvousInformationBLE = 1 << 1, // Device supports BLE
    MTRRendezvousInformationOnNetwork = 1 << 2, // Device supports On Network setup

    MTRRendezvousInformationAllMask
    = MTRRendezvousInformationSoftAP | MTRRendezvousInformationBLE | MTRRendezvousInformationOnNetwork,
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
@property (nonatomic, strong) NSNumber * infoType;
@property (nonatomic, strong) NSNumber * tag;
@property (nonatomic, strong) NSNumber * integerValue;
@property (nonatomic, strong) NSString * stringValue;
@end

@interface MTRSetupPayload : NSObject

@property (nonatomic, strong) NSNumber * version;
@property (nonatomic, strong) NSNumber * vendorID;
@property (nonatomic, strong) NSNumber * productID;
@property (nonatomic, assign) MTRCommissioningFlow commissioningFlow;
@property (nonatomic, assign) MTRRendezvousInformationFlags rendezvousInformation;
@property (nonatomic, strong) NSNumber * discriminator;
@property (nonatomic, strong) NSNumber * setUpPINCode;

@property (nonatomic, strong) NSString * serialNumber;
- (nullable NSArray<MTROptionalQRCodeInfo *> *)getAllOptionalVendorData:(NSError * __autoreleasing *)error;

@end

NS_ASSUME_NONNULL_END
