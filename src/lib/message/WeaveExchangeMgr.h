/*
 *
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
 *      This file defines the classes corresponding to Weave Exchange management.
 *
 */

// Include WeaveCore.h OUTSIDE of the include guard for WeaveExchangeMgr.h.
// This allows WeaveCore.h to enforce a canonical include order for core
// header files, making it easier to manage dependencies between these files.
#include <Weave/Core/WeaveCore.h>

#ifndef WEAVE_EXCHANGE_MGR_H
#define WEAVE_EXCHANGE_MGR_H

#include <Weave/Support/NLDLLUtil.h>
#include <Weave/Core/WeaveWRMPConfig.h>
#include <SystemLayer/SystemTimer.h>

 #define EXCHANGE_CONTEXT_ID(x)     ((x)+1)

namespace nl {
namespace Weave {

using System::PacketBuffer;

struct WeaveMessageInfo;
class WeaveExchangeManager;
class WeaveMessageLayer;
class WeaveConnection;
class Binding;

/**
 *  @def WEAVE_TRICKLE_DEFAULT_PERIOD
 *
 *  @brief
 *    Defines Trickle algorithm's default period (in milliseconds) for periodic
 *    transmissions.
 *
 */
#define WEAVE_TRICKLE_DEFAULT_PERIOD 1000

/**
 *  @def WEAVE_TRICKLE_DEFAULT_THRESHOLD
 *
 *  @brief
 *    Defines Trickle algorithm's default value for the maximum number of received
 *    duplicate messages to wait before retransmission.
 *
 */
#define WEAVE_TRICKLE_DEFAULT_THRESHOLD 2

/**
 *  @class WeaveExchangeHeader
 *
 *  @brief
 *    This specifies the Weave profile and message type of a particular Weave message
 *    within an ExchangeContext.
 */
class WeaveExchangeHeader
{
public:
    uint8_t  Version;        /**< The version of the Weave Exchange Header format */
    uint8_t  Flags;          /**< Bit flag indicators for the type of Weave message */
    uint16_t ExchangeId;     /**< The Exchange identifier for the ExchangeContext */
    uint32_t ProfileId;      /**< The Profile identifier of the Weave message */
    uint8_t  MessageType;    /**< The Message type for the specified Weave profile */
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    uint32_t AckMsgId;       /**< Optional; Message identifier being acknowledged.
                                  Specified when requiring acknowledgments. */
#endif
};

/**
 *  @brief
 *    The Weave Exchange header version.
 */
typedef enum WeaveExchangeVersion
{
    kWeaveExchangeVersion_V1 = 1
} WeaveExchangeVersion;

/**
 *  @brief
 *    The Weave Exchange header flag bits.
 */
typedef enum WeaveExchangeFlags
{
    kWeaveExchangeFlag_Initiator     = 0x1,  /**< Set when current message is sent by the initiator of an exchange */
    kWeaveExchangeFlag_AckId         = 0x2,  /**< Set when current message is an acknowledgment for a previously received message */
    kWeaveExchangeFlag_NeedsAck      = 0x4   /**< Set when current message is requesting an acknowledgment from the recipient. */
} WeaveExchangeFlags;

/**
 *  @class ExchangeContext
 *
 *  @brief
 *    This class represents an ongoing conversation (ExchangeContext) between two or more nodes.
 *    It defines methods for encoding and communicating Weave messages within an ExchangeContext
 *    over various transport mechanisms, for example, TCP, UDP, or Weave Reliable Messaging.
 *
 */
class NL_DLL_EXPORT ExchangeContext
{
    friend class WeaveExchangeManager;
    friend class WeaveMessageLayer;

public:

    typedef uint32_t Timeout;                   /**< Type used to express the timeout in this ExchangeContext, in milliseconds */

