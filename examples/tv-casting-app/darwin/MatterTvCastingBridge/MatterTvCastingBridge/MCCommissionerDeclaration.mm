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
        _cancelPasscode = cppCommissionerDeclaration->GetCancelPasscode();
    }
    return self;
}

- (instancetype)initWithOptions:(NSInteger)errorCode
                  needsPasscode:(BOOL)needsPasscode
                    noAppsFound:(BOOL)noAppsFound
        passcodeDialogDisplayed:(BOOL)passcodeDialogDisplayed
           commissionerPasscode:(BOOL)commissionerPasscode
                qRCodeDisplayed:(BOOL)qRCodeDisplayed
                 cancelPasscode:(BOOL)cancelPasscode
{
    self = [super init];
    if (self) {
        _errorCode = (CdError) errorCode;
        _needsPasscode = needsPasscode;
        _noAppsFound = noAppsFound;
        _passcodeDialogDisplayed = passcodeDialogDisplayed;
        _commissionerPasscode = commissionerPasscode;
        _qRCodeDisplayed = qRCodeDisplayed;
        _cancelPasscode = cancelPasscode;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MCCommissionerDeclaration::errorCode:               %@\nMCCommissionerDeclaration::needsPasscode:           %d\nMCCommissionerDeclaration::noAppsFound:             %d\nMCCommissionerDeclaration::passcodeDialogDisplayed: %d\nMCCommissionerDeclaration::commissionerPasscode:    %d\nMCCommissionerDeclaration::qRCodeDisplayed:         %d\nMCCommissionerDeclaration::cancelPasscode:          %d",
                     [self stringForErrorCode:self.errorCode],
                     self.needsPasscode,
                     self.noAppsFound,
                     self.passcodeDialogDisplayed,
                     self.commissionerPasscode,
                     self.qRCodeDisplayed,
                     self.cancelPasscode];
}

- (NSString *)getErrorCodeString
{
    return [self stringForErrorCode:self.errorCode];
}

- (NSString *)stringForErrorCode:(CdError)errorCode
{
    switch (errorCode) {
    case noError:
        return @"noError";
    case commissionableDiscoveryFailed:
        return @"commissionableDiscoveryFailed";
    case paseConnectionFailed:
        return @"paseConnectionFailed";
    case paseAuthFailed:
        return @"paseAuthFailed";
    case dacValidationFailed:
        return @"dacValidationFailed";
    case alreadyOnFabric:
        return @"alreadyOnFabric";
    case operationalDiscoveryFailed:
        return @"operationalDiscoveryFailed";
    case caseConnectionFailed:
        return @"caseConnectionFailed";
    case caseAuthFailed:
        return @"caseAuthFailed";
    case configurationFailed:
        return @"configurationFailed";
    case bindingConfigurationFailed:
        return @"bindingConfigurationFailed";
    case commissionerPasscodeNotSupported:
        return @"commissionerPasscodeNotSupported";
    case invalidIdentificationDeclarationParams:
        return @"invalidIdentificationDeclarationParams";
    case appInstallConsentPending:
        return @"appInstallConsentPending";
    case appInstalling:
        return @"appInstalling";
    case appInstallFailed:
        return @"appInstallFailed";
    case appInstalledRetryNeeded:
        return @"appInstalledRetryNeeded";
    case commissionerPasscodeDisabled:
        return @"commissionerPasscodeDisabled";
    case unexpectedCommissionerPasscodeReady:
        return @"unexpectedCommissionerPasscodeReady";
    default:
        return @"Unknown Error";
    }
}

- (void)logDetail
{
    ChipLogDetail(AppServer, "MCCommissionerDeclaration::logDetail()\n%@", [self description]);
}

@end
