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

#import "MCCommissionerDeclaration_Internal.h"
#import "core/Types.h"

@interface MCCommissionerDeclaration ()

/** Feature: All - Indicates errors incurred during commissioning. */
@property (nonatomic) CdError errorCode;
/**
 * Feature: Coordinate PIN Dialogs - When NoPasscode field set to true, and the Commissioner
 * determines that a Passcode code will be needed for commissioning.
 */
@property (nonatomic) BOOL needsPasscode;
/**
 * Feature: Target Content Application - No apps with AccountLogin cluster implementation were
 * found for the last IdentificationDeclaration request. Only apps which provide access to the
 * vendor id of the Commissionee will be considered.
 */
@property (nonatomic) BOOL noAppsFound;
/**
 * Feature: Coordinate PIN Dialogs - A Passcode input dialog is now displayed for the user on the
 * Commissioner.
 */
@property (nonatomic) BOOL passcodeDialogDisplayed;
/**
 * Feature: Commissioner-Generated Passcode - A Passcode is now displayed for the user by the
 * CastingPlayer/Commissioner.
 */
@property (nonatomic) BOOL commissionerPasscode;
/**
 * Feature: Commissioner-Generated Passcode - The user experience conveying a Passcode to the user
 * also displays a QR code.
 */
@property (nonatomic) BOOL qRCodeDisplayed;

@property (nonatomic, readwrite) matter::casting::memory::Strong<chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration> cppCommissionerDeclaration;

@end

@implementation MCCommissionerDeclaration

- (instancetype _Nonnull)initWithCppCommissionerDeclaration:(std::shared_ptr<chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration>)cppCommissionerDeclaration
{
    if (self = [super init]) {
        _cppCommissionerDeclaration = cppCommissionerDeclaration;
        _errorCode = static_cast<CdError>(cppCommissionerDeclaration->GetErrorCode());
        _needsPasscode = cppCommissionerDeclaration->GetNeedsPasscode();
        _noAppsFound = cppCommissionerDeclaration->GetNoAppsFound();
        _passcodeDialogDisplayed = cppCommissionerDeclaration->GetPasscodeDialogDisplayed();
        _commissionerPasscode = cppCommissionerDeclaration->GetCommissionerPasscode();
        _qRCodeDisplayed = cppCommissionerDeclaration->GetQRCodeDisplayed();
    }
    return self;
}

- (instancetype)initWithOptions:(NSInteger)errorCode
                  needsPasscode:(BOOL)needsPasscode
                    noAppsFound:(BOOL)noAppsFound
        passcodeDialogDisplayed:(BOOL)passcodeDialogDisplayed
           commissionerPasscode:(BOOL)commissionerPasscode
                qRCodeDisplayed:(BOOL)qRCodeDisplayed
{
    self = [super init];
    if (self) {
        _errorCode = (CdError) errorCode;
        _needsPasscode = needsPasscode;
        _noAppsFound = noAppsFound;
        _passcodeDialogDisplayed = passcodeDialogDisplayed;
        _commissionerPasscode = commissionerPasscode;
        _qRCodeDisplayed = qRCodeDisplayed;
    }
    return self;
}

- (CdError)getErrorCode
{
    return _errorCode;
}

- (BOOL)getNeedsPasscode
{
    return _needsPasscode;
}

- (BOOL)getNoAppsFound
{
    return _noAppsFound;
}

- (BOOL)getPasscodeDialogDisplayed
{
    return _passcodeDialogDisplayed;
}

- (BOOL)getCommissionerPasscode
{
    return _commissionerPasscode;
}

- (BOOL)getQRCodeDisplayed
{
    return _qRCodeDisplayed;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MCCommissionerDeclaration::errorCode:               %@\nMCCommissionerDeclaration::needsPasscode:           %d\nMCCommissionerDeclaration::noAppsFound:             %d\nMCCommissionerDeclaration::passcodeDialogDisplayed: %d\nMCCommissionerDeclaration::commissionerPasscode:    %d\nMCCommissionerDeclaration::qRCodeDisplayed:         %d",
                     [self stringForErrorCode:self.errorCode],
                     self.needsPasscode,
                     self.noAppsFound,
                     self.passcodeDialogDisplayed,
                     self.commissionerPasscode,
                     self.qRCodeDisplayed];
}

- (NSString *)stringForErrorCode:(CdError)errorCode
{
    switch (errorCode) {
    case kNoError:
        return @"kNoError";
    case kCommissionableDiscoveryFailed:
        return @"kCommissionableDiscoveryFailed";
    case kPaseConnectionFailed:
        return @"kPaseConnectionFailed";
    case kPaseAuthFailed:
        return @"kPaseAuthFailed";
    case kDacValidationFailed:
        return @"kDacValidationFailed";
    case kAlreadyOnFabric:
        return @"kAlreadyOnFabric";
    case kOperationalDiscoveryFailed:
        return @"kOperationalDiscoveryFailed";
    case kCaseConnectionFailed:
        return @"kCaseConnectionFailed";
    case kCaseAuthFailed:
        return @"kCaseAuthFailed";
    case kConfigurationFailed:
        return @"kConfigurationFailed";
    case kBindingConfigurationFailed:
        return @"kBindingConfigurationFailed";
    case kCommissionerPasscodeNotSupported:
        return @"kCommissionerPasscodeNotSupported";
    case kInvalidIdentificationDeclarationParams:
        return @"kInvalidIdentificationDeclarationParams";
    case kAppInstallConsentPending:
        return @"kAppInstallConsentPending";
    case kAppInstalling:
        return @"kAppInstalling";
    case kAppInstallFailed:
        return @"kAppInstallFailed";
    case kAppInstalledRetryNeeded:
        return @"kAppInstalledRetryNeeded";
    case kCommissionerPasscodeDisabled:
        return @"kCommissionerPasscodeDisabled";
    case kUnexpectedCommissionerPasscodeReady:
        return @"kUnexpectedCommissionerPasscodeReady";
    default:
        return @"Unknown Error";
    }
}

- (void)logDetail
{
    ChipLogDetail(AppServer, "MCCommissionerDeclaration::logDetail()\n%@", [self description]);
}

@end
