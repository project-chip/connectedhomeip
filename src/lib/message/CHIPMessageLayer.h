/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the classes for the CHIP Message Layer.
 *
 */

#ifndef CHIP_MESSAGE_LAYER_H
#define CHIP_MESSAGE_LAYER_H

#include <stdint.h>
#include <string.h>

#include <core/CHIPTunnelConfig.h>
#include <message/CHIPFabricState.h>
#include <message/HostPortList.h>
#include <support/DLLUtil.h>
#include <system/SystemStats.h>

namespace chip {

using System::PacketBuffer;

class ChipMessageLayer;
class ChipMessageLayerTestObject;
class ChipExchangeManager;
class ChipSecurityManager;

namespace Profiles {
namespace StatusReporting {
class StatusReport;
}
} // namespace Profiles

enum
{
    kChipPeerDescription_MaxLength =
        100, /**< Maximum length of string (including NUL character) returned by ChipMessageLayer::GetPeerDescription(). */
};

/**
 *  @brief
 *    Definitions pertaining to the header of an encoded CHIP message.
 *
 */
enum
{
    kMsgHeaderField_FlagsMask           = 0x0F0F,
    kMsgHeaderField_FlagsShift          = 0,
    kMsgHeaderField_EncryptionTypeMask  = 0x00F0,
    kMsgHeaderField_EncryptionTypeShift = 4,
    kMsgHeaderField_MessageVersionMask  = 0xF000,
    kMsgHeaderField_MessageVersionShift = 12,

    kChipHeaderFlag_DestNodeId   = 0x0100, /**< Indicates that the destination node ID is present in the CHIP message header. */
    kChipHeaderFlag_SourceNodeId = 0x0200, /**< Indicates that the source node ID is present in the CHIP message header. */
    kChipHeaderFlag_TunneledData = 0x0400, /**< Indicates that the CHIP message payload is a tunneled IP packet. */
    kChipHeaderFlag_MsgCounterSyncReq = 0x0800, /**< Indicates that the sender requests message counter synchronization. */

    kMsgHeaderField_ReservedFlagsMask = kMsgHeaderField_FlagsMask & ~kChipHeaderFlag_DestNodeId & ~kChipHeaderFlag_SourceNodeId &
        ~kChipHeaderFlag_TunneledData & ~kChipHeaderFlag_MsgCounterSyncReq,

    kMsgHeaderField_MessageHMACMask =
        ~((kChipHeaderFlag_DestNodeId | kChipHeaderFlag_SourceNodeId | kChipHeaderFlag_MsgCounterSyncReq)
          << kMsgHeaderField_FlagsShift)
};

/**
 *  @struct ChipMessageInfo
 *
 *  @brief
 *    Information about a CHIP message that is in the process of being sent or received.
 *
 */
struct ChipMessageInfo
{
    uint64_t SourceNodeId;  /**< The source node identifier of the CHIP message. */
    uint64_t DestNodeId;    /**< The destination node identifier of the CHIP message. */
    uint32_t MessageId;     /**< The message identifier of the CHIP message. */
    uint32_t Flags;         /**< Various flags associated with the CHIP message; see ChipMessageFlags. */
    uint16_t KeyId;         /**< The encryption key identifier of the CHIP message. */
    uint8_t MessageVersion; /**< The version of the CHIP message. */
    uint8_t EncryptionType; /**< The encryption type used for the CHIP message. */
    ChipAuthMode
        PeerAuthMode; /**< The means by which the sender of the message was authenticated. Only meaningful for incoming messages. */
    ChipConnection *
        InCon; /**< The connection (if any) over which the message was received. Only meaningful for incoming messages.*/
    const IPPacketInfo *
        InPacketInfo; /**< The IP Addressing information of the received message. Only meaningful for incoming messages. */

