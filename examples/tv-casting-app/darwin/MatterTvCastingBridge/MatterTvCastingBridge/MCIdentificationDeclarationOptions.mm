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
#import "MCIdentificationDeclarationOptions.h"

#import "core/IdentificationDeclarationOptions.h" // from tv-casting-common
#import "core/Types.h" // from tv-casting-common
#import "include/CHIPProjectAppConfig.h" // from tv-casting-common

@interface MCIdentificationDeclarationOptions ()

// Private properties:

/**
 * Feature: Target Content Application - Flag to instruct the Commissioner not to display a
 * Passcode input dialog, and instead send a CommissionerDeclaration message if a commissioning
 * Passcode is needed.
 */
@property (nonatomic) BOOL noPasscode;
/**
 * Feature: Coordinate Passcode Dialogs - Flag to instruct the Commissioner to send a
 * CommissionerDeclaration message when the Passcode input dialog on the Commissioner has been
 * shown to the user.
 */
@property (nonatomic) BOOL cdUponPasscodeDialog;
/**
 * Feature: Commissioner-Generated Passcode - Flag to instruct the Commissioner to use the
 * Commissioner-generated Passcode for commissioning.
 */
@property (nonatomic) BOOL commissionerPasscode;
/**
 * Feature: Commissioner-Generated Passcode - Flag to indicate whether or not the Commissionee has
 * obtained the Commissioner Passcode from the user and is therefore ready for commissioning.
 */
@property (nonatomic) BOOL commissionerPasscodeReady;
/**
 * Feature: Coordinate Passcode Dialogs Flag - to indicate when the Commissionee user has decided
 * to exit the commissioning process.
 */
@property (nonatomic) BOOL cancelPasscode;
/**
 * Feature: Target Content Application - The set of content app Vendor IDs (and optionally,
 * Product IDs) that can be used for authentication. Also, if TargetAppInfo is passed in,
 * VerifyOrEstablishConnection() will force User Directed Commissioning, in case the desired
 * TargetApp is not found in the on-device CastingStore.
 */
@property (nonatomic, strong) NSMutableArray<MCTargetAppInfo *> * targetAppInfos;

@end

@implementation MCIdentificationDeclarationOptions

- (instancetype)init
{
    self = [super init];
    if (self) {
        _noPasscode = NO;
        _cdUponPasscodeDialog = NO;
        _commissionerPasscode = NO;
        _commissionerPasscodeReady = NO;
        _cancelPasscode = NO;
        _targetAppInfos = [[NSMutableArray alloc] init];
    }
    return self;
}

- (instancetype)initWithCommissionerPasscodeOnly:(BOOL)commissionerPasscode
{
    self = [super init];
    if (self) {
        _noPasscode = NO;
        _cdUponPasscodeDialog = NO;
        _commissionerPasscode = commissionerPasscode;
        _commissionerPasscodeReady = NO;
        _cancelPasscode = NO;
        _targetAppInfos = [[NSMutableArray alloc] init];
    }
    return self;
}

// Getter methods
- (BOOL)getNoPasscode
{
    return _noPasscode;
}

- (BOOL)getCdUponPasscodeDialog
{
    return _cdUponPasscodeDialog;
}

- (BOOL)getCommissionerPasscode
{
    return _commissionerPasscode;
}

- (BOOL)getCommissionerPasscodeReady
{
    return _commissionerPasscodeReady;
}

- (BOOL)getCancelPasscode
{
    return _cancelPasscode;
}

- (BOOL)addTargetAppInfo:(MCTargetAppInfo *)targetAppInfo
{
    if (self.targetAppInfos.count >= CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS) {
        ChipLogError(AppServer, "MCIdentificationDeclarationOptions addTargetAppInfo() failed to add TargetAppInfo, max targetAppInfos list size is: %d", CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS);
        return NO;
    }
    [self.targetAppInfos addObject:targetAppInfo];
    return YES;
}

- (NSArray<MCTargetAppInfo *> *)getTargetAppInfoList
{
    return [self.targetAppInfos copy];
}

- (NSString *)description
{
    NSMutableString * sb = [NSMutableString stringWithFormat:@"MCIdentificationDeclarationOptions::noPasscode: %d\n", self.noPasscode];
    [sb appendFormat:@"MCIdentificationDeclarationOptions::cdUponPasscodeDialog: %d\n", self.cdUponPasscodeDialog];
    [sb appendFormat:@"MCIdentificationDeclarationOptions::commissionerPasscode: %d\n", self.commissionerPasscode];
    [sb appendFormat:@"MCIdentificationDeclarationOptions::commissionerPasscodeReady: %d\n", self.commissionerPasscodeReady];
    [sb appendFormat:@"MCIdentificationDeclarationOptions::cancelPasscode: %d\n", self.cancelPasscode];
    [sb appendString:@"MCIdentificationDeclarationOptions::targetAppInfos list: \n"];

    for (MCTargetAppInfo * targetAppInfo in self.targetAppInfos) {
        [sb appendFormat:@"\t\tTargetAppInfo - Vendor ID: %d, Product ID: %d\n", targetAppInfo.vendorId, targetAppInfo.productId];
    }

    return [sb copy];
}

- (void)logDetail
{
    ChipLogDetail(AppServer, "MCIdentificationDeclarationOptions::logDetail()\n%@", [self description]);
}

- (matter::casting::core::IdentificationDeclarationOptions)getCppIdentificationDeclarationOptions
{
    matter::casting::core::IdentificationDeclarationOptions cppIdOptions;
    cppIdOptions.mNoPasscode = [self getNoPasscode];
    cppIdOptions.mCdUponPasscodeDialog = [self getCdUponPasscodeDialog];
    cppIdOptions.mCommissionerPasscode = [self getCommissionerPasscode];
    cppIdOptions.mCommissionerPasscodeReady = [self getCommissionerPasscodeReady];
    cppIdOptions.mCancelPasscode = [self getCancelPasscode];

    NSArray<MCTargetAppInfo *> * targetAppInfos = [self getTargetAppInfoList];
    for (MCTargetAppInfo * appInfo in targetAppInfos) {
        chip::Protocols::UserDirectedCommissioning::TargetAppInfo targetAppInfo;
        targetAppInfo.vendorId = appInfo.vendorId;
        targetAppInfo.productId = appInfo.productId;
        CHIP_ERROR err = cppIdOptions.addTargetAppInfo(targetAppInfo);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "MCIdentificationDeclarationOptions.getCppIdentificationDeclarationOptions() Failed to add TargetAppInfo with Vendor ID: %d, Product ID: %d", appInfo.vendorId, appInfo.productId);
        }
    }

    return cppIdOptions;
}

@end