    WeaveExchangeManager *ExchangeMgr;          /**< [READ ONLY] Owning exchange manager. */
    uint16_t mMsgProtocolVersion;               /**< Message Protocol version for the ExchangeContext. */
    WeaveConnection *Con;                       /**< [READ ONLY] Associated Weave connection. */
    uint64_t PeerNodeId;                        /**< [READ ONLY] Node ID of peer node. */
    IPAddress PeerAddr;                         /**< [READ ONLY] IP address of peer node. */
    InterfaceId PeerIntf;                       /**< [READ ONLY] Outbound interface to be used when sending messages to the peer. (Only meaningful for UDP.) */
    uint16_t PeerPort;                          /**< [READ ONLY] Port of peer node. */
    uint16_t ExchangeId;                        /**< [READ ONLY] Assigned exchange ID. */
    void *AppState;                             /**< Pointer to application-specific state object. */
    bool AllowDuplicateMsgs;                    /**< Boolean indicator of whether duplicate messages are allowed for a given exchange. */
    uint8_t EncryptionType;                     /**< Encryption type to use when sending a message. */
    uint16_t KeyId;                             /**< Encryption key to use when sending a message. */
    uint32_t RetransInterval;                   /**< Time between retransmissions (in milliseconds); 0 disables retransmissions. */
    Timeout ResponseTimeout;                    /**< Maximum time to wait for response (in milliseconds); 0 disables response timeout. */
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    WRMPConfig mWRMPConfig;                     /**< WRMP configuration. */
#endif
    enum
    {
        kSendFlag_AutoRetrans                   = 0x0001, /**< Used to indicate that automatic retransmission is enabled. */
        kSendFlag_ExpectResponse                = 0x0002, /**< Used to indicate that a response is expected within a specified timeout. */
        kSendFlag_RetransmissionTrickle         = 0x0004, /**< Used to indicate the requirement of retransmissions for Trickle. */
        kSendFlag_DelaySend                     = 0x0008, /**< Used to indicate that the sending of the current message needs to be delayed. */
        kSendFlag_ReuseMessageId                = 0x0010, /**< Used to indicate that the message ID in the message header can be reused. */
        kSendFlag_ReuseSourceId                 = 0x0020, /**< Used to indicate that the source node ID in the message header can be reused. */
        kSendFlag_RetainBuffer                  = 0x0040, /**< Used to indicate that the message buffer should not be freed after sending. */
        kSendFlag_AlreadyEncoded                = 0x0080, /**< Used to indicate that the message is already encoded. */
        kSendFlag_DefaultMulticastSourceAddress = 0x0100, /**< Used to indicate that default IPv6 source address selection should be used when
                                                               sending IPv6 multicast messages. */
        kSendFlag_FromInitiator                 = 0x0200, /**< Used to indicate that the current message is the initiator of the exchange. */
        kSendFlag_RequestAck                    = 0x0400, /**< Used to send a WRM message requesting an acknowledgment. */
        kSendFlag_NoAutoRequestAck              = 0x0800, /**< Suppress the auto-request acknowledgment feature when sending a message. */

        kSendFlag_MulticastFromLinkLocal        = kSendFlag_DefaultMulticastSourceAddress,
                                                          /**< Deprecated alias for \c kSendFlag_DefaultMulticastSourceAddress */
    };

    bool IsInitiator(void) const;
    bool IsConnectionClosed(void) const;
    bool IsResponseExpected(void) const;
    void SetInitiator(bool inInitiator);
    void SetConnectionClosed(bool inConnectionClosed);
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    bool ShouldDropAck(void) const;
    bool IsAckPending(void) const;
    void SetDropAck(bool inDropAck);
    void SetAckPending(bool inAckPending);
    bool HasPeerRequestedAck(void) const;
    void SetPeerRequestedAck(bool inPeerRequestedAck);
    bool HasRcvdMsgFromPeer(void) const;
    void SetMsgRcvdFromPeer(bool inMsgRcvdFromPeer);
    WEAVE_ERROR WRMPFlushAcks(void);
    uint32_t GetCurrentRetransmitTimeout(void);
#endif
    void SetResponseExpected(bool inResponseExpected);
    bool AutoRequestAck() const;
    void SetAutoRequestAck(bool autoReqAck);
    bool GetAutoReleaseKey() const;
    void SetAutoReleaseKey(bool autoReleaseKey);
    bool ShouldAutoReleaseConnection() const;
    void SetShouldAutoReleaseConnection(bool autoReleaseCon);
    bool UseEphemeralUDPPort(void) const;
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    void SetUseEphemeralUDPPort(bool val);
#endif

