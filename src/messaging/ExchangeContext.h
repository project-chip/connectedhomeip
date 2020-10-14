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

namespace chip {

class ExchangeManager;
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
     *  @param[in]    msgType       The message type of the corresponding protocol.
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
    enum
    {
        kSendFlag_ExpectResponse = 0x0001, // Used to indicate that a response is expected within a specified timeout.
        kSendFlag_RetainBuffer   = 0x0002, // Used to indicate that the message buffer should not be freed after sending.
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
     *  @param[in]    msgType       The message type of the corresponding protocol.
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
     *  @param[in]    protocolId    The protocol identifier of the CHIP message to be sent.
     *
     *  @param[in]    msgType       The message type of the corresponding protocol.
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
    CHIP_ERROR SendMessage(uint16_t protocolId, uint8_t msgType, System::PacketBuffer * msgPayload, uint16_t sendFlags = 0,
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

    void SetDelegate(ExchangeContextDelegate * delegate) { mDelegate = delegate; }

    ExchangeContextDelegate * GetDelegate() const { return mDelegate; }

    void SetExchangeMgr(ExchangeManager * exMgr) { mExchangeMgr = exMgr; }

    ExchangeManager * GetExchangeMgr() const { return mExchangeMgr; }

    void SetPeerNodeId(uint64_t nodeId) { mPeerNodeId = nodeId; }

    uint64_t GetPeerNodeId() const { return mPeerNodeId; }

    void SetExchangeId(uint16_t exId) { mExchangeId = exId; }

    uint16_t GetExchangeId() const { return mExchangeId; }

    void SetAppState(void * state) { mAppState = state; }

    void * GetAppState() const { return mAppState; }

    void SetAllowDuplicateMsgs(bool value) { mAllowDuplicateMsgs = value; }

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
    enum class ExFlagValues : uint16_t
    {
        kFlagInitiator        = 0x0001, // This context is the initiator of the exchange.
        kFlagResponseExpected = 0x0002, // If a response is expected for a message that is being sent.
    };

    typedef uint32_t Timeout; // Type used to express the timeout in this ExchangeContext, in milliseconds

    Timeout mResponseTimeout;    // Maximum time to wait for response (in milliseconds); 0 disables response timeout.
    System::PacketBuffer * mMsg; // If we are re-transmitting, then this is the pointer to the message being retransmitted
    ExchangeContextDelegate * mDelegate = nullptr;
    ExchangeManager * mExchangeMgr;
    void * mAppState; // Pointer to application-specific state object.

    uint64_t mPeerNodeId;     // Node ID of peer node.
    uint16_t mExchangeId;     // Assigned exchange ID.
    uint8_t mRefCount;        // Reference counter of the current instance
    bool mAllowDuplicateMsgs; // Boolean indicator of whether duplicate messages are allowed for a given exchange.

    BitFlags<uint16_t, ExFlagValues> mFlags; // Internal state flags

    CHIP_ERROR ResendMessage();
    CHIP_ERROR StartResponseTimer();
    void CancelResponseTimer();
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    void DoClose(bool clearRetransTable);
};

} // namespace chip
