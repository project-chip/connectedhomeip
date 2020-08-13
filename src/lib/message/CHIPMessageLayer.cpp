/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
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
 *      This file implements the ChipMessageLayer class. It manages communication
 *      with other CHIP nodes by employing one of several Inetlayer endpoints
 *      to establish a communication channel with other CHIP nodes.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <crypto/CHIPCryptoPAL.h>
#include <message/CHIPExchangeMgr.h>
#include <message/CHIPMessageLayer.h>
#include <message/CHIPSecurityMgr.h>
#include <support/CHIPFaultInjection.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/crypto/AESBlockCipher.h>
#include <support/crypto/CHIPCrypto.h>
#include <support/crypto/CTRMode.h>
#include <support/crypto/HMAC.h>
#include <support/crypto/HashAlgos.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

using namespace chip::Crypto;
using namespace chip::Encoding;

/**
 *  @def CHIP_BIND_DETAIL_LOGGING
 *
 *  @brief
 *    Use CHIP Bind detailed logging for CHIP communication.
 *
 */
#ifndef CHIP_BIND_DETAIL_LOGGING
#define CHIP_BIND_DETAIL_LOGGING 1
#endif

/**
 *  @def ChipBindLog(MSG, ...)
 *
 *  @brief
 *    Define ChipBindLogic to be the same as ChipLogProgress based on
 *    whether both #CHIP_BIND_DETAIL_LOGGING and #CHIP_DETAIL_LOGGING
 *    are set.
 *
 */
#if CHIP_BIND_DETAIL_LOGGING && CHIP_DETAIL_LOGGING
#define ChipBindLog(MSG, ...) ChipLogProgress(MessageLayer, MSG, ##__VA_ARGS__)
#else
#define ChipBindLog(MSG, ...)
#endif

enum
{
    kKeyIdLen      = 2,
    kMinPayloadLen = 1
};

/**
 *  The CHIP Message layer constructor.
 *
 *  @note
 *    The class must be initialized via ChipMessageLayer::Init()
 *    prior to use.
 *
 */
ChipMessageLayer::ChipMessageLayer()
{
    State = kState_NotInitialized;
}

/**
 *  Initialize the CHIP Message layer object.
 *
 *  @param[in]  context  A pointer to the InitContext object.
 *
 *  @retval  #CHIP_NO_ERROR                     on successful initialization.
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT       if the passed InitContext object is NULL.
 *  @retval  #CHIP_ERROR_INCORRECT_STATE        if the state of the ChipMessageLayer object is incorrect.
 *  @retval  other errors generated from the lower Inet layer during endpoint creation.
 *
 */
