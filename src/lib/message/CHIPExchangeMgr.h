/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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

#ifndef CHIP_EXCHANGE_MGR_H
#define CHIP_EXCHANGE_MGR_H

#include <message/CHIPBinding.h>
#include <message/CHIPFabricState.h>
#include <message/CHIPMessageLayer.h>
#include <message/CHIPRMPConfig.h>
#include <support/DLLUtil.h>
#include <system/SystemTimer.h>

#define EXCHANGE_CONTEXT_ID(x) ((x) + 1)

namespace chip {

using System::PacketBuffer;

struct ChipMessageInfo;
class ChipExchangeManager;
class ChipMessageLayer;
class ChipConnection;
class Binding;

/**
 *  @class ChipExchangeHeader
 *
 *  @brief
 *    This specifies the CHIP profile and message type of a particular CHIP message
 *    within an ExchangeContext.
 */
class ChipExchangeHeader
{
public:
    uint8_t Version;     /**< The version of the CHIP Exchange Header format */
    uint8_t Flags;       /**< Bit flag indicators for the type of CHIP message */
    uint16_t ExchangeId; /**< The Exchange identifier for the ExchangeContext */
    uint32_t ProfileId;  /**< The Profile identifier of the CHIP message */
    uint8_t MessageType; /**< The Message type for the specified CHIP profile */
    uint32_t AckMsgId;   /**< Optional; Message identifier being acknowledged.
                              Specified when requiring acknowledgments. */
};

/**
 *  @brief
 *    The CHIP Exchange header version.
 */
typedef enum ChipExchangeVersion
{
    kChipExchangeVersion_V1 = 1
} ChipExchangeVersion;

/**
 *  @brief
 *    The CHIP Exchange header flag bits.
 */
typedef enum ChipExchangeFlags
{
    kChipExchangeFlag_Initiator = 0x1, /**< Set when current message is sent by the initiator of an exchange */
    kChipExchangeFlag_AckId     = 0x2, /**< Set when current message is an acknowledgment for a previously received message */
    kChipExchangeFlag_NeedsAck  = 0x4  /**< Set when current message is requesting an acknowledgment from the recipient. */
} ChipExchangeFlags;

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
    friend class ChipExchangeManager;
    friend class ChipMessageLayer;

public:
    typedef uint32_t Timeout; /**< Type used to express the timeout in this ExchangeContext, in milliseconds */

    ChipExchangeManager * ExchangeMgr; /**< [READ ONLY] Owning exchange manager. */
    uint16_t mMsgProtocolVersion;      /**< Message Protocol version for the ExchangeContext. */
    ChipConnection * Con;              /**< [READ ONLY] Associated CHIP connection. */
    uint64_t PeerNodeId;               /**< [READ ONLY] Node ID of peer node. */
    IPAddress PeerAddr;                /**< [READ ONLY] IP address of peer node. */
    InterfaceId
        PeerIntf; /**< [READ ONLY] Outbound interface to be used when sending messages to the peer. (Only meaningful for UDP.) */
    uint16_t PeerPort;        /**< [READ ONLY] Port of peer node. */
    uint16_t ExchangeId;      /**< [READ ONLY] Assigned exchange ID. */
    void * AppState;          /**< Pointer to application-specific state object. */
    bool AllowDuplicateMsgs;  /**< Boolean indicator of whether duplicate messages are allowed for a given exchange. */
    uint8_t EncryptionType;   /**< Encryption type to use when sending a message. */
    uint16_t KeyId;           /**< Encryption key to use when sending a message. */
    uint32_t RetransInterval; /**< Time between retransmissions (in milliseconds); 0 disables retransmissions. */
    Timeout ResponseTimeout;  /**< Maximum time to wait for response (in milliseconds); 0 disables response timeout. */
    RMPConfig mRMPConfig;     /**< RMP configuration. */
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

