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

#include "commands/common/Commands.h"
#include "commands/discover/DiscoverCommand.h"
#include "commands/discover/DiscoverCommissionablesCommand.h"
#include "commands/discover/DiscoverCommissionersCommand.h"
#include <lib/address_resolve/AddressResolve.h>

class Resolve : public DiscoverCommand, public chip::AddressResolve::NodeListener
{
public:
    Resolve(CredentialIssuerCommands * credsIssuerConfig) : DiscoverCommand("resolve", credsIssuerConfig)
    {
        mNodeLookupHandle.SetListener(this);
    }

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ReturnErrorOnFailure(chip::AddressResolve::Resolver::Instance().Init(&chip::DeviceLayer::SystemLayer()));

        return chip::AddressResolve::Resolver::Instance().LookupNode(
            chip::AddressResolve::NodeLookupRequest(chip::PeerId().SetNodeId(remoteId).SetCompressedFabricId(fabricId)),
            mNodeLookupHandle);
    }

    void OnNodeAddressResolved(const PeerId & peerId, const chip::AddressResolve::ResolveResult & result) override
    {
        char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];

        result.address.ToString(addrBuffer);

        ChipLogProgress(chipTool, "NodeId Resolution: %" PRIu64 " at %s", peerId.GetNodeId(), addrBuffer);
        ChipLogProgress(chipTool, "   MRP retry interval (idle): %" PRIu32 "ms",
                        result.mrpRemoteConfig.mIdleRetransTimeout.count());
        ChipLogProgress(chipTool, "   MRP retry interval (active): %" PRIu32 "ms",
                        result.mrpRemoteConfig.mActiveRetransTimeout.count());
        ChipLogProgress(chipTool, "   Supports TCP Client: %s", result.supportsTcpClient ? "yes" : "no");
        ChipLogProgress(chipTool, "   Supports TCP Server: %s", result.supportsTcpServer ? "yes" : "no");
        ChipLogProgress(chipTool, "   ICD is operating as: %s", result.isICDOperatingAsLIT ? "LIT" : "SIT");
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    void OnNodeAddressResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "NodeId %" PRIu64 " Resolution: failed!", peerId.GetNodeId());
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }

private:
    chip::AddressResolve::NodeLookupHandle mNodeLookupHandle;
};

void registerCommandsDiscover(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<Resolve>(credsIssuerConfig),
        make_unique<DiscoverCommissionablesStartCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionablesStopCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionablesListCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionablesCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByShortDiscriminatorCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByLongDiscriminatorCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByCommissioningModeCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByVendorIdCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByDeviceTypeCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionableByInstanceNameCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionersCommand>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for device discovery.");
}
