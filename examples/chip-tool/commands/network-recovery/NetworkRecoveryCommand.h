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

#pragma once

#include "../common/CHIPCommand.h"
#include <controller/NetworkRecover.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

enum class RecoveryNetworkType
{
    None,
    WiFi,
    Thread,
};

class NetworkRecoveryCommandBase : public CHIPCommand, public chip::Controller::NetworkRecoverDelegate
{
public:
    NetworkRecoveryCommandBase(const char * name, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(name, credsIssuerConfig)
    {}
    /////////// NetworkRecoverDelegate Interface /////////
    void OnNetworkRecoverDiscover(std::list<uint64_t> recoveryIds) override;
    void OnNetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error) override;

    /////////// CHIPCommand Interface /////////
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

protected:
    chip::Controller::DeviceCommissioner * mCommissioner;
};

class NetworkRecoveryDiscoverCommand : public NetworkRecoveryCommandBase
{
public:
    NetworkRecoveryDiscoverCommand(CredentialIssuerCommands * credsIssuerConfig) :
        NetworkRecoveryCommandBase("discover", credsIssuerConfig)
    {
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeout.ValueOr(30) + 10);
    }

private:
    chip::Optional<uint16_t> mTimeout;
};

class NetworkRecoveryRecoverCommand : public NetworkRecoveryCommandBase
{
public:
    NetworkRecoveryRecoverCommand(const char * commandName, RecoveryNetworkType networkType,
                                  CredentialIssuerCommands * credsIssuerConfig) :
        NetworkRecoveryCommandBase(commandName, credsIssuerConfig),
        mNetworkType(networkType)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("recovery-identifier", 0, UINT64_MAX, &mRecoveryId);

        switch (mNetworkType)
        {
        case RecoveryNetworkType::WiFi:
            AddArgument("ssid", &mSSID);
            AddArgument("password", &mPassword);
            break;
        case RecoveryNetworkType::Thread:
            AddArgument("operational-dataset", &mOperationalDataset);
            break;
        default:
            break;
        }
        AddArgument("breadcrumb", 0, UINT64_MAX, &mBreadcrumb);
    }

    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(70); }

private:
    chip::NodeId mNodeId;
    uint64_t mRecoveryId;
    RecoveryNetworkType mNetworkType;
    chip::ByteSpan mSSID;
    chip::ByteSpan mPassword;
    chip::ByteSpan mOperationalDataset;
    uint64_t mBreadcrumb;
    chip::Optional<uint16_t> mTimeout;
};
