/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *      This file defines the classes for the Weave Message Layer.
 *
 */

// Include WeaveCore.h OUTSIDE of the include guard for WeaveMessageLayer.h.
// This allows WeaveCore.h to enforce a canonical include order for core
// header files, making it easier to manage dependencies between these files.
#include <Weave/Core/WeaveCore.h>

#ifndef WEAVE_MESSAGE_LAYER_H
#define WEAVE_MESSAGE_LAYER_H

#include <stdint.h>
#include <string.h>

#include <Weave/Support/NLDLLUtil.h>
#include "HostPortList.h"
#include <SystemLayer/SystemStats.h>

namespace nl {
namespace Weave {

using System::PacketBuffer;

class WeaveMessageLayer;
class WeaveMessageLayerTestObject;
class WeaveExchangeManager;
class WeaveSecurityManager;

namespace Profiles {
namespace StatusReporting {
class StatusReport;
}
}


enum
{
    kWeavePeerDescription_MaxLength = 100,  /**< Maximum length of string (including NUL character) returned by WeaveMessageLayer::GetPeerDescription(). */
};


/**
 *  @brief
 *    Definitions pertaining to the header of an encoded Weave message.
 *
 */
enum
{
    kMsgHeaderField_FlagsMask                           = 0x0F0F,
    kMsgHeaderField_FlagsShift                          = 0,
    kMsgHeaderField_EncryptionTypeMask                  = 0x00F0,
    kMsgHeaderField_EncryptionTypeShift                 = 4,
    kMsgHeaderField_MessageVersionMask                  = 0xF000,
    kMsgHeaderField_MessageVersionShift                 = 12,

    kWeaveHeaderFlag_DestNodeId                         = 0x0100, /**< Indicates that the destination node ID is present in the Weave message header. */
    kWeaveHeaderFlag_SourceNodeId                       = 0x0200, /**< Indicates that the source node ID is present in the Weave message header. */
    kWeaveHeaderFlag_TunneledData                       = 0x0400, /**< Indicates that the Weave message payload is a tunneled IP packet. */
    kWeaveHeaderFlag_MsgCounterSyncReq                  = 0x0800, /**< Indicates that the sender requests message counter synchronization. */

    kMsgHeaderField_ReservedFlagsMask                   = kMsgHeaderField_FlagsMask &
                                                          ~kWeaveHeaderFlag_DestNodeId &
                                                          ~kWeaveHeaderFlag_SourceNodeId &
                                                          ~kWeaveHeaderFlag_TunneledData &
                                                          ~kWeaveHeaderFlag_MsgCounterSyncReq,

    kMsgHeaderField_MessageHMACMask                     = ~((kWeaveHeaderFlag_DestNodeId |
                                                             kWeaveHeaderFlag_SourceNodeId |
                                                             kWeaveHeaderFlag_MsgCounterSyncReq) << kMsgHeaderField_FlagsShift)
};


/**
 *  @struct WeaveMessageInfo
 *
 *  @brief
 *    Information about a Weave message that is in the process of being sent or received.
 *
 */
struct WeaveMessageInfo
{
    uint64_t SourceNodeId;             /**< The source node identifier of the Weave message. */
    uint64_t DestNodeId;               /**< The destination node identifier of the Weave message. */
    uint32_t MessageId;                /**< The message identifier of the Weave message. */
    uint32_t Flags;                    /**< Various flags associated with the Weave message; see WeaveMessageFlags. */
    uint16_t KeyId;                    /**< The encryption key identifier of the Weave message. */
    uint8_t MessageVersion;            /**< The version of the Weave message. */
    uint8_t EncryptionType;            /**< The encryption type used for the Weave message. */
    WeaveAuthMode PeerAuthMode;        /**< The means by which the sender of the message was authenticated. Only meaningful for incoming messages. */
    WeaveConnection *InCon;            /**< The connection (if any) over which the message was received. Only meaningful for incoming messages.*/
    const IPPacketInfo *InPacketInfo;  /**< The IP Addressing information of the received message. Only meaningful for incoming messages. */

