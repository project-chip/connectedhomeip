/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "PairingCommandBridge.h"
#include "../common/CHIPCommandBridge.h"
#include "PairingDelegateBridge.h"
#include "platform/PlatformManager.h"
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Controller;

CHIP_ERROR PairingCommandBridge::RunCommand()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (mPairingMode) {
    case PairingMode::None:
        err = Unpair();
        break;
    case PairingMode::QRCode:
        err = PairWithQRCode();
        break;
    case PairingMode::ManualCode:
        err = PairWithQRCode();
        break;
    case PairingMode::Ethernet:
        err = PairWithIPAddress();
        break;
    }

    return err;
}

CHIP_ERROR PairingCommandBridge::PairWithQRCode()
{
    NSError * error;
    NSString * payload = [NSString stringWithUTF8String:mOnboardingPayload];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    CHIPToolPairingDelegate * pairing = [[CHIPToolPairingDelegate alloc] init];
    pairing.deviceID = mNodeId;
    pairing.commandBridge = this;
    [CurrentCommissioner() setPairingDelegate:pairing queue:callbackQueue];
    [CurrentCommissioner() pairDevice:mNodeId onboardingPayload:payload error:&error];
    if ([error code] != 0) {
        NSLog(@"Pairing error: %@", error);
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingCommandBridge::PairWithIPAddress()
{
    NSError * error;
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    CHIPToolPairingDelegate * pairing = [[CHIPToolPairingDelegate alloc] init];
    pairing.deviceID = mNodeId;
    pairing.commandBridge = this;
    [CurrentCommissioner() setPairingDelegate:pairing queue:callbackQueue];
    [CurrentCommissioner() pairDevice:mNodeId
                              address:[NSString stringWithUTF8String:ipAddress]
                                 port:mRemotePort
                        discriminator:mDiscriminator
                         setupPINCode:mSetupPINCode
                                error:&error];

    if ([error code] != 0) {
        NSLog(@"Pairing error: %@", error);
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingCommandBridge::Unpair()
{
    NSError * error;
    [CurrentCommissioner() unpairDevice:mNodeId error:&error];
    if ([error code] != 0) {
        NSLog(@"Upairing error: %@", error);
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
        return CHIP_ERROR_INTERNAL;
    }
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
