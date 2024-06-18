/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include <commands/common/CHIPCommand.h>
#include <commands/pairing/OpenCommissioningWindowCommand.h>
#include <commands/pairing/PairingCommand.h>

constexpr uint32_t kSetupPinCode               = 20202021;
constexpr uint16_t kMaxCommandSize             = 64;
constexpr uint16_t kDiscriminator              = 3840;
constexpr uint16_t kWindowTimeout              = 300;
constexpr uint16_t kIteration                  = 1000;
constexpr uint16_t kAggragatorEndpointId       = 1;
constexpr uint8_t kEnhancedCommissioningMethod = 1;

class FabricSyncAddBridgeCommand : public CHIPCommand, public CommissioningDelegate
{
public:
    FabricSyncAddBridgeCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("add-bridge", credIssuerCommands)
    {
        AddArgument("nodeid", 0, UINT64_MAX, &mNodeId);
    }

    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mNodeId); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mNodeId;
    chip::NodeId mBridgeNodeId;

    CHIP_ERROR RunCommand(NodeId remoteId);
};

class FabricSyncRemoveBridgeCommand : public CHIPCommand, public PairingDelegate
{
public:
    FabricSyncRemoveBridgeCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("remove-bridge", credIssuerCommands)
    {}

    void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mBridgeNodeId;
};

class FabricSyncDeviceCommand : public CHIPCommand, public CommissioningWindowDelegate, public CommissioningDelegate
{
public:
    FabricSyncDeviceCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("sync-device", credIssuerCommands)
    {
        AddArgument("endpointid", 0, UINT16_MAX, &mRemoteEndpointId);
    }

    void OnCommissioningWindowOpened(NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mRemoteEndpointId); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::EndpointId mRemoteEndpointId = chip::kInvalidEndpointId;
    chip::NodeId mAssignedNodeId       = chip::kUndefinedNodeId;

    CHIP_ERROR RunCommand(chip::EndpointId remoteId);
};

class FabricAutoSyncCommand : public CHIPCommand
{
public:
    FabricAutoSyncCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("enable-auto-sync", credIssuerCommands)
    {
        AddArgument("state", 0, 1, &mEnableAutoSync, "Set to true to enable auto Fabric Sync, false to disable.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mEnableAutoSync); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    bool mEnableAutoSync;

    CHIP_ERROR RunCommand(bool enableAutoSync);
};