    void Clear() { memset(this, 0, sizeof(*this)); }
};

// DEPRECATED alias for WeaveMessageInfo
typedef struct WeaveMessageInfo WeaveMessageHeader;


/**
 *  @brief
 *    Flags associated with a inbound or outbound Weave message.
 *
 *    The values defined here are for use within the WeaveMessageInfo.Flags field.
 */
typedef enum WeaveMessageFlags
{
    kWeaveMessageFlag_ReuseMessageId                    = 0x00000010, /**< Indicates that the existing message identifier must be reused. */
    kWeaveMessageFlag_ReuseSourceId                     = 0x00000020, /**< Indicates that the existing source node identifier must be reused. */
    kWeaveMessageFlag_DelaySend                         = 0x00000040, /**< Indicates that the sending of the message needs to be delayed. */
    kWeaveMessageFlag_RetainBuffer                      = 0x00000080, /**< Indicates that the message buffer should not be freed after sending. */
    kWeaveMessageFlag_MessageEncoded                    = 0x00001000, /**< Indicates that the Weave message is already encoded. */
    kWeaveMessageFlag_DefaultMulticastSourceAddress     = 0x00002000, /**< Indicates that default IPv6 source address selection should be used when
                                                                           sending IPv6 multicast messages. */
    kWeaveMessageFlag_PeerRequestedAck                  = 0x00004000, /**< Indicates that the sender of the  message requested an acknowledgment. */
    kWeaveMessageFlag_DuplicateMessage                  = 0x00008000, /**< Indicates that the message is a duplicate of a previously received message. */
    kWeaveMessageFlag_PeerGroupMsgIdNotSynchronized     = 0x00010000, /**< Indicates that the peer's group key message counter is not synchronized. */
	kWeaveMessageFlag_FromInitiator                     = 0x00020000, /**< Indicates that the source of the message is the initiator of the
																		   Weave exchange. */
    kWeaveMessageFlag_ViaEphemeralUDPPort               = 0x00040000, /**< Indicates that message is being sent/received via the local ephemeral UDP port. */

    kWeaveMessageFlag_MulticastFromLinkLocal            = kWeaveMessageFlag_DefaultMulticastSourceAddress,
                                                                      /**< Deprecated alias for \c kWeaveMessageFlag_DefaultMulticastSourceAddress */

    // NOTE: The bit positions of the following flags correspond to flag fields in an encoded
    // Weave message header.

    kWeaveMessageFlag_DestNodeId                        = kWeaveHeaderFlag_DestNodeId,
                                                                      /**< Indicates that the destination node ID is present in the Weave message header. */
    kWeaveMessageFlag_SourceNodeId                      = kWeaveHeaderFlag_SourceNodeId,
                                                                      /**< Indicates that the source node ID is present in the Weave message header. */
    kWeaveMessageFlag_TunneledData                      = kWeaveHeaderFlag_TunneledData,
                                                                      /**< Indicates that the Weave message payload is a tunneled IP packet. */
    kWeaveMessageFlag_MsgCounterSyncReq                 = kWeaveHeaderFlag_MsgCounterSyncReq,
                                                                      /**< Indicates that the sender requests peer's message counter synchronization. */

} WeaveMessageFlags;


/**
 *  @brief
 *    The encryption types for the Weave message.
 *
 */
typedef enum WeaveEncryptionType
{
    kWeaveEncryptionType_None                           = 0, /**< Message not encrypted. */
    kWeaveEncryptionType_AES128CTRSHA1                  = 1  /**< Message encrypted using AES-128-CTR
                                                                  encryption with HMAC-SHA-1 message integrity. */
} WeaveEncryptionType;

/**
 *  @brief
 *    The version of the Weave Message format.
 *
 *  @details
 *    Weave will choose the appropriate message version based on the frame format required for the Weave
 *    message. By default, the message version is kWeaveMessageVersion_V1. When using Weave Reliable
 *    Messaging, for example, the version is kWeaveMessageVersion_V2.
 *
 */
typedef enum WeaveMessageVersion
{
    kWeaveMessageVersion_Unspecified                    = 0, /**< Unspecified message version. */
    kWeaveMessageVersion_V1                             = 1, /**< Message header format version V1. */
    kWeaveMessageVersion_V2                             = 2  /**< Message header format version V2. */
} WeaveMessageVersion;

/**
 *  @class WeaveConnection
 *
 *  @brief
 *    The definition of the Weave Connection class. It represents a TCP or BLE
 *    connection to another Weave node.
 *
 */
class WeaveConnection
{
    friend class WeaveMessageLayer;

public:
    /**
     *  @enum State
     *
     *  @brief
     *    The State of the Weave connection object.
     *
     */
    enum State
    {
        kState_ReadyToConnect                           = 0,  /**< State after initialization of the Weave connection. */
        kState_Resolving                                = 1,  /**< State when DNS name resolution is being performed. */
        kState_Connecting                               = 2,  /**< State when connection is being attempted. */
        kState_EstablishingSession                      = 3,  /**< State when a secure session is being established. */
        kState_Connected                                = 4,  /**< State when the connection has been established. */
        kState_SendShutdown                             = 5,  /**< State when the connection is being shut down. */
        kState_Closed                                   = 6   /**< State when the connection is closed. */
    };

    /**
     *  @enum NetworkType
     *
     *  @brief
     *    The network type of the Weave connection object.
     *
     */
    enum NetworkType
    {
        kNetworkType_Unassigned                       = 0,    /**< Unassigned network type. */
        kNetworkType_IP                               = 1,    /**< TCP/IP network type. */
        kNetworkType_BLE                              = 2     /**< BLE network type. */
    };

    uint64_t PeerNodeId;                                /**< [READ ONLY] The node identifier of the peer. */
    IPAddress PeerAddr;                                 /**< [READ ONLY] The IP address of the peer node. */
    WeaveMessageLayer *MessageLayer;                    /**< [READ ONLY] The associated WeaveMessageLayer object. */
    void *AppState;                                     /**< A pointer to the application-specific state object. */
    uint16_t PeerPort;                                  /**< [READ ONLY] The port number of the peer node. */
    uint16_t DefaultKeyId;                              /**< The default encryption key to use when sending messages. */
    WeaveAuthMode AuthMode;                             /**< [READ ONLY] The authentication mode used to establish the
                                                             default encryption keys for the connection. */
    uint8_t DefaultEncryptionType;                      /**< The default encryption type for messages. */
    uint8_t State;                                      /**< [READ ONLY] The state of the WeaveConnection object. */
    uint8_t NetworkType;                                /**< [READ ONLY] The network type of the associated end point. */
    bool ReceiveEnabled;                                /**< [READ ONLY] True if receiving is enabled, false otherwise. */
    bool SendSourceNodeId;                              /**< True if all messages sent via this connection must include
                                                             an explicitly encoded source node identifier, false otherwise. */
    bool SendDestNodeId;                                /**< True if all messages sent via this connection must include
                                                             an explicitly encoded destination node identifier, false
                                                             otherwise. */

    void AddRef(void);
    void Release(void);

