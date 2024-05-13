/*
 *   Copyright (c) 2023 Project CHIP Authors
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

#include "GetCommissionerNodeIdCommand.h"
#include "RemoteDataModelLogger.h"

CHIP_ERROR GetCommissionerNodeIdCommand::RunCommand()
{
    auto * controller = CurrentCommissioner();
    VerifyOrReturnError(nil != controller, CHIP_ERROR_INCORRECT_STATE);

    auto controllerNodeId = controller.controllerNodeId;
    ChipLogProgress(chipTool, "Commissioner Node Id 0x" ChipLogFormatX64, ChipLogValueX64(controllerNodeId.unsignedLongLongValue));

    ReturnErrorOnFailure(RemoteDataModelLogger::LogGetCommissionerNodeId(controllerNodeId));
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