    bool IsInitiator(void) const;
    bool IsConnectionClosed(void) const;
    bool IsResponseExpected(void) const;
    void SetInitiator(bool inInitiator);
    void SetConnectionClosed(bool inConnectionClosed);
    bool ShouldDropAck(void) const;
    bool IsAckPending(void) const;
    void SetDropAck(bool inDropAck);
    void SetAckPending(bool inAckPending);
    bool HasPeerRequestedAck(void) const;
    void SetPeerRequestedAck(bool inPeerRequestedAck);
    bool HasRcvdMsgFromPeer(void) const;
    void SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer);
    CHIP_ERROR RMPFlushAcks(void);
    uint32_t GetCurrentRetransmitTimeout(void);
    void SetResponseExpected(bool inResponseExpected);
    bool AutoRequestAck() const;
    void SetAutoRequestAck(bool autoReqAck);
    bool GetAutoReleaseKey() const;
    void SetAutoReleaseKey(bool autoReleaseKey);
    bool ShouldAutoReleaseConnection() const;
    void SetShouldAutoReleaseConnection(bool autoReleaseCon);
    bool UseEphemeralUDPPort(void) const;
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    void SetUseEphemeralUDPPort(bool val);
#endif

    CHIP_ERROR SendMessage(uint32_t profileId, uint8_t msgType, PacketBuffer * msgPayload, uint16_t sendFlags = 0,
                           void * msgCtxt = 0);
    CHIP_ERROR SendMessage(uint32_t profileId, uint8_t msgType, PacketBuffer * msgBuf, uint16_t sendFlags,
                           ChipMessageInfo * msgInfo, void * msgCtxt = 0);
    CHIP_ERROR SendCommonNullMessage(void);
    CHIP_ERROR EncodeExchHeader(ChipExchangeHeader * exchangeHeader, uint32_t profileId, uint8_t msgType, PacketBuffer * msgBuf,
                                uint16_t sendFlags);
    void TeardownTrickleRetransmit(void);

    /**
     * This function is the application callback for handling a received CHIP message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    pktInfo       A pointer to the IPPacketInfo object.
     *
     *  @param[in]    msgInfo       A pointer to the ChipMessageInfo object.
     *
     *  @param[in]    profileId     The profile identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @param[in]    payload       A pointer to the PacketBuffer object holding the message payload.
     */
    typedef void (*MessageReceiveFunct)(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                                        uint32_t profileId, uint8_t msgType, PacketBuffer * payload);
    MessageReceiveFunct OnMessageReceived;

    /**
     * This function is the application callback to invoke when the timeout for the receipt
     * of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*ResponseTimeoutFunct)(ExchangeContext * ec);
    ResponseTimeoutFunct OnResponseTimeout;

    /**
     * This function is the application callback to invoke when the timeout for the retransmission
     * of a previously sent message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*RetransmissionTimeoutFunct)(ExchangeContext * ec);
    RetransmissionTimeoutFunct OnRetransmissionTimeout;

    /**
     * This function is the application callback to invoke when an existing CHIP connection
     * has been closed.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    con           A pointer to the ChipConnection object.
     *
     *  @param[in]    conErr        The CHIP_ERROR type that was reported when the connection
     *                              was closed.
     *
     */
    typedef void (*ConnectionClosedFunct)(ExchangeContext * ec, ChipConnection * con, CHIP_ERROR conErr);
    ConnectionClosedFunct OnConnectionClosed;

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
     * This function is the application callback to invoke when key error message has been received
     * from the peer.
     */
    KeyErrorFunct OnKeyError;

    void CancelRetrans(void);

    CHIP_ERROR RMPSendThrottleFlow(uint32_t PauseTimeMillis);
    CHIP_ERROR RMPSendDelayedDelivery(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);

    /**
     * This function is the application callback to invoke when an Acknowledgment is received
     * for a CHIP message that requested one as part of the CHIP Reliable Messaging Protocol.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    msgCtxt       A pointer to some specific context object associated with
     *                              the original message being acknowledged.
     *
     */
    typedef void (*RMPAckRcvdFunct)(ExchangeContext * ec, void * msgCtxt);

    /**
     * This function is the application callback to invoke when a Throttle message or Delayed
     * Delivery message is received as part of the CHIP Reliable Messaging Protocol. Each of
     * these messages are accompanied with a time value (in milliseconds) that signifies the
     * time to pause sending of CHIP messages on this ExchangeContext.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    pauseTime     Time to pause transmission (in milliseconds).
     *
     */
    typedef void (*RMPPauseRcvdFunct)(ExchangeContext * ec, uint32_t pauseTime);

    /**
     * This function is the application callback to invoke when an error is encountered while
     * sending a CHIP message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    err           The CHIP_ERROR type that was encountered during the
     *                              sending of the message.
     *
     *  @param[in]    msgCtxt       A pointer to some specific context object associated with
     *                              the original message being reported on.
     *
     */
    typedef void (*RMPSendErrorFunct)(ExchangeContext * ec, CHIP_ERROR err, void * msgCtxt);

    RMPPauseRcvdFunct OnThrottleRcvd; /**< Application callback for received Throttle message. */
    RMPPauseRcvdFunct OnDDRcvd;       /**< Application callback for received Delayed Delivery message. */
    RMPSendErrorFunct OnSendError;    /**< Application callback for error while sending. */
    RMPAckRcvdFunct OnAckRcvd;        /**< Application callback for received acknowledgment. */

    /*
     * in order to use reference counting (see refCount below)
     * we use a hold/free paradigm where users of the exchange
     * can hold onto it while it's out of their direct control
     * to make sure it isn't closed before everyone's ready.
     */
    void AddRef(void);
    void Close(void);
    void Abort(void);
    void Release(void);
    CHIP_ERROR StartTimerT(void);

    enum
    {
        kGetPeerDescription_MaxLength = chip::kChipPeerDescription_MaxLength,
        /**< Maximum length of string (including NUL character)
             returned by GetPeerDescription(). */
    };

    void GetPeerDescription(char * buf, uint32_t bufSize) const;