    void Clear() { memset(this, 0, sizeof(*this)); }
};

// DEPRECATED alias for ChipMessageInfo
typedef struct ChipMessageInfo ChipMessageHeader;

/**
 *  @brief
 *    Flags associated with a inbound or outbound CHIP message.
 *
 *    The values defined here are for use within the ChipMessageInfo.Flags field.
 */
typedef enum ChipMessageFlags
{
    kChipMessageFlag_ReuseMessageId = 0x00000010, /**< Indicates that the existing message identifier must be reused. */
    kChipMessageFlag_ReuseSourceId  = 0x00000020, /**< Indicates that the existing source node identifier must be reused. */
    kChipMessageFlag_DelaySend      = 0x00000040, /**< Indicates that the sending of the message needs to be delayed. */
    kChipMessageFlag_RetainBuffer   = 0x00000080, /**< Indicates that the message buffer should not be freed after sending. */
    kChipMessageFlag_MessageEncoded = 0x00001000, /**< Indicates that the CHIP message is already encoded. */
    kChipMessageFlag_DefaultMulticastSourceAddress = 0x00002000, /**< Indicates that default IPv6 source address selection should be
                                                                    used when sending IPv6 multicast messages. */
    kChipMessageFlag_PeerRequestedAck = 0x00004000, /**< Indicates that the sender of the  message requested an acknowledgment. */
    kChipMessageFlag_DuplicateMessage =
        0x00008000, /**< Indicates that the message is a duplicate of a previously received message. */
    kChipMessageFlag_PeerGroupMsgIdNotSynchronized =
        0x00010000, /**< Indicates that the peer's group key message counter is not synchronized. */
    kChipMessageFlag_FromInitiator =
        0x00020000, /**< Indicates that the source of the message is the initiator of the
                                                                                              CHIP exchange. */
    kChipMessageFlag_ViaEphemeralUDPPort =
        0x00040000, /**< Indicates that message is being sent/received via the local ephemeral UDP port. */

    kChipMessageFlag_MulticastFromLinkLocal = kChipMessageFlag_DefaultMulticastSourceAddress,
    /**< Deprecated alias for \c kChipMessageFlag_DefaultMulticastSourceAddress */

    // NOTE: The bit positions of the following flags correspond to flag fields in an encoded
    // CHIP message header.

    kChipMessageFlag_DestNodeId = kChipHeaderFlag_DestNodeId,
    /**< Indicates that the destination node ID is present in the CHIP message header. */
    kChipMessageFlag_SourceNodeId = kChipHeaderFlag_SourceNodeId,
    /**< Indicates that the source node ID is present in the CHIP message header. */
    kChipMessageFlag_TunneledData = kChipHeaderFlag_TunneledData,
    /**< Indicates that the CHIP message payload is a tunneled IP packet. */
    kChipMessageFlag_MsgCounterSyncReq = kChipHeaderFlag_MsgCounterSyncReq,
    /**< Indicates that the sender requests peer's message counter synchronization. */

} ChipMessageFlags;

/**
 *  @brief
 *    The encryption types for the CHIP message.
 *
 */
typedef enum ChipEncryptionType
{
    kChipEncryptionType_None          = 0, /**< Message not encrypted. */
    kChipEncryptionType_AES128CTRSHA1 = 1  /**< Message encrypted using AES-128-CTR
                                                 encryption with HMAC-SHA-1 message integrity. */
} ChipEncryptionType;

/**
 *  @brief
 *    The version of the CHIP Message format.
 *
 *  @details
 *    CHIP will choose the appropriate message version based on the frame format required for the CHIP
 *    message. By default, the message version is kChipMessageVersion_V1. When using CHIP Reliable
 *    Messaging, for example, the version is kChipMessageVersion_V2.
 *
 */
typedef enum ChipMessageVersion
{
    kChipMessageVersion_Unspecified = 0, /**< Unspecified message version. */
    kChipMessageVersion_V1          = 1, /**< Message header format version V1. */
    kChipMessageVersion_V2          = 2  /**< Message header format version V2. */
} ChipMessageVersion;

/**
 *  @class ChipConnection
 *
 *  @brief
 *    The definition of the CHIP Connection class. It represents a TCP or BLE
 *    connection to another CHIP node.
 *
 */
class ChipConnection
{
    friend class ChipMessageLayer;

public:
    /**
     *  @enum State
     *
     *  @brief
     *    The State of the CHIP connection object.
     *
     */
    enum State
    {
        kState_ReadyToConnect      = 0, /**< State after initialization of the CHIP connection. */
        kState_Resolving           = 1, /**< State when DNS name resolution is being performed. */
        kState_Connecting          = 2, /**< State when connection is being attempted. */
        kState_EstablishingSession = 3, /**< State when a secure session is being established. */
        kState_Connected           = 4, /**< State when the connection has been established. */
        kState_SendShutdown        = 5, /**< State when the connection is being shut down. */
        kState_Closed              = 6  /**< State when the connection is closed. */
    };

    /**
     *  @enum NetworkType
     *
     *  @brief
     *    The network type of the CHIP connection object.
     *
     */
    enum NetworkType
    {
        kNetworkType_Unassigned = 0, /**< Unassigned network type. */
        kNetworkType_IP         = 1, /**< TCP/IP network type. */
        kNetworkType_BLE        = 2  /**< BLE network type. */
    };

    uint64_t PeerNodeId;             /**< [READ ONLY] The node identifier of the peer. */
    IPAddress PeerAddr;              /**< [READ ONLY] The IP address of the peer node. */
    ChipMessageLayer * MessageLayer; /**< [READ ONLY] The associated ChipMessageLayer object. */
    void * AppState;                 /**< A pointer to the application-specific state object. */
    uint16_t PeerPort;               /**< [READ ONLY] The port number of the peer node. */
    uint16_t DefaultKeyId;           /**< The default encryption key to use when sending messages. */
    ChipAuthMode AuthMode;           /**< [READ ONLY] The authentication mode used to establish the
                                           default encryption keys for the connection. */
    uint8_t DefaultEncryptionType;   /**< The default encryption type for messages. */
    uint8_t State;                   /**< [READ ONLY] The state of the ChipConnection object. */
    uint8_t NetworkType;             /**< [READ ONLY] The network type of the associated end point. */
    bool ReceiveEnabled;             /**< [READ ONLY] True if receiving is enabled, false otherwise. */
    bool SendSourceNodeId;           /**< True if all messages sent via this connection must include
                                          an explicitly encoded source node identifier, false otherwise. */
    bool SendDestNodeId;             /**< True if all messages sent via this connection must include
                                          an explicitly encoded destination node identifier, false
                                          otherwise. */

    void AddRef(void);
    void Release(void);

