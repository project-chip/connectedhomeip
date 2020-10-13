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

#include <support/BitFlags.h>
#include <support/DLLUtil.h>
#include <system/SystemTimer.h>
#include <transport/SecureSessionMgr.h>

#define EXCHANGE_CONTEXT_ID(x) ((x) + 1)

using namespace chip::Transport;

namespace chip {

using System::PacketBuffer;

class ExchangeManager;

enum class ExFlagValues : uint16_t
{
    kFlagInitiator        = 0x0001, /// This context is the initiator of the exchange.
    kFlagConnectionClosed = 0x0002, /// This context was associated with a Connection.
    kFlagResponseExpected = 0x0004, /// If a response is expected for a message that is being sent.
    kFlagMsgRcvdFromPeer =
        0x0008, /// When set, signifies that at least one message has been received from peer on this exchange context.
};

class ExchangeContext;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by ExchangeContext object on specific events. If the user of ExchangeContext
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT ExchangeContextDelegate
{
public:
    virtual ~ExchangeContextDelegate() {}

    /**
     * @brief
     *   This function is the protocol callback for handling a received CHIP message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *  @param[in]    protocolId    The protocol identifier of the received message.
     *  @param[in]    msgType       The message type of the corresponding profile.
     *  @param[in]    payload       A pointer to the PacketBuffer object holding the message payload.
     */
    virtual void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                        uint8_t msgType, System::PacketBuffer * payload) = 0;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the receipt
     *   of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnResponseTimeout(ExchangeContext * ec) = 0;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the retransmission
     *   of a previously sent message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnRetransmissionTimeout(ExchangeContext * ec) = 0;
};

/**
 *  @class ExchangeContext
 *
 *  @brief
 *    This class represents an ongoing conversation (ExchangeContext) between two or more nodes.
 *    It defines methods for encoding and communicating CHIP messages within an ExchangeContext
 *    over various transport mechanisms, for example, TCP, UDP, or CHIP Reliable Messaging.
 *
 */
class DLL_EXPORT ExchangeContext
{
    friend class ExchangeManager;

public:
    typedef uint32_t Timeout; /**< Type used to express the timeout in this ExchangeContext, in milliseconds */

    ExchangeManager * ExchangeMgr; /**< [READ ONLY] Owning exchange manager. */
    uint64_t NodeId;               /**< [READ ONLY] Node ID of peer node. */
    uint16_t ExchangeId;           /**< [READ ONLY] Assigned exchange ID. */
    void * AppState;               /**< Pointer to application-specific state object. */
    bool AllowDuplicateMsgs;       /**< Boolean indicator of whether duplicate messages are allowed for a given exchange. */
    uint32_t RetransInterval;      /**< Time between retransmissions (in milliseconds); 0 disables retransmissions. */
    Timeout ResponseTimeout;       /**< Maximum time to wait for response (in milliseconds); 0 disables response timeout. */

    enum
    {
        kSendFlag_AutoRetrans    = 0x0001, /**< Used to indicate that automatic retransmission is enabled. */
        kSendFlag_ExpectResponse = 0x0002, /**< Used to indicate that a response is expected within a specified timeout. */
        kSendFlag_DelaySend      = 0x0008, /**< Used to indicate that the sending of the current message needs to be delayed. */
        kSendFlag_ReuseSourceId  = 0x0020, /**< Used to indicate that the source node ID in the message header can be reused. */
        kSendFlag_RetainBuffer   = 0x0040, /**< Used to indicate that the message buffer should not be freed after sending. */
        kSendFlag_AlreadyEncoded = 0x0080, /**< Used to indicate that the message is already encoded. */
        kSendFlag_DefaultMulticastSourceAddress = 0x0100, /**< Used to indicate that default IPv6 source address selection should be
                                                             used when sending IPv6 multicast messages. */
        kSendFlag_FromInitiator    = 0x0200, /**< Used to indicate that the current message is the initiator of the exchange. */
        kSendFlag_RequestAck       = 0x0400, /**< Used to send a RMP message requesting an acknowledgment. */
        kSendFlag_NoAutoRequestAck = 0x0800, /**< Suppress the auto-request acknowledgment feature when sending a message. */
    };

