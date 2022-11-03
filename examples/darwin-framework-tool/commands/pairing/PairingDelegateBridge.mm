/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PairingDelegateBridge.h"
#import <Matter/Matter.h>

@interface CHIPToolPairingDelegate ()
@end

@implementation CHIPToolPairingDelegate
- (void)onStatusUpdate:(MTRPairingStatus)status
{
    NSLog(@"Pairing Status Update: %tu", status);
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
