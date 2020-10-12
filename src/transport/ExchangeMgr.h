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
    uint64_t PeerNodeId;           /**< [READ ONLY] Node ID of peer node. */
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

        kSendFlag_MulticastFromLinkLocal = kSendFlag_DefaultMulticastSourceAddress,
        /**< Deprecated alias for \c kSendFlag_DefaultMulticastSourceAddress */
    };

    bool IsInitiator() const;
    bool IsResponseExpected() const;
    void SetInitiator(bool inInitiator);
    void SetResponseExpected(bool inResponseExpected);

    CHIP_ERROR SendMessage(uint32_t protocolId, uint8_t msgType, System::PacketBuffer * msgPayload, uint16_t sendFlags = 0,
                           void * msgCtxt = nullptr);
    CHIP_ERROR SendCommonNullMessage();

    void TeardownTrickleRetransmit();

    /**
     * This function is the protocol callback for handling a received CHIP message.
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
    MessageReceiveFunct OnMessageReceived;

    /**
     * This function is the protocol callback to invoke when the timeout for the receipt
     * of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*ResponseTimeoutFunct)(ExchangeContext * ec);
    ResponseTimeoutFunct OnResponseTimeout;

    /**
     * This function is the protocol callback to invoke when the timeout for the retransmission
     * of a previously sent message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*RetransmissionTimeoutFunct)(ExchangeContext * ec);
    RetransmissionTimeoutFunct OnRetransmissionTimeout;

    /**
     * Type of key error message handling function.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    keyErr        The CHIP_ERROR type that was reported in the key error message.
     *
     */
    typedef void (*KeyErrorFunct)(ExchangeContext * ec, CHIP_ERROR keyErr);

    /**
     * This function is the protocol callback to invoke when key error message has been received
     * from the peer.
     */
    KeyErrorFunct OnKeyError;

    void CancelRetrans();

    /*
     * in order to use reference counting (see refCount below)
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

    uint32_t mPendingPeerAckId;
    void DoClose(bool clearRetransTable);
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf);
    CHIP_ERROR HandleMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, System::PacketBuffer * msgBuf,
                             ExchangeContext::MessageReceiveFunct umhandler);

    uint8_t mRefCount;
};

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

    SecureSessionMgrBase * MessageLayer; /**< [READ ONLY] The associated SecureSessionMgrBase object. */
    uint8_t State;                       /**< [READ ONLY] The state of the ExchangeManager object. */

    CHIP_ERROR Init(SecureSessionMgrBase * msgLayer);
    CHIP_ERROR Shutdown();

    ExchangeContext * NewContext(const uint64_t & peerNodeId, void * appState = nullptr);

    ExchangeContext * FindContext(uint64_t peerNodeId, void * appState, bool isInitiator);

    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeContext::MessageReceiveFunct handler,
                                                 void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeContext::MessageReceiveFunct handler, bool allowDups,
                                                 void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 bool allowDups, void * appState);
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId);
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType);

private:
    uint16_t NextExchangeId;

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