    // Note: a downcall to Connect() may call OnConnectionComplete before it returns.
    WEAVE_ERROR Connect(uint64_t peerNodeId);
    WEAVE_ERROR Connect(uint64_t peerNodeId, const IPAddress &peerAddr, uint16_t peerPort = 0);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, const IPAddress &peerAddr, uint16_t peerPort = 0, InterfaceId intf = INET_NULL_INTERFACEID);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, const char *peerAddr, uint16_t defaultPort = 0);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, const char *peerAddr, uint16_t peerAddrLen, uint16_t defaultPort = 0);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, const char *peerAddr, uint16_t peerAddrLen, uint8_t dnsOptions, uint16_t defaultPort);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, HostPortList hostPortList, InterfaceId intf = INET_NULL_INTERFACEID);
    WEAVE_ERROR Connect(uint64_t peerNodeId, WeaveAuthMode authMode, HostPortList hostPortList, uint8_t dnsOptions, InterfaceId intf);

    WEAVE_ERROR GetPeerAddressInfo(IPPacketInfo& addrInfo);

    enum
    {
        kGetPeerDescription_MaxLength = nl::Weave::kWeavePeerDescription_MaxLength,
                                                        /**< Maximum length of string (including NUL character) returned
                                                             by GetPeerDescription(). */
    };

    void GetPeerDescription(char * buf, size_t bufSize) const;

    WEAVE_ERROR SendMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
#if WEAVE_CONFIG_ENABLE_TUNNELING
/**
 * Function to send a Tunneled packet over a Weave connection.
 */
    WEAVE_ERROR SendTunneledMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
