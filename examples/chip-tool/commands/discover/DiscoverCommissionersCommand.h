/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommand.h"
#include <controller/CHIPCommissionableNodeController.h>

class DiscoverCommissionersCommand : public CHIPCommand
{
public:
    DiscoverCommissionersCommand(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("commissioners", credsIssuerConfig) {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(3); }
    void Shutdown() override;

private:
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
};
