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
 *      This file defines the classes corresponding to CHIP Exchange management.
 *
 */

#pragma once

#include <support/DLLUtil.h>
#include <messaging/ExchangeContext.h>
#include <transport/SecureSessionMgr.h>

#define EXCHANGE_CONTEXT_ID(x) ((x) + 1)

using namespace chip::Transport;

namespace chip {

using System::PacketBuffer;

class ExchangeContext;

/**
 *  @class ExchangeManager
 *
 *  @brief
 *    This class is used to manage ExchangeContexts with other CHIP nodes.
 *    It works on behalf of higher layers, creating ExchangeContexts and
 *    handling the registration/unregistration of unsolicited message handlers.
 *
 */
class DLL_EXPORT ExchangeManager : public SecureSessionMgrCallback
{
    friend class ExchangeContext;

public:
    enum State
    {
        kState_NotInitialized = 0, /**< Used to indicate that the ExchangeManager is not initialized */
        kState_Initialized    = 1  /**< Used to indicate that the ExchangeManager is initialized */
    };

    ExchangeManager();
    ExchangeManager(const ExchangeManager &) = delete;
    ExchangeManager operator=(const ExchangeManager &) = delete;

    /**
     *  Initialize the ExchangeManager object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    sessionMgr    A pointer to the SecureSessionMgrBase object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(SecureSessionMgrBase * sessionMgr);

    /**
     *  Shutdown the ExchangeManager. This terminates this instance
     *  of the object and releases all held resources.
     *
     *  @note
     *     The application should only call this function after ensuring that
     *     there are no active ExchangeContext objects. Furthermore, it is the
     *     onus of the application to de-allocate the ExchangeManager
     *     object after calling ExchangeManager::Shutdown().
     *
     *  @return #CHIP_NO_ERROR unconditionally.
     *
     */
    CHIP_ERROR Shutdown();

    /**
     *  Creates a new ExchangeContext with a given peer CHIP node specified by the peer node identifier.
     *
     *  @param[in]    peerNodeId    The node identifier of the peer with which the ExchangeContext is being set up.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @return   A pointer to the created ExchangeContext object On success. Otherwise NULL if no object
     *            can be allocated or is available.
     *
     */
    ExchangeContext * NewContext(const uint64_t & peerNodeId, void * appState = nullptr);

    /**
     *  Find the ExchangeContext from a pool matching a given set of parameters.
     *
     *  @param[in]    peerNodeId    The node identifier of the peer with which the ExchangeContext has been set up.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @param[in]    isInitiator   Boolean indicator of whether the local node is the initiator of the exchange.
     *
     *  @return   A pointer to the ExchangeContext object matching the provided parameters On success, NULL on no match.
     *
     */
    ExchangeContext * FindContext(uint64_t peerNodeId, void * appState, bool isInitiator);

    /**
     *  Register an unsolicited message handler for a given profile identifier. This handler would be
     *  invoked for all messages of the given profile.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @param[in]    handler       The unsolicited message handler.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeContext::MessageReceiveFunct handler,
                                                 void * appState);

    /**
     *  Register an unsolicited message handler for a given profile identifier. This handler would be invoked for all messages of the
     * given profile.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @param[in]    handler       The unsolicited message handler.
     *
     *  @param[in]    allowDups     Boolean indicator of whether duplicate messages are allowed for a given profile.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeContext::MessageReceiveFunct handler, bool allowDups,
                                                 void * appState);

    /**
     *  Register an unsolicited message handler for a given profile identifier and message type.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @param[in]    handler       The unsolicited message handler.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 void * appState);

    /**
     *  Register an unsolicited message handler for a given profile identifier and message type.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @param[in]    handler       The unsolicited message handler.
     *
     *  @param[in]    allowDups     Boolean indicator of whether duplicate messages are allowed for a given
     *                              profile identifier and message type.
     *
     *  @param[in]    appState      A pointer to a higher layer object that holds context state.
     *
     *  @retval #CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS If the unsolicited message handler pool
     *                                                             is full and a new one cannot be allocated.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 bool allowDups, void * appState);

    /**
     *  Unregister an unsolicited message handler for a given profile identifier.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @retval #CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER  If the matching unsolicited message handler
     *                                                       is not found.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId);

    /**
     *  Unregister an unsolicited message handler for a given profile identifier and message type.
     *
     *  @param[in]    protocolId     The profile identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @retval #CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER  If the matching unsolicited message handler
     *                                                       is not found.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType);

private:
    uint16_t NextExchangeId;
    uint8_t  mState;                      /**< [READ ONLY] The state of the ExchangeManager object. */
    SecureSessionMgrBase * mSessionMgr;   /**< [READ ONLY] The associated SecureSessionMgrBase object. */

    class UnsolicitedMessageHandler
    {
    public:
        ExchangeContext::MessageReceiveFunct Handler;
        void * AppState;
        uint32_t ProtocolId;
        int16_t MessageType; // -1 represents any message type
        bool AllowDuplicateMsgs;
    };

    ExchangeContext ContextPool[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS];
    size_t mContextsInUse;

    UnsolicitedMessageHandler UMHandlerPool[CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];
    void (*OnExchangeContextChanged)(size_t numContextsInUse);

    ExchangeContext * AllocContext();

    void DispatchMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf);
    CHIP_ERROR RegisterUMH(uint32_t protocolId, int16_t msgType, bool allowDups, ExchangeContext::MessageReceiveFunct handler,
                           void * appState);
    CHIP_ERROR UnregisterUMH(uint32_t protocolId, int16_t msgType);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * msgLayer) override;

    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                           SecureSessionMgrBase * msgLayer) override;
};

} // namespace chip