#endif

    // TODO COM-311: implement EnableReceived/DisableReceive for BLE WeaveConnections.
    void EnableReceive(void);
    void DisableReceive(void);

    WEAVE_ERROR Shutdown(void);

    WEAVE_ERROR Close(void);
    WEAVE_ERROR Close(bool suppressCloseLog);

    void Abort(void);

    void SetConnectTimeout(const uint32_t connTimeoutMsecs);

    WEAVE_ERROR SetIdleTimeout(uint32_t timeoutMS);

    WEAVE_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount);
    WEAVE_ERROR DisableKeepAlive(void);

    WEAVE_ERROR SetUserTimeout(uint32_t userTimeoutMillis);
    WEAVE_ERROR ResetUserTimeout(void);
    uint16_t LogId(void) const { return static_cast<uint16_t>(reinterpret_cast<intptr_t>(this)); }

    TCPEndPoint * GetTCPEndPoint(void) const { return mTcpEndPoint; }

    /**
     *  This function is the application callback that is invoked when a connection setup is complete.
     *
     *  @param[in]    con           A pointer to the WeaveConnection object.
     *
     *  @param[in]    conErr        The WEAVE_ERROR encountered during connection setup.
     *
     */
    typedef void (*ConnectionCompleteFunct)(WeaveConnection *con, WEAVE_ERROR conErr);
    ConnectionCompleteFunct OnConnectionComplete;

    /**
     *  This function is the application callback that is invoked when a connection is closed.
     *
     *  @param[in]    con           A pointer to the WeaveConnection object.
     *
     *  @param[in]    conErr        The WEAVE_ERROR encountered when the connection was closed.
     *
     */
    typedef void (*ConnectionClosedFunct)(WeaveConnection *con, WEAVE_ERROR conErr);
    ConnectionClosedFunct OnConnectionClosed;

    /**
     *  This function is the application callback that is invoked when a message is received over a
     *  Weave connection.
     *
     *  @param[in]    con           A pointer to the WeaveConnection object.
     *
     *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo structure containing information about the message.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     *
     */
    typedef void (*MessageReceiveFunct)(WeaveConnection *con, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    MessageReceiveFunct OnMessageReceived;

#if WEAVE_CONFIG_ENABLE_TUNNELING
    /**
     *  This function is the application callback that is invoked upon receipt of a Tunneled data packet over the
     *  Weave connection.
     *
     *  @param[in]     con            A pointer to the WeaveConnection object.
     *
     *  @param[in]     msgInfo        A pointer to the WeaveMessageInfo object.
     *
     *  @param[in]     msgBuf         A pointer to the PacketBuffer object containing the tunneled packet received.
     *
     */
    typedef void (*TunneledMsgReceiveFunct)(WeaveConnection *con, const WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    TunneledMsgReceiveFunct OnTunneledMessageReceived;
#endif

    /**
     *  This function is the application callback invoked upon encountering an error when receiving
     *  a Weave message.
     *
     *  @param[in]     con            A pointer to the WeaveConnection object.
     *
     *  @param[in]     err            The WEAVE_ERROR encountered when receiving data over the connection.
     *
     */
    typedef void (*ReceiveErrorFunct)(WeaveConnection *con, WEAVE_ERROR err);
    ReceiveErrorFunct OnReceiveError;

    bool IsIncoming(void) const { return GetFlag(mFlags, kFlag_IsIncoming); }
    void SetIncoming(bool val)  { SetFlag(mFlags, kFlag_IsIncoming, val); }

private:
    enum
    {
        kDoCloseFlag_SuppressCallback   = 0x01,
        kDoCloseFlag_SuppressLogging    = 0x02
    } DoCloseFlags;

    IPAddress mPeerAddrs[WEAVE_CONFIG_CONNECT_IP_ADDRS];
    TCPEndPoint *mTcpEndPoint;
    HostPortList mPeerHostPortList;
    InterfaceId mTargetInterface;
    uint32_t mConnectTimeout;
    uint8_t mRefCount;
#if WEAVE_CONFIG_ENABLE_DNS_RESOLVER
    uint8_t mDNSOptions;
#endif

    enum FlagsEnum
    {
        kFlag_IsIncoming              = 0x01,           /**< The connection was initiated by external node. */
    };

    uint8_t mFlags;                                     /**< Various flags associated with the connection. */

    void Init(WeaveMessageLayer *msgLayer);
    void MakeConnectedTcp(TCPEndPoint *endPoint, const IPAddress &localAddr, const IPAddress &peerAddr);
    WEAVE_ERROR StartConnect(void);
    void DoClose(WEAVE_ERROR err, uint8_t flags);
    WEAVE_ERROR TryNextPeerAddress(WEAVE_ERROR lastErr);
    void StartSession(void);
    bool StateAllowsSend(void) const { return State == kState_EstablishingSession || State == kState_Connected; }
    bool StateAllowsReceive(void) const { return State == kState_EstablishingSession || State == kState_Connected || State == kState_SendShutdown; }
    void DisconnectOnError(WEAVE_ERROR err);
    WEAVE_ERROR StartConnectToAddressLiteral(const char *peerAddr, size_t peerAddrLen);

    static void HandleResolveComplete(void *appState, INET_ERROR err, uint8_t addrCount, IPAddress *addrArray);
    static void HandleConnectComplete(TCPEndPoint *endPoint, INET_ERROR conRes);
    static void HandleDataReceived(TCPEndPoint *endPoint, PacketBuffer *data);
    static void HandleTcpConnectionClosed(TCPEndPoint *endPoint, INET_ERROR err);
    static void HandleSecureSessionEstablished(WeaveSecurityManager *sm, WeaveConnection *con, void *reqState, uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType);
    static void HandleSecureSessionError(WeaveSecurityManager *sm, WeaveConnection *con, void *reqState, WEAVE_ERROR localErr, uint64_t peerNodeId,
                                         Profiles::StatusReporting::StatusReport *statusReport);
    static void DefaultConnectionClosedHandler(WeaveConnection *con, WEAVE_ERROR conErr);

#if CONFIG_NETWORK_LAYER_BLE
public:
    WEAVE_ERROR ConnectBle(BLE_CONNECTION_OBJECT connObj, WeaveAuthMode authMode, bool autoClose = true);

private:
    BLEEndPoint *mBleEndPoint;

    void MakeConnectedBle(BLEEndPoint *endPoint);

    static void HandleBleConnectComplete(BLEEndPoint *endPoint, BLE_ERROR err);
    static void HandleBleMessageReceived(BLEEndPoint *endPoint, System::PacketBuffer *data);
    static void HandleBleConnectionClosed(BLEEndPoint *endPoint, BLE_ERROR err);
#endif

};

/**
 *  @class WeaveConnectionTunnel
 *
 *  @brief
 *    The definition of the WeaveConnectionTunnel class, which manages a pair of TCPEndPoints
 *    whose original WeaveConnections have been coupled, and between which the WeaveMessageLayer
 *    forwards all data and connection closures.
 */
class WeaveConnectionTunnel
{
    friend class WeaveMessageLayer;

public:
    uint16_t LogId(void) const { return (uint16_t)((intptr_t)this); }
    void Shutdown(void);

    /**
     *  This function is the application callback that is invoked when the Weave connection tunnel is shut down.
     *
     *  @param[in]    tun    A pointer to the WeaveConnectionTunnel object.
     *
     */
    typedef void (*ShutdownFunct)(WeaveConnectionTunnel *tun);
    ShutdownFunct OnShutdown;

    void *AppState;                                     /**< A pointer to application-specific state object. */

private:
    WeaveMessageLayer *mMessageLayer;
    TCPEndPoint *mEPOne;
    TCPEndPoint *mEPTwo;

    void Init(WeaveMessageLayer *messageLayer);
    WEAVE_ERROR MakeTunnelConnected(TCPEndPoint *endpointOne, TCPEndPoint *endpointTwo);
    void CloseEndPoint(TCPEndPoint **endPoint);
    inline bool IsInUse(void) const { return mMessageLayer != NULL; };

    static void HandleTunnelDataReceived(TCPEndPoint *endPoint, PacketBuffer *data);
    static void HandleTunnelConnectionClosed(TCPEndPoint *endPoint, INET_ERROR err);
    static void HandleReceiveShutdown(TCPEndPoint *endPoint);
};

/**
 *  @class WeaveMessageLayer
 *
 *  @brief
 *    The definition of the WeaveMessageLayer class, which manages communication
 *    with other Weave nodes. It employs one of several InetLayer endpoints
 *    to establish a communication channel with other Weave nodes.
 *
 */
class NL_DLL_EXPORT WeaveMessageLayer
{
    friend class WeaveMessageLayerTestObject;
    friend class WeaveConnection;
    friend class WeaveExchangeManager;
    friend class ExchangeContext;
    friend class WeaveFabricState;
public:
    /**
     *  @enum State
     *
     *  @brief
     *    The state of the WeaveMessageLayer.
     *
     */
    enum State
    {
        kState_NotInitialized = 0,          /**< State when the WeaveMessageLayer is not initialized. */
        kState_Initializing = 1,            /**< State when the WeaveMessageLayer is in the process of being initialized. */
        kState_Initialized = 2              /**< State when the WeaveMessageLayer is initialized. */
    };

    /**
     *  @class InitContext
     *
     *  @brief
     *    The definition of the InitContext class. It encapsulates the set of objects and
     *    variables into a context structure that is required for initialization of the
     *    WeaveMessageLayer.
     *
     */
    class InitContext
    {
    public:
        System::Layer*      systemLayer;    /**< A pointer to the SystemLayer object. */
        WeaveFabricState*   fabricState;    /**< A pointer to the WeaveFabricState object. */
        InetLayer*          inet;           /**< A pointer to the InetLayer object. */
        bool                listenTCP;      /**< Accept inbound Weave TCP connections from remote peers on the Weave port. */
        bool                listenUDP;      /**< Accept unsolicited inbound Weave UDP messages from remote peers on the Weave port. */
#if CONFIG_NETWORK_LAYER_BLE
        BleLayer*           ble;            /**< A pointer to the BleLayer object. */
        bool                listenBLE;      /**< Accept inbound Weave over BLE connections from remote peers. */
#endif
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
        bool                enableEphemeralUDPPort;
                                            /**< Initiate Weave UDP exchanges from an ephemeral UDP source port. */
#endif

        /**
         *  The InitContext constructor.
         *
         */
        InitContext(void)
        {
            systemLayer = NULL;
            inet = NULL;
            fabricState = NULL;
            listenTCP = true;
            listenUDP = true;
#if CONFIG_NETWORK_LAYER_BLE
            ble = NULL;
            listenBLE = true;
#endif
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
            enableEphemeralUDPPort = false;
#endif
        };
    };

    WeaveMessageLayer(void);

    System::Layer *SystemLayer;                         /*** [READ ONLY] The associated SystemLayer object. */
    InetLayer *Inet;                                    /**< [READ ONLY] The associated InetLayer object. */
    WeaveFabricState *FabricState;                      /**< [READ ONLY] The associated WeaveFabricState object. */
    void *AppState;                                     /**< A pointer to an application-specific state object. */
    WeaveExchangeManager *ExchangeMgr;                  /**< [READ ONLY] The associated WeaveExchangeManager object. */
    WeaveSecurityManager *SecurityMgr;                  /**< [READ ONLY] The associated WeaveSecurityManager object. */
    uint32_t IncomingConIdleTimeout;                    /**< Default idle timeout (in milliseconds) for incoming connections. */
    uint8_t State;                                      /**< [READ ONLY] The state of the WeaveMessageLayer object. */
    bool IsListening;                                   /**< [READ ONLY] True if listening for incoming connections/messages,
                                                             false otherwise. */
    bool mDropMessage;                                  /**< Internal and for Debug Only; When set, WeaveMessageLayer
                                                             drops the message and returns. */

    WEAVE_ERROR Init(InitContext *context);
    WEAVE_ERROR Shutdown(void);

    WEAVE_ERROR SendMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR SendMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR SendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntfId, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR ResendMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR ResendMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR ResendMessage(const IPAddress &destAddr, uint16_t destPort, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
    WEAVE_ERROR ResendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId interfaceId, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
#if WEAVE_CONFIG_ENABLE_TUNNELING
    /**
     *  Function to send a Tunneled packet over a local UDP tunnel.
     */
    WEAVE_ERROR SendUDPTunneledMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf);
#endif
    WeaveConnection *NewConnection(void);
    WeaveConnectionTunnel *NewConnectionTunnel(void);

    void GetConnectionPoolStats(nl::Weave::System::Stats::count_t &aOutInUse) const;

    WEAVE_ERROR CreateTunnel(WeaveConnectionTunnel **tunPtr, WeaveConnection &conOne, WeaveConnection &conTwo,
            uint32_t inactivityTimeoutMS);

    /**
     *  This function is the higher layer callback that is invoked upon receipt of a Weave message over UDP.
     *
     *  @param[in]     msgLayer       A pointer to the WeaveMessageLayer object.
     *
     *  @param[in]     msgInfo        A pointer to the WeaveMessageInfo object.
     *
     *  @param[in]     payload        Pointer to PacketBuffer message containing tunneled packet received.
     *
     */
    typedef void (*MessageReceiveFunct)(WeaveMessageLayer *msgLayer, WeaveMessageInfo *msgInfo, PacketBuffer *payload);
    MessageReceiveFunct OnMessageReceived;

#if WEAVE_CONFIG_ENABLE_TUNNELING
    /**
     *  This function is the higher layer callback that is invoked upon receipt of a Tunneled packet over a local
     *  UDP tunnel.
     *
     *  @param[in]     msgLayer       A pointer to the WeaveMessageLayer object.
     *
     *  @param[in]     payload        Pointer to PacketBuffer message containing tunneled packet received.
     *
     */
    typedef void (*TunneledMsgReceiveFunct)(WeaveMessageLayer *msgLayer, PacketBuffer *payload);
    TunneledMsgReceiveFunct OnUDPTunneledMessageReceived;
#endif

    /**
     *  This function is the higher layer callback invoked upon encountering an error.
     *
     *  @param[in]     msgLayer       A pointer to the WeaveMessageLayer object.
     *
     *  @param[in]     err            The WEAVE_ERROR encountered when receiving data.
     *
     *  @param[in]     pktInfo        A read-only pointer to the IPPacketInfo object.
     *
     */
    typedef void (*ReceiveErrorFunct)(WeaveMessageLayer *msgLayer, WEAVE_ERROR err, const IPPacketInfo *pktInfo);
    ReceiveErrorFunct OnReceiveError;

    /**
     *  This function is the higher layer callback for handling an incoming TCP connection.
     *
     *  @param[in]     msgLayer       A pointer to the WeaveMessageLayer object.
     *
     *  @param[in]     con            A pointer to the WeaveConnection object.
     *
     */
    typedef void (*ConnectionReceiveFunct)(WeaveMessageLayer *msgLayer, WeaveConnection *con);
    ConnectionReceiveFunct OnConnectionReceived;

    /**
     *  This function is invoked for removing a callback.
     *
     *  @param[in]     listenerState  A pointer to the application state object.
     *
     */
    typedef void (*CallbackRemovedFunct)(void *listenerState);

    // Set OnUnsecuredConnectionReceived callbacks. Return WEAVE_INCORRECT_STATE if callbacks already set unless force
    // flag set to true. If force flag is true, overwrite existing callbacks with provided values after call to
    // OnUnsecuredConnectionCallbackRemoved.
    WEAVE_ERROR SetUnsecuredConnectionListener(ConnectionReceiveFunct newOnUnsecuredConnectionReceived,
            CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved, bool force, void *listenerState);

    // Clear OnUnsecuredConnectionReceived and OnUnsecuredConnectionCallbackRemoved as requested by current
    // callback owner, using function pointer args as proof of identification.
    WEAVE_ERROR ClearUnsecuredConnectionListener(ConnectionReceiveFunct oldOnUnsecuredConnectionReceived,
            CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved);

    /**
     *  This function is the higher layer callback for reporting an error during handling of an incoming TCP
     *  connection.
     *
     *  @param[in]     msgLayer       A pointer to the WeaveMessageLayer object.
     *
     *  @param[in]     err            The WEAVE_ERROR encountered when handling an incoming TCP connection.
     *
     */
    typedef void (*AcceptErrorFunct)(WeaveMessageLayer *msgLayer, WEAVE_ERROR err);
    AcceptErrorFunct OnAcceptError;

    WEAVE_ERROR DecodeHeader(PacketBuffer *msgBuf, WeaveMessageInfo *msgInfo, uint8_t **payloadStart);
    WEAVE_ERROR ReEncodeMessage(PacketBuffer *buf);
    WEAVE_ERROR EncodeMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf, WeaveConnection *con, uint16_t maxLen,
            uint16_t reserve = 0);
    WEAVE_ERROR EncodeMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntId, WeaveMessageInfo *msgInfo, PacketBuffer *payload);

    WEAVE_ERROR RefreshEndpoints(void);
    WEAVE_ERROR CloseEndpoints(void);

    bool IPv4ListenEnabled(void) const;
    bool IPv6ListenEnabled(void) const;
    bool TCPListenEnabled(void) const;
    void SetTCPListenEnabled(bool val);
    bool UDPListenEnabled(void) const;
    void SetUDPListenEnabled(bool val);
    bool UnsecuredListenEnabled(void) const;
    bool EphemeralUDPPortEnabled(void) const;
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    void SetEphemeralUDPPortEnabled(bool val);
#endif

    bool IsBoundToLocalIPv4Address(void) const;
    bool IsBoundToLocalIPv6Address(void) const;

    /**
     *  This function is the application callback for reporting message layer activity change.
     *  Message layer is considered active if there is at least one open exchange or pending
     *  message counter synchronization request.
     *
     *  @param[in] messageLayerIsActive   A boolean value indicating whether message layer is active or not.
     *
     */
    typedef void (*MessageLayerActivityChangeHandlerFunct)(bool messageLayerIsActive);
    void SetSignalMessageLayerActivityChanged(MessageLayerActivityChangeHandlerFunct messageLayerActivityChangeHandler);
    bool IsMessageLayerActive(void);

    static uint32_t GetMaxWeavePayloadSize(const PacketBuffer *msgBuf, bool isUDP, uint32_t udpMTU);

    static void GetPeerDescription(char *buf, size_t bufSize, uint64_t nodeId, const IPAddress *addr, uint16_t port, InterfaceId interfaceId, const WeaveConnection *con);
    static void GetPeerDescription(char *buf, size_t bufSize, const WeaveMessageInfo *msgInfo);