CHIP_ERROR ChipMessageLayer::Init(InitContext * context)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(State == kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(context != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    State = kState_Initializing;

    SystemLayer = context->systemLayer;
    Inet        = context->inet;
#if CONFIG_NETWORK_LAYER_BLE
    mBle = context->ble;
#endif

#if CHIP_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    if (SystemLayer == NULL)
    {
        SystemLayer = Inet->SystemLayer();
    }
#endif // CHIP_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    FabricState                           = context->fabricState;
    FabricState->MessageLayer             = this;
    OnMessageReceived                     = NULL;
    OnReceiveError                        = NULL;
    OnConnectionReceived                  = NULL;
    OnUnsecuredConnectionReceived         = NULL;
    OnUnsecuredConnectionCallbacksRemoved = NULL;
    OnAcceptError                         = NULL;
    OnMessageLayerActivityChange          = NULL;
    memset(mConPool, 0, sizeof(mConPool));
    memset(mTunnelPool, 0, sizeof(mTunnelPool));
    AppState               = NULL;
    ExchangeMgr            = NULL;
    SecurityMgr            = NULL;
    IsListening            = context->listenTCP || context->listenUDP;
    IncomingConIdleTimeout = CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT;

    // Internal and for Debug Only; When set, Message Layer drops message and returns.
    mDropMessage = false;
    mFlags       = 0;
    SetTCPListenEnabled(context->listenTCP);
    SetUDPListenEnabled(context->listenUDP);
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    SetEphemeralUDPPortEnabled(context->enableEphemeralUDPPort);
#endif

    mIPv6TCPListen = NULL;
    mIPv6UDP       = NULL;

#if INET_CONFIG_ENABLE_IPV4
    mIPv4TCPListen = NULL;
    mIPv4UDP       = NULL;
#endif // INET_CONFIG_ENABLE_IPV4

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    mIPv6UDPMulticastRcv = NULL;
#if INET_CONFIG_ENABLE_IPV4
    mIPv4UDPBroadcastRcv = NULL;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    mIPv6EphemeralUDP = NULL;
#if INET_CONFIG_ENABLE_IPV4
    mIPv4EphemeralUDP = NULL;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    mUnsecuredIPv6TCPListen = NULL;
#endif

    err = RefreshEndpoints();
    SuccessOrExit(err);

#if CONFIG_NETWORK_LAYER_BLE
    if (context->listenBLE && mBle != NULL)
    {
        mBle->mAppState                = this;
        mBle->OnChipBleConnectReceived = HandleIncomingBleConnection;
        ChipLogProgress(MessageLayer, "Accepting WoBLE connections");
    }
    else
    {
        ChipLogProgress(MessageLayer, "WoBLE disabled%s", (mBle != NULL) ? " by application" : " (BLE layer not initialized)");
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    State = kState_Initialized;

exit:
    if (err != CHIP_NO_ERROR && State == kState_Initializing)
    {
        Shutdown();
    }
    return err;
}

/**
 *  Shutdown the ChipMessageLayer.
 *
 *  Close all open Inet layer endpoints, reset all
 *  higher layer callbacks, member variables and objects.
 *  A call to Shutdown() terminates the ChipMessageLayer
 *  object.
 *
 */
CHIP_ERROR ChipMessageLayer::Shutdown()
{
    CloseEndpoints();

#if CONFIG_NETWORK_LAYER_BLE
    if (mBle != NULL && mBle->mAppState == this)
    {
        mBle->mAppState                = NULL;
        mBle->OnChipBleConnectReceived = NULL;
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    State                         = kState_NotInitialized;
    IsListening                   = false;
    FabricState                   = NULL;
    OnMessageReceived             = NULL;
    OnReceiveError                = NULL;
    OnUnsecuredConnectionReceived = NULL;
    OnConnectionReceived          = NULL;
    OnAcceptError                 = NULL;
    OnMessageLayerActivityChange  = NULL;
    memset(mConPool, 0, sizeof(mConPool));
    memset(mTunnelPool, 0, sizeof(mTunnelPool));
    ExchangeMgr = NULL;
    AppState    = NULL;
    mFlags      = 0;

    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_ENABLE_TUNNELING
/**
 *  Send a tunneled IPv6 data message over UDP.
 *
 *  @param[in] msgInfo          A pointer to a ChipMessageInfo object.
 *
 *  @param[in] destAddr         IPAddress of the UDP tunnel destination.
 *
 *  @param[in] msgBuf           A pointer to the PacketBuffer object holding the packet to send.
 *
 *  @retval  #CHIP_NO_ERROR                    on successfully sending the message down to the network
 *                                              layer.
 *  @retval  #CHIP_ERROR_INVALID_ADDRESS       if the destAddr is not specified or cannot be determined
 *                                              from destination node id.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::SendUDPTunneledMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * msgBuf)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    // Set message version to V2
    msgInfo->MessageVersion = kChipMessageVersion_V2;

    // Set the tunneling flag
    msgInfo->Flags |= kChipMessageFlag_TunneledData;

    res    = SendMessage(destAddr, msgInfo, msgBuf);
    msgBuf = NULL;

    return res;
}
#endif // CHIP_CONFIG_ENABLE_TUNNELING

/**
 *  Encode a CHIP Message layer header into an PacketBuffer.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    sendIntId     The interface on which to send the CHIP message.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object that would hold the CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR                           on successful encoding of the CHIP message.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION  if the CHIP Message version is not supported.
 *  @retval  #CHIP_ERROR_INVALID_MESSAGE_LENGTH       if the payload length in the message buffer is zero.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE  if the encryption type is not supported.
 *  @retval  #CHIP_ERROR_MESSAGE_TOO_LONG             if the encoded message would be longer than the
 *                                                     requested maximum.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL             if there is not enough space before or after the
 *                                                     message payload.
 *  @retval  other errors generated by the fabric state object when fetching the session state.
 *
 */
CHIP_ERROR ChipMessageLayer::EncodeMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntId,
                                           ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    // Set the source node identifier in the message header.
    if ((msgInfo->Flags & kChipMessageFlag_ReuseSourceId) == 0)
        msgInfo->SourceNodeId = FabricState->LocalNodeId;

    // Force inclusion of the source node identifier if the destination address is not a local fabric address.
    //
    // Technically it should be possible to omit the source node identifier in other situations beyond the
    // ones allowed for here.  However it is difficult to determine exactly what the source IP
    // address will be when sending a UDP packet, so we err on the side of correctness and only omit
    // the source identifier if we're part of a fabric and sending to another member of the same fabric.
    if (!FabricState->IsFabricAddress(destAddr))
        msgInfo->Flags |= kChipMessageFlag_SourceNodeId;

    // Force the destination node identifier to be included if it doesn't match the interface identifier in
    // the destination address.
    if (!destAddr.IsIPv6ULA() || IPv6InterfaceIdToChipNodeId(destAddr.InterfaceId()) != msgInfo->DestNodeId)
        msgInfo->Flags |= kChipMessageFlag_DestNodeId;

    // Encode the CHIP message. NOTE that this results in the payload buffer containing the entire encoded message.
    res = EncodeMessage(msgInfo, payload, NULL, UINT16_MAX, 0);

    return res;
}

/**
 *  Send a CHIP message using the underlying Inetlayer UDP endpoint after encoding it.
 *
 *  @note
 *    The destination port used is #CHIP_PORT.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::SendMessage(ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    return SendMessage(IPAddress::Any, msgInfo, payload);
}

/**
 *  Send a CHIP message using the underlying Inetlayer UDP endpoint after encoding it.
 *
 *  @note
 *    -The destination port used is #CHIP_PORT.
 *
 *    -If the destination address has not been supplied, attempt to determine it from the node identifier in
 *     the message header. Fail if this can't be done.
 *
 *    -If the destination address is a fabric address for the local fabric, and the caller
 *     didn't specify the destination node id, extract it from the destination address.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::SendMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    return SendMessage(destAddr, CHIP_PORT, INET_NULL_INTERFACEID, msgInfo, payload);
}

/**
 *  Send a CHIP message using the underlying Inetlayer UDP endpoint after encoding it.
 *
 *  @note
 *    -If the destination address has not been supplied, attempt to determine it from the node identifier in
 *     the message header. Fail if this can't be done.
 *
 *    -If the destination address is a fabric address for the local fabric, and the caller
 *     didn't specify the destination node id, extract it from the destination address.
 *
 *  @param[in]    aDestAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    sendIntfId    The interface on which to send the CHIP message.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR                    on successfully sending the message down to the network
 *                                              layer.
 *  @retval  #CHIP_ERROR_INVALID_ADDRESS       if the destAddr is not specified or cannot be determined
 *                                              from destination node id.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::SendMessage(const IPAddress & aDestAddr, uint16_t destPort, InterfaceId sendIntfId,
                                         ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    CHIP_ERROR res     = CHIP_NO_ERROR;
    IPAddress destAddr = aDestAddr;

    // Determine the message destination address based on the destination nodeId.
    res = SelectDestNodeIdAndAddress(msgInfo->DestNodeId, destAddr);
    SuccessOrExit(res);

    res = EncodeMessage(destAddr, destPort, sendIntfId, msgInfo, payload);
    SuccessOrExit(res);

    // on delay send, we do everything except actually send the
    // message.  As a result, the payload will contain the entire
    // state required for sending it a bit later
    if (msgInfo->Flags & kChipMessageFlag_DelaySend)
        return CHIP_NO_ERROR;

    // Copy msg to a right-sized buffer if applicable
    payload = PacketBuffer::RightSize(payload);

    // Send the message using the appropriate UDP endpoint(s).
    return SendMessage(destAddr, destPort, sendIntfId, payload, msgInfo->Flags);

exit:
    if ((res != CHIP_NO_ERROR) && (payload != NULL) && ((msgInfo->Flags & kChipMessageFlag_RetainBuffer) == 0))
    {
        PacketBuffer::Free(payload);
    }

    return res;
}

bool ChipMessageLayer::IsIgnoredMulticastSendError(CHIP_ERROR err)
{
    return err == CHIP_NO_ERROR ||
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        err == System::MapErrorLwIP(ERR_RTE)
#else
        err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL)
#endif
        ;
}

CHIP_ERROR ChipMessageLayer::FilterUDPSendError(CHIP_ERROR err, bool isMulticast)
{
    // Don't report certain types of routing errors when they occur while sending multicast packets.
    // These may indicate that the underlying interface doesn't support multicast (e.g. the loopback
    // interface on linux) or that the selected interface doesn't have an appropriate source address.
    if (isMulticast)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        if (err == System::MapErrorLwIP(ERR_RTE))
        {
            err = CHIP_NO_ERROR;
        }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        if (err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL))
        {
            err = CHIP_NO_ERROR;
        }
#endif
    }

    return err;
}

/**
 *  Checks if error, while sending, is critical enough to report to the application.
 *
 *  @param[in]    err      The #CHIP_ERROR being checked for criticality.
 *
 *  @return    true if the error is NOT critical; false otherwise.
 *
 */
bool ChipMessageLayer::IsSendErrorNonCritical(CHIP_ERROR err)
{
    return (err == INET_ERROR_NOT_IMPLEMENTED || err == INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED ||
            err == INET_ERROR_MESSAGE_TOO_LONG || err == INET_ERROR_NO_MEMORY || CHIP_CONFIG_IsPlatformErrorNonCritical(err));
}

/**
 * Set the 'ForceRefreshUDPEndpoints' flag if needed.
 *
 * Based on the error returned when sending a UDP message, set a flag in the ChipMessageLayer
 * that will force a complete refresh of all UDPEndPoints the next time \c RefreshEndPoints is
 * called.
 */
void ChipMessageLayer::CheckForceRefreshUDPEndPointsNeeded(CHIP_ERROR err)
{
    // On some sockets-based systems, the OS will invalidate bound UDP endpoints when certain
    // network transitions occur.  This is known to occur on Android, although the precise
    // conditions are unclear.  When that happens, set the ForceRefreshUDPEndPoints flag to
    // force all UDPEndPoints to be closed and re-opened on the next call to RefreshEndPoints().
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (err == System::MapErrorPOSIX(EPIPE))
    {
        SetFlag(mFlags, kFlag_ForceRefreshUDPEndPoints);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

/**
 *  Send an encoded CHIP message using the appropriate underlying Inetlayer UDPEndPoint (or EndPoints).
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    sendIntfId    The interface on which to send the CHIP message.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded CHIP message.
 *
 *  @param[in]    msgSendFlags  Send flags containing metadata about the message for the lower Inet layer.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::SendMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntfId,
                                         PacketBuffer * payload, uint32_t msgFlags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    UDPEndPoint * ep;
    enum
    {
        kUnicast,
        kMulticast_OneInterface,
        kMulticast_AllInterfaces,
        kMulticast_AllFabricAddrs,
    } sendAction;
    uint16_t udpSendFlags;

    IPPacketInfo pktInfo;
    pktInfo.Clear();
    pktInfo.DestAddress = destAddr;
    pktInfo.DestPort    = destPort;
    pktInfo.Interface   = sendIntfId;

    // Check if drop flag is set; If so, do not send message; return CHIP_NO_ERROR;
    VerifyOrExit(!mDropMessage, err = CHIP_NO_ERROR);

    // Drop the message and return. Free the buffer if it does not need to be
    // retained(e.g., for WRM retransmissions).
    CHIP_FAULT_INJECT(FaultInjection::kFault_DropOutgoingUDPMsg, ExitNow(err = CHIP_NO_ERROR););

    // Select a UDP endpoint object for sending a message based on the destination address type
    // and the message send flags.
    err = SelectOutboundUDPEndPoint(destAddr, msgFlags, ep);
    SuccessOrExit(err);

    // Select an appropriate send action for the message.
    //
    // For unicast messages, send the message once to the given address.  If a target interface
    // is given, the message will be sent over that interface.
    //
    // For multicast/broadcast messages...
    //
    //     If the local node is bound to a specific address (IPv4 or IPv6) send the multicast
    //     message once over the bound interface.
    //
    //     Otherwise, if the destination is an IPv6 multicast address, and the local node is
    //     a member of a CHIP fabric, AND MulticastFromLinkLocal has NOT been specified, send
    //     the message once for each CHIP Fabric ULA assigned to a local interface that supports
    //     multicast. If a target interface is given, only consider ULAs on that interface.
    //
    //     Otherwise, if a target interface is given, send the multicast message over that
    //     interface only.
    //
    //     Otherwise, send the message over each local interface that supports multicast.
    //
    if (!destAddr.IsMulticast() && !destAddr.IsIPv4Broadcast())
    {
        sendAction = kUnicast;
    }
#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    else if (destAddr.IsIPv4() ? IsBoundToLocalIPv4Address() : IsBoundToLocalIPv6Address())
    {
        sendAction = kMulticast_OneInterface;
    }
#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    else if (destAddr.IsIPv6() && FabricState->FabricId != kFabricIdNotSpecified &&
             !GetFlag(msgFlags, kChipMessageFlag_DefaultMulticastSourceAddress))
    {
        sendAction = kMulticast_AllFabricAddrs;
    }
    else if (sendIntfId != INET_NULL_INTERFACEID)
    {
        sendAction = kMulticast_OneInterface;
    }
    else
    {
        sendAction = kMulticast_AllInterfaces;
    }

    // Send the message...
    switch (sendAction)
    {
    case kUnicast:
    case kMulticast_OneInterface:

        // Send the message once. If requested by the caller, instruct the end point code to not free the
        // message buffer. If a send interface was specified, the message is sent over that interface.
        udpSendFlags = GetFlag(msgFlags, kChipMessageFlag_RetainBuffer) ? UDPEndPoint::kSendFlag_RetainBuffer : 0;
        err          = ep->SendMsg(&pktInfo, payload, udpSendFlags);
        payload      = NULL; // Prevent call to Free() in exit code
        CheckForceRefreshUDPEndPointsNeeded(err);
        err = FilterUDPSendError(err, sendAction == kMulticast_OneInterface);
        break;

    case kMulticast_AllInterfaces:

        // Send the message over each local interface that supports multicast.
        for (InterfaceIterator intfIter; intfIter.HasCurrent(); intfIter.Next())
        {
            if (intfIter.SupportsMulticast())
            {
                pktInfo.Interface  = intfIter.GetInterface();
                CHIP_ERROR sendErr = ep->SendMsg(&pktInfo, payload, UDPEndPoint::kSendFlag_RetainBuffer);
                CheckForceRefreshUDPEndPointsNeeded(sendErr);
                if (err == CHIP_NO_ERROR)
                {
                    err = FilterUDPSendError(sendErr, true);
                }
            }
        }

        break;

    case kMulticast_AllFabricAddrs:

        // Send the message once for each CHIP Fabric ULA assigned to a local interface that supports
        // multicast/broadcast. If the caller has specified a particular interface, only send over the
        // specified interface.  For each message sent, arrange for the source address to be the Fabric ULA.
        for (InterfaceAddressIterator addrIter; addrIter.HasCurrent(); addrIter.Next())
        {
            pktInfo.SrcAddress = addrIter.GetAddress();
            pktInfo.Interface  = addrIter.GetInterface();
            if (addrIter.SupportsMulticast() && FabricState->IsLocalFabricAddress(pktInfo.SrcAddress) &&
                (sendIntfId == INET_NULL_INTERFACEID || pktInfo.Interface == sendIntfId))
            {
                CHIP_ERROR sendErr = ep->SendMsg(&pktInfo, payload, UDPEndPoint::kSendFlag_RetainBuffer);
                CheckForceRefreshUDPEndPointsNeeded(sendErr);
                if (err == CHIP_NO_ERROR)
                {
                    err = FilterUDPSendError(sendErr, true);
                }
            }
        }

        break;
    }

exit:
    if (payload != NULL && !GetFlag(msgFlags, kChipMessageFlag_RetainBuffer))
        PacketBuffer::Free(payload);
    return err;
}

/**
 *  Select an appropriate UDP endpoint for sending a CHIP message.
 */
CHIP_ERROR ChipMessageLayer::SelectOutboundUDPEndPoint(const IPAddress & destAddr, uint32_t msgFlags, UDPEndPoint *& ep)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Select a UDP endpoint object for sending a message based on the destination address type
    // and the message send flags.
    //
    // If the CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT option is set, select the ephemeral UDP
    // endpoint if the caller has specified the 'ViaEphemeralUDPPort' flag.  This will result in
    // the source port field of the UDP message being set to the currently active ephemeral
    // port. Otherwise, select the CHIP UDP endpoint. This will result in the source port
    // field being set to the well-known CHIP port.
    //
    switch (destAddr.Type())
    {
#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        if (GetFlag(msgFlags, kChipMessageFlag_ViaEphemeralUDPPort))
        {
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
            ep = mIPv4EphemeralUDP;
#else
            ep = NULL;
#endif
        }
        else
        {
            ep = mIPv4UDP;
        }
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kIPAddressType_IPv6:
        if (GetFlag(msgFlags, kChipMessageFlag_ViaEphemeralUDPPort))
        {
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
            ep = mIPv6EphemeralUDP;
#else
            ep = NULL;
#endif
        }
        else
        {
            ep = mIPv6UDP;
        }
        break;

    default:
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(ep != NULL, err = CHIP_ERROR_NO_ENDPOINT);

exit:
    return err;
}

/**
 *  Resend an encoded CHIP message using the underlying Inetlayer UDP endpoint.
 *
 *  @param[in]    msgInfo     A pointer to the ChipMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::ResendMessage(ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    IPAddress destAddr = IPAddress::Any;
    return ResendMessage(destAddr, msgInfo, payload);
}

/**
 *  Resend an encoded CHIP message using the underlying Inetlayer UDP endpoint.
 *
 *  @note
 *    The destination port used is #CHIP_PORT.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    msgInfo       A pointer to the ChipMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::ResendMessage(const IPAddress & destAddr, ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    return ResendMessage(destAddr, CHIP_PORT, msgInfo, payload);
}

/**
 *  Resend an encoded CHIP message using the underlying Inetlayer UDP endpoint.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    msgInfo       A pointer to the ChipMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::ResendMessage(const IPAddress & destAddr, uint16_t destPort, ChipMessageInfo * msgInfo,
                                           PacketBuffer * payload)
{
    return ResendMessage(destAddr, CHIP_PORT, INET_NULL_INTERFACEID, msgInfo, payload);
}

/**
 *  Resend an encoded CHIP message using the underlying Inetlayer UDP endpoint.
 *
 *  @note
 *    -If the destination address has not been supplied, attempt to determine it from the node identifier in
 *     the message header. Fail if this can't be done.
 *
 *    -If the destination address is a fabric address for the local fabric, and the caller
 *     didn't specify the destination node id, extract it from the destination address.
 *
 *  @param[in]    aDestAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    interfaceId   The interface on which to send the CHIP message.
 *
 *  @param[in]    msgInfo       A pointer to the ChipMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded CHIP message.
 *
 *  @retval  #CHIP_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
CHIP_ERROR ChipMessageLayer::ResendMessage(const IPAddress & aDestAddr, uint16_t destPort, InterfaceId interfaceId,
                                           ChipMessageInfo * msgInfo, PacketBuffer * payload)
{
    CHIP_ERROR res     = CHIP_NO_ERROR;
    IPAddress destAddr = aDestAddr;

    res = SelectDestNodeIdAndAddress(msgInfo->DestNodeId, destAddr);
    SuccessOrExit(res);

    return SendMessage(destAddr, destPort, interfaceId, payload, msgInfo->Flags);
exit:
    if ((res != CHIP_NO_ERROR) && (payload != NULL) && ((msgInfo->Flags & kChipMessageFlag_RetainBuffer) == 0))
    {
        PacketBuffer::Free(payload);
    }
    return res;
}

/**
 *  Get the number of ChipConnections in use and the size of the pool
 *
 *  @param[out]  aOutInUse  Reference to size_t, in which the number of
 *                         connections in use is stored.
 *
 */
void ChipMessageLayer::GetConnectionPoolStats(chip::System::Stats::count_t & aOutInUse) const
{
    aOutInUse = 0;

    const ChipConnection * con = (ChipConnection *) mConPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount != 0)
        {
            aOutInUse++;
        }
    }
}

/**
 *  Create a new ChipConnection object from a pool.
 *
 *  @return  a pointer to the newly created ChipConnection object if successful, otherwise
 *           NULL.
 *
 */
ChipConnection * ChipMessageLayer::NewConnection()
{
    ChipConnection * con = (ChipConnection *) mConPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount == 0)
        {
            con->Init(this);
            return con;
        }
    }

    ChipLogError(ExchangeManager, "New con FAILED");
    return NULL;
}

void ChipMessageLayer::GetIncomingTCPConCount(const IPAddress & peerAddr, uint16_t & count, uint16_t & countFromIP)
{
    count       = 0;
    countFromIP = 0;

    ChipConnection * con = (ChipConnection *) mConPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount > 0 && con->NetworkType == ChipConnection::kNetworkType_IP && con->IsIncoming())
        {
            count++;
            if (con->PeerAddr == peerAddr)
            {
                countFromIP++;
            }
        }
    }
}

