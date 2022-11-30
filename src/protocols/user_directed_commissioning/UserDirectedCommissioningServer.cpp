/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements an object for a Matter User Directed Commissioning unsolicited
 *      recipient (server).
 *
 */

#include "UserDirectedCommissioning.h"
#include <lib/core/CHIPSafeCasts.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

void UserDirectedCommissioningServer::OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msg)
{
    ChipLogProgress(AppServer, "UserDirectedCommissioningServer::OnMessageReceived");

    PacketHeader packetHeader;

    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    if (packetHeader.IsEncrypted())
    {
        ChipLogError(AppServer, "UDC encryption flag set - ignoring");
        return;
    }

    PayloadHeader payloadHeader;
    ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));

    char instanceName[Dnssd::Commission::kInstanceNameMaxLength + 1];
    size_t instanceNameLength = std::min<size_t>(msg->DataLength(), Dnssd::Commission::kInstanceNameMaxLength);
    msg->Read(Uint8::from_char(instanceName), instanceNameLength);

    instanceName[instanceNameLength] = '\0';

    ChipLogProgress(AppServer, "UDC instance=%s", instanceName);

    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName);
    if (client == nullptr)
    {
        ChipLogProgress(AppServer, "UDC new instance state received");

        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "UDC error creating new connection state");
            return;
        }

        // Call the registered InstanceNameResolver, if any.
        if (mInstanceNameResolver != nullptr)
        {
            mInstanceNameResolver->FindCommissionableNode(instanceName);
        }
        else
        {
            ChipLogError(AppServer, "UserDirectedCommissioningServer::OnMessageReceived no mInstanceNameResolver registered");
        }
    }

    mUdcClients.MarkUDCClientActive(client);
}

void UserDirectedCommissioningServer::SetUDCClientProcessingState(char * instanceName, UDCClientProcessingState state)
{
    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName);
    if (client == nullptr)
    {
        // printf("SetUDCClientProcessingState new instance state received\n");
        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer,
                         "UserDirectedCommissioningServer::SetUDCClientProcessingState error creating new connection state");
            return;
        }
    }

    ChipLogDetail(AppServer, "SetUDCClientProcessingState instance=%s new state=%d", StringOrNullMarker(instanceName), (int) state);

    client->SetUDCClientProcessingState(state);

    mUdcClients.MarkUDCClientActive(client);
}

void UserDirectedCommissioningServer::OnCommissionableNodeFound(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (nodeData.resolutionData.numIPs == 0)
    {
        ChipLogError(AppServer, "OnCommissionableNodeFound no IP addresses returned for instance name=%s",
                     nodeData.commissionData.instanceName);
        return;
    }
    if (nodeData.resolutionData.port == 0)
    {
        ChipLogError(AppServer, "OnCommissionableNodeFound no port returned for instance name=%s",
                     nodeData.commissionData.instanceName);
        return;
    }

    UDCClientState * client = mUdcClients.FindUDCClientState(nodeData.commissionData.instanceName);
    if (client != nullptr && client->GetUDCClientProcessingState() == UDCClientProcessingState::kDiscoveringNode)
    {
        ChipLogDetail(AppServer, "OnCommissionableNodeFound instance: name=%s old_state=%d new_state=%d", client->GetInstanceName(),
                      (int) client->GetUDCClientProcessingState(), (int) UDCClientProcessingState::kPromptingUser);
        client->SetUDCClientProcessingState(UDCClientProcessingState::kPromptingUser);

#if INET_CONFIG_ENABLE_IPV4
        // prefer IPv4 if its an option
        bool foundV4 = false;
        for (unsigned i = 0; i < nodeData.resolutionData.numIPs; ++i)
        {
            if (nodeData.resolutionData.ipAddress[i].IsIPv4())
            {
                foundV4 = true;
                client->SetPeerAddress(
                    chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[i], nodeData.resolutionData.port));
                break;
            }
        }
        // use IPv6 as last resort
        if (!foundV4)
        {
            client->SetPeerAddress(
                chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], nodeData.resolutionData.port));
        }
#else  // INET_CONFIG_ENABLE_IPV4
       // if we only support V6, then try to find a v6 address
        bool foundV6 = false;
        for (unsigned i = 0; i < nodeData.resolutionData.numIPs; ++i)
        {
            if (nodeData.resolutionData.ipAddress[i].IsIPv6())
            {
                foundV6 = true;
                client->SetPeerAddress(
                    chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[i], nodeData.resolutionData.port));
                break;
            }
        }
        // last resort, try with what we have
        if (!foundV6)
        {
            ChipLogError(AppServer, "OnCommissionableNodeFound no v6 returned for instance name=%s",
                         nodeData.commissionData.instanceName);
            client->SetPeerAddress(
                chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], nodeData.resolutionData.port));
        }
#endif // INET_CONFIG_ENABLE_IPV4

        client->SetDeviceName(nodeData.commissionData.deviceName);
        client->SetLongDiscriminator(nodeData.commissionData.longDiscriminator);
        client->SetVendorId(nodeData.commissionData.vendorId);
        client->SetProductId(nodeData.commissionData.productId);
        client->SetRotatingId(nodeData.commissionData.rotatingId, nodeData.commissionData.rotatingIdLen);

        // Call the registered mUserConfirmationProvider, if any.
        if (mUserConfirmationProvider != nullptr)
        {
            mUserConfirmationProvider->OnUserDirectedCommissioningRequest(*client);
        }
    }
}

void UserDirectedCommissioningServer::PrintUDCClients()
{
    for (uint8_t i = 0; i < kMaxUDCClients; i++)
    {
        UDCClientState * state = GetUDCClients().GetUDCClientState(i);
        if (state == nullptr)
        {
            ChipLogProgress(AppServer, "UDC Client[%d] null", i);
        }
        else
        {
            char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
            state->GetPeerAddress().ToString(addrBuffer);

            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(state->GetRotatingId(), chip::Dnssd::kMaxRotatingIdLen, rotatingIdString,
                                                sizeof(rotatingIdString));

            ChipLogProgress(AppServer, "UDC Client[%d] instance=%s deviceName=%s address=%s, vid/pid=%d/%d disc=%d rid=%s", i,
                            state->GetInstanceName(), state->GetDeviceName(), addrBuffer, state->GetVendorId(),
                            state->GetProductId(), state->GetLongDiscriminator(), rotatingIdString);
        }
    }
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