    WEAVE_ERROR SendMessage(uint32_t profileId, uint8_t msgType, PacketBuffer *msgPayload, uint16_t sendFlags = 0, void *msgCtxt = 0);
    WEAVE_ERROR SendMessage(uint32_t profileId, uint8_t msgType, PacketBuffer *msgBuf, uint16_t sendFlags, WeaveMessageInfo * msgInfo, void *msgCtxt = 0);
    WEAVE_ERROR SendCommonNullMessage(void);
    WEAVE_ERROR EncodeExchHeader(WeaveExchangeHeader *exchangeHeader, uint32_t profileId, uint8_t msgType, PacketBuffer *msgBuf, uint16_t sendFlags);
    void TeardownTrickleRetransmit(void);
    WEAVE_ERROR SetupTrickleRetransmit(uint32_t retransInterval=WEAVE_TRICKLE_DEFAULT_PERIOD, uint8_t threshold=WEAVE_TRICKLE_DEFAULT_THRESHOLD, uint32_t timeout=0);


    /**
     * This function is the application callback for handling a received Weave message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    pktInfo       A pointer to the IPPacketInfo object.
     *
     *  @param[in]    msgInfo       A pointer to the WeaveMessageInfo object.
     *
     *  @param[in]    profileId     The profile identifier of the received message.
     *
     *  @param[in]    msgType       The message type of the corresponding profile.
     *
     *  @param[in]    payload       A pointer to the PacketBuffer object holding the message payload.
     */
    typedef void (*MessageReceiveFunct)(ExchangeContext *ec, const IPPacketInfo *pktInfo, const WeaveMessageInfo *msgInfo, uint32_t profileId,
            uint8_t msgType, PacketBuffer *payload);
    MessageReceiveFunct OnMessageReceived;

    /**
     * This function is the application callback to invoke when the timeout for the receipt
     * of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*ResponseTimeoutFunct)(ExchangeContext *ec);
    ResponseTimeoutFunct OnResponseTimeout;

    /**
     * This function is the application callback to invoke when the timeout for the retransmission
     * of a previously sent message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     */
    typedef void (*RetransmissionTimeoutFunct)(ExchangeContext *ec);
    RetransmissionTimeoutFunct OnRetransmissionTimeout;

    /**
     * This function is the application callback to invoke when an existing Weave connection
     * has been closed.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    con           A pointer to the WeaveConnection object.
     *
     *  @param[in]    conErr        The WEAVE_ERROR type that was reported when the connection
     *                              was closed.
     *
     */
    typedef void (*ConnectionClosedFunct)(ExchangeContext *ec, WeaveConnection *con, WEAVE_ERROR conErr);
    ConnectionClosedFunct OnConnectionClosed;

    /**
     * Type of key error message handling function.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    keyErr        The WEAVE_ERROR type that was reported in the key error message.
     *
     */
    typedef void (*KeyErrorFunct)(ExchangeContext *ec, WEAVE_ERROR keyErr);

    /**
     * This function is the application callback to invoke when key error message has been received
     * from the peer.
     */
    KeyErrorFunct OnKeyError;

    void CancelRetrans(void);
    void HandleTrickleMessage(const IPPacketInfo *pktInfo, const WeaveMessageInfo *msgInfo);

#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    WEAVE_ERROR WRMPSendThrottleFlow(uint32_t PauseTimeMillis);
    WEAVE_ERROR WRMPSendDelayedDelivery(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);

    /**
     * This function is the application callback to invoke when an Acknowledgment is received
     * for a Weave message that requested one as part of the Weave Reliable Messaging Protocol.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    msgCtxt       A pointer to some specific context object associated with
     *                              the original message being acknowledged.
     *
     */
    typedef void (*WRMPAckRcvdFunct)(ExchangeContext *ec, void *msgCtxt);

    /**
     * This function is the application callback to invoke when a Throttle message or Delayed
     * Delivery message is received as part of the Weave Reliable Messaging Protocol. Each of
     * these messages are accompanied with a time value (in milliseconds) that signifies the
     * time to pause sending of Weave messages on this ExchangeContext.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    pauseTime     Time to pause transmission (in milliseconds).
     *
     */
    typedef void (*WRMPPauseRcvdFunct)(ExchangeContext *ec, uint32_t pauseTime);

