/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
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

#pragma once

#include "../common/CHIPCommand.h"

class OpenCommissioningWindowCommand : public CHIPCommand
{
public:
    OpenCommissioningWindowCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("open-commissioning-window", credIssuerCommands), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this),
        mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("option", 0, UINT8_MAX, &mCommissioningWindowOption);
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
        AddArgument("iteration", chip::kPBKDFMinimumIterations, chip::kPBKDFMaximumIterations, &mIteration);
        AddArgument("discriminator", 0, 4096, &mDiscriminator);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

private:
    NodeId mNodeId;
    uint8_t mCommissioningWindowOption;
    uint16_t mTimeout;
    uint32_t mIteration;
    uint16_t mDiscriminator;

    CHIP_ERROR OpenCommissioningWindow();
    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);
    static void OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
};
