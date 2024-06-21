/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#import "MCTargetAppInfo.h"
#import <Foundation/Foundation.h>

#ifndef MCIdentificationDeclarationOptions_h
#define MCIdentificationDeclarationOptions_h

/**
 * This class contains the optional parameters used in the IdentificationDeclaration Message, sent
 * by the Commissionee (CastingApp) to the Commissioner (CastingPlayer). The options specify
 * information relating to the requested UDC commissioning session.
 */
@interface MCIdentificationDeclarationOptions : NSObject

/**
 * Feature: Target Content Application - Flag to instruct the Commissioner not to display a
 * Passcode input dialog, and instead send a CommissionerDeclaration message if a commissioning
 * Passcode is needed.
 */
@property (nonatomic, readonly) BOOL noPasscode;
/**
 * Feature: Coordinate Passcode Dialogs - Flag to instruct the Commissioner to send a
 * CommissionerDeclaration message when the Passcode input dialog on the Commissioner has been
 * shown to the user.
 */
@property (nonatomic, readonly) BOOL cdUponPasscodeDialog;
/**
 * Feature: Commissioner-Generated Passcode - Flag to instruct the Commissioner to use the
 * Commissioner-generated Passcode for commissioning.
 */
@property (nonatomic, readonly) BOOL commissionerPasscode;
/**
 * Feature: Commissioner-Generated Passcode - Flag to indicate whether or not the Commissionee has
 * obtained the Commissioner Passcode from the user and is therefore ready for commissioning.
 */
@property (nonatomic, readonly) BOOL commissionerPasscodeReady;
/**
 * Feature: Coordinate Passcode Dialogs Flag - to indicate when the Commissionee user has decided
 * to exit the commissioning process.
 */
@property (nonatomic, readonly) BOOL cancelPasscode;

- (instancetype)init;

- (instancetype)initWithCommissionerPasscodeOnly:(BOOL)commissionerPasscode;

/**
 * @brief Adds a TargetAppInfo to the IdentificationDeclarationOptions.java TargetAppInfos list,
 *     up to a maximum of CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS.
 */
- (BOOL)addTargetAppInfo:(MCTargetAppInfo *)targetAppInfo NS_SWIFT_NAME(addTargetAppInfo(_:));
/**
 * Feature: Target Content Application - The set of content app Vendor IDs (and optionally,
 * Product IDs) that can be used for authentication. Also, if TargetAppInfo is passed in,
 * VerifyOrEstablishConnection() will force User Directed Commissioning, in case the desired
 * TargetApp is not found in the on-device CastingStore.
 */
- (NSArray<MCTargetAppInfo *> *)getTargetAppInfoList;

- (NSString *)description;

@end

#endif /* MCIdentificationDeclarationOptions_h */