    // Note: a downcall to Connect() may call OnConnectionComplete before it returns.
    CHIP_ERROR Connect(uint64_t peerNodeId);
    CHIP_ERROR Connect(uint64_t peerNodeId, const IPAddress & peerAddr, uint16_t peerPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const IPAddress & peerAddr, uint16_t peerPort = 0,
                       InterfaceId intf = INET_NULL_INTERFACEID);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t defaultPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t peerAddrLen,
                       uint16_t defaultPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t peerAddrLen, uint8_t dnsOptions,
                       uint16_t defaultPort);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, HostPortList hostPortList,
                       InterfaceId intf = INET_NULL_INTERFACEID);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, HostPortList hostPortList, uint8_t dnsOptions, InterfaceId intf);

    CHIP_ERROR GetPeerAddressInfo(IPPacketInfo & addrInfo);

    enum
    {
        kGetPeerDescription_MaxLength = chip::kChipPeerDescription_MaxLength,
        /**< Maximum length of string (including NUL character) returned
             by GetPeerDescription(). */
    };

    void GetPeerDescription(char * buf, size_t bufSize) const;

    CHIP_ERROR SendMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
#if CHIP_CONFIG_ENABLE_TUNNELING
    /**
     * Function to send a Tunneled packet over a CHIP connection.
     */
    CHIP_ERROR SendTunneledMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
#endif

    // TODO COM-311: implement EnableReceived/DisableReceive for BLE ChipConnections.
    void EnableReceive(void);
    void DisableReceive(void);

    CHIP_ERROR Shutdown(void);

    CHIP_ERROR Close(void);
    CHIP_ERROR Close(bool suppressCloseLog);

    void Abort(void);

    void SetConnectTimeout(const uint32_t connTimeoutMsecs);

    CHIP_ERROR SetIdleTimeout(uint32_t timeoutMS);

    CHIP_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount);
    CHIP_ERROR DisableKeepAlive(void);

    CHIP_ERROR SetUserTimeout(uint32_t userTimeoutMillis);
    CHIP_ERROR ResetUserTimeout(void);
    uint16_t LogId(void) const { return static_cast<uint16_t>(reinterpret_cast<intptr_t>(this)); }

    TCPEndPoint * GetTCPEndPoint(void) const { return mTcpEndPoint; }

    /**
     *  This function is the application callback that is invoked when a connection setup is complete.
     *
     *  @param[in]    con           A pointer to the ChipConnection object.
     *
     *  @param[in]    conErr        The CHIP_ERROR encountered during connection setup.
     *
     */
    typedef void (*ConnectionCompleteFunct)(ChipConnection * con, CHIP_ERROR conErr);
    ConnectionCompleteFunct OnConnectionComplete;

    /**
     *  This function is the application callback that is invoked when a connection is closed.
     *
     *  @param[in]    con           A pointer to the ChipConnection object.
     *
     *  @param[in]    conErr        The CHIP_ERROR encountered when the connection was closed.
     *
     */
    typedef void (*ConnectionClosedFunct)(ChipConnection * con, CHIP_ERROR conErr);
    ConnectionClosedFunct OnConnectionClosed;

    /**
     *  This function is the application callback that is invoked when a message is received over a
     *  CHIP connection.
     *
     *  @param[in]    con           A pointer to the ChipConnection object.
     *
     *  @param[in]    msgInfo       A pointer to a ChipMessageInfo structure containing information about the message.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     *
     */
    typedef void (*MessageReceiveFunct)(ChipConnection * con, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    MessageReceiveFunct OnMessageReceived;

#if CHIP_CONFIG_ENABLE_TUNNELING
    /**
     *  This function is the application callback that is invoked upon receipt of a Tunneled data packet over the
     *  CHIP connection.
     *
     *  @param[in]     con            A pointer to the ChipConnection object.
     *
     *  @param[in]     msgInfo        A pointer to the ChipMessageInfo object.
     *
     *  @param[in]     msgBuf         A pointer to the PacketBuffer object containing the tunneled packet received.
     *
     */
    typedef void (*TunneledMsgReceiveFunct)(ChipConnection * con, const ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    TunneledMsgReceiveFunct OnTunneledMessageReceived;
#endif

    /**
     *  This function is the application callback invoked upon encountering an error when receiving
     *  a CHIP message.
     *
     *  @param[in]     con            A pointer to the ChipConnection object.
     *
     *  @param[in]     err            The CHIP_ERROR encountered when receiving data over the connection.
     *
     */
    typedef void (*ReceiveErrorFunct)(ChipConnection * con, CHIP_ERROR err);
    ReceiveErrorFunct OnReceiveError;

    bool IsIncoming(void) const { return GetFlag(mFlags, kFlag_IsIncoming); }
    void SetIncoming(bool val) { SetFlag(mFlags, kFlag_IsIncoming, val); }

private:
    enum
    {
        kDoCloseFlag_SuppressCallback = 0x01,
        kDoCloseFlag_SuppressLogging  = 0x02
    } DoCloseFlags;

    IPAddress mPeerAddrs[CHIP_CONFIG_CONNECT_IP_ADDRS];
    TCPEndPoint * mTcpEndPoint;
    HostPortList mPeerHostPortList;
    InterfaceId mTargetInterface;
    uint32_t mConnectTimeout;
    uint8_t mRefCount;
#if CHIP_CONFIG_ENABLE_DNS_RESOLVER
    uint8_t mDNSOptions;
#endif

    enum FlagsEnum
    {
        kFlag_IsIncoming = 0x01, /**< The connection was initiated by external node. */
    };

    uint8_t mFlags; /**< Various flags associated with the connection. */

    void Init(ChipMessageLayer * msgLayer);
    void MakeConnectedTcp(TCPEndPoint * endPoint, const IPAddress & localAddr, const IPAddress & peerAddr);
    CHIP_ERROR StartConnect(void);
    void DoClose(CHIP_ERROR err, uint8_t flags);
    CHIP_ERROR TryNextPeerAddress(CHIP_ERROR lastErr);
    void StartSession(void);
    bool StateAllowsSend(void) const { return State == kState_EstablishingSession || State == kState_Connected; }
    bool StateAllowsReceive(void) const
    {
        return State == kState_EstablishingSession || State == kState_Connected || State == kState_SendShutdown;
    }
    void DisconnectOnError(CHIP_ERROR err);
    CHIP_ERROR StartConnectToAddressLiteral(const char * peerAddr, size_t peerAddrLen);

    static void HandleResolveComplete(void * appState, INET_ERROR err, uint8_t addrCount, IPAddress * addrArray);
    static void HandleConnectComplete(TCPEndPoint * endPoint, INET_ERROR conRes);
    static void HandleDataReceived(TCPEndPoint * endPoint, PacketBuffer * data);
    static void HandleTcpConnectionClosed(TCPEndPoint * endPoint, INET_ERROR err);
    static void HandleSecureSessionEstablished(ChipSecurityManager * sm, ChipConnection * con, void * reqState,
                                               uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType);
    static void HandleSecureSessionError(ChipSecurityManager * sm, ChipConnection * con, void * reqState, CHIP_ERROR localErr,
                                         uint64_t peerNodeId, Profiles::StatusReporting::StatusReport * statusReport);
    static void DefaultConnectionClosedHandler(ChipConnection * con, CHIP_ERROR conErr);

#if CONFIG_NETWORK_LAYER_BLE
public:
    CHIP_ERROR ConnectBle(BLE_CONNECTION_OBJECT connObj, ChipAuthMode authMode, bool autoClose = true);

private:
    BLEEndPoint * mBleEndPoint;

    void MakeConnectedBle(BLEEndPoint * endPoint);

    static void HandleBleConnectComplete(BLEEndPoint * endPoint, BLE_ERROR err);
    static void HandleBleMessageReceived(BLEEndPoint * endPoint, System::PacketBuffer * data);
    static void HandleBleConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err);
#endif
};

