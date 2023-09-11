/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/Matter.h>

#import "MTRError_Utils.h"

#include "OpenCommissioningWindowCommand.h"

CHIP_ERROR OpenCommissioningWindowCommand::RunCommand()
{
    mWorkQueue = dispatch_queue_create("com.chip.open_commissioning_window", DISPATCH_QUEUE_SERIAL);
    auto * controller = CurrentCommissioner();
    auto * device = [MTRDevice deviceWithNodeID:mNodeId deviceController:controller];

    auto * self = this;
    if (mCommissioningWindowOption == 0) {
        auto * cluster = [[MTRClusterAdministratorCommissioning alloc] initWithDevice:device endpointID:@(0) queue:mWorkQueue];
        auto * params = [[MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams alloc] init];
        params.commissioningTimeout = @(mCommissioningWindowTimeoutMs);
        params.timedInvokeTimeoutMs = @(10000);
        [cluster openBasicCommissioningWindowWithParams:params
                                         expectedValues:nil
                                  expectedValueInterval:nil
                                             completion:^(NSError * _Nullable error) {
                                                 if (error == nil) {
                                                     self->SetCommandExitStatus(CHIP_NO_ERROR);
                                                 } else {
                                                     self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(error));
                                                 }
                                             }];
    } else {
        [device
            openCommissioningWindowWithSetupPasscode:[MTRSetupPayload generateRandomSetupPasscode]
                                       discriminator:@(mDiscriminator)
                                            duration:@(mCommissioningWindowTimeoutMs)
                                               queue:mWorkQueue
                                          completion:^(MTRSetupPayload * _Nullable payload, NSError * error) {
                                              if (error != nil) {
                                                  self->SetCommandExitStatus(MTRErrorToCHIPErrorCode(error));
                                                  return;
                                              }

                                              if (payload == nil) {
                                                  self->SetCommandExitStatus(CHIP_ERROR_INVALID_ARGUMENT);
                                                  return;
                                              }

                                              auto * pairingCode = [payload manualEntryCode];
                                              if (pairingCode == nil) {
                                                  self->SetCommandExitStatus(CHIP_ERROR_INVALID_ARGUMENT);
                                                  return;
                                              }

                                              ChipLogProgress(chipTool, "Setup code: %s\n", [[payload manualEntryCode] UTF8String]);
                                              self->SetCommandExitStatus(CHIP_NO_ERROR);
                                          }];
    }

    return CHIP_NO_ERROR;
}
