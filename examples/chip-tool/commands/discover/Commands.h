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
#include "DiscoverCommissionablesCommand.h"
#include "DiscoverCommissionersCommand.h"
#include <controller/DeviceAddressUpdateDelegate.h>
#include <lib/mdns/Resolver.h>

constexpr uint16_t kMdnsPort = 5353;

class Resolve : public DiscoverCommand, public chip::Mdns::ResolverDelegate
{
public:
    Resolve() : DiscoverCommand("resolve") {}

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().StartResolver(&chip::DeviceLayer::InetLayer, kMdnsPort));
        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().SetResolverDelegate(nullptr));
        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().SetResolverDelegate(this));
        ChipLogProgress(chipTool, "Mdns: Searching for NodeId: %" PRIx64 " FabricId: %" PRIx64 " ...", remoteId, fabricId);
        return chip::Mdns::Resolver::Instance().ResolveNodeId(chip::PeerId().SetNodeId(remoteId).SetCompressedFabricId(fabricId),
                                                              chip::Inet::kIPAddressType_Any);
    }

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
        nodeData.mAddress.ToString(addrBuffer);
        ChipLogProgress(chipTool, "NodeId Resolution: %" PRIu64 " Address: %s, Port: %" PRIu16, nodeData.mPeerId.GetNodeId(),
                        addrBuffer, nodeData.mPort);
        ChipLogProgress(chipTool, "    Hostname: %s", nodeData.mHostName);

        auto retryInterval = nodeData.GetMrpRetryIntervalIdle();

        if (retryInterval.HasValue())
            ChipLogProgress(chipTool, "   MRP retry interval (idle): %" PRIu32 "ms", retryInterval.Value());

        retryInterval = nodeData.GetMrpRetryIntervalActive();

        if (retryInterval.HasValue())
            ChipLogProgress(chipTool, "   MRP retry interval (active): %" PRIu32 "ms", retryInterval.Value());

        ChipLogProgress(chipTool, "   Supports TCP: %s", nodeData.mSupportsTcp ? "yes" : "no");
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "NodeId Resolution: failed!");
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }
    void OnNodeDiscoveryComplete(const chip::Mdns::DiscoveredNodeData & nodeData) override {}
};

class Update : public DiscoverCommand
{
public:
    Update() : DiscoverCommand("update") {}

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ChipLogProgress(chipTool, "Mdns: Updating NodeId: %" PRIx64 " Compressed FabricId: %" PRIx64 " ...", remoteId,
                        GetExecContext()->commissioner->GetCompressedFabricId());
        return GetExecContext()->commissioner->UpdateDevice(remoteId);
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

        SetCommandExitStatus(error);
    }
};

void registerCommandsDiscover(Commands & commands)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<Resolve>(),
        make_unique<Update>(),
        make_unique<DiscoverCommissionablesCommand>(),
        make_unique<DiscoverCommissionersCommand>(),
    };

    commands.Register(clusterName, clusterCommands);
}