    /**
     * This function is the application callback to invoke when an error is encountered while
     * sending a Weave message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *
     *  @param[in]    err           The WEAVE_ERROR type that was encountered during the
     *                              sending of the message.
     *
     *  @param[in]    msgCtxt       A pointer to some specific context object associated with
     *                              the original message being reported on.
     *
     */
    typedef void (*WRMPSendErrorFunct)(ExchangeContext *ec, WEAVE_ERROR err, void *msgCtxt);

    WRMPPauseRcvdFunct OnThrottleRcvd;            /**< Application callback for received Throttle message. */
    WRMPPauseRcvdFunct OnDDRcvd;                  /**< Application callback for received Delayed Delivery message. */
    WRMPSendErrorFunct OnSendError;               /**< Application callback for error while sending. */
    WRMPAckRcvdFunct   OnAckRcvd;                 /**< Application callback for received acknowledgment. */
#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING

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
    WEAVE_ERROR StartTimerT(void);

    enum
    {
        kGetPeerDescription_MaxLength = nl::Weave::kWeavePeerDescription_MaxLength,
                                                             /**< Maximum length of string (including NUL character)
                                                                  returned by GetPeerDescription(). */
    };

    void GetPeerDescription(char * buf, uint32_t bufSize) const;

private:
    PacketBuffer *msg;                            // If we are re-transmitting, then this is the pointer to the message being retransmitted
    // Trickle-controlled retransmissions:
    uint32_t backoff;                           // backoff for sampling the numner of messages
    uint32_t currentBcastMsgID;
    uint8_t msgsReceived;                       // number of messages heard during the backoff period
    uint8_t rebroadcastThreshold;               // re-broadcast threshold

    uint16_t mFlags;                            // Internal state flags

    WEAVE_ERROR ResendMessage(void);
    bool MatchExchange(WeaveConnection *msgCon, const WeaveMessageInfo *msgInfo, const WeaveExchangeHeader *exchangeHeader);
    static void TimerTau(System::Layer* aSystemLayer, void* aAppState, System::Error aError);
    static void CancelRetransmissionTimer(System::Layer* aSystemLayer, void* aAppState, System::Error aError);
    static void TimerT(System::Layer* aSystemLayer, void* aAppState, System::Error aError);

    WEAVE_ERROR StartResponseTimer(void);
    void CancelResponseTimer(void);
    static void HandleResponseTimeout(System::Layer* aSystemLayer, void* aAppState, System::Error aError);

    uint32_t mPendingPeerAckId;
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    uint16_t mWRMPNextAckTime;                  //Next time for triggering Solo Ack
    uint16_t mWRMPThrottleTimeout;              //Timeout until when Throttle is On when WRMPThrottleEnabled is set
#endif
    void DoClose(bool clearRetransTable);
    WEAVE_ERROR HandleMessage(WeaveMessageInfo *msgInfo, const WeaveExchangeHeader *exchHeader, PacketBuffer *msgBuf);
    WEAVE_ERROR HandleMessage(WeaveMessageInfo *msgInfo, const WeaveExchangeHeader *exchHeader, PacketBuffer *msgBuf,
                              ExchangeContext::MessageReceiveFunct umhandler);
    void HandleConnectionClosed(WEAVE_ERROR conErr);

#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    bool WRMPCheckAndRemRetransTable(uint32_t msgId, void **rCtxt);
    WEAVE_ERROR WRMPHandleRcvdAck(const WeaveExchangeHeader *exchHeader, const WeaveMessageInfo *msgInfo);
    WEAVE_ERROR WRMPHandleNeedsAck(const WeaveMessageInfo *msgInfo);
    WEAVE_ERROR HandleThrottleFlow(uint32_t PauseTimeMillis);
#endif

