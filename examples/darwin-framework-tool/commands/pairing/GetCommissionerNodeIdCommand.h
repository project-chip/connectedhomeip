/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommandBridge.h"

class GetCommissionerNodeIdCommand : public CHIPCommandBridge
{
public:
    GetCommissionerNodeIdCommand() : CHIPCommandBridge("get-commissioner-node-id") {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }
};
