/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "DeviceControllerDelegateBridge.h"
#import <Matter/Matter.h>

@interface CHIPToolDeviceControllerDelegate ()
@end

@implementation CHIPToolDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status
{
    NSLog(@"Pairing Status Update: %tu", status);
    switch (status) {
    case MTRCommissioningStatusSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        ChipLogProgress(chipTool, "CASE establishment successful");
        break;
    case MTRCommissioningStatusFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        _commandBridge->SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
        break;
    case MTRCommissioningStatusDiscoveringMoreDevices:
        ChipLogError(chipTool, "MTRCommissioningStatusDiscoveringMoreDevices: This should not happen.");
        break;
    case MTRCommissioningStatusUnknown:
        ChipLogError(chipTool, "Uknown Pairing Status");
        break;
    }
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    if (error != nil) {
        ChipLogProgress(chipTool, "PASE establishment failed");
        _commandBridge->SetCommandExitStatus(error);
        return;
    }
    ChipLogProgress(chipTool, "Pairing Success");
    ChipLogProgress(chipTool, "PASE establishment successful");
    NSError * commissionError;
    [_commissioner commissionNodeWithID:@(_deviceID) commissioningParams:_params error:&commissionError];
    if (commissionError != nil) {
        _commandBridge->SetCommandExitStatus(commissionError);
        return;
    }
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    _commandBridge->SetCommandExitStatus(error, "Pairing Commissioning Complete");
}

@end