private:
    enum
    {
        kFlag_ListenTCP                 = 0x01,
        kFlag_ListenUDP                 = 0x02,
        kFlag_ListenUnsecured           = 0x04,
        kFlag_EphemeralUDPPortEnabled   = 0x08,
        kFlag_ForceRefreshUDPEndPoints  = 0x10,
    };

    TCPEndPoint *mIPv6TCPListen;
    UDPEndPoint *mIPv6UDP;
    WeaveConnection mConPool[WEAVE_CONFIG_MAX_CONNECTIONS];
    WeaveConnectionTunnel mTunnelPool[WEAVE_CONFIG_MAX_TUNNELS];
    uint8_t mFlags;

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    UDPEndPoint *mIPv6UDPMulticastRcv;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4UDPBroadcastRcv;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

#if WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    TCPEndPoint *mUnsecuredIPv6TCPListen;
#endif

#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4UDP;
    TCPEndPoint *mIPv4TCPListen;
#endif // INET_CONFIG_ENABLE_IPV4

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    UDPEndPoint *mIPv6EphemeralUDP;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4EphemeralUDP;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    // To set and clear, use SetOnUnsecuredConnectionReceived() and ClearOnUnsecuredConnectionReceived().
    ConnectionReceiveFunct OnUnsecuredConnectionReceived;
    CallbackRemovedFunct OnUnsecuredConnectionCallbacksRemoved;
    void *UnsecuredConnectionReceivedAppState;
    MessageLayerActivityChangeHandlerFunct OnMessageLayerActivityChange;

    WEAVE_ERROR EnableUnsecuredListen(void);
    WEAVE_ERROR DisableUnsecuredListen(void);

    void SignalMessageLayerActivityChanged(void);

    void CloseListeningEndpoints(void);

    WEAVE_ERROR RefreshEndpoint(TCPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port);
    WEAVE_ERROR RefreshEndpoint(UDPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port, InterfaceId intfId);

    WEAVE_ERROR SendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntfId, PacketBuffer *payload, uint32_t msgFlags);
    WEAVE_ERROR SelectOutboundUDPEndPoint(const IPAddress & destAddr, uint32_t msgFlags, UDPEndPoint *& ep);
    WEAVE_ERROR SelectDestNodeIdAndAddress(uint64_t& destNodeId, IPAddress& destAddr);
    WEAVE_ERROR DecodeMessage(PacketBuffer *msgBuf, uint64_t sourceNodeId, WeaveConnection *con,
            WeaveMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen);
    WEAVE_ERROR EncodeMessageWithLength(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf, WeaveConnection *con,
            uint16_t maxLen);
    WEAVE_ERROR DecodeMessageWithLength(PacketBuffer *msgBuf, uint64_t sourceNodeId, WeaveConnection *con,
            WeaveMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen, uint32_t *rFrameLen);
    void GetIncomingTCPConCount(const IPAddress &peerAddr, uint16_t &count, uint16_t &countFromIP);
    void CheckForceRefreshUDPEndPointsNeeded(WEAVE_ERROR udpSendErr);

    static void HandleUDPMessage(UDPEndPoint *endPoint, PacketBuffer *msg, const IPPacketInfo *pktInfo);
    static void HandleUDPReceiveError(UDPEndPoint *endPoint, INET_ERROR err, const IPPacketInfo *pktInfo);
    static void HandleIncomingTcpConnection(TCPEndPoint *listeningEndPoint, TCPEndPoint *conEndPoint, const IPAddress &peerAddr,
            uint16_t peerPort);
    static void HandleAcceptError(TCPEndPoint *endPoint, INET_ERROR err);
    static void Encrypt_AES128CTRSHA1(const WeaveMessageInfo *msgInfo, const uint8_t *key,
                                      const uint8_t *inData, uint16_t inLen, uint8_t *outBuf);
    static void ComputeIntegrityCheck_AES128CTRSHA1(const WeaveMessageInfo *msgInfo, const uint8_t *key,
                                                    const uint8_t *inData, uint16_t inLen, uint8_t *outBuf);
    static WEAVE_ERROR FilterUDPSendError(WEAVE_ERROR err, bool isMulticast);
    static bool IsIgnoredMulticastSendError(WEAVE_ERROR err);

    static bool IsSendErrorNonCritical(WEAVE_ERROR err);

    WeaveMessageLayer(const WeaveMessageLayer&);   // not defined

