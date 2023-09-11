/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommandBridge.h"
#include <lib/core/CHIPEncoding.h>

class ModelCommand : public CHIPCommandBridge
{
public:
    ModelCommand(const char * _Nonnull commandName) : CHIPCommandBridge(commandName) {}

    void AddArguments()
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("endpoint-id", 0, UINT16_MAX, &mEndPointId);
    }

    void Shutdown() override;

    void Cleanup() override { ModelCommand::Shutdown(); }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

    virtual CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endPointId) = 0;

private:
    chip::NodeId mNodeId;
    chip::EndpointId mEndPointId;
};
