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
 *      This file defines the CHIP Connection class.
 *
 */

#ifndef CHIP_CONNECTION_H
#define CHIP_CONNECTION_H

#include <stdint.h>
#include <string.h>

#include <ble/BleConfig.h>
#include <ble/BLEEndPoint.h>
#include <inet/InetInterface.h>
#include <inet/IPAddress.h>
#include <inet/TCPEndPoint.h>
#include <inet/InetLayer.h>
#include <support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemStats.h>

#include "HostPortList.h"
#include "IdConstants.h"

namespace chip {
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
    kChipMessageFlag_DefaultMulticastSourceAddress = 0x00002000, /**< Indicates that default IPv6 source address selection should
                                                                     be used when sending IPv6 multicast messages. */
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

/** Identifies how a peer node is authenticated.
 *
 * @note ChipAuthMode is an API data type only; it should never be sent over-the-wire.
 */
typedef uint16_t ChipAuthMode;
enum
{
    kChipAuthMode_NotSpecified = 0x0000, /**< Authentication mode not specified. */

    // ===== Major authentication categories =====
    kChipAuthModeCategory_General        = 0x0000, /**< Peer authenticated using one of a set of general mechanisms */
    kChipAuthModeCategory_AppDefinedBase = 0xC000, /**< Base value for application-defined authentication categories */

    // ===== General authentication modes =====
    kChipAuthMode_Unauthenticated = kChipAuthModeCategory_General | 0x001,
    /**< Peer not authenticated. */

    // ===== Subfield Masks =====
    kChipAuthModeCategory_Mask = 0xF000,
};

class ChipConnection;

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

    ChipAuthMode PeerAuthMode;
    /**< The means by which the sender of the message was authenticated. Only meaningful for incoming messages. */

    ChipConnection * InCon;
    /**< The connection (if any) over which the message was received. Only meaningful for incoming messages.*/

    const Inet::IPPacketInfo * InPacketInfo;
    /**< The IP Addressing information of the received message. Only meaningful for incoming messages. */

    void Clear() { memset(this, 0, sizeof(*this)); }
};

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
    Inet::IPAddress PeerAddr;        /**< [READ ONLY] The IP address of the peer node. */
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
    CHIP_ERROR Connect(uint64_t peerNodeId, const Inet::IPAddress & peerAddr, uint16_t peerPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const Inet::IPAddress & peerAddr, uint16_t peerPort = 0,
                       Inet::InterfaceId intf = INET_NULL_INTERFACEID);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t defaultPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t peerAddrLen,
                       uint16_t defaultPort = 0);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, const char * peerAddr, uint16_t peerAddrLen, uint8_t dnsOptions,
                       uint16_t defaultPort);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, HostPortList hostPortList,
                       Inet::InterfaceId intf = INET_NULL_INTERFACEID);
    CHIP_ERROR Connect(uint64_t peerNodeId, ChipAuthMode authMode, HostPortList hostPortList, uint8_t dnsOptions,
                       Inet::InterfaceId intf);

    CHIP_ERROR GetPeerAddressInfo(Inet::IPPacketInfo & addrInfo);

    enum
    {
        kGetPeerDescription_MaxLength = chip::kChipPeerDescription_MaxLength,
        /**< Maximum length of string (including NUL character) returned
             by GetPeerDescription(). */
    };

    void GetPeerDescription(char * buf, size_t bufSize) const;

    CHIP_ERROR SendMessage(ChipMessageInfo * msgInfo, System::PacketBuffer * msgBuf);
#if CHIP_CONFIG_ENABLE_TUNNELING
    /**
     * Function to send a Tunneled packet over a CHIP connection.
     */
    CHIP_ERROR SendTunneledMessage(ChipMessageInfo * msgInfo, System::PacketBuffer * msgBuf);
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

    Inet::TCPEndPoint * GetTCPEndPoint(void) const { return mTcpEndPoint; }

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
     *  @param[in]    msgBuf        A pointer to the System::PacketBuffer object holding the message.
     *
     */
    typedef void (*MessageReceiveFunct)(ChipConnection * con, ChipMessageInfo * msgInfo, System::PacketBuffer * msgBuf);
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
     *  @param[in]     msgBuf         A pointer to the System::PacketBuffer object containing the tunneled packet received.
     *
     */
    typedef void (*TunneledMsgReceiveFunct)(ChipConnection * con, const ChipMessageInfo * msgInfo, System::PacketBuffer * msgBuf);
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

    Inet::IPAddress mPeerAddrs[CHIP_CONFIG_CONNECT_IP_ADDRS];
    Inet::TCPEndPoint * mTcpEndPoint;
    HostPortList mPeerHostPortList;
    Inet::InterfaceId mTargetInterface;
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
    void MakeConnectedTcp(Inet::TCPEndPoint * endPoint, const Inet::IPAddress & localAddr, const Inet::IPAddress & peerAddr);
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

    static void HandleResolveComplete(void * appState, INET_ERROR err, uint8_t addrCount, Inet::IPAddress * addrArray);
    static void HandleConnectComplete(Inet::TCPEndPoint * endPoint, INET_ERROR conRes);
    static void HandleDataReceived(Inet::TCPEndPoint * endPoint, System::PacketBuffer * data);
    static void HandleTcpConnectionClosed(Inet::TCPEndPoint * endPoint, INET_ERROR err);
    static void DefaultConnectionClosedHandler(ChipConnection * con, CHIP_ERROR conErr);

#if CONFIG_NETWORK_LAYER_BLE
public:
    CHIP_ERROR ConnectBle(BLE_CONNECTION_OBJECT connObj, ChipAuthMode authMode, bool autoClose = true);

private:
    Ble::BLEEndPoint * mBleEndPoint;

    void MakeConnectedBle(Ble::BLEEndPoint * endPoint);

    static void HandleBleConnectComplete(Ble::BLEEndPoint * endPoint, BLE_ERROR err);
    static void HandleBleMessageReceived(Ble::BLEEndPoint * endPoint, System::PacketBuffer * data);
    static void HandleBleConnectionClosed(Ble::BLEEndPoint * endPoint, BLE_ERROR err);
#endif
};

} // namespace chip

#endif // CHIP_CONNECTION_H
