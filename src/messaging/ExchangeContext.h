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
 *
 */
class DLL_EXPORT ExchangeContext : public ReliableMessageContext,
                                   public ReferenceCounted<ExchangeContext, ExchangeContextDeletor>,
                                   public SessionDelegate
{
    friend class ExchangeManager;
    friend class ExchangeContextDeletor;

public:
    typedef System::Clock::Timeout Timeout; // Type used to express the timeout in this ExchangeContext

    ExchangeContext(ExchangeManager * em, uint16_t ExchangeId, const SessionHandle & session, bool Initiator,
                    ExchangeDelegate * delegate, bool isEphemeralExchange = false);

    ~ExchangeContext() override;

    /**
     *  Determine whether the context is the initiator of the exchange.
     *
     *  @return Returns 'true' if it is the initiator, else 'false'.
     */
    bool IsInitiator() const;

    bool IsEncryptionRequired() const { return mDispatch.IsEncryptionRequired(); }

    bool IsGroupExchangeContext() const { return mSession && mSession->IsGroupSession(); }

    // Implement SessionDelegate
    NewSessionHandlingPolicy GetNewSessionHandlingPolicy() override { return NewSessionHandlingPolicy::kStayAtOldSession; }
    void OnSessionReleased() override;

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

    /**
     * A notification that we will have SendMessage called on us in the future
     * (and should stay open until that happens).
     */
    void WillSendMessage() { mFlags.Set(Flags::kFlagWillSendMessage); }

    /**
     *  Handle a received CHIP message on this exchange.
     *
     *  @param[in]    messageCounter  The message counter of the packet.
     *  @param[in]    payloadHeader   A reference to the PayloadHeader object.
     *  @param[in]    msgFlags        The message flags corresponding to the received message
     *  @param[in]    msgBuf          A handle to the packet buffer holding the CHIP message.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this HandleMessage API.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully delivered the message up to the
     *                                                       protocol layer.
     */
    CHIP_ERROR HandleMessage(uint32_t messageCounter, const PayloadHeader & payloadHeader, MessageFlags msgFlags,
                             System::PacketBufferHandle && msgBuf);

    ExchangeDelegate * GetDelegate() const { return mDelegate; }
    void SetDelegate(ExchangeDelegate * delegate) { mDelegate = delegate; }

    ExchangeManager * GetExchangeMgr() const { return mExchangeMgr; }

    ReliableMessageContext * GetReliableMessageContext() { return static_cast<ReliableMessageContext *>(this); };

    SessionHandle GetSessionHandle() const
    {
        VerifyOrDie(mSession);
        auto sessionHandle = mSession.Get();
        return std::move(sessionHandle.Value());
    }

    bool HasSessionHandle() const { return mSession; }

    uint16_t GetExchangeId() const { return mExchangeId; }

    /*
     * In order to use reference counting (see refCount below) we use a hold/free paradigm where users of the exchange
     * can hold onto it while it's out of their direct control to make sure it isn't closed before everyone's ready.
     * A customized version of reference counting is used since there are some extra stuff to do within Release.
     */
    void Close();
    void Abort();

    // Applies a suggested response timeout value based on the session type and the given upper layer processing time for
    // the next message to the exchange. The exchange context must have a valid session when calling this function.
    //
    // This function is an equivalent of SetResponseTimeout(mSession->ComputeRoundTripTimeout(applicationProcessingTimeout))
    void UseSuggestedResponseTimeout(Timeout applicationProcessingTimeout);

    // Set the response timeout for the exchange context, regardless of the underlying session type. Using
    // UseSuggestedResponseTimeout to set a timeout based on the type of the session and the application processing time instead of
    // using this function is recommended.
    //
    // If a timeout of 0 is provided, it implies no response is expected. Consequently, ExchangeDelegate::OnResponseTimeout will not
    // be called.
    //
    void SetResponseTimeout(Timeout timeout);

    // This API is used by commands that need to shut down all existing
    // sessions/exchanges on a fabric but need to make sure the response to the
    // command still goes out on the exchange the command came in on.  This API
    // will ensure that all secure sessions for the fabric this exchanges is on
    // are released except the one this exchange is using, and will release
    // that session once this exchange is done sending the response.
    //
    // This API is a no-op if called on an exchange that is not using a
    // SecureSession.
    void AbortAllOtherCommunicationOnFabric();

    /**
     *  Determine whether a response is currently expected for a message that was sent over
     *  this exchange.  While this is true, attempts to send other messages that expect a response
     *  will fail.
     *
     *  @return Returns 'true' if response expected, else 'false'.
     */
    bool IsResponseExpected() const;

    /**
     * Determine whether we are expecting our consumer to send a message on
     * this exchange (i.e. WillSendMessage was called and the message has not
     * yet been sent).
     */
    bool IsSendExpected() const { return mFlags.Has(Flags::kFlagWillSendMessage); }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    SessionHolder & GetSessionHolder() { return mSession; }

    enum class InjectedFailureType : uint8_t
    {
        kFailOnSend = 0x01
    };

    void InjectFailure(InjectedFailureType failureType) { mInjectedFailures.Set(failureType); }

    void ClearInjectedFailures() { mInjectedFailures.ClearAll(); }
#endif

private:
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    BitFlags<InjectedFailureType> mInjectedFailures;
#endif

    class ExchangeSessionHolder : public SessionHolderWithDelegate
    {
    public:
        ExchangeSessionHolder(ExchangeContext & exchange) : SessionHolderWithDelegate(exchange) {}
        void GrabExpiredSession(const SessionHandle & session);
    };

    Timeout mResponseTimeout{ 0 }; // Maximum time to wait for response (in milliseconds); 0 disables response timeout.
    ExchangeDelegate * mDelegate   = nullptr;
    ExchangeManager * mExchangeMgr = nullptr;

    ExchangeMessageDispatch & mDispatch;

    ExchangeSessionHolder mSession; // The connection state
    uint16_t mExchangeId;           // Assigned exchange ID.

    /**
     *  Track whether we are now expecting a response to a message sent via this exchange (because that
     *  message had the kExpectResponse flag set in its sendFlags).
     *
     *  @param[in]  inResponseExpected  A Boolean indicating whether (true) or not
     *                                  (false) a response is currently expected on this
     *                                  exchange.
     */
    void SetResponseExpected(bool inResponseExpected);

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
    bool MatchExchange(const SessionHandle & session, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader);

    /**
     * Notify our delegate, if any, that we have timed out waiting for a
     * response.  If aCloseIfNeeded is true, check whether the exchange needs to
     * be closed.
     */
    void NotifyResponseTimeout(bool aCloseIfNeeded);

    CHIP_ERROR StartResponseTimer();

    void CancelResponseTimer();
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState);

    void DoClose(bool clearRetransTable);

    /**
     * We have handled an application-level message in some way and should
     * re-evaluate out state to see whether we should still be open.
     */
    void MessageHandled();

    /**
     * Updates Sleepy End Device intervals mode in the following way:
     * - does nothing for exchanges over Bluetooth LE
     * - requests active mode if there are more messages,
     *   including MRP acknowledgements, expected to be sent or received on
     *   this exchange.
     * - withdraws the request for active mode, otherwise.
     */
    void UpdateSEDIntervalMode();

    /**
     * Requests or withdraws the request for Sleepy End Device active mode
     * based on the argument value.
     *
     * Note that the device switches to the idle mode if no
     * exchange nor other component requests the active mode.
     */
    void UpdateSEDIntervalMode(bool activeMode);

    static ExchangeMessageDispatch & GetMessageDispatch(bool isEphemeralExchange, ExchangeDelegate * delegate);

    // If SetAutoReleaseSession() is called, this exchange must be using a SecureSession, and should
    // evict it when the exchange is done with all its work (including any MRP traffic).
    inline void SetIgnoreSessionRelease(bool ignore);
    inline bool ShouldIgnoreSessionRelease();
};

inline void ExchangeContext::SetIgnoreSessionRelease(bool ignore)
{
    mFlags.Set(Flags::kFlagIgnoreSessionRelease, ignore);
}

inline bool ExchangeContext::ShouldIgnoreSessionRelease()
{
    return mFlags.Has(Flags::kFlagIgnoreSessionRelease);
}

} // namespace Messaging
} // namespace chip
