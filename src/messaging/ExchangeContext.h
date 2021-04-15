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
#include <messaging/ExchangeACL.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>
#include <protocols/Protocols.h>
#include <support/BitFlags.h>
#include <support/DLLUtil.h>
#include <system/SystemTimer.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

namespace Messaging {

class ExchangeManager;
class ExchangeContext;
class ExchangeMessageDispatch;

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
class DLL_EXPORT ExchangeContext : public ReliableMessageContext,
                                   public ReferenceCounted<ExchangeContext, ExchangeContextDeletor, 0>
{
    friend class ExchangeManager;
    friend class ExchangeContextDeletor;
    friend class MessageCounterSyncMgr;

public:
    typedef uint32_t Timeout; // Type used to express the timeout in this ExchangeContext, in milliseconds

    /**
     *  Determine whether the context is the initiator of the exchange.
     *
     *  @return Returns 'true' if it is the initiator, else 'false'.
     */
    bool IsInitiator() const;

    /**
     *  Determine whether a response is expected for messages sent over
     *  this exchange.
     *
     *  @return Returns 'true' if response expected, else 'false'.
     */
    bool IsResponseExpected() const;

    /**
     *  Set whether a response is expected on this exchange.
     *
     *  @param[in]  inResponseExpected  A Boolean indicating whether (true) or not
     *                                  (false) a response is expected on this
     *                                  exchange.
     */
    void SetResponseExpected(bool inResponseExpected);

    /**
     *  Send a CHIP message on this exchange.
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
    CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, System::PacketBufferHandle msgPayload,
                           const SendFlags & sendFlags);

    /**
     * A strongly-message-typed version of SendMessage.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR SendMessage(MessageType msgType, System::PacketBufferHandle && msgPayload, const SendFlags & sendFlags)
    {
        static_assert(std::is_same<std::underlying_type_t<MessageType>, uint8_t>::value, "Enum is wrong size; cast is not safe");
        return SendMessage(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), static_cast<uint8_t>(msgType),
                           std::move(msgPayload), sendFlags);
    }

    /**
     *  Handle a received CHIP message on this exchange.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *
     *  @param[in]    peerAddress   The address of the sender
     *
     *  @param[in]    msgBuf        A handle to the packet buffer holding the CHIP message.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this HandleMessage API.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully delivered the message up to the
     *                                                       protocol layer.
     */
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                             const Transport::PeerAddress & peerAddress, System::PacketBufferHandle msgBuf);

    ExchangeDelegateBase * GetDelegate() const { return mDelegate; }
    void SetDelegate(ExchangeDelegateBase * delegate) { mDelegate = delegate; }

    ExchangeManager * GetExchangeMgr() const { return mExchangeMgr; }

    ReliableMessageContext * GetReliableMessageContext() { return static_cast<ReliableMessageContext *>(this); };

    ExchangeMessageDispatch * GetMessageDispatch();

    ExchangeACL * GetExchangeACL(Transport::AdminPairingTable & table)
    {
        if (mExchangeACL == nullptr)
        {
            Transport::AdminPairingInfo * admin = table.FindAdmin(mSecureSession.GetAdminId());
            if (admin != nullptr)
            {
                mExchangeACL = chip::Platform::New<CASEExchangeACL>(admin);
            }
        }

        return mExchangeACL;
    }

    SecureSessionHandle GetSecureSession() { return mSecureSession; }

    uint16_t GetExchangeId() const { return mExchangeId; }

    void SetAppState(void * state) { mAppState = state; }

    void * GetAppState() const { return mAppState; }

    SecureSessionHandle GetSecureSessionHandle() const { return mSecureSession; }

    /*
     * In order to use reference counting (see refCount below) we use a hold/free paradigm where users of the exchange
     * can hold onto it while it's out of their direct control to make sure it isn't closed before everyone's ready.
     * A customized version of reference counting is used since there are some extra stuff to do within Release.
     */
    void Close();
    void Abort();

    ExchangeContext * Alloc(ExchangeManager * em, uint16_t ExchangeId, SecureSessionHandle session, bool Initiator,
                            ExchangeDelegateBase * delegate);
    void Free();
    void Reset();

    void SetResponseTimeout(Timeout timeout);

private:
    Timeout mResponseTimeout; // Maximum time to wait for response (in milliseconds); 0 disables response timeout.
    ExchangeDelegateBase * mDelegate = nullptr;
    ExchangeManager * mExchangeMgr   = nullptr;
    ExchangeACL * mExchangeACL       = nullptr;
    void * mAppState                 = nullptr;

    SecureSessionHandle mSecureSession; // The connection state
    uint16_t mExchangeId;               // Assigned exchange ID.

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
    bool MatchExchange(SecureSessionHandle session, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader);

    CHIP_ERROR StartResponseTimer();

    /**
     * A subset of SendMessage functionality that does not perform message
     * counter sync for group keys.
     */
    CHIP_ERROR SendMessageImpl(Protocols::Id protocolId, uint8_t msgType, System::PacketBufferHandle msgBuf,
                               const SendFlags & sendFlags, Transport::PeerConnectionState * state = nullptr);
    void CancelResponseTimer();
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    void DoClose(bool clearRetransTable);
};

inline void ExchangeContextDeletor::Release(ExchangeContext * obj)
{
    obj->Free();
}

} // namespace Messaging
} // namespace chip
