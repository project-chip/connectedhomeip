/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines objects for a User-Directed Commissioning unsolicited
 *      initiator (client) and recipient (server).
 *
 */

#pragma once

#include "UDCClients.h"
#include <lib/core/CHIPCore.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <transport/TransportMgr.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

constexpr const char * kProtocolName = "UserDirectedCommissioning";

// Cache contains 16 clients. This may need to be tweaked.
constexpr uint8_t kMaxUDCClients = 16;

/**
 * User Directed Commissioning Protocol Message Types
 */
enum class MsgType : uint8_t
{
    IdentificationDeclaration = 0x00,
};

class DLL_EXPORT InstanceNameResolver
{
public:
    /**
     * @brief
     *   Called when a UDC message is received specifying the given instanceName
     * This method indicates that UDC Server needs the Commissionable Node corresponding to
     * the given instance name to be found. UDC Server will wait for OnCommissionableNodeFound.
     *
     * @param instanceName DNS-SD instance name for the client requesting commissioning
     *
     */
    virtual void FindCommissionableNode(char * instanceName) = 0;

    virtual ~InstanceNameResolver() = default;
};

class DLL_EXPORT UserConfirmationProvider
{
public:
    /**
     * @brief
     *   Called when a UDC message has been received and corresponding nodeData has been found.
     * It is expected that the implementer will prompt the user to confirm their intention to
     * commission the given node, and obtain the setup code to allow commissioning to proceed,
     * and then invoke commissioning on the given Node (using CHIP Device Controller, for example)
     *
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    virtual void OnUserDirectedCommissioningRequest(UDCClientState state) = 0;

    virtual ~UserConfirmationProvider() = default;
};

class DLL_EXPORT UserDirectedCommissioningClient
{
public:
    /**
     * Send a User Directed Commissioning message to a CHIP node.
     *
     * @param transportMgr  A transport to use for sending the message.
     * @param payload       A PacketBufferHandle with the payload.
     * @param peerAddress   Address of destination.
     *
     * @return CHIP_ERROR_NO_MEMORY if allocation fails.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */

    CHIP_ERROR SendUDCMessage(TransportMgrBase * transportMgr, System::PacketBufferHandle && payload,
                              chip::Transport::PeerAddress peerAddress);

    /**
     * Encode a User Directed Commissioning message.
     *
     * @param payload       A PacketBufferHandle with the payload.
     *
     * @return CHIP_ERROR_NO_MEMORY if allocation fails.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */

    CHIP_ERROR EncodeUDCMessage(const System::PacketBufferHandle & payload);
};

class DLL_EXPORT UserDirectedCommissioningServer : public TransportMgrDelegate
{
public:
    /**
     * Set the listener to be called when a UDC request is received
     * and the Instance Name provided needs to be resolved.
     *
     * The resolver should call OnCommissionableNodeFound when the instance is found
     *
     *  @param[in]    instanceNameResolver    The callback function to receive UDC request instance name.
     *
     */
    void SetInstanceNameResolver(InstanceNameResolver * instanceNameResolver) { mInstanceNameResolver = instanceNameResolver; }

    /**
     * Set the listener to be called when a UDC request is received
     * and the Instance Name has been resolved.
     *
     * The provider should prompt the user to allow commissioning of the node and provide the setup code.
     *
     *  @param[in]    userConfirmationProvider    The callback function to obtain user confirmation.
     *
     */
    void SetUserConfirmationProvider(UserConfirmationProvider * userConfirmationProvider)
    {
        mUserConfirmationProvider = userConfirmationProvider;
    }

    /**
     * Update the processing state for a UDC Client based upon instance name.
     *
     * This can be used by the UX to set the state to one of the following values:
     * - kUserDeclined
     * - kObtainingOnboardingPayload
     * - kCommissioningNode
     * - kCommissioningFailed
     *
     *  @param[in]    instanceName    The instance name for the UDC Client.
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    void SetUDCClientProcessingState(char * instanceName, UDCClientProcessingState state);

    /**
     * Reset the processing states for all UDC Clients
     *
     */
    void ResetUDCClientProcessingStates() { mUdcClients.ResetUDCClientStates(); }

    /**
     * Called when a CHIP Node in commissioning mode is found.
     *
     * Lookup instanceName from nodeData in the active UDC Client states
     * and if current state is kDiscoveringNode then change to kPromptingUser and
     * call UX Prompt callback
     *
     *  @param[in]    nodeData        DNS-SD response data.
     *
     */
    void OnCommissionableNodeFound(const Dnssd::DiscoveredNodeData & nodeData);

    /**
     * Get the cache of UDC Clients
     *
     */
    UDCClients<kMaxUDCClients> & GetUDCClients() { return mUdcClients; }

    /**
     * Print the cache of UDC Clients
     *
     */
    void PrintUDCClients();

private:
    InstanceNameResolver * mInstanceNameResolver         = nullptr;
    UserConfirmationProvider * mUserConfirmationProvider = nullptr;

    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override;

    UDCClients<kMaxUDCClients> mUdcClients; // < Active UDC clients
};

} // namespace UserDirectedCommissioning

template <>
struct MessageTypeTraits<UserDirectedCommissioning::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return UserDirectedCommissioning::Id; }

    static auto GetTypeToNameTable()
    {
        static const std::array<MessageTypeNameLookup, 1> typeToNameTable = {
            {
                { UserDirectedCommissioning::MsgType::IdentificationDeclaration, "IdentificationDeclaration" },
            },
        };

        return &typeToNameTable;
    }
};

} // namespace Protocols
} // namespace chip
