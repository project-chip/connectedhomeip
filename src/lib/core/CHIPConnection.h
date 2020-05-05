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
 *      This file defines the CHIP Connection class.
 *
 */

#ifndef CHIP_CONNECTION_H
#define CHIP_CONNECTION_H

#include <stdint.h>
#include <string.h>

#include <support/DLLUtil.h>
#include "HostPortList.h"
#include <system/SystemStats.h>

namespace chip {

using System::PacketBuffer;

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

} // namespace chip

#endif // CHIP_CONNECTION_H