private:
    PacketBuffer * msg; // If we are re-transmitting, then this is the pointer to the message being retransmitted

    // Trickle-controlled retransmissions:
    uint32_t backoff;             // backoff for sampling the numner of messages
    uint8_t msgsReceived;         // number of messages heard during the backoff period
    uint8_t rebroadcastThreshold; // re-broadcast threshold
    uint16_t mFlags;              // Internal state flags

    CHIP_ERROR ResendMessage(void);
    bool MatchExchange(ChipConnection * msgCon, const ChipMessageInfo * msgInfo, const ChipExchangeHeader * exchangeHeader);
    static void CancelRetransmissionTimer(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerTau(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static void TimerT(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR StartResponseTimer(void);
    void CancelResponseTimer(void);
    static void HandleResponseTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    uint32_t mPendingPeerAckId;
    uint16_t mRMPNextAckTime;     // Next time for triggering Solo Ack
    uint16_t mRMPThrottleTimeout; // Timeout until when Throttle is On when RMPThrottleEnabled is set
    void DoClose(bool clearRetransTable);
    CHIP_ERROR HandleMessage(ChipMessageInfo * msgInfo, const ChipExchangeHeader * exchHeader, PacketBuffer * msgBuf);
    CHIP_ERROR HandleMessage(ChipMessageInfo * msgInfo, const ChipExchangeHeader * exchHeader, PacketBuffer * msgBuf,
                             ExchangeContext::MessageReceiveFunct umhandler);
    void HandleConnectionClosed(CHIP_ERROR conErr);

    bool RMPCheckAndRemRetransTable(uint32_t msgId, void ** rCtxt);
    CHIP_ERROR RMPHandleRcvdAck(const ChipExchangeHeader * exchHeader, const ChipMessageInfo * msgInfo);
    CHIP_ERROR RMPHandleNeedsAck(const ChipMessageInfo * msgInfo);
    CHIP_ERROR HandleThrottleFlow(uint32_t PauseTimeMillis);

    uint8_t mRefCount;
};

/**
 *  @class ChipExchangeManager
 *
 *  @brief
 *    This class is used to manage ExchangeContexts with other CHIP nodes.
 *    It works on behalf of higher layers, creating ExchangeContexts and
 *    handling the registration/unregistration of unsolicited message handlers.
 *
 */
class DLL_EXPORT ChipExchangeManager
{
    friend class Binding;
    friend class ExchangeContext;
    friend class ChipMessageLayer;
    friend class ChipConnection;
    friend class ChipSecurityManager;
    friend class ChipFabricState;

public:
    enum State
    {
        kState_NotInitialized = 0, /**< Used to indicate that the ChipExchangeManager is not initialized */
        kState_Initialized    = 1  /**< Used to indicate that the ChipExchangeManager is initialized */
    };

    ChipExchangeManager(void);
    ChipExchangeManager(const ChipExchangeManager &) = delete;
    ChipExchangeManager operator=(const ChipExchangeManager &) = delete;

    ChipMessageLayer * MessageLayer; /**< [READ ONLY] The associated ChipMessageLayer object. */
    ChipFabricState * FabricState;   /**< [READ ONLY] The associated FabricState object. */
    uint8_t State;                   /**< [READ ONLY] The state of the ChipExchangeManager object. */

    CHIP_ERROR Init(ChipMessageLayer * msgLayer);
    CHIP_ERROR Shutdown(void);

#if CHIP_CONFIG_TEST
    size_t ExpireExchangeTimers(void);
#endif

    ExchangeContext * NewContext(const uint64_t & peerNodeId, void * appState = NULL);
    ExchangeContext * NewContext(const uint64_t & peerNodeId, const IPAddress & peerAddr, void * appState = NULL);
    ExchangeContext * NewContext(const uint64_t & peerNodeId, const IPAddress & peerAddr, uint16_t peerPort, InterfaceId sendIntfId,
                                 void * appState = NULL);
    ExchangeContext * NewContext(ChipConnection * con, void * appState = NULL);

    ExchangeContext * FindContext(uint64_t peerNodeId, ChipConnection * con, void * appState, bool isInitiator);

    Binding * NewBinding(Binding::EventCallback eventCallback = Binding::DefaultEventHandler, void * appState = NULL);

    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, ExchangeContext::MessageReceiveFunct handler, void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, ExchangeContext::MessageReceiveFunct handler, bool allowDups,
                                                 void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                                 bool allowDups, void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, ChipConnection * con,
                                                 ExchangeContext::MessageReceiveFunct handler, void * appState);
    CHIP_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, ChipConnection * con,
                                                 ExchangeContext::MessageReceiveFunct handler, bool allowDups, void * appState);
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId);
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType);
    CHIP_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, ChipConnection * con);

    void AllowUnsolicitedMessages(ChipConnection * con);
    void ClearMsgCounterSyncReq(uint64_t peerNodeId);

