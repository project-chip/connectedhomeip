/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#import <Matter/Matter.h>

#include "GetCommissionerNodeIdCommand.h"

CHIP_ERROR GetCommissionerNodeIdCommand::RunCommand()
{
    auto * controller = CurrentCommissioner();
    VerifyOrReturnError(nil != controller, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(
        chipTool, "Commissioner Node Id 0x" ChipLogFormatX64, ChipLogValueX64(controller.controllerNodeId.unsignedLongLongValue));

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
