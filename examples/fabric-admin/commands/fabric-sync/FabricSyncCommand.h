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
#include <device_manager/PairingManager.h>

// Constants
constexpr uint32_t kCommissionPrepareTimeMs = 500;

class FabricSyncAddBridgeCommand : public CHIPCommand, public CommissioningDelegate
{
public:
    FabricSyncAddBridgeCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("add-bridge", credIssuerCommands)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("setup-pin-code", 0, 0x7FFFFFF, &mSetupPINCode, "Setup PIN code for the remote bridge device.");
        AddArgument("device-remote-ip", &mRemoteAddr, "The IP address of the remote bridge device.");
        AddArgument("device-remote-port", 0, UINT16_MAX, &mRemotePort, "The secured device port of the remote bridge device.");
    }

    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mNodeId); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mNodeId;
    chip::NodeId mBridgeNodeId;
    uint32_t mSetupPINCode;
    chip::ByteSpan mRemoteAddr;
    uint16_t mRemotePort;

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

class FabricSyncAddLocalBridgeCommand : public CHIPCommand, public CommissioningDelegate
{
public:
    FabricSyncAddLocalBridgeCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("add-local-bridge", credIssuerCommands)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("setup-pin-code", 0, 0x7FFFFFF, &mSetupPINCode, "Setup PIN code for the local bridge device.");
        AddArgument("local-port", 0, UINT16_MAX, &mLocalPort, "The secured device port of the local bridge device.");
    }

    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mNodeId); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mNodeId;
    chip::Optional<uint32_t> mSetupPINCode;
    chip::Optional<uint16_t> mLocalPort;
    chip::NodeId mLocalBridgeNodeId;

    CHIP_ERROR RunCommand(chip::NodeId deviceId);
};

class FabricSyncRemoveLocalBridgeCommand : public CHIPCommand, public PairingDelegate
{
public:
    FabricSyncRemoveLocalBridgeCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("remove-local-bridge", credIssuerCommands)
    {}

    void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) override;

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(1); }

private:
    chip::NodeId mLocalBridgeNodeId;
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