/**
 *  @class ChipConnectionTunnel
 *
 *  @brief
 *    The definition of the ChipConnectionTunnel class, which manages a pair of TCPEndPoints
 *    whose original ChipConnections have been coupled, and between which the ChipMessageLayer
 *    forwards all data and connection closures.
 */
class ChipConnectionTunnel
{
    friend class ChipMessageLayer;

public:
    uint16_t LogId(void) const { return (uint16_t)((intptr_t) this); }
    void Shutdown(void);

    /**
     *  This function is the application callback that is invoked when the CHIP connection tunnel is shut down.
     *
     *  @param[in]    tun    A pointer to the ChipConnectionTunnel object.
     *
     */
    typedef void (*ShutdownFunct)(ChipConnectionTunnel * tun);
    ShutdownFunct OnShutdown;

    void * AppState; /**< A pointer to application-specific state object. */

private:
    ChipMessageLayer * mMessageLayer;
    TCPEndPoint * mEPOne;
    TCPEndPoint * mEPTwo;

    void Init(ChipMessageLayer * messageLayer);
    CHIP_ERROR MakeTunnelConnected(TCPEndPoint * endpointOne, TCPEndPoint * endpointTwo);
    void CloseEndPoint(TCPEndPoint ** endPoint);
    inline bool IsInUse(void) const { return mMessageLayer != NULL; };

    static void HandleTunnelDataReceived(TCPEndPoint * endPoint, PacketBuffer * data);
    static void HandleTunnelConnectionClosed(TCPEndPoint * endPoint, INET_ERROR err);
    static void HandleReceiveShutdown(TCPEndPoint * endPoint);
};

/**
 *  @class ChipMessageLayer
 *
 *  @brief
 *    The definition of the ChipMessageLayer class, which manages communication
 *    with other CHIP nodes. It employs one of several InetLayer endpoints
 *    to establish a communication channel with other CHIP nodes.
 *
 */
class DLL_EXPORT ChipMessageLayer
{
    friend class ChipMessageLayerTestObject;
    friend class ChipConnection;
    friend class ChipExchangeManager;
    friend class ExchangeContext;
    friend class ChipFabricState;

public:
    /**
     *  @enum State
     *
     *  @brief
     *    The state of the ChipMessageLayer.
     *
     */
    enum State
    {
        kState_NotInitialized = 0, /**< State when the ChipMessageLayer is not initialized. */
        kState_Initializing   = 1, /**< State when the ChipMessageLayer is in the process of being initialized. */
        kState_Initialized    = 2  /**< State when the ChipMessageLayer is initialized. */
    };

