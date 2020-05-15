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
 *      This file defines the classes for the CHIP Message Layer.
 *
 */

// Include ChipCore.h OUTSIDE of the include guard for ChipMessageLayer.h.
// This allows ChipCore.h to enforce a canonical include order for core
// header files, making it easier to manage dependencies between these files.
#include <core/CHIPCore.h>

#ifndef CHIP_MESSAGE_LAYER_H
#define CHIP_MESSAGE_LAYER_H

#include <stdint.h>
#include <string.h>

#include <ble/BleLayer.h>
#include <core/NodeIdentifiers.h>
#include <core/CHIPFabricState.h>
#include <core/CHIPConnection.h>
#include <inet/InetLayer.h>
#include <support/DLLUtil.h>
#include <system/SystemStats.h>

namespace chip {

using System::PacketBuffer;

class ChipMessageLayer;
class ChipMessageLayerTestObject;
class ChipExchangeManager;
class ChipConnection;

namespace Profiles {
namespace StatusReporting {
class StatusReport;
}
}

/**
 *  @class ChipMessageLayer
 *
 *  @brief
 *    The definition of the ChipMessageLayer class, which manages communication
 *    with other CHIP nodes. It employs one of several InetLayer endpoints
 *    to establish a communication channel with other CHIP nodes.
 *
 */
class DLL_EXPORT ChipMessageLayer: public ChipConnectionContext
{
    friend class ChipMessageLayerTestObject;
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
        kState_NotInitialized = 0,          /**< State when the ChipMessageLayer is not initialized. */
        kState_Initializing = 1,            /**< State when the ChipMessageLayer is in the process of being initialized. */
        kState_Initialized = 2              /**< State when the ChipMessageLayer is initialized. */
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
        System::Layer*      systemLayer;    /**< A pointer to the SystemLayer object. */
        ChipFabricState*    fabricState;    /**< A pointer to the ChipFabricState object. */
        Inet::InetLayer*    inet;           /**< A pointer to the InetLayer object. */
        bool                listenTCP;      /**< Accept inbound CHIP TCP connections from remote peers on the CHIP port. */
        bool                listenUDP;      /**< Accept unsolicited inbound CHIP UDP messages from remote peers on the CHIP port. */
#if CONFIG_NETWORK_LAYER_BLE
        Ble::BleLayer*      ble;            /**< A pointer to the BleLayer object. */
        bool                listenBLE;      /**< Accept inbound CHIP over BLE connections from remote peers. */
#endif
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
        bool                enableEphemeralUDPPort;
                                            /**< Initiate CHIP UDP exchanges from an ephemeral UDP source port. */
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
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
            enableEphemeralUDPPort = false;
#endif
        };
    };

    ChipMessageLayer(void);

    System::Layer *SystemLayer;                         /*** [READ ONLY] The associated SystemLayer object. */
    Inet::InetLayer *Inet;                              /**< [READ ONLY] The associated InetLayer object. */
    ChipFabricState *FabricState;                       /**< [READ ONLY] The associated ChipFabricState object. */
    void *AppState;                                     /**< A pointer to an application-specific state object. */
    ChipExchangeManager *ExchangeMgr;                   /**< [READ ONLY] The associated ChipExchangeManager object. */
    uint32_t IncomingConIdleTimeout;                    /**< Default idle timeout (in milliseconds) for incoming connections. */
    uint8_t State;                                      /**< [READ ONLY] The state of the ChipMessageLayer object. */
    bool IsListening;                                   /**< [READ ONLY] True if listening for incoming connections/messages,
                                                             false otherwise. */
    bool mDropMessage;                                  /**< Internal and for Debug Only; When set, ChipMessageLayer
                                                             drops the message and returns. */

    CHIP_ERROR Init(InitContext *context);
    CHIP_ERROR Shutdown(void);

    CHIP_ERROR SendMessage(ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR SendMessage(const IPAddress &destAddr, ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR SendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntfId, ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR ResendMessage(ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress &destAddr, ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress &destAddr, uint16_t destPort, ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    CHIP_ERROR ResendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId interfaceId, ChipMessageInfo *msgInfo, PacketBuffer *msgBuf);
    ChipConnection *NewConnection(void);

    void GetConnectionPoolStats(chip::System::Stats::count_t &aOutInUse) const;


    /**
     *  This function is the higher layer callback that is invoked upon receipt of a CHIP message over UDP.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     msgInfo        A pointer to the ChipMessageInfo object.
     *
     *  @param[in]     payload        Pointer to PacketBuffer message containing the packet received.
     *
     */
    typedef void (*MessageReceiveFunct)(ChipMessageLayer *msgLayer, ChipMessageInfo *msgInfo, PacketBuffer *payload);
    MessageReceiveFunct OnMessageReceived;

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
    typedef void (*ReceiveErrorFunct)(ChipMessageLayer *msgLayer, CHIP_ERROR err, const IPPacketInfo *pktInfo);
    ReceiveErrorFunct OnReceiveError;

    /**
     *  This function is the higher layer callback for handling an incoming TCP connection.
     *
     *  @param[in]     msgLayer       A pointer to the ChipMessageLayer object.
     *
     *  @param[in]     con            A pointer to the ChipConnection object.
     *
     */
    typedef void (*ConnectionReceiveFunct)(ChipMessageLayer *msgLayer, ChipConnection *con);
    ConnectionReceiveFunct OnConnectionReceived;

    /**
     *  This function is invoked for removing a callback.
     *
     *  @param[in]     listenerState  A pointer to the application state object.
     *
     */
    typedef void (*CallbackRemovedFunct)(void *listenerState);

    // Set OnUnsecuredConnectionReceived callbacks. Return CHIP_INCORRECT_STATE if callbacks already set unless force
    // flag set to true. If force flag is true, overwrite existing callbacks with provided values after call to
    // OnUnsecuredConnectionCallbackRemoved.
    CHIP_ERROR SetUnsecuredConnectionListener(ConnectionReceiveFunct newOnUnsecuredConnectionReceived,
            CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved, bool force, void *listenerState);

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
    typedef void (*AcceptErrorFunct)(ChipMessageLayer *msgLayer, CHIP_ERROR err);
    AcceptErrorFunct OnAcceptError;

    CHIP_ERROR DecodeHeader(PacketBuffer *msgBuf, ChipMessageInfo *msgInfo, uint8_t **payloadStart);
    CHIP_ERROR ReEncodeMessage(PacketBuffer *buf);
    CHIP_ERROR EncodeMessage(ChipMessageInfo *msgInfo, PacketBuffer *msgBuf, ChipConnection *con, uint16_t maxLen,
            uint16_t reserve = 0);
    CHIP_ERROR EncodeMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntId, ChipMessageInfo *msgInfo, PacketBuffer *payload);

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

    static uint32_t GetMaxChipPayloadSize(const PacketBuffer *msgBuf, bool isUDP, uint32_t udpMTU);

    static void GetPeerDescription(char *buf, size_t bufSize, uint64_t nodeId, const IPAddress *addr, uint16_t port, InterfaceId interfaceId, const ChipConnection *con);
    static void GetPeerDescription(char *buf, size_t bufSize, const ChipMessageInfo *msgInfo);

    /************** ChipConnectionContext implementation ******************/
    Inet::InetLayer * InetLayer() override { return Inet; }
    Ble::BleLayer * BleLayer() override { return mBle; }


    CHIP_ERROR EncodeMessageWithLength(ChipMessageInfo *msgInfo, PacketBuffer *msgBuf, ChipConnection *con,
            uint16_t maxLen) override;
    CHIP_ERROR DecodeMessageWithLength(PacketBuffer *msgBuf, uint64_t sourceNodeId, ChipConnection *con,
            ChipMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen, uint32_t *rFrameLen) override;

    uint64_t LocalNodeId() override { return FabricState->LocalNodeId; }

    const Inet::IPAddress & ListenIPv6Addr() const override {
	    return FabricState->ListenIPv6Addr;
    }

    CHIP_ERROR SelectDestNodeIdAndAddress(uint64_t& destNodeId, IPAddress& destAddr) override;
    void HandleConnectionClosed(ChipConnection * con, CHIP_ERROR err) override;

    void HandleOnReceiveError(ChipConnection * con, CHIP_ERROR err, const Inet::IPPacketInfo * pktInfo) override {
        OnReceiveError(this, err, pktInfo);
    }

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
    ChipConnection mConPool[CHIP_CONFIG_MAX_CONNECTIONS];
    uint8_t mFlags;

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    UDPEndPoint *mIPv6UDPMulticastRcv;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4UDPBroadcastRcv;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

#if CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    TCPEndPoint *mUnsecuredIPv6TCPListen;
#endif

#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4UDP;
    TCPEndPoint *mIPv4TCPListen;
#endif // INET_CONFIG_ENABLE_IPV4

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    UDPEndPoint *mIPv6EphemeralUDP;
#if INET_CONFIG_ENABLE_IPV4
    UDPEndPoint *mIPv4EphemeralUDP;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    // To set and clear, use SetOnUnsecuredConnectionReceived() and ClearOnUnsecuredConnectionReceived().
    ConnectionReceiveFunct OnUnsecuredConnectionReceived;
    CallbackRemovedFunct OnUnsecuredConnectionCallbacksRemoved;
    void *UnsecuredConnectionReceivedAppState;
    MessageLayerActivityChangeHandlerFunct OnMessageLayerActivityChange;

    CHIP_ERROR EnableUnsecuredListen(void);
    CHIP_ERROR DisableUnsecuredListen(void);

    void SignalMessageLayerActivityChanged(void);

    void CloseListeningEndpoints(void);

    CHIP_ERROR RefreshEndpoint(TCPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port);
    CHIP_ERROR RefreshEndpoint(UDPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port, InterfaceId intfId);

    CHIP_ERROR SendMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntfId, PacketBuffer *payload, uint32_t msgFlags);
    CHIP_ERROR SelectOutboundUDPEndPoint(const IPAddress & destAddr, uint32_t msgFlags, UDPEndPoint *& ep);
    CHIP_ERROR DecodeMessage(PacketBuffer *msgBuf, uint64_t sourceNodeId, ChipConnection *con,
            ChipMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen);
    void GetIncomingTCPConCount(const IPAddress &peerAddr, uint16_t &count, uint16_t &countFromIP);
    void CheckForceRefreshUDPEndPointsNeeded(CHIP_ERROR udpSendErr);

    static void HandleUDPMessage(UDPEndPoint *endPoint, PacketBuffer *msg, const IPPacketInfo *pktInfo);
    static void HandleUDPReceiveError(UDPEndPoint *endPoint, INET_ERROR err, const IPPacketInfo *pktInfo);
    static void HandleIncomingTcpConnection(TCPEndPoint *listeningEndPoint, TCPEndPoint *conEndPoint, const IPAddress &peerAddr,
            uint16_t peerPort);
    static void HandleAcceptError(TCPEndPoint *endPoint, INET_ERROR err);
    static CHIP_ERROR FilterUDPSendError(CHIP_ERROR err, bool isMulticast);
    static bool IsIgnoredMulticastSendError(CHIP_ERROR err);

    static bool IsSendErrorNonCritical(CHIP_ERROR err);

    ChipMessageLayer(const ChipMessageLayer&);   // not defined

