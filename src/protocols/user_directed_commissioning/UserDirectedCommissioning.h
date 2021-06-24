/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines objects for a CHIP Echo unsolicitied
 *      initaitor (client) and responder (server).
 *
 */

#pragma once

#include "UDCClients.h"
#include <core/CHIPCore.h>
#include <mdns/Resolver.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

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
    virtual void FindCommissionableNode(Messaging::ExchangeContext * ec, char * instanceName) {}

    virtual ~InstanceNameResolver() {}
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
     * @param nodeData DNS-SD node information for the client requesting commissioning
     *
     */
    virtual void OnUserDirectedCommissioningRequest(const Mdns::CommissionableNodeData & nodeData) {}

    virtual ~UserConfirmationProvider() {}
};

class DLL_EXPORT UserDirectedCommissioningClient : public Messaging::ExchangeDelegate
{
public:
    // TODO: Init function will take a Channel instead a SecureSessionHandle, when Channel API is ready
    /**
     *  Initialize the UserDirectedCommissioningClient object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    exchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    sessoin        A handle to the session.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session);

    /**
     *  Shutdown the UserDirectedCommissioningClient. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    /**
     * Send a User Directed Commissioning message to a CHIP node.
     *
     * @param payload       A PacketBufferHandle with the payload.
     * @param sendFlags     Flags set by the application for the CHIP message being sent.
     *
     * @return CHIP_ERROR_NO_MEMORY if no ExchangeContext is available.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */
    CHIP_ERROR
    SendUDCRequest(System::PacketBufferHandle && payload,
                   const Messaging::SendFlags & sendFlags = Messaging::SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));

private:
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    Messaging::ExchangeContext * mExchangeCtx = nullptr;
    SecureSessionHandle mSecureSession;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
};

class DLL_EXPORT UserDirectedCommissioningServer : public Messaging::ExchangeDelegate
{
public:
    /**
     *  Initialize the UserDirectedCommissioningServer object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    exchangeMgr    A pointer to the ExchangeManager object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);

    static UserDirectedCommissioningServer & GetInstance()
    {
        static UserDirectedCommissioningServer instance;
        return instance;
    }

    /**
     *  Shutdown the EchoServer. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

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
    void OnCommissionableNodeFound(const Mdns::CommissionableNodeData & nodeData);

private:
    Messaging::ExchangeManager * mExchangeMgr            = nullptr;
    InstanceNameResolver * mInstanceNameResolver         = nullptr;
    UserConfirmationProvider * mUserConfirmationProvider = nullptr;

    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}

    UDCClients<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mUdcClients; // < Active UDC clients
};

} // namespace UserDirectedCommissioning

template <>
struct MessageTypeTraits<UserDirectedCommissioning::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return UserDirectedCommissioning::Id; }
};

} // namespace Protocols
} // namespace chip
