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
#include <app/chip-zcl-zpro-codec.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPEncoding.h>

// Limits on endpoint values.
#define CHIP_ZCL_ENDPOINT_MIN 0x00
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

class ModelCommand : public CHIPCommand
{
public:
    using ChipDevice = ::chip::Controller::Device;

    ModelCommand(const char * commandName) :
        CHIPCommand(commandName), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    void AddArguments()
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("endpoint-id", CHIP_ZCL_ENDPOINT_MIN, CHIP_ZCL_ENDPOINT_MAX, &mEndPointId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    uint16_t GetWaitDurationInSeconds() const override { return 10; }

    virtual CHIP_ERROR SendCommand(ChipDevice * device, uint8_t endPointId) = 0;

private:
    chip::NodeId mNodeId;
    uint8_t mEndPointId;

    static void OnDeviceConnectedFn(void * context, ChipDevice * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);

    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};
