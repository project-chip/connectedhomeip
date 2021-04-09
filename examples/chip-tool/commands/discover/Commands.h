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

#include "Command.h"
#include <mdns/Resolver.h>

class Discover : public Command, public chip::Mdns::ResolverDelegate
{
public:
    Discover() : Command("resolve-node-id")
    {
        AddArgument("nodeid", 0, UINT64_MAX, &mNodeId);
        AddArgument("fabricid", 0, UINT64_MAX, &mFabricId);
    }

    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override
    {
        ReturnErrorOnFailure(mCommissioner.SetUdpListenPort(storage.GetListenPort()));
        ReturnErrorOnFailure(mCommissioner.Init(localId, &storage));
        ReturnErrorOnFailure(mCommissioner.ServiceEvents());

        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().SetResolverDelegate(this));
        ReturnErrorOnFailure(chip::Mdns::Resolver::Instance().ResolveNodeId(
            chip::PeerId().SetNodeId(mNodeId).SetFabricId(mFabricId), chip::Inet::kIPAddressType_Any));

        UpdateWaitForResponse(true);
        WaitForResponse(mWaitDurationInSeconds);

        mCommissioner.ServiceEventSignal();
        mCommissioner.Shutdown();

        VerifyOrReturnError(GetCommandExitStatus(), CHIP_ERROR_INTERNAL);

        return CHIP_NO_ERROR;
    }

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
        nodeData.mAddress.ToString(addrBuffer);
        ChipLogProgress(chipTool, "NodeId Resolution: %" PRIu64 " Address: %s, Port: %" PRIu16, nodeData.mPeerId.GetNodeId(),
                        addrBuffer, nodeData.mPort);
        SetCommandExitStatus(true);
    };

    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "NodeId Resolution: failed!");
        SetCommandExitStatus(false);
    };

private:
    uint16_t mWaitDurationInSeconds = 30;
    ChipDeviceCommissioner mCommissioner;
    chip::NodeId mNodeId;
    uint64_t mFabricId;
};

void registerCommandsDiscover(Commands & commands)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<Discover>(),
    };

    commands.Register(clusterName, clusterCommands);
}