    /**
     *  @class InitContext
     *
     *  @brief
     *    The definition of the InitContext class. It encapsulates the set of objects and
     *    variables into a context structure that is required for initialization of the
     *    ChipMessageLayer.
     *
     */
    class InitContext
    {
    public:
        System::Layer * systemLayer;   /**< A pointer to the SystemLayer object. */
        ChipFabricState * fabricState; /**< A pointer to the ChipFabricState object. */
        InetLayer * inet;              /**< A pointer to the InetLayer object. */
        bool listenTCP;                /**< Accept inbound CHIP TCP connections from remote peers on the CHIP port. */
        bool listenUDP;                /**< Accept unsolicited inbound CHIP UDP messages from remote peers on the CHIP port. */
#if CONFIG_NETWORK_LAYER_BLE
        BleLayer * ble; /**< A pointer to the BleLayer object. */
        bool listenBLE; /**< Accept inbound CHIP over BLE connections from remote peers. */
#endif
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
        bool enableEphemeralUDPPort;
        /**< Initiate CHIP UDP exchanges from an ephemeral UDP source port. */
#endif

        /**
         *  The InitContext constructor.
         *
         */
        InitContext(void)
        {
            systemLayer = NULL;
            inet        = NULL;
            fabricState = NULL;
            listenTCP   = true;
            listenUDP   = true;
#if CONFIG_NETWORK_LAYER_BLE
            ble       = NULL;
            listenBLE = true;
#endif
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
            enableEphemeralUDPPort = false;
#endif
        };
    };

    ChipMessageLayer(void);

    System::Layer * SystemLayer;       /*** [READ ONLY] The associated SystemLayer object. */
    InetLayer * Inet;                  /**< [READ ONLY] The associated InetLayer object. */
    ChipFabricState * FabricState;     /**< [READ ONLY] The associated ChipFabricState object. */
    void * AppState;                   /**< A pointer to an application-specific state object. */
    ChipExchangeManager * ExchangeMgr; /**< [READ ONLY] The associated ChipExchangeManager object. */
    ChipSecurityManager * SecurityMgr; /**< [READ ONLY] The associated ChipSecurityManager object. */
    uint32_t IncomingConIdleTimeout;   /**< Default idle timeout (in milliseconds) for incoming connections. */
    uint8_t State;                     /**< [READ ONLY] The state of the ChipMessageLayer object. */
    bool IsListening;                  /**< [READ ONLY] True if listening for incoming connections/messages,
                                            false otherwise. */
    bool mDropMessage;                 /**< Internal and for Debug Only; When set, ChipMessageLayer
                                            drops the message and returns. */

    CHIP_ERROR Init(InitContext * context);
    CHIP_ERROR Shutdown(void);

    CHIP_ERROR SendMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR SendMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR SendMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntfId, ChipMessageInfo * msgInfo,
                           PacketBuffer * msgBuf);
    CHIP_ERROR ResendMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress & destAddr, uint16_t destPort, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId interfaceId, ChipMessageInfo * msgInfo,
                             PacketBuffer * msgBuf);
#if CHIP_CONFIG_ENABLE_TUNNELING
    /**
     *  Function to send a Tunneled packet over a local UDP tunnel.
     */
    CHIP_ERROR SendUDPTunneledMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
#endif
    ChipConnection * NewConnection(void);
    ChipConnectionTunnel * NewConnectionTunnel(void);

    void GetConnectionPoolStats(chip::System::Stats::count_t & aOutInUse) const;

    CHIP_ERROR CreateTunnel(ChipConnectionTunnel ** tunPtr, ChipConnection & conOne, ChipConnection & conTwo,
                            uint32_t inactivityTimeoutMS);

    /**
     *  This function is the higher layer callback that is invoked upon receipt of a CHIP message over UDP.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     msgInfo        A pointer to the ChipMessageInfo object.
     *
     *  @param[in]     payload        Pointer to PacketBuffer message containing tunneled packet received.
     *
     */
    typedef void (*MessageReceiveFunct)(ChipMessageLayer * msgLayer, ChipMessageInfo * msgInfo, PacketBuffer * payload);
    MessageReceiveFunct OnMessageReceived;

#if CHIP_CONFIG_ENABLE_TUNNELING
    /**
     *  This function is the higher layer callback that is invoked upon receipt of a Tunneled packet over a local
     *  UDP tunnel.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     payload        Pointer to PacketBuffer message containing tunneled packet received.
     *
     */
    typedef void (*TunneledMsgReceiveFunct)(ChipMessageLayer * msgLayer, PacketBuffer * payload);
    TunneledMsgReceiveFunct OnUDPTunneledMessageReceived;
