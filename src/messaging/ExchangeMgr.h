/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Defines the CHIP ExchangeManager class and its supporting types
 *      for Exchange management.
 *
 */

#pragma once

#include <array>

#include <messaging/ExchangeContext.h>
#include <support/DLLUtil.h>
#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ExchangeContext;
class ExchangeDelegate;

static constexpr int16_t kAnyMessageType = -1;

/**
 *  @brief
 *    This class is used to manage ExchangeContexts with other CHIP nodes.
 *    It works on be behalf of higher layers, creating ExchangeContexts and
 *    handling the registration/unregistration of unsolicited message handlers.
 */
class DLL_EXPORT ExchangeManager : public SecureSessionMgrDelegate
{
public:
    ExchangeManager();
    ExchangeManager(const ExchangeManager &) = delete;
    ExchangeManager operator=(const ExchangeManager &) = delete;

    /**
     *  Initialize the ExchangeManager object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    sessionMgr    A pointer to the SecureSessionMgr object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(SecureSessionMgr * sessionMgr);

    /**
     *  Shutdown the ExchangeManager. This terminates this instance
     *  of the object and releases all held resources.
     *
     *  @note
     *     The protocol should only call this function after ensuring that
     *     there are no active ExchangeContext objects. Furthermore, it is the
     *     onus of the application to de-allocate the ExchangeManager
     *     object after calling ExchangeManager::Shutdown().
     *
     *  @return #CHIP_NO_ERROR unconditionally.
     */
    CHIP_ERROR Shutdown();

    /**
     *  Creates a new ExchangeContext with a given peer CHIP node specified by the peer node identifier.
     *
     *  @param[in]    peerNodeId    The node identifier of the peer with which the ExchangeContext is being set up.
     *
     *  @param[in]    delegate      A pointer to ExchangeDelegate.
     *
     *  @return   A pointer to the created ExchangeContext object On success. Otherwise NULL if no object
     *            can be allocated or is available.
     */
    ExchangeContext * NewContext(const NodeId & peerNodeId, ExchangeDelegate * delegate);

    /**
     *  Find the ExchangeContext from a pool matching a given set of parameters.
     *
     *  @param[in]    peerNodeId    The node identifier of the peer with which the ExchangeContext has been set up.
     *
     *  @param[in]    delegate      A pointer to ExchangeDelegate.
     *
     *  @param[in]    isInitiator   Boolean indicator of whether the local node is the initiator of the exchange.
     *
     *  @return   A pointer to the ExchangeContext object matching the provided parameters On success, NULL on no match.
     */
    ExchangeContext * FindContext(NodeId peerNodeId, ExchangeDelegate * delegate, bool isInitiator);

    /**
     *  Register an unsolicited message handler for a given protocol identifier. This handler would be
     *  invoked for all messages of the given protocol.
     *
     *  @param[in]    protocolId      The protocol identifier of the received message.
     *
     *  @param[in]    handler         The unsolicited message handler.
     *
     *  @param[in]    delegate        A pointer to ExchangeDelegate.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeDelegate * delegate);

    /**
     *  Register an unsolicited message handler for a given protocol identifier and message type.
     *
     *  @param[in]    protocolId      The protocol identifier of the received message.
     *
     *  @param[in]    msgType         The message type of the corresponding protocol.
     *
     *  @param[in]    delegate        A pointer to ExchangeDelegate.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeDelegate * delegate);

    /**
     *  Unregister an unsolicited message handler for a given protocol identifier.
     *
     *  @param[in]    protocolId     The protocol identifier of the received message.
     *
     *  @retval #CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER  If the matching unsolicited message handler
     *                                                       is not found.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId);

    /**
     *  Unregister an unsolicited message handler for a given protocol identifier and message type.
     *
     *  @param[in]    protocolId     The protocol identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding protocol.
     *
     *  @retval #CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER  If the matching unsolicited message handler
     *                                                       is not found.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType);

    void IncrementContextsInUse();
    void DecrementContextsInUse();

    SecureSessionMgr * GetSessionMgr() const { return mSessionMgr; }

    size_t GetContextsInUse() const { return mContextsInUse; }

private:
    enum class State
    {
        kState_NotInitialized = 0, // Used to indicate that the ExchangeManager is not initialized.
        kState_Initialized    = 1  // Used to indicate that the ExchangeManager is initialized.
    };

    struct UnsolicitedMessageHandler
    {
        ExchangeDelegate * Delegate;
        uint32_t ProtocolId;
        int16_t MessageType;
    };

    uint16_t mNextExchangeId;
    State mState;
    SecureSessionMgr * mSessionMgr;

    std::array<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> ContextPool;
    size_t mContextsInUse;

    UnsolicitedMessageHandler UMHandlerPool[CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];
    void (*OnExchangeContextChanged)(size_t numContextsInUse);

    ExchangeContext * AllocContext(uint16_t ExchangeId, uint64_t PeerNodeId, bool Initiator, ExchangeDelegate * delegate);

    void DispatchMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf);

    CHIP_ERROR RegisterUMH(uint32_t protocolId, int16_t msgType, ExchangeDelegate * delegate);
    CHIP_ERROR UnregisterUMH(uint32_t protocolId, int16_t msgType);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * msgLayer) override;

    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           const Transport::PeerConnectionState * state, System::PacketBufferHandle msgBuf,
                           SecureSessionMgr * msgLayer) override;

    void OnConnectionExpired(const Transport::PeerConnectionState * state, SecureSessionMgr * mgr) override;
};

} // namespace Messaging
} // namespace chip
