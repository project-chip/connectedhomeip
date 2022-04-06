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

#pragma once

#include "../../config/PersistentStorage.h"
#include "../common/CHIPCommand.h"
#include <lib/core/CHIPEncoding.h>

class ModelCommand : public CHIPCommand
{
public:
    using ChipDevice = ::chip::OperationalDeviceProxy;

    ModelCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(commandName, credsIssuerConfig), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    void AddArguments()
    {
        AddArgument("node-id/group-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("endpoint-id-ignored-for-group-commands", 0, UINT16_MAX, &mEndPointId);
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(10)); }

    virtual CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endPointIds) = 0;

    virtual CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) { return CHIP_ERROR_BAD_REQUEST; };

    void Shutdown() override;

protected:
    chip::Optional<uint16_t> mTimeout;

private:
    chip::NodeId mNodeId;
    std::vector<chip::EndpointId> mEndPointId;

    static void OnDeviceConnectedFn(void * context, ChipDevice * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};
