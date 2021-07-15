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
#include <messaging/ExchangeMgrDelegate.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <support/DLLUtil.h>
#include <support/Pool.h>
#include <support/TypeTraits.h>
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
class DLL_EXPORT ExchangeManager : public SecureSessionMgrDelegate
{
    friend class ExchangeContext;

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
     *  @param[in]    session    The identifier of the secure session (possibly
     *                           the empty session for a non-secure exchange)
     *                           for which the ExchangeContext is being set up.
     *
     *  @param[in]    delegate   A pointer to ExchangeDelegate.
     *
     *  @return   A pointer to the created ExchangeContext object On success. Otherwise NULL if no object
     *            can be allocated or is available.
     */
    ExchangeContext * NewContext(SecureSessionHandle session, ExchangeDelegate * delegate);

    void ReleaseContext(ExchangeContext * ec) { mContextPool.ReleaseObject(ec); }

    /**
     *  Register an unsolicited message handler for a given protocol identifier. This handler would be
     *  invoked for all messages of the given protocol.
     *
     *  @param[in]    protocolId      The protocol identifier of the received message.
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
        return RegisterUnsolicitedMessageHandlerForType(Protocols::MessageTypeTraits<MessageType>::ProtocolId(),
                                                        to_underlying(msgType), delegate);
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
        return UnregisterUnsolicitedMessageHandlerForType(Protocols::MessageTypeTraits<MessageType>::ProtocolId(),
                                                          to_underlying(msgType));
    }

    /**
     * A method to call Close() on all contexts that have a given delegate as
     * their delegate.  To be used if the delegate is being destroyed.  This
     * method will guarantee that it does not call into the delegate.
     */
    void CloseAllContextsForDelegate(const ExchangeDelegate * delegate);

    void SetDelegate(ExchangeMgrDelegate * delegate) { mDelegate = delegate; }

    SecureSessionMgr * GetSessionMgr() const { return mSessionMgr; }

    ReliableMessageMgr * GetReliableMessageMgr() { return &mReliableMessageMgr; };

    Transport::AdminId GetAdminId() { return mAdminId; }

    uint16_t GetNextKeyId() { return ++mNextKeyId; }

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

    ExchangeMgrDelegate * mDelegate;
    SecureSessionMgr * mSessionMgr;
    ReliableMessageMgr mReliableMessageMgr;

    ApplicationExchangeDispatch mDefaultExchangeDispatch;

    Transport::AdminId mAdminId = 0;

    BitMapObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> mContextPool;

    UnsolicitedMessageHandler UMHandlerPool[CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];

    CHIP_ERROR RegisterUMH(Protocols::Id protocolId, int16_t msgType, ExchangeDelegate * delegate);
    CHIP_ERROR UnregisterUMH(Protocols::Id protocolId, int16_t msgType);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source) override;

    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           const Transport::PeerAddress & source, DuplicateMessage isDuplicate,
                           System::PacketBufferHandle && msgBuf) override;

    void OnNewConnection(SecureSessionHandle session) override;
#if CHIP_CONFIG_TEST
public: // Allow OnConnectionExpired to be called directly from tests.
#endif  // CHIP_CONFIG_TEST
    void OnConnectionExpired(SecureSessionHandle session) override;
};

} // namespace Messaging
} // namespace chip