/**
 *  Create a new ChipConnectionTunnel object from a pool.
 *
 *  @return  a pointer to the newly created ChipConnectionTunnel object if successful,
 *           otherwise NULL.
 *
 */
ChipConnectionTunnel * ChipMessageLayer::NewConnectionTunnel()
{
    ChipConnectionTunnel * tun = (ChipConnectionTunnel *) mTunnelPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_TUNNELS; i++, tun++)
    {
        if (tun->IsInUse() == false)
        {
            tun->Init(this);
            return tun;
        }
    }

    ChipLogError(ExchangeManager, "New tun FAILED");
    return NULL;
}

/**
 *  Create a ChipConnectionTunnel by coupling together two specified ChipConnections.
    On successful creation, the TCPEndPoints corresponding to the component ChipConnection
    objects are handed over to the ChipConnectionTunnel, otherwise the ChipConnections are
    closed.
 *
 *  @param[out]    tunPtr                 A pointer to pointer of a ChipConnectionTunnel object.
 *
 *  @param[in]     conOne                 A reference to the first ChipConnection object.
 *
 *  @param[in]     conTwo                 A reference to the second ChipConnection object.
 *
 *  @param[in]     inactivityTimeoutMS    The maximum time in milliseconds that the CHIP
 *                                        connection tunnel could be idle.
 *
 *  @retval    #CHIP_NO_ERROR            on successful creation of the ChipConnectionTunnel.
 *  @retval    #CHIP_ERROR_INCORRECT_STATE if the component ChipConnection objects of the
 *                                          ChipConnectionTunnel is not in the correct state.
 *  @retval    #CHIP_ERROR_NO_MEMORY       if a new ChipConnectionTunnel object cannot be created.
 *
 */
CHIP_ERROR ChipMessageLayer::CreateTunnel(ChipConnectionTunnel ** tunPtr, ChipConnection & conOne, ChipConnection & conTwo,
                                          uint32_t inactivityTimeoutMS)
{
    ChipLogDetail(ExchangeManager, "Entering CreateTunnel");
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(conOne.State == ChipConnection::kState_Connected && conTwo.State == ChipConnection::kState_Connected,
                 err = CHIP_ERROR_INCORRECT_STATE);

    *tunPtr = NewConnectionTunnel();
    VerifyOrExit(*tunPtr != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Form ChipConnectionTunnel from former ChipConnections' TCPEndPoints.
    err = (*tunPtr)->MakeTunnelConnected(conOne.mTcpEndPoint, conTwo.mTcpEndPoint);
    SuccessOrExit(err);

    ChipLogProgress(ExchangeManager, "Created CHIP tunnel from Cons (%04X, %04X) with EPs (%04X, %04X)", conOne.LogId(),
                    conTwo.LogId(), conOne.mTcpEndPoint->LogId(), conTwo.mTcpEndPoint->LogId());

    if (inactivityTimeoutMS > 0)
    {
        // Set TCPEndPoint inactivity timeouts.
        conOne.mTcpEndPoint->SetIdleTimeout(inactivityTimeoutMS);
        conTwo.mTcpEndPoint->SetIdleTimeout(inactivityTimeoutMS);
    }

    // Remove TCPEndPoints from ChipConnections now that we've handed the former to our new ChipConnectionTunnel.
    conOne.mTcpEndPoint = NULL;
    conTwo.mTcpEndPoint = NULL;

exit:
    ChipLogDetail(ExchangeManager, "Exiting CreateTunnel");

    // Close ChipConnection args.
    conOne.Close(true);
    conTwo.Close(true);

    return err;
}

CHIP_ERROR ChipMessageLayer::SetUnsecuredConnectionListener(ConnectionReceiveFunct newOnUnsecuredConnectionReceived,
                                                            CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved,
                                                            bool force, void * listenerState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(ExchangeManager, "Entered SetUnsecuredConnectionReceived, cb = %p, %p", newOnUnsecuredConnectionReceived,
                    newOnUnsecuredConnectionCallbacksRemoved);

    if (UnsecuredListenEnabled() == false)
    {
        err = EnableUnsecuredListen();
        SuccessOrExit(err);
    }

    // New OnUnsecuredConnectionReceived cannot be null. To clear, use ClearOnUnsecuredConnectionReceived().
    VerifyOrExit(newOnUnsecuredConnectionReceived != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (OnUnsecuredConnectionReceived != NULL)
    {
        if (force == false)
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            ExitNow();
        }
        else if (OnUnsecuredConnectionCallbacksRemoved != NULL)
        {
            // Notify application that its previous OnUnsecuredConnectionReceived callback has been removed.
            OnUnsecuredConnectionCallbacksRemoved(UnsecuredConnectionReceivedAppState);
        }
    }

    OnUnsecuredConnectionReceived         = newOnUnsecuredConnectionReceived;
    OnUnsecuredConnectionCallbacksRemoved = newOnUnsecuredConnectionCallbacksRemoved;
    UnsecuredConnectionReceivedAppState   = listenerState;

exit:
    return err;
}

CHIP_ERROR ChipMessageLayer::ClearUnsecuredConnectionListener(ConnectionReceiveFunct oldOnUnsecuredConnectionReceived,
                                                              CallbackRemovedFunct oldOnUnsecuredConnectionCallbacksRemoved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(ExchangeManager, "Entered ClearUnsecuredConnectionListener, cbs = %p, %p", oldOnUnsecuredConnectionReceived,
                    oldOnUnsecuredConnectionCallbacksRemoved);

    // Only clear callbacks and suppress OnUnsecuredConnectionCallbacksRemoved if caller can prove it owns current
    // callbacks. For proof of identification, we accept copies of callback function pointers.
    if (oldOnUnsecuredConnectionReceived != OnUnsecuredConnectionReceived ||
        oldOnUnsecuredConnectionCallbacksRemoved != OnUnsecuredConnectionCallbacksRemoved)
    {
        if (oldOnUnsecuredConnectionReceived != OnUnsecuredConnectionReceived)
            ChipLogError(ExchangeManager, "bad arg: OnUnsecuredConnectionReceived");
        else
            ChipLogError(ExchangeManager, "bad arg: OnUnsecuredConnectionCallbacksRemoved");
        err = CHIP_ERROR_INVALID_ARGUMENT;
        ExitNow();
    }

    if (UnsecuredListenEnabled() == true)
    {
        err = DisableUnsecuredListen();
        SuccessOrExit(err);
    }

    OnUnsecuredConnectionReceived         = NULL;
    OnUnsecuredConnectionCallbacksRemoved = NULL;
    UnsecuredConnectionReceivedAppState   = NULL;

exit:
    return err;
}

CHIP_ERROR ChipMessageLayer::SelectDestNodeIdAndAddress(uint64_t & destNodeId, IPAddress & destAddr)
{
    // If the destination address has not been supplied, attempt to determine it from the node id.
    // Fail if this can't be done.
    if (destAddr == IPAddress::Any)
    {
        destAddr = FabricState->SelectNodeAddress(destNodeId);
        if (destAddr == IPAddress::Any)
            return CHIP_ERROR_INVALID_ADDRESS;
    }

    // If the destination address is a fabric address for the local fabric, and the caller
    // didn't specify the destination node id, extract it from the destination address.
    if (FabricState->IsFabricAddress(destAddr) && destNodeId == kNodeIdNotSpecified)
        destNodeId = IPv6InterfaceIdToChipNodeId(destAddr.InterfaceId());

    return CHIP_NO_ERROR;
}

// Encode and return message header field value.
static uint16_t EncodeHeaderField(const ChipMessageInfo * msgInfo)
{
    return ((((uint16_t) msgInfo->Flags) << kMsgHeaderField_FlagsShift) & kMsgHeaderField_FlagsMask) |
        ((((uint16_t) msgInfo->EncryptionType) << kMsgHeaderField_EncryptionTypeShift) & kMsgHeaderField_EncryptionTypeMask) |
        ((((uint16_t) msgInfo->MessageVersion) << kMsgHeaderField_MessageVersionShift) & kMsgHeaderField_MessageVersionMask);
}

// Decode message header field value.
static void DecodeHeaderField(const uint16_t headerField, ChipMessageInfo * msgInfo)
{
    msgInfo->Flags          = (uint16_t)((headerField & kMsgHeaderField_FlagsMask) >> kMsgHeaderField_FlagsShift);
    msgInfo->EncryptionType = (uint8_t)((headerField & kMsgHeaderField_EncryptionTypeMask) >> kMsgHeaderField_EncryptionTypeShift);
    msgInfo->MessageVersion = (uint8_t)((headerField & kMsgHeaderField_MessageVersionMask) >> kMsgHeaderField_MessageVersionShift);
}

/**
 *  Decode a CHIP Message layer header from a received CHIP message.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the CHIP message.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object which will receive information
 *                              about the message.
 *
 *  @param[out]   payloadStart  A pointer to a pointer to the position in the message buffer after
 *                              decoding is complete.
 *
 *  @retval  #CHIP_NO_ERROR    On successful decoding of the message header.
 *  @retval  #CHIP_ERROR_INVALID_MESSAGE_LENGTH
 *                              If the message buffer passed is of invalid length.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION
 *                              If the CHIP Message header format version is not supported.
 *
 */
CHIP_ERROR ChipMessageLayer::DecodeHeader(PacketBuffer * msgBuf, ChipMessageInfo * msgInfo, uint8_t ** payloadStart)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t * msgStart = msgBuf->Start();
    uint16_t msgLen    = msgBuf->DataLength();
    uint8_t * msgEnd   = msgStart + msgLen;
    uint8_t * p        = msgStart;
    uint16_t headerField;

    if (msgLen < 6)
    {
        ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    }

    // Read and verify the header field.
    headerField = LittleEndian::Read16(p);
    VerifyOrExit((headerField & kMsgHeaderField_ReservedFlagsMask) == 0, err = CHIP_ERROR_INVALID_MESSAGE_FLAG);

    // Decode the header field.
    DecodeHeaderField(headerField, msgInfo);

    // Error if the message version is unsupported.
    if (msgInfo->MessageVersion != kChipMessageVersion_V1 && msgInfo->MessageVersion != kChipMessageVersion_V2)
    {
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION);
    }

    // Decode the message id.
    msgInfo->MessageId = LittleEndian::Read32(p);

    // Decode the source node identifier if included in the message.
    if (msgInfo->Flags & kChipMessageFlag_SourceNodeId)
    {
        if ((p + 8) > msgEnd)
        {
            ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        }
        msgInfo->SourceNodeId = LittleEndian::Read64(p);
    }

    // Decode the destination node identifier if included in the message.
    if (msgInfo->Flags & kChipMessageFlag_DestNodeId)
    {
        if ((p + 8) > msgEnd)
        {
            ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        }
        msgInfo->DestNodeId = LittleEndian::Read64(p);
    }
    else
        // TODO: This is wrong. If not specified in the message, the destination node identifier must be
        // derived from destination IPv6 address to which the message was sent.  This is relatively
        // easy to determine for messages received over TCP (specifically by the inspecting the local
        // address of the connection). However it is much harder for UDP (no support in LwIP; requires
        // use of IP_PKTINFO socket option in sockets). For now we just assume the intended destination
        // is the local node.
        msgInfo->DestNodeId = FabricState->LocalNodeId;
    // Decode the encryption key identifier if present.
    if (msgInfo->EncryptionType != kChipEncryptionType_None)
    {
        if ((p + kKeyIdLen) > msgEnd)
        {
            ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
        }
        msgInfo->KeyId = LittleEndian::Read16(p);
    }
    else
    {
        // Clear flag, which could have been accidentally set in the older version of code only for unencrypted messages.
        msgInfo->Flags &= ~kChipMessageFlag_MsgCounterSyncReq;

        msgInfo->KeyId = ChipKeyId::kNone;
    }

    if (payloadStart != NULL)
    {
        *payloadStart = p;
    }