#if CONFIG_NETWORK_LAYER_BLE
public:
    BleLayer *mBle;                                      /**< [READ ONLY] Associated BleLayer object. */

private:
    static void HandleIncomingBleConnection(BLEEndPoint *bleEndPoint);
#endif

};

/**
 *  Check if the WeaveMessageLayer is configured to listen for inbound communications
 *  over IPv4.
 */
inline bool WeaveMessageLayer::IPv4ListenEnabled(void) const
{
    // When IPv4 is supported, and the targeted listen feature is enabled, enable IPv4 listening
    // when the message layer has been bound to a specific IPv4 listening address OR the message
    // layer has not been bound to any address (IPv4 or IPv6).
    //
    // Otherwise, when the targeted listen feature is NOT enabled, always listen on IPv4 if it is
    // supported.
    //
#if INET_CONFIG_ENABLE_IPV4
    return IsBoundToLocalIPv4Address() || !IsBoundToLocalIPv6Address();
#else
    return false;
#endif
}

/**
 *  Check if the WeaveMessageLayer is configured to listen for inbound communications
 *  over IPv4.
 */
inline bool WeaveMessageLayer::IPv6ListenEnabled(void) const
{
    // When the targeted listen feature is enabled, enable IPv6 listening when the message layer has
    // been bound to a specific IPv6 listening address OR the message
    // layer has not been bound to any address (IPv4 or IPv6).
    //
    // Otherwise, when the targeted listen feature is NOT enabled, always listen on IPv6.
    //
    return IsBoundToLocalIPv6Address() || !IsBoundToLocalIPv4Address();
}

