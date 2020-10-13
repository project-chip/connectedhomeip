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
 *      This file defines the classes corresponding to CHIP Exchange Context.
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
    virtual void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                   System::PacketBuffer * payload) = 0;

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
 *  @brief
 *    This class represents an ongoing conversation (ExchangeContext) between two or more nodes.
 *    It defines methods for encoding and communicating CHIP messages within an ExchangeContext
 *    over various transport mechanisms, for example, TCP, UDP, or CHIP Reliable Messaging.
 */
class DLL_EXPORT ExchangeContext
{
public:
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
     *                                                       CHIP message protocol version that is supported.
     *  @retval  #CHIP_ERROR_NOT_CONNECTED                  if the context was associated with a connection that is now
     *                                                       closed.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully sent the message down to the
     *                                                       network layer.
     */
    CHIP_ERROR SendMessage(uint32_t protocolId, uint8_t msgType, System::PacketBuffer * msgPayload, uint16_t sendFlags = 0,
                           void * msgCtxt = nullptr);

    /**
     *  Handle a received CHIP message on this exchange.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the CHIP message.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this HandleMessage API.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully delivered the message up to the
     *                                                       protocol layer.
     */
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf);

    /**
     *  Handle a received CHIP message on this exchange.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the CHIP message.
     *
     *  @param[in]    umhandler     A unsolicited message callback handler.
     *
     *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               if an invalid argument was passed to this HandleMessage API.
     *  @retval  #CHIP_ERROR_INCORRECT_STATE                if the state of the exchange context is incorrect.
     *  @retval  #CHIP_NO_ERROR                             if the CHIP layer successfully delivered the message up to the
     *                                                       protocol layer.
     */
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf,
                             ExchangeContext::MessageReceiveFunct umhandler);

    /**
     *  Search for an existing exchange that the message applies to.
     *
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *
     *  @retval  true                                       If a match is found.
     *  @retval  false                                      If a match is not found.
     */
    bool MatchExchange(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader);

    /**
     *  Start the Trickle rebroadcast algorithm's periodic retransmission timer mechanism.
     *
     *  @return  #CHIP_NO_ERROR if successful, else an INET_ERROR mapped into a CHIP_ERROR.
     */
    CHIP_ERROR StartTimerT();

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
    void SetDelegate(ExchangeContextDelegate * delegate) { mDelegate = delegate; }

    /**
     *  Return the delegate pointer of the current ExchangeContext instance.
     */
    ExchangeContextDelegate * GetDelegate() { return mDelegate; }

    /**
     *  Set the current ExchangeManager instance.
     */
    void SetExchangeMgr(ExchangeManager * exMgr) { mExchangeMgr = exMgr; }

    /**
     *  Return the pointer of the current associated ExchangeManager instance.
     */
    ExchangeManager * GetExchangeMgr() { return mExchangeMgr; }

    /**
     *  Set the Node ID of peer node.
     */
    void SetNodeId(uint64_t nodeId) { mNodeId = nodeId; }

    /**
     *  Return the Node ID of peer node.
     */
    uint64_t GetNodeId() { return mNodeId; }

    /**
     *  Set exchange ID.
     */
    void SetExchangeId(uint16_t exId) { mExchangeId = exId; }

    /**
     *  Return assigned exchange ID.
     */
    uint16_t GetExchangeId() { return mExchangeId; }

    /**
     *  Set application-specific state object.
     */
    void SetAppState(void * state) { mAppState = state; }

    /**
     *  Return application-specific state object.
     */
    void * GetAppState() { return mAppState; }

    /**
     *  Set application-specific state object.
     */
    void SetAllowDuplicateMsgs(bool value) { mAllowDuplicateMsgs = value; }

    /**
     *  Return application-specific state object.
     */
    bool isDuplicateMsgsAllowed() { return mAllowDuplicateMsgs; }

    /*
     * In order to use reference counting (see refCount below) we use a hold/free paradigm where users of the exchange
     * can hold onto it while it's out of their direct control to make sure it isn't closed before everyone's ready.
     * A customized version of reference counting is used since there are some extra stuff to do within Release.
     */
    void AddRef();
    void Close();
    void Abort();
    void Release();
    void SetRefCount(uint8_t value) { mRefCount = value; }

private:
    enum
    {
        kSendFlag_ExpectResponse = 0x0001, /**< Used to indicate that a response is expected within a specified timeout. */
        kSendFlag_RetainBuffer   = 0x0002, /**< Used to indicate that the message buffer should not be freed after sending. */
        kSendFlag_FromInitiator  = 0x0004, /**< Used to indicate that the current message is the initiator of the exchange. */
    };

    typedef uint32_t Timeout; // Type used to express the timeout in this ExchangeContext, in milliseconds

    Timeout mResponseTimeout;    // Maximum time to wait for response (in milliseconds); 0 disables response timeout.
    System::PacketBuffer * mMsg; // If we are re-transmitting, then this is the pointer to the message being retransmitted
    ExchangeContextDelegate * mDelegate = nullptr;
    ExchangeManager * mExchangeMgr;
    void * mAppState; // Pointer to application-specific state object.

    uint64_t mNodeId;          // Node ID of peer node.
    uint32_t mRetransInterval; // Time between retransmissions (in milliseconds); 0 disables retransmissions.
    uint16_t mExchangeId;      // Assigned exchange ID.
    bool mAllowDuplicateMsgs;  // Boolean indicator of whether duplicate messages are allowed for a given exchange.

    // Trickle-controlled retransmissions:
    uint32_t mBackoff;                       // mBackoff for sampling the numner of messages
    uint8_t mRefCount;                       // Reference counter of the current instance
    uint8_t mMsgsReceived;                   // number of messages heard during the mBackoff period
    uint8_t mRebroadcastThreshold;           // re-broadcast threshold
    BitFlags<uint16_t, ExFlagValues> mFlags; // Internal state flags

    CHIP_ERROR ResendMessage();
    static void CancelRetransmissionTimer(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerTau(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerT(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR StartResponseTimer();
    void CancelResponseTimer();
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    void DoClose(bool clearRetransTable);
};

} // namespace chip