exit:
    return err;
}

CHIP_ERROR ChipMessageLayer::ReEncodeMessage(PacketBuffer * msgBuf)
{
    ChipMessageInfo msgInfo;
    CHIP_ERROR err;
    uint8_t * p;
    ChipSessionState sessionState;
    uint16_t msgLen    = msgBuf->DataLength();
    uint8_t * msgStart = msgBuf->Start();
    uint16_t encryptionLen;

    msgInfo.Clear();
    msgInfo.SourceNodeId = kNodeIdNotSpecified;

    err = DecodeHeader(msgBuf, &msgInfo, &p);
    if (err != CHIP_NO_ERROR)
        return err;

    encryptionLen = msgLen - (p - msgStart);

    err = FabricState->GetSessionState(msgInfo.SourceNodeId, msgInfo.KeyId, msgInfo.EncryptionType, NULL, sessionState);
    if (err != CHIP_NO_ERROR)
        return err;

    switch (msgInfo.EncryptionType)
    {
    case kChipEncryptionType_None:
        break;

    case kChipEncryptionType_AES128CTRSHA1: {
        // TODO: re-validate MIC to ensure that no part of the message has been altered since the time it was received.

        // Re-encrypt the payload.
        AES128CTRMode aes128CTR;
        aes128CTR.SetKey(sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey);
        aes128CTR.SetChipMessageCounter(msgInfo.SourceNodeId, msgInfo.MessageId);
        aes128CTR.EncryptData(p, encryptionLen, p);
    }
    break;
    default:
        return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // signature remains untouched -- we have not modified it.

    return CHIP_NO_ERROR;
}

/**
 *  Encode a ChipMessageLayer header into an PacketBuffer.
 *
 *  @param[in]    msgInfo       A pointer to a ChipMessageInfo object containing information
 *                              about the message to be encoded.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer object that would hold the CHIP message.
 *
 *  @param[in]    con           A pointer to the ChipConnection object.
 *
 *  @param[in]    maxLen        The maximum length of the encoded CHIP message.
 *
 *  @param[in]    reserve       The reserved space before the payload to hold the CHIP message header.
 *
 *  @retval  #CHIP_NO_ERROR    on successful encoding of the message.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION  if the CHIP Message header format version is
 *                                                     not supported.
 *  @retval  #CHIP_ERROR_INVALID_MESSAGE_LENGTH       if the payload length in the message buffer is zero.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE  if the encryption type in the message header is not
 *                                                     supported.
 *  @retval  #CHIP_ERROR_MESSAGE_TOO_LONG             if the encoded message would be longer than the
 *                                                     requested maximum.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL             if there is not enough space before or after the
 *                                                     message payload.
 *  @retval  other errors generated by the fabric state object when fetching the session state.
 *
 */
CHIP_ERROR ChipMessageLayer::EncodeMessage(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf, ChipConnection * con, uint16_t maxLen,
                                           uint16_t reserve)
{
    CHIP_ERROR err;
    uint8_t * p1;
    // Error if an unsupported message version requested.
    if (msgInfo->MessageVersion != kChipMessageVersion_V1 && msgInfo->MessageVersion != kChipMessageVersion_V2)
        return CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION;

    // Message already encoded, don't do anything
    if (msgInfo->Flags & kChipMessageFlag_MessageEncoded)
    {
        ChipMessageInfo existingMsgInfo;
        existingMsgInfo.Clear();
        err = DecodeHeader(msgBuf, &existingMsgInfo, &p1);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        msgInfo->DestNodeId = existingMsgInfo.DestNodeId;
        return CHIP_NO_ERROR;
    }

    // Compute the number of bytes that will appear before and after the message payload
    // in the final encoded message.
    uint16_t headLen    = 6;
    uint16_t tailLen    = 0;
    uint16_t payloadLen = msgBuf->DataLength();
    if (msgInfo->Flags & kChipMessageFlag_SourceNodeId)
        headLen += 8;
    if (msgInfo->Flags & kChipMessageFlag_DestNodeId)
        headLen += 8;
    switch (msgInfo->EncryptionType)
    {
    case kChipEncryptionType_None:
        break;
    case kChipEncryptionType_AES128CTRSHA1:
        // Can only encrypt non-zero length payloads.
        if (payloadLen == 0)
            return CHIP_ERROR_INVALID_MESSAGE_LENGTH;
        headLen += 2;
        tailLen += HMACSHA1::kDigestLength;
        break;
    default:
        return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // Error if the encoded message would be longer than the requested maximum.
    if ((headLen + msgBuf->DataLength() + tailLen) > maxLen)
        return CHIP_ERROR_MESSAGE_TOO_LONG;

    // Ensure there's enough room before the payload to hold the message header.
    // Return an error if there's not enough room in the buffer.
    if (!msgBuf->EnsureReservedSize(headLen + reserve))
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    // Error if not enough space after the message payload.
    if ((msgBuf->DataLength() + tailLen) > msgBuf->MaxDataLength())
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    uint8_t * payloadStart = msgBuf->Start();

    // Get the session state for the given destination node and encryption key.
    ChipSessionState sessionState;

    if (msgInfo->DestNodeId == kAnyNodeId)
    {
        err = FabricState->GetSessionState(msgInfo->SourceNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    }
    else
    {
        err = FabricState->GetSessionState(msgInfo->DestNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    }
    if (err != CHIP_NO_ERROR)
        return err;

    // Starting encoding at the appropriate point in the buffer before the payload data.
    uint8_t * p = payloadStart - headLen;

    // Allocate a new message identifier and write the message identifier field.
    if ((msgInfo->Flags & kChipMessageFlag_ReuseMessageId) == 0)
        msgInfo->MessageId = sessionState.NewMessageId();

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    // Request message counter synchronization if peer group key counter is not synchronized.
    if (sessionState.MessageIdNotSynchronized() && ChipKeyId::IsAppGroupKey(msgInfo->KeyId))
    {
        // Set the flag.
        msgInfo->Flags |= kChipMessageFlag_MsgCounterSyncReq;

        // Update fabric state.
        FabricState->OnMsgCounterSyncReqSent(msgInfo->MessageId);
    }
#endif

    // Adjust the buffer so that the start points to the start of the encoded message.
    msgBuf->SetStart(p);

    // Encode and verify the header field.
    uint16_t headerField = EncodeHeaderField(msgInfo);
    if ((headerField & kMsgHeaderField_ReservedFlagsMask) != 0)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Write the header field.
    LittleEndian::Write16(p, headerField);

    if (msgInfo->DestNodeId == kAnyNodeId)
    {
        sessionState.IsDuplicateMessage(msgInfo->MessageId);
    }

    LittleEndian::Write32(p, msgInfo->MessageId);

    // If specified, encode the source node id.
    if (msgInfo->Flags & kChipMessageFlag_SourceNodeId)
    {
        LittleEndian::Write64(p, msgInfo->SourceNodeId);
    }

    // If specified, encode the destination node id.
    if (msgInfo->Flags & kChipMessageFlag_DestNodeId)
    {
        LittleEndian::Write64(p, msgInfo->DestNodeId);
    }

    switch (msgInfo->EncryptionType)
    {
    case kChipEncryptionType_None:
        // If no encryption requested, skip over the payload in the message buffer.
        p += payloadLen;
        break;

    case kChipEncryptionType_AES128CTRSHA1:
        // Encode the key id.
        LittleEndian::Write16(p, msgInfo->KeyId);

        // At this point we've completed encoding the head of the message (and therefore p == payloadStart),
        // so skip over the payload data.
        p += payloadLen;

        // Compute the integrity check value and store it immediately after the payload data.
        ComputeIntegrityCheck_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.IntegrityKey, payloadStart,
                                            payloadLen, p);
        p += HMACSHA1::kDigestLength;

        // Encrypt the message payload and the integrity check value that follows it, in place, in the message buffer.
        Encrypt_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey, payloadStart,
                              payloadLen + HMACSHA1::kDigestLength, payloadStart);

        break;
    }

    msgInfo->Flags |= kChipMessageFlag_MessageEncoded;
    // Update the buffer length to reflect the entire encoded message.
    msgBuf->SetDataLength(headLen + payloadLen + tailLen);

    // We update the cursor (p) out of good hygiene,
    // such that if the code is extended in the future such that the cursor is used,
    // it will be in the correct position for such code.
    IgnoreUnusedVariable(p);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMessageLayer::DecodeMessage(PacketBuffer * msgBuf, uint64_t sourceNodeId, ChipConnection * con,
                                           ChipMessageInfo * msgInfo, uint8_t ** rPayload,
                                           uint16_t * rPayloadLen) // TODO: use references
{
    CHIP_ERROR err;
    uint8_t * msgStart    = msgBuf->Start();
    uint16_t msgLen       = msgBuf->DataLength();
    uint8_t * msgEnd      = msgStart + msgLen;
    uint8_t * p           = msgStart;
    msgInfo->SourceNodeId = sourceNodeId;
    err                   = DecodeHeader(msgBuf, msgInfo, &p);
    sourceNodeId          = msgInfo->SourceNodeId;

    if (err != CHIP_NO_ERROR)
        return err;

    // Get the session state for the given source node and encryption key.
    ChipSessionState sessionState;

    err = FabricState->GetSessionState(sourceNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    if (err != CHIP_NO_ERROR)
        return err;

    switch (msgInfo->EncryptionType)
    {
    case kChipEncryptionType_None:
        // Return the position and length of the payload within the message.
        *rPayloadLen = msgLen - (p - msgStart);
        *rPayload    = p;

        // Skip over the payload.
        p += *rPayloadLen;
        break;

    case kChipEncryptionType_AES128CTRSHA1: {
        // Error if the message is short given the expected fields.
        if ((p + kMinPayloadLen + HMACSHA1::kDigestLength) > msgEnd)
            return CHIP_ERROR_INVALID_MESSAGE_LENGTH;

        // Return the position and length of the payload within the message.
        uint16_t payloadLen = msgLen - ((p - msgStart) + HMACSHA1::kDigestLength);
        *rPayloadLen        = payloadLen;
        *rPayload           = p;

        // Decrypt the message payload and the integrity check value that follows it, in place, in the message buffer.
        Encrypt_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey, p,
                              payloadLen + HMACSHA1::kDigestLength, p);

        // Compute the expected integrity check value from the decrypted payload.
        uint8_t expectedIntegrityCheck[HMACSHA1::kDigestLength];
        ComputeIntegrityCheck_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.IntegrityKey, p, payloadLen,
                                            expectedIntegrityCheck);
        // Error if the expected integrity check doesn't match the integrity check in the message.
        if (!ConstantTimeCompare(p + payloadLen, expectedIntegrityCheck, HMACSHA1::kDigestLength))
            return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
        // Skip past the payload and the integrity check value.
        p += payloadLen + HMACSHA1::kDigestLength;

        break;
    }

    default:
        return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // Set flag in the message header indicating that the message is a duplicate if:
    //  - A message with the same message identifier has already been received from that peer.
    //  - This is the first message from that peer encrypted with application keys.
    if (sessionState.IsDuplicateMessage(msgInfo->MessageId))
        msgInfo->Flags |= kChipMessageFlag_DuplicateMessage;

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    // Set flag if peer group key message counter is not synchronized.
    if (sessionState.MessageIdNotSynchronized() && ChipKeyId::IsAppGroupKey(msgInfo->KeyId))
        msgInfo->Flags |= kChipMessageFlag_PeerGroupMsgIdNotSynchronized;
#endif

    // Pass the peer authentication mode back to the application via the CHIP message header structure.
    msgInfo->PeerAuthMode = sessionState.AuthMode;

    return err;
}

CHIP_ERROR ChipMessageLayer::EncodeMessageWithLength(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf, ChipConnection * con,
                                                     uint16_t maxLen)
{
    // Encode the message, reserving 2 bytes for the length.
    CHIP_ERROR err = EncodeMessage(msgInfo, msgBuf, con, maxLen - 2, 2);
    if (err != CHIP_NO_ERROR)
        return err;

    // Prepend the message length to the beginning of the message.
    uint8_t * newMsgStart = msgBuf->Start() - 2;
    uint16_t msgLen       = msgBuf->DataLength();
    msgBuf->SetStart(newMsgStart);
    LittleEndian::Put16(newMsgStart, msgLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMessageLayer::DecodeMessageWithLength(PacketBuffer * msgBuf, uint64_t sourceNodeId, ChipConnection * con,
                                                     ChipMessageInfo * msgInfo, uint8_t ** rPayload, uint16_t * rPayloadLen,
                                                     uint32_t * rFrameLen)
{
    uint8_t * dataStart = msgBuf->Start();
    uint16_t dataLen    = msgBuf->DataLength();

    // Error if the message buffer doesn't contain the entire message length field.
    if (dataLen < 2)
    {
        *rFrameLen = 8; // Assume absolute minimum frame length.
        return CHIP_ERROR_MESSAGE_INCOMPLETE;
    }

    // Read the message length.
    uint16_t msgLen = LittleEndian::Get16(dataStart);

    // The frame length is the length of the message plus the length of the length field.
    *rFrameLen = static_cast<uint32_t>(msgLen) + 2;

    // Error if the message buffer doesn't contain the entire message, or is too
    // long to ever fit in the buffer.
    if (dataLen < *rFrameLen)
    {
        if (*rFrameLen > msgBuf->MaxDataLength() + msgBuf->ReservedSize())
            return CHIP_ERROR_MESSAGE_TOO_LONG;
        return CHIP_ERROR_MESSAGE_INCOMPLETE;
    }

    // Adjust the message buffer to point at the message, not including the message length field that precedes it,
    // and not including any data that may follow it.
    msgBuf->SetStart(dataStart + 2);
    msgBuf->SetDataLength(msgLen);

    // Decode the message.
    CHIP_ERROR err = DecodeMessage(msgBuf, sourceNodeId, con, msgInfo, rPayload, rPayloadLen);

    // If successful, adjust the message buffer to point at any remaining data beyond the end of the message.
    // (This may in fact represent another message).
    if (err == CHIP_NO_ERROR)
    {
        msgBuf->SetStart(dataStart + msgLen + 2);
        msgBuf->SetDataLength(dataLen - (msgLen + 2));
    }

    // Otherwise, reset the buffer to its original position/length.
    else
    {
        msgBuf->SetStart(dataStart);
        msgBuf->SetDataLength(dataLen);
    }

    return err;
}

void ChipMessageLayer::HandleUDPMessage(UDPEndPoint * endPoint, PacketBuffer * msg, const IPPacketInfo * pktInfo)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    ChipMessageLayer * msgLayer = (ChipMessageLayer *) endPoint->AppState;
    ChipMessageInfo msgInfo;
    uint64_t sourceNodeId;
    uint8_t * payload;
    uint16_t payloadLen;

    CHIP_FAULT_INJECT(FaultInjection::kFault_DropIncomingUDPMsg, PacketBuffer::Free(msg); ExitNow(err = CHIP_NO_ERROR));

    msgInfo.Clear();
    msgInfo.InPacketInfo = pktInfo;

    // If the message was sent to an IPv6 multicast address, verify that the sending address matches
    // one of the prefixes assigned to a local interface.  If not, ignore the message and report a
    // receive error to the application.
    //
    // Because the message was multicast, we will receive it regardless of what the sender's address is.
    // However, if we don't have a local address in the same prefix, it won't be possible for us to
    // respond. Furthermore, if we accept the message and then the sender retransmits it using a source
    // prefix that DOES match one of our address, the latter message will be discarded as a duplicate,
    // because we already accepted it when it was sent from the original address.
    //
    if (pktInfo->DestAddress.IsMulticast() && !msgLayer->Inet->MatchLocalIPv6Subnet(pktInfo->SrcAddress))
        err = CHIP_ERROR_INVALID_ADDRESS;

    if (err == CHIP_NO_ERROR)
    {
        // If the source address is a ULA, derive a node identifier from it.  Depending on what's in the
        // message header, this may in fact be the node identifier of the sending node.
        sourceNodeId = (pktInfo->SrcAddress.IsIPv6ULA()) ? IPv6InterfaceIdToChipNodeId(pktInfo->SrcAddress.InterfaceId())
                                                         : kNodeIdNotSpecified;

        // Attempt to decode the message.
        err = msgLayer->DecodeMessage(msg, sourceNodeId, NULL, &msgInfo, &payload, &payloadLen);

        if (err == CHIP_NO_ERROR)
        {
            // Set the message buffer to point at the payload data.
            msg->SetStart(payload);
            msg->SetDataLength(payloadLen);
        }
    }

    // Verify that destination node identifier refers to the local node.
    if (err == CHIP_NO_ERROR)
    {
        if (msgInfo.DestNodeId != msgLayer->FabricState->LocalNodeId && msgInfo.DestNodeId != kAnyNodeId)
            err = CHIP_ERROR_INVALID_DESTINATION_NODE_ID;
    }

    // If an error occurred, discard the message and call the on receive error handler.
    SuccessOrExit(err);

    // Record whether the message was sent to the local node's ephemeral port.
#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    SetFlag(msgInfo.Flags, kChipMessageFlag_ViaEphemeralUDPPort,
            (endPoint == msgLayer->mIPv6EphemeralUDP
#if INET_CONFIG_ENABLE_IPV4
             || endPoint == msgLayer->mIPv4EphemeralUDP
#endif // INET_CONFIG_ENABLE_IPV4
             ));
#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    // Check if message carries tunneled data and needs to be sent to Tunnel Agent
    if (msgInfo.MessageVersion == kChipMessageVersion_V2)
    {
        if (msgInfo.Flags & kChipMessageFlag_TunneledData)
        {
#if CHIP_CONFIG_ENABLE_TUNNELING
            // Policy for handling duplicate tunneled UDP message:
            //  - Eliminate duplicate tunneled encrypted messages to prevent replay of messages by
            //    a malicious man-in-the-middle.
            //  - Handle duplicate tunneled unencrypted message.
            // Dispatch the tunneled data message to the application if it is not a duplicate or unencrypted.
            if (!(msgInfo.Flags & kChipMessageFlag_DuplicateMessage) || msgInfo.KeyId == ChipKeyId::kNone)
            {
                if (msgLayer->OnUDPTunneledMessageReceived)
                {
                    msgLayer->OnUDPTunneledMessageReceived(msgLayer, msg);
                }
                else
                {
                    ExitNow(err = CHIP_ERROR_NO_MESSAGE_HANDLER);
                }
            }
#endif
        }
        else
        {
            // Call the supplied OnMessageReceived callback.
            if (msgLayer->OnMessageReceived != NULL)
            {
                msgLayer->OnMessageReceived(msgLayer, &msgInfo, msg);
            }
            else
            {
                ExitNow(err = CHIP_ERROR_NO_MESSAGE_HANDLER);
            }
        }
    }
    else if (msgInfo.MessageVersion == kChipMessageVersion_V1)
    {
        // Call the supplied OnMessageReceived callback.
        if (msgLayer->OnMessageReceived != NULL)
            msgLayer->OnMessageReceived(msgLayer, &msgInfo, msg);
        else
        {
            ExitNow(err = CHIP_ERROR_NO_MESSAGE_HANDLER);
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(MessageLayer, "HandleUDPMessage Error %s", ErrorStr(err));

        PacketBuffer::Free(msg);

        // Send key error response to the peer if required.
        // Key error response is sent only if the received message is not a multicast.
        if (!pktInfo->DestAddress.IsMulticast() && msgLayer->SecurityMgr->IsKeyError(err))
            msgLayer->SecurityMgr->SendKeyErrorMsg(&msgInfo, pktInfo, NULL, err);

        if (msgLayer->OnReceiveError != NULL)
            msgLayer->OnReceiveError(msgLayer, err, pktInfo);
    }

    return;
}

void ChipMessageLayer::HandleUDPReceiveError(UDPEndPoint * endPoint, INET_ERROR err, const IPPacketInfo * pktInfo)
{
    ChipLogError(MessageLayer, "HandleUDPReceiveError Error %s", ErrorStr(err));

    ChipMessageLayer * msgLayer = (ChipMessageLayer *) endPoint->AppState;
    if (msgLayer->OnReceiveError != NULL)
        msgLayer->OnReceiveError(msgLayer, err, pktInfo);
}

#if CONFIG_NETWORK_LAYER_BLE
void ChipMessageLayer::HandleIncomingBleConnection(BLEEndPoint * bleEP)
{
    ChipMessageLayer * msgLayer = (ChipMessageLayer *) bleEP->mAppState;

    // Immediately close the connection if there's no callback registered.
    if (msgLayer->OnConnectionReceived == NULL && msgLayer->ExchangeMgr == NULL)
    {
        bleEP->Close();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, CHIP_ERROR_NO_CONNECTION_HANDLER);
        return;
    }

    // Attempt to allocate a connection object. Fail if too many connections.
    ChipConnection * con = msgLayer->NewConnection();
    if (con == NULL)
    {
        bleEP->Close();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, CHIP_ERROR_TOO_MANY_CONNECTIONS);
        return;
    }

    // Setup the connection object.
    con->MakeConnectedBle(bleEP);

#if CHIP_PROGRESS_LOGGING
    {
        ChipLogProgress(MessageLayer, "WoBle con rcvd");
    }
#endif

    // Set the default idle timeout.
    con->SetIdleTimeout(msgLayer->IncomingConIdleTimeout);

    // Set incoming connection flag.
    con->SetIncoming(true);

    // If the exchange manager has been initialized, call its callback.
    if (msgLayer->ExchangeMgr != NULL)
        msgLayer->ExchangeMgr->HandleConnectionReceived(con);

    // Call the app's OnConnectionReceived callback.
    if (msgLayer->OnConnectionReceived != NULL)
        msgLayer->OnConnectionReceived(msgLayer, con);
}
#endif /* CONFIG_NETWORK_LAYER_BLE */

void ChipMessageLayer::HandleIncomingTcpConnection(TCPEndPoint * listeningEP, TCPEndPoint * conEP, const IPAddress & peerAddr,
                                                   uint16_t peerPort)
{
    INET_ERROR err;
    IPAddress localAddr;
    uint16_t localPort;
    uint16_t incomingTCPConCount;
    uint16_t incomingTCPConCountFromIP;
    ChipMessageLayer * msgLayer = (ChipMessageLayer *) listeningEP->AppState;

    // Immediately close the connection if there's no callback registered.
    if (msgLayer->OnConnectionReceived == NULL && msgLayer->ExchangeMgr == NULL)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, CHIP_ERROR_NO_CONNECTION_HANDLER);
        return;
    }

    // Fail if too many incoming TCP connections.
    msgLayer->GetIncomingTCPConCount(peerAddr, incomingTCPConCount, incomingTCPConCountFromIP);
    if (incomingTCPConCount == CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS ||
        incomingTCPConCountFromIP == CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, CHIP_ERROR_TOO_MANY_CONNECTIONS);
        return;
    }

    // Attempt to allocate a connection object. Fail if too many connections.
    ChipConnection * con = msgLayer->NewConnection();
    if (con == NULL)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, CHIP_ERROR_TOO_MANY_CONNECTIONS);
        return;
    }

    // Get the local address that was used for the connection.
    err = conEP->GetLocalInfo(&localAddr, &localPort);
    if (err != INET_NO_ERROR)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, err);
        return;
    }

    // Setup the connection object.
    con->MakeConnectedTcp(conEP, localAddr, peerAddr);