    /**
     * This function is the protocol callback of an unsolicited message handler.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    protocolId    The protocol identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @param[in]    payload       A pointer to the PacketBuffer object holding the message payload.
     */
    typedef void (*MessageReceiveFunct)(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId,
                                        uint8_t msgType, System::PacketBuffer * payload);

    /**
     *  Determine whether the context is the initiator of the exchange.
     *
     *  @return Returns 'true' if it is the initiator, else 'false'.
     *
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
     *  Set the kFlagInitiator flag bit. This flag is set by the node that
     *  initiates an exchange.
         *
     *  @param[in]  inInitiator  A Boolean indicating whether (true) or not
     *                           (false) the context is the initiator of
     *                           the exchange.
     */
    void SetInitiator(bool inInitiator);

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
     *  @param[in]    protocolId     The profile identifier of the CHIP message to be sent.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
         *  @param[in]    msgPayload    A pointer to the PacketBuffer object holding the CHIP message.
     *
     *  @param[in]    sendFlags     Flags set by the application for the CHIP message being sent.
     *
     *  @param[in]    msgCtxt       A pointer to an application-specific context object to be associated
     *                              with the message being sent.

     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this SendMessage API.
     *  @retval  #CHIP_ERROR_SEND_THROTTLED                 if this exchange context has been throttled when using the
     *                                                       CHIP reliable messaging protocol.
     *  @retval  #CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE if there is a mismatch in the specific send operation and the
     *                                                       CHIP message protocol version that is supported. For example,
     *                                                       this error would be generated if CHIP Reliable Messaging
     *                                                       semantics are being attempted when the CHIP message protocol
     *                                                       version is V1.
     *  @retval  #CHIP_ERROR_NOT_CONNECTED                  if the context was associated with a connection that is now
     *                                                       closed.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully sent the message down to the
     *                                                       network layer.
     */
    CHIP_ERROR SendMessage(uint32_t protocolId, uint8_t msgType, System::PacketBuffer * msgPayload, uint16_t sendFlags = 0,
                           void * msgCtxt = nullptr);

    /**
     *  Tear down the Trickle retransmission mechanism by canceling the periodic timers
     *  within Trickle and freeing the message buffer holding the Weave
     *  message.
     */
    void TeardownTrickleRetransmit();

    /**
     *  Cancel the Trickle retransmission mechanism.
     */
    void CancelRetrans();

    /**
     *  Set delegate of the current ExchangeContext instance.
     */
    void SetDelegate(ExchangeContextDelegate * delegate)
    {
        mDelegate = delegate;
    }

    /**
     *  Return the delegate pointer of the current ExchangeContext instance.
     */
    ExchangeContextDelegate * GetDelegate()
    {
        return mDelegate;
    }

    /*
     * In order to use reference counting (see refCount below)
     * we use a hold/free paradigm where users of the exchange
     * can hold onto it while it's out of their direct control
     * to make sure it isn't closed before everyone's ready.
     */
    void AddRef();
    void Close();
    void Abort();
    void Release();
    CHIP_ERROR StartTimerT();

private:
    System::PacketBuffer * msg; // If we are re-transmitting, then this is the pointer to the message being retransmitted
    ExchangeContextDelegate * mDelegate = nullptr;

    // Trickle-controlled retransmissions:
    uint32_t backoff;                        // backoff for sampling the numner of messages
    uint8_t msgsReceived;                    // number of messages heard during the backoff period
    uint8_t rebroadcastThreshold;            // re-broadcast threshold
    BitFlags<uint16_t, ExFlagValues> mFlags; // Internal state flags

    CHIP_ERROR ResendMessage();
    bool MatchExchange(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader);
    static void CancelRetransmissionTimer(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerTau(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerT(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR StartResponseTimer();
    void CancelResponseTimer();
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    void DoClose(bool clearRetransTable);
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf);
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf,
                             ExchangeContext::MessageReceiveFunct umhandler);

    uint8_t mRefCount;
};

} // namespace chip
