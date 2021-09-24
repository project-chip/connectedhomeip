/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines the classes corresponding to CHIP Exchange Context.
 *
 */

#pragma once

#include <lib/core/ReferenceCounted.h>
#include <lib/support/BitFlags.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ReferenceCountedHandle.h>
#include <lib/support/TypeTraits.h>
#include <messaging/ExchangeACL.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>

namespace chip {

namespace Messaging {

class ExchangeManager;
class ExchangeContext;
class ExchangeMessageDispatch;
using ExchangeHandle = ReferenceCountedHandle<ExchangeContext>;

class ExchangeContextDeletor
{
public:
    static void Release(ExchangeContext * obj);
};

/**
 *  @brief
 *    This class represents an ongoing conversation (ExchangeContext) between two or more nodes.
 *    It defines methods for encoding and communicating CHIP messages within an ExchangeContext
 *    over various transport mechanisms, for example, TCP, UDP, or CHIP Reliable Messaging.
 */
class DLL_EXPORT ExchangeContext : public ReliableMessageContext, public ReferenceCounted<ExchangeContext, ExchangeContextDeletor>
{
    friend class ExchangeManager;
    friend class ExchangeContextDeletor;

public:
    typedef uint32_t Timeout; // Type used to express the timeout in this ExchangeContext, in milliseconds

    ExchangeContext(ExchangeManager * em, uint16_t ExchangeId, SessionHandle session, bool Initiator, ExchangeDelegate * delegate);
    ~ExchangeContext();

    /// Determine whether the context is the initiator of the exchange.
    bool IsInitiator() const;

    bool IsEncryptionRequired() const { return mDispatch->IsEncryptionRequired(); }

    /**
     *  Send a CHIP message on this exchange.
     *
     *  If SendMessage returns success and the message was not expecting a
     *  response, the exchange will close itself before returning, unless the
     *  message being sent is a standalone ack.  If SendMessage returns failure,
     *  the caller is responsible for deciding what to do (e.g. closing the
     *  exchange, trying to re-establish a secure session, etc).
     *
     *  @param[in]    protocolId    The protocol identifier of the CHIP message to be sent.
     *
     *  @param[in]    msgType       The message type of the corresponding protocol.
     *
     *  @param[in]    msgPayload    A handle to the packet buffer holding the CHIP message.
     *
     *  @param[in]    sendFlags     Flags set by the application for the CHIP message being sent.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this SendMessage API.
     *  @retval  #CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE if there is a mismatch in the specific send operation and the
     *                                                       CHIP message protocol version that is supported.
     *  @retval  #CHIP_ERROR_NOT_CONNECTED                  if the context was associated with a connection that is now
     *                                                       closed.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully sent the message down to the
     *                                                       network layer.
     */
    CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, System::PacketBufferHandle && msgPayload,
                           const SendFlags & sendFlags = SendFlags(SendMessageFlags::kNone));

    /**
     * A strongly-message-typed version of SendMessage.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR SendMessage(MessageType msgType, System::PacketBufferHandle && msgPayload,
                           const SendFlags & sendFlags = SendFlags(SendMessageFlags::kNone))
    {
        return SendMessage(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), to_underlying(msgType), std::move(msgPayload),
                           sendFlags);
    }

    /// A notification that we will have SendMessage called on us in the future (and should stay open until that happens).
    void WillSendMessage();

    /// A notification that we will receive more requests in the future (and should stay open until that happens).
    CHIP_ERROR WillHandleMoreMessage(Timeout idleTimeout);

    /// Return whether this exchange is in a state which can handle incoming messages
    bool CanHandleMessage();

    /**
     *  Handle a received CHIP message on this exchange.
     *
     *  @param[in]    messageCounter  The message counter of the packet.
     *  @param[in]    payloadHeader   A reference to the PayloadHeader object.
     *  @param[in]    peerAddress     The address of the sender
     *  @param[in]    msgFlags        The message flags corresponding to the received message
     *  @param[in]    msgBuf          A handle to the packet buffer holding the CHIP message.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this HandleMessage API.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully delivered the message up to the
     *                                                       protocol layer.
     */
    CHIP_ERROR HandleMessage(uint32_t messageCounter, const PayloadHeader & payloadHeader,
                             const Transport::PeerAddress & peerAddress, MessageFlags msgFlags,
                             System::PacketBufferHandle && msgBuf);

    ExchangeDelegate * GetDelegate() const { return mDelegate; }
    void SetDelegate(ExchangeDelegate * delegate) { mDelegate = delegate; }

    ExchangeManager * GetExchangeMgr() const { return mExchangeMgr; }

    ReliableMessageContext * GetReliableMessageContext() { return static_cast<ReliableMessageContext *>(this); };

    ExchangeMessageDispatch * GetMessageDispatch() { return mDispatch; }

    ExchangeACL * GetExchangeACL(Transport::FabricTable & table)
    {
        if (mExchangeACL == nullptr)
        {
            Transport::FabricInfo * fabric = table.FindFabricWithIndex(mSecureSession.Value().GetFabricIndex());
            if (fabric != nullptr)
            {
                mExchangeACL = chip::Platform::New<CASEExchangeACL>(fabric);
            }
        }

        return mExchangeACL;
    }

    SessionHandle GetSecureSession() { return mSecureSession.Value(); }
    bool HasSecureSession() const { return mSecureSession.HasValue(); }