private:
    uint16_t NextExchangeId;
    uint64_t mRMPTimeStampBase;                  // RMP timer base value to add offsets to evaluate timeouts
    System::Timer::Epoch mRMPCurrentTimerExpiry; // Tracks when the RMP timer will next expire
    uint16_t mRMPTimerInterval;                  // RMP Timer tick period
    /**
     *  @class RetransTableEntry
     *
     *  @brief
     *    This class is part of the CHIP Reliable Messaging Protocol and is used
     *    to keep track of CHIP messages that have been sent and are expecting an
     *    acknowledgment back. If the acknowledgment is not received within a
     *    specific timeout, the message would be retransmitted from this table.
     *
     */
    class RetransTableEntry
    {
    public:
        uint32_t msgId;                /**< The message identifier of the CHIP message awaiting acknowledgment. */
        ExchangeContext * exchContext; /**< The ExchangeContext for the stored CHIP message. */
        PacketBuffer * msgBuf;         /**< A pointer to the PacketBuffer object holding the CHIP message. */
        void * msgCtxt;                /**< A pointer to an application level context object associated with the message. */
        uint16_t nextRetransTime;      /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;             /**< A counter representing the number of times the message has been sent. */
    };
    void RMPExecuteActions(void);
    void RMPExpireTicks(void);
    void RMPStartTimer(void);
    void RMPStopTimer(void);
    void RMPProcessDDMessage(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);
    uint32_t GetTickCounterFromTimeDelta(uint64_t newTime, uint64_t oldTime);
    static void RMPTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);
    static bool isLaterInRMP(uint64_t t2, uint64_t t1);
    bool IsSendErrorCritical(CHIP_ERROR err) const;
    CHIP_ERROR AddToRetransTable(ExchangeContext * ec, PacketBuffer * inetBuff, uint32_t msgId, void * msgCtxt,
                                 RetransTableEntry ** rEntry);
    CHIP_ERROR SendFromRetransTable(RetransTableEntry * entry);
    void ClearRetransmitTable(ExchangeContext * ec);
    void ClearRetransmitTable(RetransTableEntry & rEntry);
    void FailRetransmitTableEntries(ExchangeContext * ec, CHIP_ERROR err);
    void RetransPendingAppGroupMsgs(uint64_t peerNodeId);

    void TicklessDebugDumpRetransTable(const char * log);

    // RMP Global tables for timer context
    RetransTableEntry RetransTable[CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE];

    class UnsolicitedMessageHandler
    {
    public:
        ExchangeContext::MessageReceiveFunct Handler;
        void * AppState;
        uint32_t ProfileId;
        ChipConnection * Con; // NULL means any connection, or no connection (i.e. UDP)
        int16_t MessageType;  // -1 represents any message type
        bool AllowDuplicateMsgs;
    };

    ExchangeContext ContextPool[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS];
    size_t mContextsInUse;

    Binding BindingPool[CHIP_CONFIG_MAX_BINDINGS];
    size_t mBindingsInUse;

    UnsolicitedMessageHandler UMHandlerPool[CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];
    void (*OnExchangeContextChanged)(size_t numContextsInUse);

    ExchangeContext * AllocContext(void);

    void HandleConnectionReceived(ChipConnection * con);
    void HandleConnectionClosed(ChipConnection * con, CHIP_ERROR conErr);
    void DispatchMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR RegisterUMH(uint32_t profileId, int16_t msgType, ChipConnection * con, bool allowDups,
                           ExchangeContext::MessageReceiveFunct handler, void * appState);
    CHIP_ERROR UnregisterUMH(uint32_t profileId, int16_t msgType, ChipConnection * con);

    static void HandleAcceptError(ChipMessageLayer * msgLayer, CHIP_ERROR err);
    static void HandleMessageReceived(ChipMessageLayer * msgLayer, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    static void HandleMessageReceived(ChipConnection * con, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    static CHIP_ERROR PrependHeader(ChipExchangeHeader * exchangeHeader, PacketBuffer * buf);
    static CHIP_ERROR DecodeHeader(ChipExchangeHeader * exchangeHeader, ChipMessageInfo * msgInfo, PacketBuffer * buf);

    void InitBindingPool(void);
    Binding * AllocBinding(void);
    void FreeBinding(Binding * binding);
    uint16_t GetBindingLogId(const Binding * const binding) const;

    void NotifySecurityManagerAvailable();
    void NotifyKeyFailed(uint64_t peerNodeId, uint16_t keyId, CHIP_ERROR keyErr);
};

#if !CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

inline bool ExchangeContext::UseEphemeralUDPPort(void) const
{
    return false;
}

#endif // !CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

} // namespace chip

#endif // CHIP_EXCHANGE_MGR_H