#if CHIP_PROGRESS_LOGGING
    {
        char ipAddrStr[64];
        peerAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
        ChipLogProgress(MessageLayer, "Con %s %04" PRIX16 " %s %d", "rcvd", con->LogId(), ipAddrStr, (int) peerPort);
    }
#endif

    // Set the default idle timeout.
    con->SetIdleTimeout(msgLayer->IncomingConIdleTimeout);

    // Set incoming connection flag.
    con->SetIncoming(true);

    // If the exchange manager has been initialized, call its callback.
    if (msgLayer->ExchangeMgr != NULL)
        msgLayer->ExchangeMgr->HandleConnectionReceived(con);

    // Call the app's OnConnectionReceived callback.
    if (msgLayer->OnConnectionReceived != NULL)
        msgLayer->OnConnectionReceived(msgLayer, con);

    // If connection was received on unsecured port, call the app's OnUnsecuredConnectionReceived callback.
    if (msgLayer->OnUnsecuredConnectionReceived != NULL && conEP->GetLocalInfo(&localAddr, &localPort) == CHIP_NO_ERROR &&
        localPort == CHIP_UNSECURED_PORT)
        msgLayer->OnUnsecuredConnectionReceived(msgLayer, con);
}

void ChipMessageLayer::HandleAcceptError(TCPEndPoint * ep, INET_ERROR err)
{
    ChipMessageLayer * msgLayer = (ChipMessageLayer *) ep->AppState;
    if (msgLayer->OnAcceptError != NULL)
        msgLayer->OnAcceptError(msgLayer, err);
}

