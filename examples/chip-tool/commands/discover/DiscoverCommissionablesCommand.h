/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../common/CHIPCommand.h"

class DiscoverCommissionablesCommand : public CHIPCommand, public chip::Controller::DeviceDiscoveryDelegate
{
public:
    DiscoverCommissionablesCommand(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("commissionables", credsIssuerConfig)
    {}

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }
};
