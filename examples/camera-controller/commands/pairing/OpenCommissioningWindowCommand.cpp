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

#include "OpenCommissioningWindowCommand.h"

#include <system/SystemClock.h>

using namespace ::chip;

CHIP_ERROR OpenCommissioningWindowCommand::RunCommand()
{
    mWindowOpener = Platform::MakeUnique<Controller::CommissioningWindowOpener>(&CurrentCommissioner());
    if (mCommissioningWindowOption == Controller::CommissioningWindowOpener::CommissioningWindowOption::kOriginalSetupCode)
    {
        return mWindowOpener->OpenBasicCommissioningWindow(mNodeId, System::Clock::Seconds16(mCommissioningWindowTimeout),
                                                           &mOnOpenBasicCommissioningWindowCallback);
    }

    if (mCommissioningWindowOption == Controller::CommissioningWindowOpener::CommissioningWindowOption::kTokenWithRandomPIN)
    {
        if (mVerifier.HasValue())
        {
            VerifyOrReturnError(mSalt.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            return mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                              .SetNodeId(mNodeId)
                                                              .SetEndpointId(mEndpointId)
                                                              .SetTimeout(mCommissioningWindowTimeout)
                                                              .SetIteration(mIteration)
                                                              .SetDiscriminator(mDiscriminator)
                                                              .SetVerifier(mVerifier.Value())
                                                              .SetSalt(mSalt.Value())
                                                              .SetCallback(&mOnOpenCommissioningWindowVerifierCallback));
        }
        else
        {
            SetupPayload ignored;
            return mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                              .SetNodeId(mNodeId)
                                                              .SetEndpointId(mEndpointId)
                                                              .SetTimeout(mCommissioningWindowTimeout)
                                                              .SetIteration(mIteration)
                                                              .SetDiscriminator(mDiscriminator)
                                                              .SetSetupPIN(mSetupPIN)
                                                              .SetSalt(mSalt)
                                                              .SetReadVIDPIDAttributes(true)
                                                              .SetCallback(&mOnOpenCommissioningWindowCallback),
                                                          ignored);
        }
    }

    ChipLogError(NotSpecified, "Unknown commissioning window option: %d", to_underlying(mCommissioningWindowOption));
    return CHIP_ERROR_INVALID_ARGUMENT;
}

void OpenCommissioningWindowCommand::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err,
                                                                       SetupPayload payload)
{
    OnOpenBasicCommissioningWindowResponse(context, remoteId, err);
}

void OpenCommissioningWindowCommand::OnOpenCommissioningWindowVerifierResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    OnOpenBasicCommissioningWindowResponse(context, remoteId, err);
}

void OpenCommissioningWindowCommand::OnOpenBasicCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    OpenCommissioningWindowCommand * command = reinterpret_cast<OpenCommissioningWindowCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(NotSpecified, "OnOpenCommissioningWindowCommand: context is null"));
    command->SetCommandExitStatus(err);
}
