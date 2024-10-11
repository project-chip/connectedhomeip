/*
 *   Copyright (c) 2021 Project CHIP Authors
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

class DiscoverCommissionablesCommandBase : public CHIPCommand, public chip::Controller::DeviceDiscoveryDelegate
{
public:
    DiscoverCommissionablesCommandBase(const char * name, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(name, credsIssuerConfig)
    {
        AddArgument("discover-once", 0, 1, &mDiscoverOnce,
                    "Boolean indicating whether to stop discovery after the first result. Defaults to true.");
    }

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData & nodeData) override;

    /////////// CHIPCommand Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

protected:
    chip::Controller::DeviceCommissioner * mCommissioner;

private:
    chip::Optional<bool> mDiscoverOnce;
};

class DiscoverCommissionablesStartCommand : public CHIPCommand
{
public:
    DiscoverCommissionablesStartCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("start", credIssuerCommands) {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }
};

class DiscoverCommissionablesStopCommand : public CHIPCommand
{
public:
    DiscoverCommissionablesStopCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("stop", credIssuerCommands) {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }
};

class DiscoverCommissionablesListCommand : public CHIPCommand
{
public:
    DiscoverCommissionablesListCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("list", credIssuerCommands) {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }
};

class DiscoverCommissionablesCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionablesCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("commissionables", credsIssuerConfig)
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
};

class DiscoverCommissionableByShortDiscriminatorCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByShortDiscriminatorCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-short-discriminator", credsIssuerConfig)
    {
        AddArgument("value", 0, UINT16_MAX, &mDiscriminator);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    uint16_t mDiscriminator;
};

class DiscoverCommissionableByLongDiscriminatorCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByLongDiscriminatorCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-long-discriminator", credsIssuerConfig)
    {
        AddArgument("value", 0, UINT16_MAX, &mDiscriminator);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    uint16_t mDiscriminator;
};

class DiscoverCommissionableByCommissioningModeCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByCommissioningModeCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-commissioning-mode", credsIssuerConfig)
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
};

class DiscoverCommissionableByVendorIdCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByVendorIdCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-vendor-id", credsIssuerConfig)
    {
        AddArgument("value", 0, UINT16_MAX, &mVendorId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    uint16_t mVendorId;
};

class DiscoverCommissionableByDeviceTypeCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByDeviceTypeCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-device-type", credsIssuerConfig)
    {
        AddArgument("value", 0, UINT16_MAX, &mDeviceType);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    // TODO: possibly 32-bit - see spec issue #3226
    uint16_t mDeviceType;
};

class DiscoverCommissionableByInstanceNameCommand : public DiscoverCommissionablesCommandBase
{
public:
    DiscoverCommissionableByInstanceNameCommand(CredentialIssuerCommands * credsIssuerConfig) :
        DiscoverCommissionablesCommandBase("find-commissionable-by-instance-name", credsIssuerConfig)
    {
        AddArgument("value", &mInstanceName);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

private:
    char * mInstanceName;
};
