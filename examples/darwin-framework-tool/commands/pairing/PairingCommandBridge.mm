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
#include "PairingCommandBridge.h"
#include "PairingDelegateBridge.h"
#include <lib/support/logging/CHIPLogging.h>

#import "MTRError_Utils.h"

using namespace ::chip;
using namespace ::chip::Controller;

void PairingCommandBridge::SetUpPairingDelegate()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);
    CHIPToolPairingDelegate * pairing = [[CHIPToolPairingDelegate alloc] init];
    MTRCommissioningParameters * params = [[MTRCommissioningParameters alloc] init];
    MTRDeviceController * commissioner = CurrentCommissioner();

    [pairing setDeviceID:mNodeId];
    switch (mNetworkType) {
    case PairingNetworkType::None:
    case PairingNetworkType::Ethernet:
        break;
    case PairingNetworkType::WiFi:
        [params setWifiSSID:[NSData dataWithBytes:mSSID.data() length:mSSID.size()]];
        [params setWifiCredentials:[NSData dataWithBytes:mPassword.data() length:mPassword.size()]];
        break;
    case PairingNetworkType::Thread:
        [params setThreadOperationalDataset:[NSData dataWithBytes:mOperationalDataset.data() length:mOperationalDataset.size()]];
        break;
    }

    [pairing setCommandBridge:this];
    [pairing setParams:params];
    [pairing setCommissioner:commissioner];

    [commissioner setPairingDelegate:pairing queue:callbackQueue];
}

CHIP_ERROR PairingCommandBridge::RunCommand()
{
    NSError * error;
    switch (mPairingMode) {
    case PairingMode::None:
        Unpair();
        break;
    case PairingMode::Code:
        PairWithPayload(&error);
        break;
    case PairingMode::Ble:
        PairWithCode(&error);
        break;
    }

    if (error != nil) {
        SetCommandExitStatus(error);
    }
    return CHIP_NO_ERROR;
}

void PairingCommandBridge::PairWithCode(NSError * __autoreleasing * error)
{
    SetUpPairingDelegate();
    auto * payload = [[MTRSetupPayload alloc] initWithSetupPasscode:@(mSetupPINCode) discriminator:@(mDiscriminator)];
    MTRDeviceController * commissioner = CurrentCommissioner();
    [commissioner setupCommissioningSessionWithPayload:payload newNodeID:@(mNodeId) error:error];
}

void PairingCommandBridge::PairWithPayload(NSError * __autoreleasing * error)
{
    NSString * onboardingPayload = [NSString stringWithUTF8String:mOnboardingPayload];
    SetUpPairingDelegate();
    auto * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:onboardingPayload error:error];
    if (payload == nil) {
        return;
    }
    MTRDeviceController * commissioner = CurrentCommissioner();
    [commissioner setupCommissioningSessionWithPayload:payload newNodeID:@(mNodeId) error:error];
}

void PairingCommandBridge::Unpair()
{
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip-tool.command", DISPATCH_QUEUE_SERIAL);
    MTRDeviceController * commissioner = CurrentCommissioner();
    [commissioner
        getBaseDevice:mNodeId
                queue:callbackQueue
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               CHIP_ERROR err = CHIP_NO_ERROR;
               if (error) {
                   err = MTRErrorToCHIPErrorCode(error);
                   LogNSError("Error: ", error);
                   SetCommandExitStatus(err);
               } else if (device == nil) {
                   ChipLogError(chipTool, "Error: %s", chip::ErrorStr(CHIP_ERROR_INTERNAL));
                   SetCommandExitStatus(CHIP_ERROR_INTERNAL);
               } else {
                   ChipLogProgress(chipTool, "Attempting to unpair device %llu", mNodeId);
                   MTRBaseClusterOperationalCredentials * opCredsCluster =
                       [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:callbackQueue];
                   [opCredsCluster
                       readAttributeCurrentFabricIndexWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
                           if (readError) {
                               CHIP_ERROR readErr = MTRErrorToCHIPErrorCode(readError);
                               LogNSError("Failed to get current fabric: ", readError);
                               SetCommandExitStatus(readErr);
                               return;
                           }
                           MTROperationalCredentialsClusterRemoveFabricParams * params =
                               [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
                           params.fabricIndex = value;
                           [opCredsCluster
                               removeFabricWithParams:params
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
           }];
}