    uint16_t GetExchangeId() const { return mExchangeId; }

    void Close();
    void Abort();

    void SetResponseTimeout(Timeout timeout);

private:
    /* State machine of an exchange
     *
     *  +-------+       +-------+
     *  |       |       |       |
     *  |  +-+  |   R   |  +-+  |
     *  |  |A|<---------|  |E|  |
     *  |  +-+  |       |  +-+  |
     *  |   |   |       |       |
     *  |   |W  |       |       |
     *  |   v   |       |       |
     *  |  +-+  |   S   |  +-+  |
     *  |  |B|  |------>|  |N|  |
     *  |  +-+  |       |  +-+  |
     *  |       |       |   |   |
     *  |       |       |   |H  |
     *  |       |       |   v   |
     *  |  +-+  |       |  +-+  |
     *  |  |I|  |       |  |S|  |
     *  |  +-+  |       |  +-+  |
     *  |       |       |       |
     *  |       |       |       |
     *  |       |       |  +-+  |
     *  |       |       |  |R|  |
     *  |       |       |  +-+  |
     *  |       |       |       |
     *  +-------+       +-------+
     *
     *
     * States:
     *   I(Initiator): The exchange is created as an initiator, it is going to send a message soon.
     *   R(Responder): The exchange is created as an responder, it is going to handle a message soon.
     *   A(Active): The exchange is active working on a request, and is about to send a message within current executing
     *              context
     *   B(Background): It is our turn to send a message, but WillSendMessage is called
     *   E(SentExpectResponse): A packet has been sent, The exchange is expecting a response.
     *   N(SentNoExpectResponse): A packet has been sent, The exchange is not expecting a response.
     *   S(Sleep): A packet has been sent, The exchange is not expecting a response, and WillHandleMoreMessage is called
     *   C(Closed): The exchange is graceful closed. Achieved by finishing handling a packet w/o sending a message and
     *              WillSendMessage is not set. Or the user explicit called Close().
     *   E(Error): Error state.
     *
     * Events:
     *   S(Sent): A packet has been sent
     *   R(Received): A packet has been received
     *   W(WillSendMessage): WillSendMessage called
     *   H(WillHandleMoreMessage): WillHandleMoreMessage called
     *   T(ResponseTimeout): Timeout waiting for an response. (mResponseTimeout)
     *
     * Note: The difference between Initiator and Background is that AckMessageCounter is valid in state Background, but
     *       invalid in state Initiator
     *       The difference between SentExpectResponse and Sleep is that SentExpectResponse has response timer armed, but
     *       Sleep has IdleTimer armed.
     *
     * Life-cycle management: The lifespan of the exchange is managed by reference counter, the counter can be retained
     *                       by following cases:
     *    1. Upper-layer: The upper layer is considered as holding a reference in Initiator, Responder and Background states.
     *    2. Handler: While an exchange is handling a packet, it will hold a reference in case that the application
     *                delegate calls Close() in the middle of the handler process. This is the case for Active state
     *    3. Timer: When the response or idle timer is registered into system timer, it will hold a reference to the
     *              context. This is the case for SentExpectResponse and Sleep state.
     *    4. RMP: RMP will hold a reference in SentExpectResponse and SentNoExpectResponse state.
     *    5. In Closed and Error state, no one is holding the reference to the exchange, and it should be release soon
     *       after goes out of current execution context. It is actually holding by Handler case in a very short time.
     */
    enum class State
    {
        Initiator            = 0,
        Active               = 1,
        Background           = 2,
        Responder            = 3,
        SentExpectResponse   = 4,
        SentNoExpectResponse = 5,
        Sleep                = 6,
        Closed               = 7,
        Error                = 8,
        Released             = 9,
    } mState;

    // Hide Retain/Release API, redeclare them as private
    using ReferenceCounted<ExchangeContext, ExchangeContextDeletor>::Retain;
    using ReferenceCounted<ExchangeContext, ExchangeContextDeletor>::Release;
    friend class ReferenceCountedHandle<ExchangeContext>;

    static constexpr const Timeout kDefaultResponseTimeout = 5000;
    Timeout mResponseTimeout       = kDefaultResponseTimeout; // Maximum time to wait for response (in milliseconds)
    ExchangeDelegate * mDelegate   = nullptr;
    ExchangeManager * mExchangeMgr = nullptr;
    ExchangeACL * mExchangeACL     = nullptr;

    ExchangeMessageDispatch * mDispatch = nullptr;

    Optional<SessionHandle> mSecureSession; // The connection state
    uint16_t mExchangeId;                   // Assigned exchange ID.

    /**
     *  Search for an existing exchange that the message applies to.
     *
     *  @param[in]    session       The secure session of the received message.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *
     *  @retval  true                                       If a match is found.
     *  @retval  false                                      If a match is not found.
     */
    bool MatchExchange(SessionHandle session, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader);

    /// Notify the exchange that its connection has expired.
    void OnConnectionExpired();

    // Timer
    void NotifyTimeout();
    CHIP_ERROR StartTimer(Timeout timeout);
    void CancelTimer();
    static void HandleTimeout(System::Layer * aSystemLayer, void * aAppState);

    void DoClose(State originalState, bool clearRetransTable);
    void CleanUp(State originalState);
};

} // namespace Messaging
} // namespace chip