/**
 *  Refresh the InetLayer endpoints based on the current state of the system's network interfaces.
 *
 *  @note
 *     This function is designed to be called multiple times. The first call will setup all the
 *     TCP / UDP endpoints needed for the messaging layer to communicate, based on the specified
 *     configuration (i.e. IPv4 listen enabled, IPv6 listen enabled, etc.). Subsequent calls will
 *     re-initialize the active endpoints based on the current state of the system's network
 *     interfaces.
 *
 *  @retval #CHIP_NO_ERROR on successful refreshing of endpoints.
 *  @retval InetLayer errors based on calls to create TCP/UDP endpoints.
 *
 */
CHIP_ERROR ChipMessageLayer::RefreshEndpoints()
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    const bool listenIPv6 = IPv6ListenEnabled();
#if INET_CONFIG_ENABLE_IPV4
    const bool listenIPv4 = IPv4ListenEnabled();
#endif // INET_CONFIG_ENABLE_IPV4

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    IPAddress & listenIPv6Addr = FabricState->ListenIPv6Addr;
    InterfaceId listenIPv6Intf = INET_NULL_INTERFACEID;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress & listenIPv4Addr = FabricState->ListenIPv4Addr;
#endif // INET_CONFIG_ENABLE_IPV4

    // If configured to use a specific IPv6 address, determine the interface associated
    // with that address.  Store it as the only interface in the interface list.
    if (IsBoundToLocalIPv6Address())
    {
        err = Inet->GetInterfaceFromAddr(listenIPv6Addr, listenIPv6Intf);
        SuccessOrExit(err);
    }

#else // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    IPAddress & listenIPv6Addr = IPAddress::Any;
    InterfaceId listenIPv6Intf = INET_NULL_INTERFACEID;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress & listenIPv4Addr = IPAddress::Any;
#endif // INET_CONFIG_ENABLE_IPV4