#endif

    /**
     *  This function is the higher layer callback invoked upon encountering an error.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     err            The CHIP_ERROR encountered when receiving data.
     *
     *  @param[in]     pktInfo        A read-only pointer to the IPPacketInfo object.
     *
     */
    typedef void (*ReceiveErrorFunct)(ChipMessageLayer * msgLayer, CHIP_ERROR err, const IPPacketInfo * pktInfo);
    ReceiveErrorFunct OnReceiveError;

    /**
     *  This function is the higher layer callback for handling an incoming TCP connection.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     con            A pointer to the ChipConnection object.
     *
     */
    typedef void (*ConnectionReceiveFunct)(ChipMessageLayer * msgLayer, ChipConnection * con);
    ConnectionReceiveFunct OnConnectionReceived;

    /**
     *  This function is invoked for removing a callback.
     *
     *  @param[in]     listenerState  A pointer to the application state object.
     *
     */
    typedef void (*CallbackRemovedFunct)(void * listenerState);

    // Set OnUnsecuredConnectionReceived callbacks. Return CHIP_INCORRECT_STATE if callbacks already set unless force
    // flag set to true. If force flag is true, overwrite existing callbacks with provided values after call to
    // OnUnsecuredConnectionCallbackRemoved.
    CHIP_ERROR SetUnsecuredConnectionListener(ConnectionReceiveFunct newOnUnsecuredConnectionReceived,
                                              CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved, bool force,
                                              void * listenerState);

    // Clear OnUnsecuredConnectionReceived and OnUnsecuredConnectionCallbackRemoved as requested by current
    // callback owner, using function pointer args as proof of identification.
    CHIP_ERROR ClearUnsecuredConnectionListener(ConnectionReceiveFunct oldOnUnsecuredConnectionReceived,
                                                CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved);

    /**
     *  This function is the higher layer callback for reporting an error during handling of an incoming TCP
     *  connection.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     err            The CHIP_ERROR encountered when handling an incoming TCP connection.
     *
     */
    typedef void (*AcceptErrorFunct)(ChipMessageLayer * msgLayer, CHIP_ERROR err);
    AcceptErrorFunct OnAcceptError;

    CHIP_ERROR DecodeHeader(PacketBuffer * msgBuf, ChipMessageInfo * msgInfo, uint8_t ** payloadStart);
    CHIP_ERROR ReEncodeMessage(PacketBuffer * buf);
    CHIP_ERROR EncodeMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf, ChipConnection * con, uint16_t maxLen,
                             uint16_t reserve = 0);
    CHIP_ERROR EncodeMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntId, ChipMessageInfo * msgInfo,
                             PacketBuffer * payload);

    CHIP_ERROR RefreshEndpoints(void);
    CHIP_ERROR CloseEndpoints(void);

    bool IPv4ListenEnabled(void) const;
    bool IPv6ListenEnabled(void) const;
    bool TCPListenEnabled(void) const;
    void SetTCPListenEnabled(bool val);
    bool UDPListenEnabled(void) const;
    void SetUDPListenEnabled(bool val);
    bool UnsecuredListenEnabled(void) const;
    bool EphemeralUDPPortEnabled(void) const;
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
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

    static uint32_t GetMaxChipPayloadSize(const PacketBuffer * msgBuf, bool isUDP, uint32_t udpMTU);

    static void GetPeerDescription(char * buf, size_t bufSize, uint64_t nodeId, const IPAddress * addr, uint16_t port,
                                   InterfaceId interfaceId, const ChipConnection * con);
    static void GetPeerDescription(char * buf, size_t bufSize, const ChipMessageInfo * msgInfo);

private:
    enum
    {
        kFlag_ListenTCP                = 0x01,
        kFlag_ListenUDP                = 0x02,
        kFlag_ListenUnsecured          = 0x04,
        kFlag_EphemeralUDPPortEnabled  = 0x08,
        kFlag_ForceRefreshUDPEndPoints = 0x10,
    };

    TCPEndPoint * mIPv6TCPListen;
    UDPEndPoint * mIPv6UDP;
    ChipConnection mConPool[CHIP_CONFIG_MAX_CONNECTIONS];
    ChipConnectionTunnel mTunnelPool[CHIP_CONFIG_MAX_TUNNELS];
    uint8_t mFlags;

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    UDPEndPoint * mIPv6UDPMulticastRcv;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint * mIPv4UDPBroadcastRcv;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

#if CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    TCPEndPoint * mUnsecuredIPv6TCPListen;
#endif

#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint * mIPv4UDP;
    TCPEndPoint * mIPv4TCPListen;
