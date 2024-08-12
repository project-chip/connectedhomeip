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
#import <Foundation/Foundation.h>

#ifndef MCCommissionerDeclaration_h
#define MCCommissionerDeclaration_h

/**
 * Represents the Commissioner Declaration message sent by a User Directed Commissioning server
 * (CastingPlayer/Commissioner) to a UDC client (Casting Client/Commissionee).
 */
@interface MCCommissionerDeclaration : NSObject

/** The allowed values for the ErrorCode field are the following */
typedef NS_ENUM(NSInteger, CdError) {
    kNoError = 0,
    kCommissionableDiscoveryFailed = 1,
    kPaseConnectionFailed = 2,
    kPaseAuthFailed = 3,
    kDacValidationFailed = 4,
    kAlreadyOnFabric = 5,
    kOperationalDiscoveryFailed = 6,
    kCaseConnectionFailed = 7,
    kCaseAuthFailed = 8,
    kConfigurationFailed = 9,
    kBindingConfigurationFailed = 10,
    kCommissionerPasscodeNotSupported = 11,
    kInvalidIdentificationDeclarationParams = 12,
    kAppInstallConsentPending = 13,
    kAppInstalling = 14,
    kAppInstallFailed = 15,
    kAppInstalledRetryNeeded = 16,
    kCommissionerPasscodeDisabled = 17,
    kUnexpectedCommissionerPasscodeReady = 18
};

/** Feature: All - Indicates errors incurred during commissioning. */
@property (nonatomic, readonly) CdError errorCode;
/**
 * Feature: Coordinate PIN Dialogs - When NoPasscode field set to true, and the Commissioner
 * determines that a Passcode code will be needed for commissioning.
 */
@property (nonatomic, readonly) BOOL needsPasscode;
/**
 * Feature: Target Content Application - No apps with AccountLogin cluster implementation were
 * found for the last IdentificationDeclaration request. Only apps which provide access to the
 * vendor id of the Commissionee will be considered.
 */
@property (nonatomic, readonly) BOOL noAppsFound;
/**
 * Feature: Coordinate PIN Dialogs - A Passcode input dialog is now displayed for the user on the
 * Commissioner.
 */
@property (nonatomic, readonly) BOOL passcodeDialogDisplayed;
/**
 * Feature: Commissioner-Generated Passcode - A Passcode is now displayed for the user by the
 * CastingPlayer/Commissioner.
 */
@property (nonatomic, readonly) BOOL commissionerPasscode;
/**
 * Feature: Commissioner-Generated Passcode - The user experience conveying a Passcode to the user
 * also displays a QR code.
 */
@property (nonatomic, readonly) BOOL qRCodeDisplayed;

- (instancetype)initWithOptions:(NSInteger)errorCode
                  needsPasscode:(BOOL)needsPasscode
                    noAppsFound:(BOOL)noAppsFound
        passcodeDialogDisplayed:(BOOL)passcodeDialogDisplayed
           commissionerPasscode:(BOOL)commissionerPasscode
                qRCodeDisplayed:(BOOL)qRCodeDisplayed;

- (NSString *)description;
- (void)logDetail;

@end

#endif /* MCCommissionerDeclaration_h */