/**
 *  Check if the WeaveMessageLayer is configured to listen for inbound TCP connections.
 */
inline bool WeaveMessageLayer::TCPListenEnabled(void) const
{
    return GetFlag(mFlags, kFlag_ListenTCP);
}

/**
 *  Enable or disable listening for inbound TCP connections in the WeaveMessageLayer.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the TCP listening state is changed.
 */
inline void WeaveMessageLayer::SetTCPListenEnabled(bool val)
{
    SetFlag(mFlags, kFlag_ListenTCP, val);
}

/**
 *  Check if the WeaveMessageLayer is configured to listen for inbound UDP messages.
 */
inline bool WeaveMessageLayer::UDPListenEnabled(void) const
{
    return GetFlag(mFlags, kFlag_ListenUDP);
}

/**
 *  Enable or disable listening for inbound UDP messages in the WeaveMessageLayer.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the UDP listening state is changed.
 */
inline void WeaveMessageLayer::SetUDPListenEnabled(bool val)
{
    SetFlag(mFlags, kFlag_ListenUDP, val);
}

/**
 *  Check if locally initiated Weave UDP exchanges should be sent from an ephemeral
 *  UDP source port.
 */
inline bool WeaveMessageLayer::EphemeralUDPPortEnabled(void) const
{
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    return GetFlag(mFlags, kFlag_EphemeralUDPPortEnabled);
#else
    return false;
#endif
}

/**
 *  Enable or disabled initiating Weave UDP exchanges from an ephemeral UDP source port.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the ephemeral port state is changed.
 */
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

inline void WeaveMessageLayer::SetEphemeralUDPPortEnabled(bool val)
{
    SetFlag(mFlags, kFlag_EphemeralUDPPortEnabled, val);
}

#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

/**
 *  Check if unsecured listening is enabled.
 */
inline bool WeaveMessageLayer::UnsecuredListenEnabled() const
{
    return GetFlag(mFlags, kFlag_ListenUnsecured);
}

/**
 *  Check if the WeaveMessageLayer is bound to a local IPv4 address.
 */
inline bool WeaveMessageLayer::IsBoundToLocalIPv4Address(void) const
{
#if INET_CONFIG_ENABLE_IPV4 && WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    return FabricState->ListenIPv4Addr != IPAddress::Any;
#else
    return false;
#endif
}

/**
 *  Check if the WeaveMessageLayer is bound to a local IPv6 address.
 */
inline bool WeaveMessageLayer::IsBoundToLocalIPv6Address(void) const
{
#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    return FabricState->ListenIPv6Addr != IPAddress::Any;
#else
    return false;
#endif
}



/**
 * Bit field definitions for IEEE EUI-64 Identifiers.
 */
enum
{
    kEUI64_UL_Mask          = 0x0200000000000000ULL,        /**< Bitmask for the Universal/Local (U/L) bit within an EUI-64 identifier.
                                                                 A value of 0 indicates the id is Universally (globally) administered.
                                                                 A value of 1 indicates the id is Locally administered. */

    kEUI64_UL_Unversal      = 0,                            /**< Universal/Local bit value indicating a Universally administered EUI-64 identifier. */

    kEUI64_UL_Local         = kEUI64_UL_Mask,               /**< Universal/Local bit value indicating a Locally administered EUI-64 identifier. */

    kEUI64_IG_Mask          = 0x0100000000000000ULL,        /**< Bitmask for the Individual/Group (I/G) bit within an EUI-64 identifier.
                                                                 A value of 0 indicates the id is an individual address.
                                                                 A value of 1 indicates the id is a group address. */

