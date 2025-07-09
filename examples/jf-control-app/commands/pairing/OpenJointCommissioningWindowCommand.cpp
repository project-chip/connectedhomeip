/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include "OpenJointCommissioningWindowCommand.h"

#include <system/SystemClock.h>

using namespace ::chip;

CHIP_ERROR OpenJointCommissioningWindowCommand::RunCommand()
{
    mWindowOpener = Platform::MakeUnique<Controller::CommissioningWindowOpener>(&CurrentCommissioner());

    SetupPayload ignored;
    return mWindowOpener->OpenJointCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                           .SetNodeId(mNodeId)
                                                           .SetTimeout(mCommissioningWindowTimeout)
                                                           .SetIteration(mIteration)
                                                           .SetDiscriminator(mDiscriminator)
                                                           .SetReadVIDPIDAttributes(true)
                                                           .SetCallback(&mOnOpenCommissioningWindowCallback)
                                                           .SetEndpointId(mEndpointId),
                                                       ignored);
}

void OpenJointCommissioningWindowCommand::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err,
                                                                            SetupPayload payload)
{
    LogErrorOnFailure(err);

    OpenJointCommissioningWindowCommand * command = reinterpret_cast<OpenJointCommissioningWindowCommand *>(context);
    VerifyOrReturn(
        command != nullptr,
        ChipLogError(chipTool, "OpenJointCommissioningWindowCommand::OnOpenCommissioningWindowResponse: context is null"));
    command->SetCommandExitStatus(err);
}