#endif // INET_CONFIG_ENABLE_IPV4

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    UDPEndPoint * mIPv6EphemeralUDP;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint * mIPv4EphemeralUDP;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    // To set and clear, use SetOnUnsecuredConnectionReceived() and ClearOnUnsecuredConnectionReceived().
    ConnectionReceiveFunct OnUnsecuredConnectionReceived;
    CallbackRemovedFunct OnUnsecuredConnectionCallbacksRemoved;
    void * UnsecuredConnectionReceivedAppState;
    MessageLayerActivityChangeHandlerFunct OnMessageLayerActivityChange;

    CHIP_ERROR EnableUnsecuredListen(void);
    CHIP_ERROR DisableUnsecuredListen(void);

    void SignalMessageLayerActivityChanged(void);

    void CloseListeningEndpoints(void);

    CHIP_ERROR RefreshEndpoint(TCPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr,
                               uint16_t port);
    CHIP_ERROR RefreshEndpoint(UDPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr,
                               uint16_t port, InterfaceId intfId);

    CHIP_ERROR SendMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntfId, PacketBuffer * payload,
                           uint32_t msgFlags);
    CHIP_ERROR SelectOutboundUDPEndPoint(const IPAddress & destAddr, uint32_t msgFlags, UDPEndPoint *& ep);
    CHIP_ERROR SelectDestNodeIdAndAddress(uint64_t & destNodeId, IPAddress & destAddr);
    CHIP_ERROR DecodeMessage(PacketBuffer * msgBuf, uint64_t sourceNodeId, ChipConnection * con, ChipMessageInfo * msgInfo,
                             uint8_t ** rPayload, uint16_t * rPayloadLen);
    CHIP_ERROR EncodeMessageWithLength(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf, ChipConnection * con, uint16_t maxLen);
    CHIP_ERROR DecodeMessageWithLength(PacketBuffer * msgBuf, uint64_t sourceNodeId, ChipConnection * con,
                                       ChipMessageInfo * msgInfo, uint8_t ** rPayload, uint16_t * rPayloadLen,
                                       uint32_t * rFrameLen);
    void GetIncomingTCPConCount(const IPAddress & peerAddr, uint16_t & count, uint16_t & countFromIP);
    void CheckForceRefreshUDPEndPointsNeeded(CHIP_ERROR udpSendErr);

    static void HandleUDPMessage(UDPEndPoint * endPoint, PacketBuffer * msg, const IPPacketInfo * pktInfo);
    static void HandleUDPReceiveError(UDPEndPoint * endPoint, INET_ERROR err, const IPPacketInfo * pktInfo);
    static void HandleIncomingTcpConnection(TCPEndPoint * listeningEndPoint, TCPEndPoint * conEndPoint, const IPAddress & peerAddr,
                                            uint16_t peerPort);
    static void HandleAcceptError(TCPEndPoint * endPoint, INET_ERROR err);
    static void Encrypt_AES128CTRSHA1(const ChipMessageInfo * msgInfo, const uint8_t * key, const uint8_t * inData, uint16_t inLen,
                                      uint8_t * outBuf);
    static void ComputeIntegrityCheck_AES128CTRSHA1(const ChipMessageInfo * msgInfo, const uint8_t * key, const uint8_t * inData,
                                                    uint16_t inLen, uint8_t * outBuf);
    static CHIP_ERROR FilterUDPSendError(CHIP_ERROR err, bool isMulticast);
    static bool IsIgnoredMulticastSendError(CHIP_ERROR err);

    static bool IsSendErrorNonCritical(CHIP_ERROR err);

    ChipMessageLayer(const ChipMessageLayer &); // not defined

#if CONFIG_NETWORK_LAYER_BLE
public:
    BleLayer * mBle; /**< [READ ONLY] Associated BleLayer object. */

private:
    static void HandleIncomingBleConnection(BLEEndPoint * bleEndPoint);
#endif
};

/**
 *  Check if the ChipMessageLayer is configured to listen for inbound communications
 *  over IPv4.
 */
inline bool ChipMessageLayer::IPv4ListenEnabled(void) const
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
 *  Check if the ChipMessageLayer is configured to listen for inbound communications
 *  over IPv4.
 */
inline bool ChipMessageLayer::IPv6ListenEnabled(void) const
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
 *  Check if the ChipMessageLayer is configured to listen for inbound TCP connections.
 */
inline bool ChipMessageLayer::TCPListenEnabled(void) const
{
    return GetFlag(mFlags, kFlag_ListenTCP);
}

/**
 *  Enable or disable listening for inbound TCP connections in the ChipMessageLayer.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the TCP listening state is changed.
 */
inline void ChipMessageLayer::SetTCPListenEnabled(bool val)
{
    SetFlag(mFlags, kFlag_ListenTCP, val);
}

/**
 *  Check if the ChipMessageLayer is configured to listen for inbound UDP messages.
 */
inline bool ChipMessageLayer::UDPListenEnabled(void) const
{
    return GetFlag(mFlags, kFlag_ListenUDP);
}

/**
 *  Enable or disable listening for inbound UDP messages in the ChipMessageLayer.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the UDP listening state is changed.
 */
inline void ChipMessageLayer::SetUDPListenEnabled(bool val)
{
    SetFlag(mFlags, kFlag_ListenUDP, val);
}

/**
 *  Check if locally initiated CHIP UDP exchanges should be sent from an ephemeral
 *  UDP source port.
 */
inline bool ChipMessageLayer::EphemeralUDPPortEnabled(void) const
{
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    return GetFlag(mFlags, kFlag_EphemeralUDPPortEnabled);
#else
    return false;
#endif
}

/**
 *  Enable or disabled initiating CHIP UDP exchanges from an ephemeral UDP source port.
 *
 *  NOTE: \c RefreshEndpoints() must be called after the ephemeral port state is changed.
 */
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

inline void ChipMessageLayer::SetEphemeralUDPPortEnabled(bool val)
{
    SetFlag(mFlags, kFlag_EphemeralUDPPortEnabled, val);
}

#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

/**
 *  Check if unsecured listening is enabled.
 */
inline bool ChipMessageLayer::UnsecuredListenEnabled() const
{
    return GetFlag(mFlags, kFlag_ListenUnsecured);
}

/**
 *  Check if the ChipMessageLayer is bound to a local IPv4 address.
 */
inline bool ChipMessageLayer::IsBoundToLocalIPv4Address(void) const
{
#if INET_CONFIG_ENABLE_IPV4 && CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    return FabricState->ListenIPv4Addr != IPAddress::Any;
#else
    return false;
#endif
}

/**
 *  Check if the ChipMessageLayer is bound to a local IPv6 address.
 */