    kEUI64_IG_Individual    = 0,                            /**< Individual/Group bit value indicating an individual address EUI-64 identifier. */

    kEUI64_IG_Group         = kEUI64_IG_Mask,               /**< Individual/Group bit value indicating an group address EUI-64 identifier. */
};

/**
 * Special ranges of Weave Node Ids.
 */
enum
{
    kMaxAlwaysLocalWeaveNodeId  = 0x000000000000FFFFULL,    /**< Weave node identifiers less or equal than this value are considered local for testing convenience. */
};

/**
 *  Convert a Weave fabric identifier to an IPv6 ULA global identifier.
 *
 *  The ULA global identifier for a fabric address is the lower 40 bits of the fabric's
 *  64-bit fabric ID.
 *
 *  @param[in]    fabricId    The Weave fabric identifier.
 *
 *  @return the mapped IPv6 global identifier.
 *
 */
inline uint64_t WeaveFabricIdToIPv6GlobalId(uint64_t fabricId) { return (fabricId & 0xFFFFFFFFFFULL); }

/**
 *  Convert an IPv6 address interface identifier to a Weave node identifier.
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalWeaveNodeId
 *  (65535) are considered 'local', and have their universal/local bit is set to zero.
 *  This simplifies the string representation of the corresponding IPv6 addresses.
 *  For example a ULA for node identifier \c 10 would be \c FD00:0:1:1\::A.
 *
 *  @note
 *    When trying to determine if an interface identifier matches a particular node identifier,
 *    always convert the interface identifier to a node identifier and then compare, not
 *    the other way around. This allows for cases where the universal/local bit may not
 *    have been set to 1, either by error or because another addressing convention was
 *    being followed.
 *
 *  @param[in]    interfaceId    The 64 bit interface identifier.
 *
 *  @return the mapped 64 bit Weave node identifier.
 *
 */
inline uint64_t IPv6InterfaceIdToWeaveNodeId(uint64_t interfaceId)
{
    return (interfaceId <= kMaxAlwaysLocalWeaveNodeId) ? interfaceId : (interfaceId ^ kEUI64_UL_Mask);
}

/**
 *  Convert a Weave node identifier to an IPv6 address interface identifier.
 *
 *  Weave node identifiers are Universal/Local EUI-64s, which per RFC-3513 are converted to
 *  interface identifiers by inverting the universal/local bit (bit 57 counting the LSB as 0).
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalWeaveNodeId
 *  (65535) are considered 'local', and have their universal/local bit is set to zero.
 *  This simplifies the string representation of the corresponding IPv6 addresses.
 *  For example a ULA for node identifier \c 10 would be \c FD00:0:1:1\::A.
 *
 *  @note
 *    When trying to determine if an interface identifier matches a particular node identifier,
 *    always convert the interface identifier to a node identifier and then compare, not
 *    the other way around. This allows for cases where the universal/local bit may not
 *    have been set to 1, either by error or because another addressing convention was
 *    being followed.
 *
 *  @param[in]    nodeId    The 64-bit Weave node identifier.
 *
 *  @return the IPv6 interface identifier.
 *
 */
inline uint64_t WeaveNodeIdToIPv6InterfaceId(uint64_t nodeId)
{
    return (nodeId <= kMaxAlwaysLocalWeaveNodeId) ? nodeId : (nodeId ^ kEUI64_UL_Mask);
}

/**
 *  Weave has some reserved subnet numbers for distinguished network interfaces
 *  on typical devices. These numbers are assigned here to symbolic constants.
 *  These subnet numbers are used by Weave to configure IPv6 ULA addresses on
 *  appropriate interfaces.
 */
typedef enum WeaveSubnetId
{
    kWeaveSubnetId_NotSpecified                         = 0, /**< Reserved as an unspecified or null value. */
    kWeaveSubnetId_PrimaryWiFi                          = 1, /**< The WiFi radio interface subnet number. */
    kWeaveSubnetId_ThreadAlarm                          = 2, /**< The Thread alarm radio interface subnet number. */
    kWeaveSubnetId_WiFiAP                               = 3, /**< The Local Wi-Fi AP interface subnet number. */
    kWeaveSubnetId_MobileDevice                         = 4, /**< The subnet identifier for all Mobile devices. */
    kWeaveSubnetId_Service                              = 5, /**< The subnet identifier for the Nest Service endpoints. */
    kWeaveSubnetId_ThreadMesh                           = 6, /**< The Thread mesh radio interface subnet identifier. */
} WeaveSubnetId;

#define WEAVE_MAX_NODE_ADDR_STR_LENGTH (nl::Weave::kWeavePeerDescription_MaxLength)
#define WEAVE_MAX_MESSAGE_SOURCE_STR_LENGTH (nl::Weave::kWeavePeerDescription_MaxLength)

/**
 * DEPRECATED -- Use WeaveMessageLayer::GetPeerDescription() instead.
 */
inline void WeaveNodeAddrToStr(char *buf, uint32_t bufSize, uint64_t nodeId, const IPAddress *addr, uint16_t port, WeaveConnection *con)
{
    WeaveMessageLayer::GetPeerDescription(buf, (size_t)bufSize, nodeId, addr, port, INET_NULL_INTERFACEID, con);
}

/**
 * DEPRECATED -- Use WeaveMessageLayer::GetPeerDescription() instead.
 */
inline void WeaveMessageSourceToStr(char *buf, uint32_t bufSize, const WeaveMessageInfo *msgInfo)
{
    WeaveMessageLayer::GetPeerDescription(buf, (size_t)bufSize, msgInfo);
}

extern WEAVE_ERROR GenerateWeaveNodeId(uint64_t & nodeId);

} // namespace nl
} // namespace Weave

#endif // WEAVE_MESSAGE_LAYER_H
