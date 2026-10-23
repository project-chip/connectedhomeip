/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"
#include "../common/CertificateIssuer.h"
#include "DeviceControllerDelegateBridge.h"
#include "PairingCommandBridge.h"
#include <commands/dcl/DCLClient.h>
#include <commands/dcl/DisplayTermsAndConditions.h>
#include <lib/support/logging/CHIPLogging.h>

#import "MTRError_Utils.h"

using namespace ::chip;
using namespace ::chip::Controller;

extern NSMutableArray * gDiscoveredDevices;

// A no-op MTRDeviceAttestationDelegate which lets us test (by default, in CI)
// commissioning flows that have such a delegate.
@interface NoOpAttestationDelegate : NSObject <MTRDeviceAttestationDelegate>
@end

@implementation NoOpAttestationDelegate
- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
{
    [controller continueCommissioningDevice:opaqueDeviceHandle ignoreAttestationFailure:NO error:nil];
}

@end

void PairingCommandBridge::SetUpDeviceControllerDelegate(NSError * __autoreleasing * error)
{
    CHIPToolDeviceControllerDelegate * deviceControllerDelegate = [[CHIPToolDeviceControllerDelegate alloc] init];
    [deviceControllerDelegate setCommandBridge:this];
    [deviceControllerDelegate setDeviceID:mNodeId];

    // With per-controller storage, the certificate issuer creates the operational certificate.
    // When using shared storage, this step is a no-op.
    auto * certificateIssuer = [CertificateIssuer sharedInstance];
    certificateIssuer.nextNodeID = @(mNodeId);
    certificateIssuer.fabricID = CurrentCommissionerFabricId();

    if (mCommissioningType != CommissioningType::None) {
        MTRCommissioningParameters * params = [[MTRCommissioningParameters alloc] init];
        switch (mCommissioningType) {
        case CommissioningType::None:
        case CommissioningType::WithoutNetwork:
            break;
        case CommissioningType::WithWiFi:
            [params setWifiSSID:[NSData dataWithBytes:mSSID.data() length:mSSID.size()]];
            [params setWifiCredentials:[NSData dataWithBytes:mPassword.data() length:mPassword.size()]];
            break;
        case CommissioningType::WithThread:
            [params setThreadOperationalDataset:[NSData dataWithBytes:mOperationalDataset.data() length:mOperationalDataset.size()]];
            break;
        }

        if (mUseDeviceAttestationDelegate.ValueOr(false)) {
            params.deviceAttestationDelegate = [[NoOpAttestationDelegate alloc] init];
            if (mDeviceAttestationFailsafeTime.HasValue()) {
                params.failSafeTimeout = @(mDeviceAttestationFailsafeTime.Value());
            }
        }

        if (mCountryCode.HasValue()) {
            params.countryCode = [NSString stringWithUTF8String:mCountryCode.Value()];
        }

        MaybeDisplayTermsAndConditions(params, error);
        VerifyOrReturn(*error == nil);
        [deviceControllerDelegate setParams:params];
    }

    MTRDeviceController * commissioner = CurrentCommissioner();
    [deviceControllerDelegate setCommissioner:commissioner];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    [commissioner setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];
}