#if CONFIG_NETWORK_LAYER_BLE
public:
    Ble::BleLayer *mBle;                                      /**< [READ ONLY] Associated BleLayer object. */

private:
    static void HandleIncomingBleConnection(BLEEndPoint *bleEndPoint);
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

#define CHIP_MAX_NODE_ADDR_STR_LENGTH (chip::kChipPeerDescription_MaxLength)
#define CHIP_MAX_MESSAGE_SOURCE_STR_LENGTH (chip::kChipPeerDescription_MaxLength)

/**
 * DEPRECATED -- Use ChipMessageLayer::GetPeerDescription() instead.
 */
inline void ChipNodeAddrToStr(char *buf, uint32_t bufSize, uint64_t nodeId, const IPAddress *addr, uint16_t port, ChipConnection *con)
{
    ChipMessageLayer::GetPeerDescription(buf, (size_t)bufSize, nodeId, addr, port, INET_NULL_INTERFACEID, con);
}

/**
 * DEPRECATED -- Use ChipMessageLayer::GetPeerDescription() instead.
 */
inline void ChipMessageSourceToStr(char *buf, uint32_t bufSize, const ChipMessageInfo *msgInfo)
{
    ChipMessageLayer::GetPeerDescription(buf, (size_t)bufSize, msgInfo);
}

extern CHIP_ERROR GenerateChipNodeId(uint64_t & nodeId);

} // namespace chip

#endif // CHIP_MESSAGE_LAYER_H
