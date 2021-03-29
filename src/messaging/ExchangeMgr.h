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

#include <messaging/Channel.h>
#include <messaging/ChannelContext.h>
#include <messaging/ExchangeContext.h>
#include <messaging/MessageCounterSync.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <support/DLLUtil.h>
#include <support/Pool.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

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
class DLL_EXPORT ExchangeManager : public SecureSessionMgrDelegate, public TransportMgrDelegate
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
    ExchangeContext * NewContext(SecureSessionHandle session, ExchangeDelegate * delegate);

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
    CHIP_ERROR RegisterUnsolicitedMessageHandlerForProtocol(Protocols::Id protocolId, ExchangeDelegate * delegate);

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
    CHIP_ERROR RegisterUnsolicitedMessageHandlerForType(Protocols::Id protocolId, uint8_t msgType, ExchangeDelegate * delegate);

    /**
     * A strongly-message-typed version of RegisterUnsolicitedMessageHandlerForType.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR RegisterUnsolicitedMessageHandlerForType(MessageType msgType, ExchangeDelegate * delegate)
    {
        static_assert(std::is_same<std::underlying_type_t<MessageType>, uint8_t>::value, "Enum is wrong size; cast is not safe");
        return RegisterUnsolicitedMessageHandlerForType(Protocols::MessageTypeTraits<MessageType>::ProtocolId(),
                                                        static_cast<uint8_t>(msgType), delegate);
    }

    /**
     *  Unregister an unsolicited message handler for a given protocol identifier.
     *
     *  @param[in]    protocolId     The protocol identifier of the received message.
     *
     *  @retval #CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER  If the matching unsolicited message handler
     *                                                       is not found.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::Id protocolId);

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
    CHIP_ERROR UnregisterUnsolicitedMessageHandlerForType(Protocols::Id protocolId, uint8_t msgType);

    /**
     * A strongly-message-typed version of UnregisterUnsolicitedMessageHandlerForType.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR UnregisterUnsolicitedMessageHandlerForType(MessageType msgType)
    {
        static_assert(std::is_same<std::underlying_type_t<MessageType>, uint8_t>::value, "Enum is wrong size; cast is not safe");
        return UnregisterUnsolicitedMessageHandlerForType(Protocols::MessageTypeTraits<MessageType>::ProtocolId(),
                                                          static_cast<uint8_t>(msgType));
    }

    /**
     * @brief
     *   Called when a cached group message that was waiting for message counter
     *   sync shold be reprocessed.
     *
     * @param packetHeader  The message header
     * @param payloadHeader The payload header
     * @param session       The handle to the secure session
     * @param msgBuf        The received message
     */
    void HandleGroupMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                    const SecureSessionHandle & session, System::PacketBufferHandle msgBuf);

    // Channel public APIs
    ChannelHandle EstablishChannel(const ChannelBuilder & builder, ChannelDelegate * delegate);

    // Internal APIs used for channel
    void ReleaseChannelContext(ChannelContext * channel) { mChannelContexts.ReleaseObject(channel); }

    void ReleaseChannelHandle(ChannelContextHandleAssociation * association) { mChannelHandles.ReleaseObject(association); }

    template <typename Event>
    void NotifyChannelEvent(ChannelContext * channel, Event event)
    {
        mChannelHandles.ForEachActiveObject([&](ChannelContextHandleAssociation * association) {
            if (association->mChannelContext == channel)
                event(association->mChannelDelegate);
            return true;
        });
    }

    void IncrementContextsInUse();
    void DecrementContextsInUse();

    SecureSessionMgr * GetSessionMgr() const { return mSessionMgr; }

    ReliableMessageMgr * GetReliableMessageMgr() { return &mReliableMessageMgr; };

    MessageCounterSyncMgr * GetMessageCounterSyncMgr() { return &mMessageCounterSyncMgr; };
    Transport::AdminId GetAdminId() { return mAdminId; }

    uint16_t GetNextKeyId() { return ++mNextKeyId; }
    size_t GetContextsInUse() const { return mContextsInUse; }

private:
    enum class State
    {
        kState_NotInitialized = 0, // Used to indicate that the ExchangeManager is not initialized.
        kState_Initialized    = 1  // Used to indicate that the ExchangeManager is initialized.
    };

    struct UnsolicitedMessageHandler
    {
        UnsolicitedMessageHandler() : ProtocolId(Protocols::NotSpecified) {}

        constexpr void Reset() { Delegate = nullptr; }
        constexpr bool IsInUse() const { return Delegate != nullptr; }
        // Matches() only returns a sensible value if IsInUse() is true.
        constexpr bool Matches(Protocols::Id aProtocolId, int16_t aMessageType) const
        {
            return ProtocolId == aProtocolId && MessageType == aMessageType;
        }

        ExchangeDelegate * Delegate;
        Protocols::Id ProtocolId;
        // Message types are normally 8-bit unsigned ints, but we use
        // kAnyMessageType, which is negative, to represent a wildcard handler,
        // so need a type that can store both that and all valid message type
        // values.
        int16_t MessageType;
    };

    uint16_t mNextExchangeId;
    uint16_t mNextKeyId;
    State mState;
    SecureSessionMgr * mSessionMgr;
    ReliableMessageMgr mReliableMessageMgr;
    MessageCounterSyncMgr mMessageCounterSyncMgr;

    Transport::AdminId mAdminId = 0;

    std::array<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> mContextPool;
    size_t mContextsInUse;

    UnsolicitedMessageHandler UMHandlerPool[CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];
    BitMapObjectPool<ChannelContext, CHIP_CONFIG_MAX_ACTIVE_CHANNELS> mChannelContexts;
    BitMapObjectPool<ChannelContextHandleAssociation, CHIP_CONFIG_MAX_CHANNEL_HANDLES> mChannelHandles;

    ExchangeContext * AllocContext(uint16_t ExchangeId, SecureSessionHandle session, bool Initiator, ExchangeDelegate * delegate);

    CHIP_ERROR RegisterUMH(Protocols::Id protocolId, int16_t msgType, ExchangeDelegate * delegate);
    CHIP_ERROR UnregisterUMH(Protocols::Id protocolId, int16_t msgType);

    static bool IsMsgCounterSyncMessage(const PayloadHeader & payloadHeader);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * msgLayer) override;

    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle msgBuf, SecureSessionMgr * msgLayer) override;

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override;
    void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) override;

    // TransportMgrDelegate interface for rendezvous sessions
    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                           System::PacketBufferHandle msgBuf) override;
};

} // namespace Messaging
} // namespace chip
