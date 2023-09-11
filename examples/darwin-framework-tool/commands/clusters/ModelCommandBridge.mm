/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ModelCommandBridge.h"
#include <inttypes.h>

#import <Matter/Matter.h>

using namespace ::chip;

CHIP_ERROR ModelCommand::RunCommand()
{
    MTRDeviceController * commissioner = CurrentCommissioner();
    ChipLogProgress(chipTool, "Sending command to node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));
    auto * device = [MTRBaseDevice deviceWithNodeID:@(mNodeId) controller:commissioner];
    CHIP_ERROR err = SendCommand(device, mEndPointId);

    if (err != CHIP_NO_ERROR) {
        ChipLogError(chipTool, "Error: %s", chip::ErrorStr(err));
        return err;
    }
    return CHIP_NO_ERROR;
}

void ModelCommand::Shutdown()
{
    ResetArguments();
    CHIPCommandBridge::Shutdown();
}