    uint8_t mRefCount;
};

/**
 *  @class WeaveExchangeManager
 *
 *  @brief
 *    This class is used to manage ExchangeContexts with other Weave nodes.
 *    It works on behalf of higher layers, creating ExchangeContexts and
 *    handling the registration/unregistration of unsolicited message handlers.
 *
 */
class NL_DLL_EXPORT WeaveExchangeManager
{
    friend class Binding;
    friend class ExchangeContext;
    friend class WeaveMessageLayer;
    friend class WeaveConnection;
    friend class WeaveSecurityManager;
    friend class WeaveFabricState;

public:
    enum State
    {
        kState_NotInitialized = 0,              /**< Used to indicate that the WeaveExchangeManager is not initialized */
        kState_Initialized = 1                  /**< Used to indicate that the WeaveExchangeManager is initialized */
    };

    WeaveExchangeManager(void);

    WeaveMessageLayer *MessageLayer;            /**< [READ ONLY] The associated WeaveMessageLayer object. */
    WeaveFabricState *FabricState;              /**< [READ ONLY] The associated FabricState object. */
    uint8_t State;                              /**< [READ ONLY] The state of the WeaveExchangeManager object. */

    WEAVE_ERROR Init(WeaveMessageLayer *msgLayer);
    WEAVE_ERROR Shutdown(void);

#if WEAVE_CONFIG_TEST
    size_t ExpireExchangeTimers(void);
#endif

    ExchangeContext *NewContext(const uint64_t &peerNodeId, void *appState = NULL);
    ExchangeContext *NewContext(const uint64_t &peerNodeId, const IPAddress &peerAddr, void *appState = NULL);
    ExchangeContext *NewContext(const uint64_t &peerNodeId, const IPAddress &peerAddr, uint16_t peerPort, InterfaceId sendIntfId, void *appState = NULL);
    ExchangeContext *NewContext(WeaveConnection *con, void *appState = NULL);

    ExchangeContext *FindContext(uint64_t peerNodeId, WeaveConnection *con, void *appState, bool isInitiator);

    Binding * NewBinding(Binding::EventCallback eventCallback = Binding::DefaultEventHandler, void *appState = NULL);

    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, ExchangeContext::MessageReceiveFunct handler,
            void *appState);
    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, ExchangeContext::MessageReceiveFunct handler,
            bool allowDups, void *appState);
    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType,
            ExchangeContext::MessageReceiveFunct handler, void *appState);
    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType,
            ExchangeContext::MessageReceiveFunct handler, bool allowDups, void *appState);
    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, WeaveConnection *con,
            ExchangeContext::MessageReceiveFunct handler, void *appState);
    WEAVE_ERROR RegisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, WeaveConnection *con,
            ExchangeContext::MessageReceiveFunct handler, bool allowDups, void *appState);
    WEAVE_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId);
    WEAVE_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType);
    WEAVE_ERROR UnregisterUnsolicitedMessageHandler(uint32_t profileId, uint8_t msgType, WeaveConnection *con);

    void AllowUnsolicitedMessages(WeaveConnection *con);

#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    void ClearMsgCounterSyncReq(uint64_t peerNodeId);
#endif

private:
    uint16_t NextExchangeId;