void PairingCommandBridge::MaybeDisplayTermsAndConditions(MTRCommissioningParameters * params, NSError * __autoreleasing * error)
{
    VerifyOrReturn(mUseDCL.ValueOr(false));

    Json::Value tc;
    auto securityMode = mDCLDisableHttps.ValueOr(false) ? tool::https::HttpsSecurityMode::kDisableHttps : mDCLDisableHttpsValidation.ValueOr(false) ? tool::https::HttpsSecurityMode::kDisableValidation
                                                                                                                                                    : tool::https::HttpsSecurityMode::kDefault;
    auto client = tool::dcl::DCLClient(mDCLHostName, mDCLPort, securityMode);
    CHIP_ERROR err = client.TermsAndConditions(mOnboardingPayload, tc);

    if (CHIP_NO_ERROR != err) {
        auto errorString = [NSString stringWithFormat:@"Error retrieving terms and conditions."];
        *error = [[NSError alloc] initWithDomain:@"PairingDomain"
                                            code:MTRErrorCodeGeneralError
                                        userInfo:@ { NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
        return;
    }

    if (tc != Json::nullValue) {
        uint16_t version = 0;
        uint16_t userResponse = 0;
        err = tool::dcl::DisplayTermsAndConditions(tc, version, userResponse, mCountryCode);
        if (CHIP_NO_ERROR != err) {
            auto errorString = [NSString stringWithFormat:@"Error displaying terms and conditions."];
            *error = [[NSError alloc] initWithDomain:@"PairingDomain"
                                                code:MTRErrorCodeGeneralError
                                            userInfo:@ { NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
            return;
        }

        params.acceptedTermsAndConditions = @(userResponse);
        params.acceptedTermsAndConditionsVersion = @(version);
    }
}

CHIP_ERROR PairingCommandBridge::RunCommand()
{
    NSError * error;
    switch (mPairingMode) {
    case PairingMode::Unpair:
        Unpair();
        break;
    case PairingMode::Code:
        PairWithPayload(&error);
        break;
    case PairingMode::Ble:
        PairWithCode(&error);
        break;
    case PairingMode::AlreadyDiscoveredByIndex:
        PairWithIndex(&error);
        break;
    }

    if (error != nil) {
        SetCommandExitStatus(error);
    }
    return CHIP_NO_ERROR;
}

void PairingCommandBridge::PairWithCode(NSError * __autoreleasing * error)
{
    SetUpDeviceControllerDelegate(error);
    VerifyOrReturn(*error == nil);

    auto * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@(mSetupPINCode) discriminator:@(mDiscriminator)];
    MTRDeviceController * commissioner = CurrentCommissioner();
    [commissioner setupCommissioningSessionWithPayload:payload newNodeID:@(mNodeId) error:error];
}

void PairingCommandBridge::PairWithIndex(NSError * __autoreleasing * error)
{
    SetUpDeviceControllerDelegate(error);
    VerifyOrReturn(*error == nil);

    MTRDeviceController * commissioner = CurrentCommissioner();
    if (mIndex >= [gDiscoveredDevices count]) {
        auto errorString = [NSString stringWithFormat:@"Error retrieving discovered device at index %@", @(mIndex)];
        *error = [[NSError alloc] initWithDomain:@"PairingDomain"
                                            code:MTRErrorCodeGeneralError
                                        userInfo:@ { NSLocalizedDescriptionKey : NSLocalizedString(errorString, nil) }];
        return;
    }

    NSString * onboardingPayload = [NSString stringWithUTF8String:mOnboardingPayload];
    auto * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:onboardingPayload error:error];
    if (payload == nil) {
        return;
    }

    auto discoveredDevice = (MTRCommissionableBrowserResult *) gDiscoveredDevices[mIndex];
    [commissioner setupCommissioningSessionWithDiscoveredDevice:discoveredDevice payload:payload newNodeID:@(mNodeId) error:error];
}

void PairingCommandBridge::PairWithPayload(NSError * __autoreleasing * error)
{
    NSString * onboardingPayload = [NSString stringWithUTF8String:mOnboardingPayload];
    SetUpDeviceControllerDelegate(error);
    VerifyOrReturn(*error == nil);

    auto * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:onboardingPayload error:error];
    if (payload == nil) {
        return;
    }
    MTRDeviceController * commissioner = CurrentCommissioner();
    [commissioner setupCommissioningSessionWithPayload:payload newNodeID:@(mNodeId) error:error];
}

void PairingCommandBridge::Unpair()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip-tool.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    auto * device = BaseDeviceWithNodeId(mNodeId);

    ChipLogProgress(chipTool, "Attempting to unpair device %llu", mNodeId);
    MTRBaseClusterOperationalCredentials * opCredsCluster =
        [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:callbackQueue];
    [opCredsCluster readAttributeCurrentFabricIndexWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
        if (readError) {
            CHIP_ERROR readErr = MTRErrorToCHIPErrorCode(readError);
            LogNSError("Failed to get current fabric: ", readError);
            SetCommandExitStatus(readErr);
            return;
        }
        MTROperationalCredentialsClusterRemoveFabricParams * params =
            [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
        params.fabricIndex = value;
        [opCredsCluster removeFabricWithParams:params
                                    completion:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                        NSError * _Nullable removeError) {
                                        CHIP_ERROR removeErr = CHIP_NO_ERROR;
                                        if (removeError) {
                                            removeErr = MTRErrorToCHIPErrorCode(removeError);
                                            LogNSError("Failed to remove current fabric: ", removeError);
                                        } else {
                                            ChipLogProgress(chipTool, "Successfully unpaired deviceId %llu", mNodeId);
                                        }
                                        SetCommandExitStatus(removeErr);
                                    }];
    }];
}