#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    // ================================================================================
    // Enable / disable TCP listening endpoints...
    // ================================================================================

    {
        const bool listenTCP     = TCPListenEnabled();
        const bool listenIPv6TCP = (listenTCP && listenIPv6);

#if INET_CONFIG_ENABLE_IPV4

        const bool listenIPv4TCP = (listenTCP && listenIPv4);

        // Enable / disable the CHIP IPv4 TCP listening endpoint
        //
        // The CHIP IPv4 TCP listening endpoint is use to listen for incoming IPv4 TCP connections
        // to the local node's CHIP port.
        //
        err =
            RefreshEndpoint(mIPv4TCPListen, listenIPv4TCP, "CHIP IPv4 TCP listen", kIPAddressType_IPv4, listenIPv4Addr, CHIP_PORT);
        SuccessOrExit(err);

#endif // INET_CONFIG_ENABLE_IPV4

        // Enable / disable the CHIP IPv6 TCP listening endpoint
        //
        // The CHIP IPv6 TCP listening endpoint is use to listen for incoming IPv6 TCP connections
        // to the local node's CHIP port.
        //
        err =
            RefreshEndpoint(mIPv6TCPListen, listenIPv6TCP, "CHIP IPv6 TCP listen", kIPAddressType_IPv6, listenIPv6Addr, CHIP_PORT);
        SuccessOrExit(err);

#if CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

        // Enable / disable the Unsecured IPv6 TCP listening endpoint
        //
        // The Unsecured IPv6 TCP listening endpoint is use to listen for incoming IPv6 TCP connections
        // to the local node's unsecured CHIP port.  This endpoint is only enabled if the unsecured TCP
        // listen feature has been enabled.
        //
        const bool listenUnsecuredIPv6TCP = (listenIPv6TCP && UnsecuredListenEnabled());
        err = RefreshEndpoint(mUnsecuredIPv6TCPListen, listenUnsecuredIPv6TCP, "unsecured IPv6 TCP listen", kIPAddressType_IPv6,
                              listenIPv6Addr, CHIP_UNSECURED_PORT);
        SuccessOrExit(err);

#endif // CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    }

    // ================================================================================
    // Enable / disable UDP endpoints...
    // ================================================================================

    {
        const bool listenUDP = UDPListenEnabled();

#if INET_CONFIG_ENABLE_IPV4

        // Enabled / disable the CHIP IPv4 UDP endpoint as necessary.
        //
        // The CHIP IPv4 UDP endpoint is used to listen for unsolicited IPv4 UDP CHIP messages sent
        // to the local node's CHIP port.  Is is also used by the local node to send IPv4 UDP CHIP
        // messages to other nodes, and to receive their replies, unless the outbound ephemeral UDP port
        // feature has been enabled.
        //
        const bool listenIPv4UDP = (listenIPv4 && listenUDP);
        err = RefreshEndpoint(mIPv4UDP, listenIPv4UDP, "CHIP IPv4 UDP", kIPAddressType_IPv4, listenIPv4Addr, CHIP_PORT,
                              INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

        // Enabled / disable the ephemeral IPv4 UDP endpoint as necessary.
        //
        // The ephemeral IPv4 UDP endpoint is used to send IPv4 UDP CHIP messages to other nodes and to
        // receive their replies.  It is only enabled when the outbound ephemeral UDP port feature has been
        // enabled.
        //
        const bool listenIPv4EphemeralUDP = (listenIPv4UDP && EphemeralUDPPortEnabled());
        err = RefreshEndpoint(mIPv4EphemeralUDP, listenIPv4EphemeralUDP, "ephemeral IPv4 UDP", kIPAddressType_IPv4, listenIPv4Addr,
                              0, INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#endif // INET_CONFIG_ENABLE_IPV4

        // Enabled / disable the CHIP IPv6 UDP endpoint as necessary.
        //
        // The CHIP IPv6 UDP endpoint is used to listen for unsolicited IPv6 UDP CHIP messages sent
        // to the local node's CHIP port.  Is is also used by the local node to send IPv6 UDP CHIP
        // messages to other nodes, and to receive their replies, unless the outbound ephemeral UDP port
        // feature has been enabled.
        //
        const bool listenIPv6UDP = (listenIPv6 && listenUDP);
        err = RefreshEndpoint(mIPv6UDP, listenIPv6UDP, "CHIP IPv6 UDP", kIPAddressType_IPv6, listenIPv6Addr, CHIP_PORT,
                              listenIPv6Intf);
        SuccessOrExit(err);

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

        // Enabled / disable the ephemeral IPv6 UDP endpoint as necessary.
        //
        // The ephemeral IPv6 UDP endpoint is used to send IPv6 UDP CHIP messages to other nodes and to
        // receive their replies.  It is only enabled when the outbound ephemeral UDP port feature has been
        // enabled.
        //
        const bool listenIPv6EphemeralUDP = (listenIPv6UDP && EphemeralUDPPortEnabled());
        err = RefreshEndpoint(mIPv6EphemeralUDP, listenIPv6EphemeralUDP, "ephemeral IPv6 UDP", kIPAddressType_IPv6, listenIPv6Addr,
                              0, listenIPv6Intf);
        SuccessOrExit(err);

#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN

        // Enable / disable the CHIP IPv6 UDP multicast endpoint.
        //
        // The CHIP IPv6 UDP multicast endpoint is used to listen for unsolicited IPv6 UDP CHIP messages sent
        // to the all-nodes, link-local multicast address. It is only enabled when the message layer has been bound
        // to a specific IPv6 address.  This is required because the CHIP IPv6 UDP endpoint will not receive multicast
        // messages in this configuration.
        //
        IPAddress ipv6LinkLocalAllNodes =
            IPAddress::MakeIPv6WellKnownMulticast(kIPv6MulticastScope_Link, kIPV6MulticastGroup_AllNodes);
        const bool listenChipIPv6UDPMulticastEP = (listenIPv6UDP && IsBoundToLocalIPv6Address());
        err = RefreshEndpoint(mIPv6UDPMulticastRcv, listenChipIPv6UDPMulticastEP, "CHIP IPv6 UDP multicast", kIPAddressType_IPv6,
                              ipv6LinkLocalAllNodes, CHIP_PORT, listenIPv6Intf);
        SuccessOrExit(err);

#if INET_CONFIG_ENABLE_IPV4

        // Enable / disable the CHIP IPv4 UDP broadcast endpoint.
        //
        // Similar to the IPv6 UDP multicast endpoint, this endpoint is used to receive IPv4 broadcast messages
        // when the message layer has been bound to a specific IPv4 address.
        //
        IPAddress ipv4Broadcast                 = IPAddress::MakeIPv4Broadcast();
        const bool listenChipIPv4UDPBroadcastEP = (listenIPv4UDP && IsBoundToLocalIPv4Address());
        err = RefreshEndpoint(mIPv4UDPBroadcastRcv, listenChipIPv4UDPBroadcastEP, "CHIP IPv4 UDP broadcast", kIPAddressType_IPv4,
                              ipv4Broadcast, CHIP_PORT, INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#endif // INET_CONFIG_ENABLE_IPV4

#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    }

exit:
    if (err != CHIP_NO_ERROR)
        ChipBindLog("RefreshEndpoints failed: %s", ErrorStr(err));
    return err;
}

CHIP_ERROR ChipMessageLayer::RefreshEndpoint(TCPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType,
                                             IPAddress addr, uint16_t port)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Release any existing endpoint as needed.
    if (endPoint != NULL && !enable)
    {
        endPoint->Free();
        endPoint = NULL;
    }

    // If needed, create and bind a new endpoint...
    if (endPoint == NULL && enable)
    {
        // Create a new TCP endpoint object.
        err = Inet->NewTCPEndPoint(&endPoint);
        SuccessOrExit(err);

        // Bind the endpoint to the given address, port and interface.
        err = endPoint->Bind(addrType, addr, port, true);
        SuccessOrExit(err);

        // Accept incoming TCP connections.
        endPoint->AppState             = this;
        endPoint->OnConnectionReceived = HandleIncomingTcpConnection;
        endPoint->OnAcceptError        = HandleAcceptError;
        err                            = endPoint->Listen(1);
        SuccessOrExit(err);

#if CHIP_BIND_DETAIL_LOGGING && CHIP_DETAIL_LOGGING
        {
            char ipAddrStr[64];
            addr.ToString(ipAddrStr, sizeof(ipAddrStr));
            ChipBindLog("Listening on %s endpoint ([%s]:%" PRIu16 ")", name, ipAddrStr, port);
        }
#endif // CHIP_BIND_DETAIL_LOGGING && CHIP_DETAIL_LOGGING
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (endPoint != NULL)
        {
            endPoint->Free();
            endPoint = NULL;
        }
        ChipLogError(MessageLayer, "Error initializing %s endpoint: %s", name, ErrorStr(err));
    }
    return err;
}

CHIP_ERROR ChipMessageLayer::RefreshEndpoint(UDPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType,
                                             IPAddress addr, uint16_t port, InterfaceId intfId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Release any existing endpoint as needed.
    if (endPoint != NULL && (!enable || GetFlag(mFlags, kFlag_ForceRefreshUDPEndPoints)))
    {
        endPoint->Free();
        endPoint = NULL;
    }

    // If needed, create and bind a new endpoint...
    if (endPoint == NULL && enable)
    {
        // Create a new UDP endpoint object.
        err = Inet->NewUDPEndPoint(&endPoint);
        SuccessOrExit(err);

        // Bind the endpoint to the given address, port and interface.
        err = endPoint->Bind(addrType, addr, port, intfId);
        SuccessOrExit(err);

        // Accept incoming packets on the endpoint.
        endPoint->AppState          = this;
        endPoint->OnMessageReceived = reinterpret_cast<IPEndPointBasis::OnMessageReceivedFunct>(HandleUDPMessage);
        endPoint->OnReceiveError    = reinterpret_cast<IPEndPointBasis::OnReceiveErrorFunct>(HandleUDPReceiveError);
        err                         = endPoint->Listen();
        SuccessOrExit(err);

#if CHIP_BIND_DETAIL_LOGGING && CHIP_DETAIL_LOGGING
        {
            char ipAddrStr[64];
            char intfStr[64];
            addr.ToString(ipAddrStr, sizeof(ipAddrStr));
            if (intfId != INET_NULL_INTERFACEID)
            {
                intfStr[0] = '%';
                GetInterfaceName(intfId, intfStr + 1, sizeof(intfStr) - 1);
            }
            else
            {
                intfStr[0] = '\0';
            }
            ChipBindLog("Listening on %s endpoint ([%s]:%" PRIu16 "%s)", name, ipAddrStr, endPoint->GetBoundPort(), intfStr);
        }
#endif // CHIP_BIND_DETAIL_LOGGING && CHIP_DETAIL_LOGGING
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (endPoint != NULL)
        {
            endPoint->Free();
            endPoint = NULL;
        }
        ChipLogError(MessageLayer, "Error initializing %s endpoint: %s", name, ErrorStr(err));
    }
    return err;
}

void ChipMessageLayer::Encrypt_AES128CTRSHA1(const ChipMessageInfo * msgInfo, const uint8_t * key, const uint8_t * inData,
                                             uint16_t inLen, uint8_t * outBuf)
{
    AES128CTRMode aes128CTR;
    aes128CTR.SetKey(key);
    aes128CTR.SetChipMessageCounter(msgInfo->SourceNodeId, msgInfo->MessageId);
    aes128CTR.EncryptData(inData, inLen, outBuf);
}

void ChipMessageLayer::ComputeIntegrityCheck_AES128CTRSHA1(const ChipMessageInfo * msgInfo, const uint8_t * key,
                                                           const uint8_t * inData, uint16_t inLen, uint8_t * outBuf)
{
    HMACSHA1 hmacSHA1;
    uint8_t encodedBuf[2 * sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint32_t)];
    uint8_t * p = encodedBuf;

    // Initialize HMAC Key.
    hmacSHA1.Begin(key, ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);

    // Encode the source and destination node identifiers in a little-endian format.
    Encoding::LittleEndian::Write64(p, msgInfo->SourceNodeId);
    Encoding::LittleEndian::Write64(p, msgInfo->DestNodeId);

    // Hash the message header field and the message Id for the message version V2.
    if (msgInfo->MessageVersion == kChipMessageVersion_V2)
    {
        // Encode message header field value.
        uint16_t headerField = EncodeHeaderField(msgInfo);

        // Mask destination and source node Id flags.
        headerField &= kMsgHeaderField_MessageHMACMask;

        // Encode the message header field and the message Id in a little-endian format.
        Encoding::LittleEndian::Write16(p, headerField);
        Encoding::LittleEndian::Write32(p, msgInfo->MessageId);
    }

    // Hash encoded message header fields.
    hmacSHA1.AddData(encodedBuf, p - encodedBuf);

    // Handle payload data.
    hmacSHA1.AddData(inData, inLen);

    // Generate the MAC.
    hmacSHA1.Finish(outBuf);
}

/**
 *  Close all open TCP and UDP endpoints. Then abort any
 *  open ChipConnections and shutdown any open
 *  ChipConnectionTunnel objects.
 *
 *  @note
 *    A call to CloseEndpoints() terminates all communication
 *    channels within the ChipMessageLayer but does not terminate
 *    the ChipMessageLayer object.
 *
 *  @sa Shutdown().
 *
 */
CHIP_ERROR ChipMessageLayer::CloseEndpoints()
{
    // Close all endpoints used for listening.
    CloseListeningEndpoints();

    // Abort any open connections.
    ChipConnection * con = static_cast<ChipConnection *>(mConPool);
    for (int i = 0; i < CHIP_CONFIG_MAX_CONNECTIONS; i++, con++)
        if (con->mRefCount > 0)
            con->Abort();

    // Shut down any open tunnels.
    ChipConnectionTunnel * tun = static_cast<ChipConnectionTunnel *>(mTunnelPool);
    for (int i = 0; i < CHIP_CONFIG_MAX_TUNNELS; i++, tun++)
    {
        if (tun->mMessageLayer != NULL)
        {
            // Suppress callback as we're shutting down the whole stack.
            tun->OnShutdown = NULL;
            tun->Shutdown();
        }
    }

    return CHIP_NO_ERROR;
}

void ChipMessageLayer::CloseListeningEndpoints(void)
{
    ChipBindLog("Closing endpoints");

    if (mIPv6TCPListen != NULL)
    {
        mIPv6TCPListen->Free();
        mIPv6TCPListen = NULL;
    }

    if (mIPv6UDP != NULL)
    {
        mIPv6UDP->Free();
        mIPv6UDP = NULL;
    }

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    if (mIPv6EphemeralUDP != NULL)
    {
        mIPv6EphemeralUDP->Free();
        mIPv6EphemeralUDP = NULL;
    }

#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    if (mIPv6UDPMulticastRcv != NULL)
    {
        mIPv6UDPMulticastRcv->Free();
        mIPv6UDPMulticastRcv = NULL;
    }

#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

#if CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

    if (mUnsecuredIPv6TCPListen != NULL)
    {
        mUnsecuredIPv6TCPListen->Free();
        mUnsecuredIPv6TCPListen = NULL;
    }

#endif // CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

#if INET_CONFIG_ENABLE_IPV4

    if (mIPv4TCPListen != NULL)
    {
        mIPv4TCPListen->Free();
        mIPv4TCPListen = NULL;
    }

    if (mIPv4UDP != NULL)
    {
        mIPv4UDP->Free();
        mIPv4UDP = NULL;
    }

#if CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    if (mIPv4EphemeralUDP != NULL)
    {
        mIPv4EphemeralUDP->Free();
        mIPv4EphemeralUDP = NULL;
    }

#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    if (mIPv4UDPBroadcastRcv != NULL)
    {
        mIPv4UDPBroadcastRcv->Free();
        mIPv4UDPBroadcastRcv = NULL;
    }

#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

#endif // INET_CONFIG_ENABLE_IPV4
}

CHIP_ERROR ChipMessageLayer::EnableUnsecuredListen()
{
    // Enable reception of connections on the unsecured CHIP port. This allows devices to establish
    // a connection while provisionally connected (i.e. without security) at the network layer.
    SetFlag(mFlags, kFlag_ListenUnsecured);
    return RefreshEndpoints();
}

CHIP_ERROR ChipMessageLayer::DisableUnsecuredListen()
{
    ClearFlag(mFlags, kFlag_ListenUnsecured);
    return RefreshEndpoints();
}

/**
 *  Set an application handler that will get called every time the
 *  activity of the message layer changes.
 *  Specifically, application will be notified every time:
 *     - the number of opened exchanges changes.
 *     - the number of pending message counter synchronization requests
 *       changes from zero to at least one and back to zero.
 *  The handler is served as general signal indicating whether there
 *  are any ongoing CHIP conversations or pending responses.
 *  The handler must be set after the ChipMessageLayer has been initialized;
 *  shutting down the ChipMessageLayer will clear out the current handler.
 *
 *  @param[in] messageLayerActivityChangeHandler A pointer to a function to
 *             be called whenever the message layer activity changes.
 *
 *  @retval None.
 */
void ChipMessageLayer::SetSignalMessageLayerActivityChanged(
    MessageLayerActivityChangeHandlerFunct messageLayerActivityChangeHandler)
{
    OnMessageLayerActivityChange = messageLayerActivityChangeHandler;
}

bool ChipMessageLayer::IsMessageLayerActive(void)
{
    return (ExchangeMgr->mContextsInUse != 0)
#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
        || FabricState->IsMsgCounterSyncReqInProgress()
#endif
        ;
}

/**
 *  This method is called every time the message layer activity changes.
 *  Specifically, it will be called every time:
 *     - the number of opened exchanges changes.
 *     - the number of pending message counter synchronization requests
 *       changes from zero to at least one and back to zero.
 *  New events can be added to this list in the future as needed.
 *
 *  @retval None.
 */
void ChipMessageLayer::SignalMessageLayerActivityChanged(void)
{
    if (OnMessageLayerActivityChange)
    {
        bool messageLayerIsActive = IsMessageLayerActive();
        OnMessageLayerActivityChange(messageLayerIsActive);
    }
}

/**
 *  Get the max CHIP payload size for a message configuration and supplied
 *  PacketBuffer.
 *
 *  The maximum payload size returned will not exceed the available space
 *  for a payload inside the supplied PacketBuffer.
 *
 *  If the message is UDP, the maximum payload size returned will not result in
 *  a CHIP message that will not overflow the specified UDP MTU.
 *
 *  Finally, the maximum payload size returned will not result in a CHIP
 *  message that will overflow the max CHIP message size.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer to which the message
 *                              payload will be written.
 *
 *  @param[in]    isUDP         True if message is a UDP message.
 *
 *  @param[in]    udpMTU        The size of the UDP MTU. Ignored if isUDP is false.
 *
 *  @return the max CHIP payload size.
 */
uint32_t ChipMessageLayer::GetMaxChipPayloadSize(const PacketBuffer * msgBuf, bool isUDP, uint32_t udpMTU)
{
    uint32_t maxChipMessageSize       = isUDP ? udpMTU - INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE : UINT16_MAX;
    uint32_t maxChipPayloadSize       = maxChipMessageSize - CHIP_HEADER_RESERVE_SIZE - CHIP_TRAILER_RESERVE_SIZE;
    uint32_t maxBufferablePayloadSize = msgBuf->AvailableDataLength() - CHIP_TRAILER_RESERVE_SIZE;

    return maxBufferablePayloadSize < maxChipPayloadSize ? maxBufferablePayloadSize : maxChipPayloadSize;
}

/**
 * Constructs a string describing a peer node and its associated address / connection information.
 *
 * The generated string has the following format:
 *
 *     <node-id> ([<ip-address>]:<port>%<interface>, con <con-id>)
 *
 * @param[in] buf                       A pointer to a buffer into which the string should be written. The supplied
 *                                      buffer should be at least as big as kChipPeerDescription_MaxLength. If a
 *                                      smaller buffer is given the string will be truncated to fit. The output
 *                                      will include a NUL termination character in all cases.
 * @param[in] bufSize                   The size of the buffer pointed at by buf.
 * @param[in] nodeId                    The node id to be printed.
 * @param[in] addr                      A pointer to an IP address to be printed; or NULL if no IP address should
 *                                      be printed.
 * @param[in] port                      An IP port number to be printed. No port number will be printed if addr
 *                                      is NULL.
 * @param[in] interfaceId               An InterfaceId identifying the interface to be printed. The output string
 *                                      will contain the name of the interface as known to the underlying network
 *                                      stack. No interface name will be printed if interfaceId is INET_NULL_INTERFACEID
 *                                      or if addr is NULL.
 * @param[in] con                       A pointer to a ChipConnection object whose logging id should be printed;
 *                                      or NULL if no connection id should be printed.
 */
void ChipMessageLayer::GetPeerDescription(char * buf, size_t bufSize, uint64_t nodeId, const IPAddress * addr, uint16_t port,
                                          InterfaceId interfaceId, const ChipConnection * con)
{
    enum
    {
        kMaxInterfaceNameLength = 20
    }; // Arbitrarily capped at 20 characters so long interface
       // names do not blow out the available space.

    uint32_t len;
    const char * sep = "";

    if (nodeId != kNodeIdNotSpecified)
    {
        len = snprintf(buf, bufSize, "%" PRIX64 " (", nodeId);
    }
    else
    {
        len = snprintf(buf, bufSize, "unknown (");
    }
    VerifyOrExit(len < bufSize, /* no-op */);

    if (addr != NULL)
    {
        buf[len++] = '[';
        VerifyOrExit(len < bufSize, /* no-op */);

        addr->ToString(buf + len, bufSize - len);
        len = strlen(buf);

        if (port > 0)
        {
            len += snprintf(buf + len, bufSize - len, "]:%" PRIu16, port);
        }
        else
        {
            len += snprintf(buf + len, bufSize - len, "]");
        }
        VerifyOrExit(len < bufSize, /* no-op */);

        if (interfaceId != INET_NULL_INTERFACEID)
        {
            char interfaceName[kMaxInterfaceNameLength + 1];
            Inet::GetInterfaceName(interfaceId, interfaceName, sizeof(interfaceName));
            interfaceName[kMaxInterfaceNameLength] = 0;
            len += snprintf(buf + len, bufSize - len, "%%%s", interfaceName);
            VerifyOrExit(len < bufSize, /* no-op */);
        }

        sep = ", ";
    }

    if (con != NULL)
    {
        const char * conType;
        switch (con->NetworkType)
        {
        case ChipConnection::kNetworkType_BLE:
            conType = "ble ";
            break;
        case ChipConnection::kNetworkType_IP:
        default:
            conType = "";
            break;
        }

        len += snprintf(buf + len, bufSize - len, "%s%scon %04" PRIX16, sep, conType, con->LogId());
        VerifyOrExit(len < bufSize, /* no-op */);
    }

    snprintf(buf + len, bufSize - len, ")");

exit:
    if (bufSize > 0)
    {
        buf[bufSize - 1] = 0;
    }
    return;
}

/**
 * Constructs a string describing a peer node based on the information associated with a message received from the peer.
 *
 * @param[in] buf                       A pointer to a buffer into which the string should be written. The supplied
 *                                      buffer should be at least as big as kChipPeerDescription_MaxLength. If a
 *                                      smaller buffer is given the string will be truncated to fit. The output
 *                                      will include a NUL termination character in all cases.
 * @param[in] bufSize                   The size of the buffer pointed at by buf.
 * @param[in] msgInfo                   A pointer to a ChipMessageInfo structure containing information about the message.
 *
 */
void ChipMessageLayer::GetPeerDescription(char * buf, size_t bufSize, const ChipMessageInfo * msgInfo)
{
    GetPeerDescription(buf, bufSize, msgInfo->SourceNodeId,
                       (msgInfo->InPacketInfo != NULL) ? &msgInfo->InPacketInfo->SrcAddress : NULL,
                       (msgInfo->InPacketInfo != NULL) ? msgInfo->InPacketInfo->SrcPort : 0,
                       (msgInfo->InPacketInfo != NULL) ? msgInfo->InPacketInfo->Interface : INET_NULL_INTERFACEID, msgInfo->InCon);
}

/**
 * @brief
 *   Generate random CHIP node Id.
 *
 * @details
 *   This function generates 64-bit locally unique CHIP node Id. This function uses cryptographically strong
 *   random data source to guarantee uniqueness of generated value. Note that bit 57 of the generated CHIP
 *   node Id is set to 1 to indicate that generated CHIP node Id is locally (not globally) unique.
 *
 * @param nodeId                        A reference to the 64-bit CHIP node Id.
 *
 * @retval  #CHIP_NO_ERROR             If CHIP node Id was successfully generated.
 */
DLL_EXPORT CHIP_ERROR GenerateChipNodeId(uint64_t & nodeId)
{
    CHIP_ERROR err;
    uint64_t id = 0;

    while (id <= kMaxAlwaysLocalChipNodeId)
    {
        err = chip::Platform::Security::GetSecureRandomData(reinterpret_cast<uint8_t *>(&id), sizeof(id));
        SuccessOrExit(err);

        id &= ~kEUI64_UL_Local;
    }

    nodeId = id | kEUI64_UL_Local;

exit:
    return err;
}

} // namespace chip
