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

#include "PairingDelegateBridge.h"
#import <Matter/Matter.h>

@interface CHIPToolPairingDelegate ()
@end

@implementation CHIPToolPairingDelegate
- (void)onStatusUpdate:(MTRPairingStatus)status
{
    NSLog(@"Pairing Status Update: %lu", status);
    switch (status) {
    case MTRPairingStatusSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        ChipLogProgress(chipTool, "CASE establishment successful");
        break;
    case MTRPairingStatusFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        _commandBridge->SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
        break;
    case MTRPairingStatusDiscoveringMoreDevices:
        ChipLogProgress(chipTool, "Secure Pairing Discovering More Devices");
        break;
    case MTRPairingStatusUnknown:
        ChipLogError(chipTool, "Uknown Pairing Status");
        break;
    }
}

- (void)onPairingComplete:(NSError *)error
{
    if (error != nil) {
        ChipLogProgress(chipTool, "PASE establishment failed");
        _commandBridge->SetCommandExitStatus(error);
        return;
    }
    ChipLogProgress(chipTool, "Pairing Success");
    ChipLogProgress(chipTool, "PASE establishment successful");
    NSError * commissionError;
    [_commissioner commissionDevice:_deviceID commissioningParams:_params error:&commissionError];
    if (commissionError != nil) {
        _commandBridge->SetCommandExitStatus(commissionError);
        return;
    }
}

- (void)onPairingDeleted:(NSError *)error
{
    _commandBridge->SetCommandExitStatus(error, "Pairing Delete");
}

- (void)onCommissioningComplete:(NSError *)error
{
    _commandBridge->SetCommandExitStatus(error, "Pairing Commissioning Complete");
}

@end
