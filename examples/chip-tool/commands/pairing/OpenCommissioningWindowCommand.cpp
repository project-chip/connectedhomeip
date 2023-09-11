/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        SetupPayload ignored;
        return mWindowOpener->OpenCommissioningWindow(mNodeId, System::Clock::Seconds16(mCommissioningWindowTimeout), mIteration,
                                                      mDiscriminator, NullOptional, NullOptional,
                                                      &mOnOpenCommissioningWindowCallback, ignored,
                                                      /* readVIDPIDAttributes */ true);
    }

    ChipLogError(chipTool, "Unknown commissioning window option: %d", to_underlying(mCommissioningWindowOption));
    return CHIP_ERROR_INVALID_ARGUMENT;
}

void OpenCommissioningWindowCommand::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err,
                                                                       chip::SetupPayload payload)
{
    LogErrorOnFailure(err);

    OnOpenBasicCommissioningWindowResponse(context, remoteId, err);
}

void OpenCommissioningWindowCommand::OnOpenBasicCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    OpenCommissioningWindowCommand * command = reinterpret_cast<OpenCommissioningWindowCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnOpenCommissioningWindowCommand: context is null"));
    command->SetCommandExitStatus(err);
}