inline bool ChipMessageLayer::IsBoundToLocalIPv6Address(void) const
{
#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
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
    kEUI64_UL_Mask = 0x0200000000000000ULL, /**< Bitmask for the Universal/Local (U/L) bit within an EUI-64 identifier.
                                                 A value of 0 indicates the id is Universally (globally) administered.
                                                 A value of 1 indicates the id is Locally administered. */

    kEUI64_UL_Unversal = 0, /**< Universal/Local bit value indicating a Universally administered EUI-64 identifier. */

    kEUI64_UL_Local = kEUI64_UL_Mask, /**< Universal/Local bit value indicating a Locally administered EUI-64 identifier. */

    kEUI64_IG_Mask = 0x0100000000000000ULL, /**< Bitmask for the Individual/Group (I/G) bit within an EUI-64 identifier.
                                                 A value of 0 indicates the id is an individual address.
                                                 A value of 1 indicates the id is a group address. */

    kEUI64_IG_Individual = 0, /**< Individual/Group bit value indicating an individual address EUI-64 identifier. */

    kEUI64_IG_Group = kEUI64_IG_Mask, /**< Individual/Group bit value indicating an group address EUI-64 identifier. */
};

/**
 * Special ranges of CHIP Node Ids.
 */
enum
{
    kMaxAlwaysLocalChipNodeId = 0x000000000000FFFFULL, /**< CHIP node identifiers less or equal than this value are considered local
                                                          for testing convenience. */
};

/**
 *  Convert a CHIP fabric identifier to an IPv6 ULA global identifier.
 *
 *  The ULA global identifier for a fabric address is the lower 40 bits of the fabric's
 *  64-bit fabric ID.
 *
 *  @param[in]    fabricId    The CHIP fabric identifier.
 *
 *  @return the mapped IPv6 global identifier.
 *
 */
inline uint64_t ChipFabricIdToIPv6GlobalId(uint64_t fabricId)
{
    return (fabricId & 0xFFFFFFFFFFULL);
}

/**
 *  Convert an IPv6 address interface identifier to a CHIP node identifier.
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalChipNodeId
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
 *  @return the mapped 64 bit CHIP node identifier.
 *
 */
inline uint64_t IPv6InterfaceIdToChipNodeId(uint64_t interfaceId)
{
    return (interfaceId <= kMaxAlwaysLocalChipNodeId) ? interfaceId : (interfaceId ^ kEUI64_UL_Mask);
}

/**
 *  Convert a CHIP node identifier to an IPv6 address interface identifier.
 *
 *  CHIP node identifiers are Universal/Local EUI-64s, which per RFC-3513 are converted to
 *  interface identifiers by inverting the universal/local bit (bit 57 counting the LSB as 0).
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalChipNodeId
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
 *  @param[in]    nodeId    The 64-bit CHIP node identifier.
 *
 *  @return the IPv6 interface identifier.
 *
 */
inline uint64_t ChipNodeIdToIPv6InterfaceId(uint64_t nodeId)
{
    return (nodeId <= kMaxAlwaysLocalChipNodeId) ? nodeId : (nodeId ^ kEUI64_UL_Mask);
}

/**
 *  CHIP has some reserved subnet numbers for distinguished network interfaces
 *  on typical devices. These numbers are assigned here to symbolic constants.
 *  These subnet numbers are used by CHIP to configure IPv6 ULA addresses on
 *  appropriate interfaces.
 */
typedef enum ChipSubnetId
{
    kChipSubnetId_NotSpecified = 0, /**< Reserved as an unspecified or null value. */
    kChipSubnetId_PrimaryWiFi  = 1, /**< The WiFi radio interface subnet number. */
    kChipSubnetId_ThreadAlarm  = 2, /**< The Thread alarm radio interface subnet number. */
    kChipSubnetId_WiFiAP       = 3, /**< The Local Wi-Fi AP interface subnet number. */
    kChipSubnetId_MobileDevice = 4, /**< The subnet identifier for all Mobile devices. */
    kChipSubnetId_Service      = 5, /**< The subnet identifier for the Nest Service endpoints. */
    kChipSubnetId_ThreadMesh   = 6, /**< The Thread mesh radio interface subnet identifier. */
} ChipSubnetId;

#define CHIP_MAX_NODE_ADDR_STR_LENGTH (chip::kChipPeerDescription_MaxLength)
#define CHIP_MAX_MESSAGE_SOURCE_STR_LENGTH (chip::kChipPeerDescription_MaxLength)

/**
 * DEPRECATED -- Use ChipMessageLayer::GetPeerDescription() instead.
 */
inline void ChipNodeAddrToStr(char * buf, uint32_t bufSize, uint64_t nodeId, const IPAddress * addr, uint16_t port,
                              ChipConnection * con)
{
    ChipMessageLayer::GetPeerDescription(buf, (size_t) bufSize, nodeId, addr, port, INET_NULL_INTERFACEID, con);
}

/**
 * DEPRECATED -- Use ChipMessageLayer::GetPeerDescription() instead.
 */
inline void ChipMessageSourceToStr(char * buf, uint32_t bufSize, const ChipMessageInfo * msgInfo)
{
    ChipMessageLayer::GetPeerDescription(buf, (size_t) bufSize, msgInfo);
}

extern CHIP_ERROR GenerateChipNodeId(uint64_t & nodeId);

} // namespace chip

#endif // CHIP_MESSAGE_LAYER_H
