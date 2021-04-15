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

#include "DiscoverCommand.h"
#include <controller/DeviceAddressUpdateDelegate.h>
#include <mdns/Resolver.h>

class Resolve : public DiscoverCommand, public chip::Mdns::ResolverDelegate
{
public:
    Resolve() : DiscoverCommand("resolve") {}

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().SetResolverDelegate(this));
        return chip::Mdns::Resolver::Instance().ResolveNodeId(remoteId, fabricId, chip::Inet::kIPAddressType_Any);
    }

    /////////// ResolverDelegate Interface /////////
    void OnNodeIdResolved(NodeId nodeId, const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
        nodeData.mAddress.ToString(addrBuffer);
        ChipLogProgress(chipTool, "NodeId Resolution: %" PRIu64 " Address: %s, Port: %" PRIu16, nodeId, addrBuffer, nodeData.mPort);
        SetCommandExitStatus(true);
    };

    void OnNodeIdResolutionFailed(NodeId nodeId, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "NodeId Resolution: failed!");
        SetCommandExitStatus(false);
    };
};

class Update : public DiscoverCommand
{
public:
    Update() : DiscoverCommand("update") {}

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ChipDevice * device;
        ReturnErrorOnFailure(mCommissioner.GetDevice(remoteId, &device));
        return mCommissioner.UpdateDevice(device, fabricId);
    }

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override
    {
        if (CHIP_NO_ERROR == error)
        {
            ChipLogProgress(chipTool, "Device address updated successfully");
        }
        else
        {
            ChipLogError(chipTool, "Failed to update the device address: %s", chip::ErrorStr(error));
        }

        SetCommandExitStatus(CHIP_NO_ERROR == error);
    }
};

void registerCommandsDiscover(Commands & commands)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<Resolve>(),
        make_unique<Update>(),
    };

    commands.Register(clusterName, clusterCommands);
}