#if WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING
    uint64_t mWRMPTimeStampBase;    //WRMP timer base value to add offsets to evaluate timeouts
    System::Timer::Epoch mWRMPCurrentTimerExpiry; //Tracks when the WRM timer will next expire
    uint16_t mWRMPTimerInterval;    //WRMP Timer tick period
    /**
     *  @class RetransTableEntry
     *
     *  @brief
     *    This class is part of the Weave Reliable Messaging Protocol and is used
     *    to keep track of Weave messages that have been sent and are expecting an
     *    acknowledgment back. If the acknowledgment is not received within a
     *    specific timeout, the message would be retransmitted from this table.
     *
     */
    class RetransTableEntry
    {
      public:
       uint32_t             msgId;              /**< The message identifier of the Weave message awaiting acknowledgment. */
       ExchangeContext      *exchContext;       /**< The ExchangeContext for the stored Weave message. */
       PacketBuffer         *msgBuf;            /**< A pointer to the PacketBuffer object holding the Weave message. */
       void                 *msgCtxt;           /**< A pointer to an application level context object associated with the message. */
       uint16_t             nextRetransTime;    /**< A counter representing the next retransmission time for the message. */
       uint8_t              sendCount;          /**< A counter representing the number of times the message has been sent. */
    };
    void     WRMPExecuteActions(void);
    void     WRMPExpireTicks(void);
    void     WRMPStartTimer(void);
    void     WRMPStopTimer(void);
    void     WRMPProcessDDMessage(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);
    uint32_t GetTickCounterFromTimeDelta (uint64_t newTime,
                                          uint64_t oldTime);
    static void WRMPTimeout(System::Layer* aSystemLayer, void* aAppState, System::Error aError);
    static bool isLaterInWRMP(uint64_t t2, uint64_t t1);
    bool IsSendErrorCritical(WEAVE_ERROR err) const;
    WEAVE_ERROR AddToRetransTable(ExchangeContext *ec, PacketBuffer *inetBuff, uint32_t msgId, void *msgCtxt, RetransTableEntry **rEntry);
    WEAVE_ERROR SendFromRetransTable(RetransTableEntry *entry);
    void ClearRetransmitTable(ExchangeContext *ec);
    void ClearRetransmitTable(RetransTableEntry &rEntry);
    void FailRetransmitTableEntries(ExchangeContext *ec, WEAVE_ERROR err);
    void RetransPendingAppGroupMsgs(uint64_t peerNodeId);

    void TicklessDebugDumpRetransTable(const char *log);

    //WRMP Global tables for timer context
    RetransTableEntry RetransTable[WEAVE_CONFIG_WRMP_RETRANS_TABLE_SIZE];
#endif // WEAVE_CONFIG_ENABLE_RELIABLE_MESSAGING

    class UnsolicitedMessageHandler
    {
    public:
        ExchangeContext::MessageReceiveFunct Handler;
        void *AppState;
        uint32_t ProfileId;
        WeaveConnection *Con; // NULL means any connection, or no connection (i.e. UDP)
        int16_t MessageType; // -1 represents any message type
        bool AllowDuplicateMsgs;
    };


    ExchangeContext ContextPool[WEAVE_CONFIG_MAX_EXCHANGE_CONTEXTS];
    size_t mContextsInUse;

    Binding BindingPool[WEAVE_CONFIG_MAX_BINDINGS];
    size_t mBindingsInUse;

    UnsolicitedMessageHandler UMHandlerPool[WEAVE_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS];
    void (*OnExchangeContextChanged)(size_t numContextsInUse);

    ExchangeContext *AllocContext(void);

    void HandleConnectionReceived(WeaveConnection *con);
    void HandleConnectionClosed(WeaveConnection *con, WEAVE_ERROR conErr);
    void DispatchMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR RegisterUMH(uint32_t profileId, int16_t msgType, WeaveConnection *con, bool allowDups,
            ExchangeContext::MessageReceiveFunct handler, void *appState);
    WEAVE_ERROR UnregisterUMH(uint32_t profileId, int16_t msgType, WeaveConnection *con);

    static void HandleAcceptError(WeaveMessageLayer *msgLayer, WEAVE_ERROR err);
    static void HandleMessageReceived(WeaveMessageLayer *msgLayer, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    static void HandleMessageReceived(WeaveConnection *con, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    static WEAVE_ERROR PrependHeader(WeaveExchangeHeader *exchangeHeader, PacketBuffer *buf);
    static WEAVE_ERROR DecodeHeader(WeaveExchangeHeader *exchangeHeader, WeaveMessageInfo *msgInfo, PacketBuffer *buf);

    void InitBindingPool(void);
    Binding * AllocBinding(void);
    void FreeBinding(Binding *binding);
    uint16_t GetBindingLogId(const Binding * const binding) const;

    void NotifySecurityManagerAvailable();
    void NotifyKeyFailed(uint64_t peerNodeId, uint16_t keyId, WEAVE_ERROR keyErr);

    WeaveExchangeManager(const WeaveExchangeManager&); // not defined
};

#if !WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

inline bool ExchangeContext::UseEphemeralUDPPort(void) const
{
    return false;
}

#endif // !WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT


} // namespace Weave
} // namespace nl

#endif // WEAVE_EXCHANGE_MGR_H
