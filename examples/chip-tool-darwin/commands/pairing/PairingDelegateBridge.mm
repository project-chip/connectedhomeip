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
#import <CHIP/CHIPError_Internal.h>

@interface CHIPToolPairingDelegate ()
@property (nonatomic, strong) CHIPBasic * cluster;
@property (nonatomic, strong) ResponseHandler responseHandler;
@end
@implementation CHIPToolPairingDelegate
- (void)onStatusUpdate:(CHIPPairingStatus)status
{
    NSLog(@"Pairing Status Update: %lu", status);
    switch (status)
    {
    case kSecurePairingSuccess:
        ChipLogProgress(chipTool, "Secure Pairing Success");
        break;
    case kSecurePairingFailed:
        ChipLogError(chipTool, "Secure Pairing Failed");
        break;
    case kUnknownStatus:
        ChipLogError(chipTool, "Uknown Pairing Status");
        break;
    }
}

- (void)onPairingComplete:(NSError *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NSLog(@"Pairing Complete: %@", error);
    err = [CHIPError errorToCHIPErrorCode:error];
    _commandBridge->SetCommandExitStatus(err);
}

- (void)onPairingDeleted:(NSError *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NSLog(@"Pairing Deleted: %@", error);
    err = [CHIPError errorToCHIPErrorCode:error];
    _commandBridge->SetCommandExitStatus(err);
}

- (void)onCommissioningComplete:(NSError *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    NSLog(@"Pairing Commissioning Complete: %@", error);
    err = [CHIPError errorToCHIPErrorCode:error];
    _commandBridge->SetCommandExitStatus(err);
}

@end
