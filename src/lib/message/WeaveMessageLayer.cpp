/*
 *
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
 *      This file implements the WeaveMessageLayer class. It manages communication
 *      with other Weave nodes by employing one of several Inetlayer endpoints
 *      to establish a communication channel with other Weave nodes.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <Weave/Core/WeaveCore.h>
#include <Weave/Core/WeaveMessageLayer.h>
#include <Weave/Core/WeaveExchangeMgr.h>
#include <Weave/Core/WeaveEncoding.h>
#include <Weave/Support/crypto/WeaveCrypto.h>
#include <Weave/Support/crypto/HashAlgos.h>
#include <Weave/Support/crypto/HMAC.h>
#include <Weave/Support/crypto/AESBlockCipher.h>
#include <Weave/Support/crypto/CTRMode.h>
#include <Weave/Support/logging/WeaveLogging.h>
#include <Weave/Support/ErrorStr.h>
#include <Weave/Support/CodeUtils.h>
#include <Weave/Support/WeaveFaultInjection.h>


namespace nl {
namespace Weave {

using namespace nl::Weave::Crypto;
using namespace nl::Weave::Encoding;

/**
 *  @def WEAVE_BIND_DETAIL_LOGGING
 *
 *  @brief
 *    Use Weave Bind detailed logging for Weave communication.
 *
 */
#ifndef WEAVE_BIND_DETAIL_LOGGING
#define WEAVE_BIND_DETAIL_LOGGING 1
#endif

/**
 *  @def WeaveBindLog(MSG, ...)
 *
 *  @brief
 *    Define WeaveBindLogic to be the same as WeaveLogProgress based on
 *    whether both #WEAVE_BIND_DETAIL_LOGGING and #WEAVE_DETAIL_LOGGING
 *    are set.
 *
 */
#if WEAVE_BIND_DETAIL_LOGGING && WEAVE_DETAIL_LOGGING
#define WeaveBindLog(MSG, ...) WeaveLogProgress(MessageLayer, MSG, ## __VA_ARGS__ )
#else
#define WeaveBindLog(MSG, ...)
#endif


enum
{
    kKeyIdLen = 2,
    kMinPayloadLen = 1
};

/**
 *  The Weave Message layer constructor.
 *
 *  @note
 *    The class must be initialized via WeaveMessageLayer::Init()
 *    prior to use.
 *
 */
WeaveMessageLayer::WeaveMessageLayer()
{
    State = kState_NotInitialized;
}

/**
 *  Initialize the Weave Message layer object.
 *
 *  @param[in]  context  A pointer to the InitContext object.
 *
 *  @retval  #WEAVE_NO_ERROR                     on successful initialization.
 *  @retval  #WEAVE_ERROR_INVALID_ARGUMENT       if the passed InitContext object is NULL.
 *  @retval  #WEAVE_ERROR_INCORRECT_STATE        if the state of the WeaveMessageLayer object is incorrect.
 *  @retval  other errors generated from the lower Inet layer during endpoint creation.
 *
 */
WEAVE_ERROR WeaveMessageLayer::Init(InitContext *context)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(State == kState_NotInitialized, err = WEAVE_ERROR_INCORRECT_STATE);
    VerifyOrExit(context != NULL, err = WEAVE_ERROR_INVALID_ARGUMENT);

    State = kState_Initializing;

    SystemLayer = context->systemLayer;
    Inet = context->inet;
#if CONFIG_NETWORK_LAYER_BLE
    mBle = context->ble;
#endif

#if WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    if (SystemLayer == NULL)
    {
        SystemLayer = Inet->SystemLayer();
    }
#endif // WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    FabricState = context->fabricState;
    FabricState->MessageLayer = this;
    OnMessageReceived = NULL;
    OnReceiveError = NULL;
    OnConnectionReceived = NULL;
    OnUnsecuredConnectionReceived = NULL;
    OnUnsecuredConnectionCallbacksRemoved = NULL;
    OnAcceptError = NULL;
    OnMessageLayerActivityChange = NULL;
    memset(mConPool, 0, sizeof(mConPool));
    memset(mTunnelPool, 0, sizeof(mTunnelPool));
    AppState = NULL;
    ExchangeMgr = NULL;
    SecurityMgr = NULL;
    IsListening = context->listenTCP || context->listenUDP;
    IncomingConIdleTimeout = WEAVE_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT;

    //Internal and for Debug Only; When set, Message Layer drops message and returns.
    mDropMessage = false;
    mFlags = 0;
    SetTCPListenEnabled(context->listenTCP);
    SetUDPListenEnabled(context->listenUDP);
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    SetEphemeralUDPPortEnabled(context->enableEphemeralUDPPort);
#endif

    mIPv6TCPListen = NULL;
    mIPv6UDP = NULL;

#if INET_CONFIG_ENABLE_IPV4
    mIPv4TCPListen = NULL;
    mIPv4UDP = NULL;
#endif // INET_CONFIG_ENABLE_IPV4

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    mIPv6UDPMulticastRcv = NULL;
#if INET_CONFIG_ENABLE_IPV4
    mIPv4UDPBroadcastRcv = NULL;
#endif // INET_CONFIG_ENABLE_IPV4
#endif //WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    mIPv6EphemeralUDP = NULL;
#if INET_CONFIG_ENABLE_IPV4
    mIPv4EphemeralUDP = NULL;
#endif // INET_CONFIG_ENABLE_IPV4
#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
    mUnsecuredIPv6TCPListen = NULL;
#endif

    err = RefreshEndpoints();
    SuccessOrExit(err);

#if CONFIG_NETWORK_LAYER_BLE
    if (context->listenBLE && mBle != NULL)
    {
        mBle->mAppState = this;
        mBle->OnWeaveBleConnectReceived = HandleIncomingBleConnection;
        WeaveLogProgress(MessageLayer, "Accepting WoBLE connections");
    }
    else
    {
        WeaveLogProgress(MessageLayer, "WoBLE disabled%s", (mBle != NULL) ? " by application" : " (BLE layer not initialized)");
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    State = kState_Initialized;

exit:
    if (err != WEAVE_NO_ERROR && State == kState_Initializing)
    {
        Shutdown();
    }
    return err;
}

/**
 *  Shutdown the WeaveMessageLayer.
 *
 *  Close all open Inet layer endpoints, reset all
 *  higher layer callbacks, member variables and objects.
 *  A call to Shutdown() terminates the WeaveMessageLayer
 *  object.
 *
 */
WEAVE_ERROR WeaveMessageLayer::Shutdown()
{
    CloseEndpoints();

#if CONFIG_NETWORK_LAYER_BLE
    if (mBle != NULL && mBle->mAppState == this)
    {
        mBle->mAppState = NULL;
        mBle->OnWeaveBleConnectReceived = NULL;
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    State = kState_NotInitialized;
    IsListening = false;
    FabricState = NULL;
    OnMessageReceived = NULL;
    OnReceiveError = NULL;
    OnUnsecuredConnectionReceived = NULL;
    OnConnectionReceived = NULL;
    OnAcceptError = NULL;
    OnMessageLayerActivityChange = NULL;
    memset(mConPool, 0, sizeof(mConPool));
    memset(mTunnelPool, 0, sizeof(mTunnelPool));
    ExchangeMgr = NULL;
    AppState = NULL;
    mFlags = 0;

    return WEAVE_NO_ERROR;
}

#if WEAVE_CONFIG_ENABLE_TUNNELING
/**
 *  Send a tunneled IPv6 data message over UDP.
 *
 *  @param[in] msgInfo          A pointer to a WeaveMessageInfo object.
 *
 *  @param[in] destAddr         IPAddress of the UDP tunnel destination.
 *
 *  @param[in] msgBuf           A pointer to the PacketBuffer object holding the packet to send.
 *
 *  @retval  #WEAVE_NO_ERROR                    on successfully sending the message down to the network
 *                                              layer.
 *  @retval  #WEAVE_ERROR_INVALID_ADDRESS       if the destAddr is not specified or cannot be determined
 *                                              from destination node id.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::SendUDPTunneledMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf)
{
    WEAVE_ERROR res = WEAVE_NO_ERROR;

    //Set message version to V2
    msgInfo->MessageVersion = kWeaveMessageVersion_V2;

    //Set the tunneling flag
    msgInfo->Flags |= kWeaveMessageFlag_TunneledData;

    res = SendMessage(destAddr, msgInfo, msgBuf);
    msgBuf = NULL;

    return res;
}
#endif // WEAVE_CONFIG_ENABLE_TUNNELING

/**
 *  Encode a Weave Message layer header into an PacketBuffer.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    sendIntId     The interface on which to send the Weave message.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object that would hold the Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR                           on successful encoding of the Weave message.
 *  @retval  #WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION  if the Weave Message version is not supported.
 *  @retval  #WEAVE_ERROR_INVALID_MESSAGE_LENGTH       if the payload length in the message buffer is zero.
 *  @retval  #WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE  if the encryption type is not supported.
 *  @retval  #WEAVE_ERROR_MESSAGE_TOO_LONG             if the encoded message would be longer than the
 *                                                     requested maximum.
 *  @retval  #WEAVE_ERROR_BUFFER_TOO_SMALL             if there is not enough space before or after the
 *                                                     message payload.
 *  @retval  other errors generated by the fabric state object when fetching the session state.
 *
 */
WEAVE_ERROR WeaveMessageLayer::EncodeMessage(const IPAddress &destAddr, uint16_t destPort, InterfaceId sendIntId,
                                             WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    WEAVE_ERROR res = WEAVE_NO_ERROR;

    // Set the source node identifier in the message header.
    if ((msgInfo->Flags & kWeaveMessageFlag_ReuseSourceId) == 0)
        msgInfo->SourceNodeId = FabricState->LocalNodeId;

    // Force inclusion of the source node identifier if the destination address is not a local fabric address.
    //
    // Technically it should be possible to omit the source node identifier in other situations beyond the
    // ones allowed for here.  However it is difficult to determine exactly what the source IP
    // address will be when sending a UDP packet, so we err on the side of correctness and only omit
    // the source identifier if we're part of a fabric and sending to another member of the same fabric.
    if (!FabricState->IsFabricAddress(destAddr))
        msgInfo->Flags |= kWeaveMessageFlag_SourceNodeId;

    // Force the destination node identifier to be included if it doesn't match the interface identifier in
    // the destination address.
    if (!destAddr.IsIPv6ULA() || IPv6InterfaceIdToWeaveNodeId(destAddr.InterfaceId()) != msgInfo->DestNodeId)
        msgInfo->Flags |= kWeaveMessageFlag_DestNodeId;

    // Encode the Weave message. NOTE that this results in the payload buffer containing the entire encoded message.
    res = EncodeMessage(msgInfo, payload, NULL, UINT16_MAX, 0);

    return res;
}

/**
 *  Send a Weave message using the underlying Inetlayer UDP endpoint after encoding it.
 *
 *  @note
 *    The destination port used is #WEAVE_PORT.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::SendMessage(WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    return SendMessage(IPAddress::Any, msgInfo, payload);
}

/**
 *  Send a Weave message using the underlying Inetlayer UDP endpoint after encoding it.
 *
 *  @note
 *    -The destination port used is #WEAVE_PORT.
 *
 *    -If the destination address has not been supplied, attempt to determine it from the node identifier in
 *     the message header. Fail if this can't be done.
 *
 *    -If the destination address is a fabric address for the local fabric, and the caller
 *     didn't specify the destination node id, extract it from the destination address.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::SendMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo,
                                           PacketBuffer *payload)
{
    return SendMessage(destAddr, WEAVE_PORT, INET_NULL_INTERFACEID, msgInfo, payload);
}

/**
 *  Send a Weave message using the underlying Inetlayer UDP endpoint after encoding it.
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
 *  @param[in]    sendIntfId    The interface on which to send the Weave message.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object containing information
 *                              about the message to be sent.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the
 *                              encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR                    on successfully sending the message down to the network
 *                                              layer.
 *  @retval  #WEAVE_ERROR_INVALID_ADDRESS       if the destAddr is not specified or cannot be determined
 *                                              from destination node id.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::SendMessage(const IPAddress &aDestAddr, uint16_t destPort, InterfaceId sendIntfId,
                                           WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    WEAVE_ERROR res = WEAVE_NO_ERROR;
    IPAddress destAddr = aDestAddr;

    // Determine the message destination address based on the destination nodeId.
    res = SelectDestNodeIdAndAddress(msgInfo->DestNodeId, destAddr);
    SuccessOrExit(res);

    res = EncodeMessage(destAddr, destPort, sendIntfId, msgInfo, payload);
    SuccessOrExit(res);

    // on delay send, we do everything except actually send the
    // message.  As a result, the payload will contain the entire
    // state required for sending it a bit later
    if (msgInfo->Flags & kWeaveMessageFlag_DelaySend)
        return WEAVE_NO_ERROR;

    // Copy msg to a right-sized buffer if applicable
    payload = PacketBuffer::RightSize(payload);

    // Send the message using the appropriate UDP endpoint(s).
    return SendMessage(destAddr, destPort, sendIntfId, payload, msgInfo->Flags);

exit:
    if ((res != WEAVE_NO_ERROR) &&
        (payload != NULL) &&
        ((msgInfo->Flags & kWeaveMessageFlag_RetainBuffer) == 0))
    {
        PacketBuffer::Free(payload);
    }

    return res;
}

bool WeaveMessageLayer::IsIgnoredMulticastSendError(WEAVE_ERROR err)
{
    return err == WEAVE_NO_ERROR ||
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
           err == System::MapErrorLwIP(ERR_RTE)
#else
           err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL)
#endif
           ;
}

WEAVE_ERROR WeaveMessageLayer::FilterUDPSendError(WEAVE_ERROR err, bool isMulticast)
{
    // Don't report certain types of routing errors when they occur while sending multicast packets.
    // These may indicate that the underlying interface doesn't support multicast (e.g. the loopback
    // interface on linux) or that the selected interface doesn't have an appropriate source address.
    if (isMulticast)
    {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
        if (err == System::MapErrorLwIP(ERR_RTE))
        {
            err = WEAVE_NO_ERROR;
        }
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        if (err == System::MapErrorPOSIX(ENETUNREACH) || err == System::MapErrorPOSIX(EADDRNOTAVAIL))
        {
            err = WEAVE_NO_ERROR;
        }
#endif
    }

    return err;
}


/**
 *  Checks if error, while sending, is critical enough to report to the application.
 *
 *  @param[in]    err      The #WEAVE_ERROR being checked for criticality.
 *
 *  @return    true if the error is NOT critical; false otherwise.
 *
 */
bool WeaveMessageLayer::IsSendErrorNonCritical(WEAVE_ERROR err)
{
    return (err == INET_ERROR_NOT_IMPLEMENTED || err == INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED ||
            err == INET_ERROR_MESSAGE_TOO_LONG || err == INET_ERROR_NO_MEMORY ||
            WEAVE_CONFIG_IsPlatformErrorNonCritical(err));
}

/**
 * Set the 'ForceRefreshUDPEndpoints' flag if needed.
 *
 * Based on the error returned when sending a UDP message, set a flag in the WeaveMessageLayer
 * that will force a complete refresh of all UDPEndPoints the next time \c RefreshEndPoints is
 * called.
 */
void WeaveMessageLayer::CheckForceRefreshUDPEndPointsNeeded(WEAVE_ERROR err)
{
    // On some sockets-based systems, the OS will invalidate bound UDP endpoints when certain
    // network transitions occur.  This is known to occur on Android, although the precise
    // conditions are unclear.  When that happens, set the ForceRefreshUDPEndPoints flag to
    // force all UDPEndPoints to be closed and re-opened on the next call to RefreshEndPoints().
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    if (err == System::MapErrorPOSIX(EPIPE))
    {
        SetFlag(mFlags, kFlag_ForceRefreshUDPEndPoints);
    }
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
}

/**
 *  Send an encoded Weave message using the appropriate underlying Inetlayer UDPEndPoint (or EndPoints).
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    sendIntfId    The interface on which to send the Weave message.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded Weave message.
 *
 *  @param[in]    msgSendFlags  Send flags containing metadata about the message for the lower Inet layer.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::SendMessage(const IPAddress & destAddr, uint16_t destPort, InterfaceId sendIntfId,
                                           PacketBuffer * payload, uint32_t msgFlags)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
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
    pktInfo.DestPort = destPort;
    pktInfo.Interface = sendIntfId;

    // Check if drop flag is set; If so, do not send message; return WEAVE_NO_ERROR;
    VerifyOrExit(!mDropMessage, err = WEAVE_NO_ERROR);

    // Drop the message and return. Free the buffer if it does not need to be
    // retained(e.g., for WRM retransmissions).
    WEAVE_FAULT_INJECT(FaultInjection::kFault_DropOutgoingUDPMsg,
            ExitNow(err = WEAVE_NO_ERROR);
            );

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
    //     a member of a Weave fabric, AND MulticastFromLinkLocal has NOT been specified, send
    //     the message once for each Weave Fabric ULA assigned to a local interface that supports
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
#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    else if (destAddr.IsIPv4() ? IsBoundToLocalIPv4Address() : IsBoundToLocalIPv6Address())
    {
        sendAction = kMulticast_OneInterface;
    }
#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    else if (destAddr.IsIPv6() && FabricState->FabricId != kFabricIdNotSpecified &&
             !GetFlag(msgFlags, kWeaveMessageFlag_DefaultMulticastSourceAddress))
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
        udpSendFlags = GetFlag(msgFlags, kWeaveMessageFlag_RetainBuffer) ? UDPEndPoint::kSendFlag_RetainBuffer : 0;
        err = ep->SendMsg(&pktInfo, payload, udpSendFlags);
        payload = NULL; // Prevent call to Free() in exit code
        CheckForceRefreshUDPEndPointsNeeded(err);
        err = FilterUDPSendError(err, sendAction == kMulticast_OneInterface);
        break;

    case kMulticast_AllInterfaces:

        // Send the message over each local interface that supports multicast.
        for (InterfaceIterator intfIter; intfIter.HasCurrent(); intfIter.Next())
        {
            if (intfIter.SupportsMulticast())
            {
                pktInfo.Interface = intfIter.GetInterface();
                WEAVE_ERROR sendErr = ep->SendMsg(&pktInfo, payload, UDPEndPoint::kSendFlag_RetainBuffer);
                CheckForceRefreshUDPEndPointsNeeded(sendErr);
                if (err == WEAVE_NO_ERROR)
                {
                    err = FilterUDPSendError(sendErr, true);
                }
            }
        }

        break;

    case kMulticast_AllFabricAddrs:

        // Send the message once for each Weave Fabric ULA assigned to a local interface that supports
        // multicast/broadcast. If the caller has specified a particular interface, only send over the
        // specified interface.  For each message sent, arrange for the source address to be the Fabric ULA.
        for (InterfaceAddressIterator addrIter; addrIter.HasCurrent(); addrIter.Next())
        {
            pktInfo.SrcAddress = addrIter.GetAddress();
            pktInfo.Interface = addrIter.GetInterface();
            if (addrIter.SupportsMulticast() &&
                FabricState->IsLocalFabricAddress(pktInfo.SrcAddress) &&
                (sendIntfId == INET_NULL_INTERFACEID || pktInfo.Interface == sendIntfId))
            {
                WEAVE_ERROR sendErr = ep->SendMsg(&pktInfo, payload, UDPEndPoint::kSendFlag_RetainBuffer);
                CheckForceRefreshUDPEndPointsNeeded(sendErr);
                if (err == WEAVE_NO_ERROR)
                {
                    err = FilterUDPSendError(sendErr, true);
                }
            }
        }

        break;
    }

exit:
    if (payload != NULL && !GetFlag(msgFlags, kWeaveMessageFlag_RetainBuffer))
        PacketBuffer::Free(payload);
    return err;
}

/**
 *  Select an appropriate UDP endpoint for sending a Weave message.
 */
WEAVE_ERROR WeaveMessageLayer::SelectOutboundUDPEndPoint(const IPAddress & destAddr, uint32_t msgFlags, UDPEndPoint *& ep)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Select a UDP endpoint object for sending a message based on the destination address type
    // and the message send flags.
    //
    // If the WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT option is set, select the ephemeral UDP
    // endpoint if the caller has specified the 'ViaEphemeralUDPPort' flag.  This will result in
    // the source port field of the UDP message being set to the currently active ephemeral
    // port. Otherwise, select the Weave UDP endpoint. This will result in the source port
    // field being set to the well-known Weave port.
    //
    switch (destAddr.Type())
    {
#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        if (GetFlag(msgFlags, kWeaveMessageFlag_ViaEphemeralUDPPort))
        {
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
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
        if (GetFlag(msgFlags, kWeaveMessageFlag_ViaEphemeralUDPPort))
        {
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
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
        ExitNow(err = WEAVE_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(ep != NULL, err = WEAVE_ERROR_NO_ENDPOINT);

exit:
    return err;
}


/**
 *  Resend an encoded Weave message using the underlying Inetlayer UDP endpoint.
 *
 *  @param[in]    msgInfo     A pointer to the WeaveMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::ResendMessage(WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    IPAddress destAddr = IPAddress::Any;
    return ResendMessage(destAddr, msgInfo, payload);
}

/**
 *  Resend an encoded Weave message using the underlying Inetlayer UDP endpoint.
 *
 *  @note
 *    The destination port used is #WEAVE_PORT.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    msgInfo       A pointer to the WeaveMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::ResendMessage(const IPAddress &destAddr, WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    return ResendMessage(destAddr, WEAVE_PORT, msgInfo, payload);
}

/**
 *  Resend an encoded Weave message using the underlying Inetlayer UDP endpoint.
 *
 *  @param[in]    destAddr      The destination IP Address.
 *
 *  @param[in]    destPort      The destination port.
 *
 *  @param[in]    msgInfo       A pointer to the WeaveMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::ResendMessage(const IPAddress &destAddr, uint16_t destPort, WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    return ResendMessage(destAddr, WEAVE_PORT, INET_NULL_INTERFACEID, msgInfo, payload);
}

/**
 *  Resend an encoded Weave message using the underlying Inetlayer UDP endpoint.
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
 *  @param[in]    interfaceId   The interface on which to send the Weave message.
 *
 *  @param[in]    msgInfo       A pointer to the WeaveMessageInfo object.
 *
 *  @param[in]    payload       A pointer to the PacketBuffer object holding the encoded Weave message.
 *
 *  @retval  #WEAVE_NO_ERROR    on successfully sending the message down to the network layer.
 *  @retval  errors generated from the lower Inet layer UDP endpoint during sending.
 *
 */
WEAVE_ERROR WeaveMessageLayer::ResendMessage(const IPAddress &aDestAddr, uint16_t destPort, InterfaceId interfaceId,
                                             WeaveMessageInfo *msgInfo, PacketBuffer *payload)
{
    WEAVE_ERROR res = WEAVE_NO_ERROR;
    IPAddress destAddr = aDestAddr;

    res = SelectDestNodeIdAndAddress(msgInfo->DestNodeId, destAddr);
    SuccessOrExit(res);

    return SendMessage(destAddr, destPort, interfaceId, payload, msgInfo->Flags);
exit:
    if ((res != WEAVE_NO_ERROR) &&
        (payload != NULL) &&
        ((msgInfo->Flags & kWeaveMessageFlag_RetainBuffer) == 0))
    {
        PacketBuffer::Free(payload);
    }
    return res;
}

/**
 *  Get the number of WeaveConnections in use and the size of the pool
 *
 *  @param[out]  aOutInUse  Reference to size_t, in which the number of
 *                         connections in use is stored.
 *
 */
void WeaveMessageLayer::GetConnectionPoolStats(nl::Weave::System::Stats::count_t &aOutInUse) const
{
    aOutInUse = 0;

    const WeaveConnection *con = (WeaveConnection *) mConPool;
    for (int i = 0; i < WEAVE_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount != 0)
        {
            aOutInUse++;
        }
    }
}

/**
 *  Create a new WeaveConnection object from a pool.
 *
 *  @return  a pointer to the newly created WeaveConnection object if successful, otherwise
 *           NULL.
 *
 */
WeaveConnection *WeaveMessageLayer::NewConnection()
{
    WeaveConnection *con = (WeaveConnection *) mConPool;
    for (int i = 0; i < WEAVE_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount == 0)
        {
            con->Init(this);
            return con;
        }
    }

    WeaveLogError(ExchangeManager, "New con FAILED");
    return NULL;
}

void WeaveMessageLayer::GetIncomingTCPConCount(const IPAddress &peerAddr, uint16_t &count, uint16_t &countFromIP)
{
    count = 0;
    countFromIP = 0;

    WeaveConnection *con = (WeaveConnection *) mConPool;
    for (int i = 0; i < WEAVE_CONFIG_MAX_CONNECTIONS; i++, con++)
    {
        if (con->mRefCount > 0 &&
            con->NetworkType == WeaveConnection::kNetworkType_IP &&
            con->IsIncoming())
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
 *  Create a new WeaveConnectionTunnel object from a pool.
 *
 *  @return  a pointer to the newly created WeaveConnectionTunnel object if successful,
 *           otherwise NULL.
 *
 */
WeaveConnectionTunnel *WeaveMessageLayer::NewConnectionTunnel()
{
    WeaveConnectionTunnel *tun = (WeaveConnectionTunnel *) mTunnelPool;
    for (int i = 0; i < WEAVE_CONFIG_MAX_TUNNELS; i++, tun++)
    {
        if (tun->IsInUse() == false)
        {
            tun->Init(this);
            return tun;
        }
    }

    WeaveLogError(ExchangeManager, "New tun FAILED");
    return NULL;
}

/**
 *  Create a WeaveConnectionTunnel by coupling together two specified WeaveConnections.
    On successful creation, the TCPEndPoints corresponding to the component WeaveConnection
    objects are handed over to the WeaveConnectionTunnel, otherwise the WeaveConnections are
    closed.
 *
 *  @param[out]    tunPtr                 A pointer to pointer of a WeaveConnectionTunnel object.
 *
 *  @param[in]     conOne                 A reference to the first WeaveConnection object.
 *
 *  @param[in]     conTwo                 A reference to the second WeaveConnection object.
 *
 *  @param[in]     inactivityTimeoutMS    The maximum time in milliseconds that the Weave
 *                                        connection tunnel could be idle.
 *
 *  @retval    #WEAVE_NO_ERROR            on successful creation of the WeaveConnectionTunnel.
 *  @retval    #WEAVE_ERROR_INCORRECT_STATE if the component WeaveConnection objects of the
 *                                          WeaveConnectionTunnel is not in the correct state.
 *  @retval    #WEAVE_ERROR_NO_MEMORY       if a new WeaveConnectionTunnel object cannot be created.
 *
 */
WEAVE_ERROR WeaveMessageLayer::CreateTunnel(WeaveConnectionTunnel **tunPtr, WeaveConnection &conOne,
        WeaveConnection &conTwo, uint32_t inactivityTimeoutMS)
{
    WeaveLogDetail(ExchangeManager, "Entering CreateTunnel");
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(conOne.State == WeaveConnection::kState_Connected && conTwo.State ==
            WeaveConnection::kState_Connected, err = WEAVE_ERROR_INCORRECT_STATE);

    *tunPtr = NewConnectionTunnel();
    VerifyOrExit(*tunPtr != NULL, err = WEAVE_ERROR_NO_MEMORY);

    // Form WeaveConnectionTunnel from former WeaveConnections' TCPEndPoints.
    err = (*tunPtr)->MakeTunnelConnected(conOne.mTcpEndPoint, conTwo.mTcpEndPoint);
    SuccessOrExit(err);

    WeaveLogProgress(ExchangeManager, "Created Weave tunnel from Cons (%04X, %04X) with EPs (%04X, %04X)",
            conOne.LogId(), conTwo.LogId(), conOne.mTcpEndPoint->LogId(), conTwo.mTcpEndPoint->LogId());

    if (inactivityTimeoutMS > 0)
    {
        // Set TCPEndPoint inactivity timeouts.
        conOne.mTcpEndPoint->SetIdleTimeout(inactivityTimeoutMS);
        conTwo.mTcpEndPoint->SetIdleTimeout(inactivityTimeoutMS);
    }

    // Remove TCPEndPoints from WeaveConnections now that we've handed the former to our new WeaveConnectionTunnel.
    conOne.mTcpEndPoint = NULL;
    conTwo.mTcpEndPoint = NULL;

exit:
    WeaveLogDetail(ExchangeManager, "Exiting CreateTunnel");

    // Close WeaveConnection args.
    conOne.Close(true);
    conTwo.Close(true);

    return err;
}

WEAVE_ERROR WeaveMessageLayer::SetUnsecuredConnectionListener(ConnectionReceiveFunct
        newOnUnsecuredConnectionReceived, CallbackRemovedFunct newOnUnsecuredConnectionCallbacksRemoved, bool force,
        void *listenerState)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    WeaveLogProgress(ExchangeManager, "Entered SetUnsecuredConnectionReceived, cb = %p, %p",
            newOnUnsecuredConnectionReceived, newOnUnsecuredConnectionCallbacksRemoved);

    if (UnsecuredListenEnabled() == false)
    {
        err = EnableUnsecuredListen();
        SuccessOrExit(err);
    }

    // New OnUnsecuredConnectionReceived cannot be null. To clear, use ClearOnUnsecuredConnectionReceived().
    VerifyOrExit(newOnUnsecuredConnectionReceived != NULL, err = WEAVE_ERROR_INVALID_ARGUMENT);

    if (OnUnsecuredConnectionReceived != NULL)
    {
        if (force == false)
        {
            err = WEAVE_ERROR_INCORRECT_STATE;
            ExitNow();
        }
        else if (OnUnsecuredConnectionCallbacksRemoved != NULL)
        {
            // Notify application that its previous OnUnsecuredConnectionReceived callback has been removed.
            OnUnsecuredConnectionCallbacksRemoved(UnsecuredConnectionReceivedAppState);
        }
    }

    OnUnsecuredConnectionReceived = newOnUnsecuredConnectionReceived;
    OnUnsecuredConnectionCallbacksRemoved = newOnUnsecuredConnectionCallbacksRemoved;
    UnsecuredConnectionReceivedAppState = listenerState;

exit:
    return err;
}

WEAVE_ERROR WeaveMessageLayer::ClearUnsecuredConnectionListener(ConnectionReceiveFunct
        oldOnUnsecuredConnectionReceived, CallbackRemovedFunct oldOnUnsecuredConnectionCallbacksRemoved)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    WeaveLogProgress(ExchangeManager, "Entered ClearUnsecuredConnectionListener, cbs = %p, %p",
            oldOnUnsecuredConnectionReceived, oldOnUnsecuredConnectionCallbacksRemoved);

    // Only clear callbacks and suppress OnUnsecuredConnectionCallbacksRemoved if caller can prove it owns current
    // callbacks. For proof of identification, we accept copies of callback function pointers.
    if (oldOnUnsecuredConnectionReceived != OnUnsecuredConnectionReceived || oldOnUnsecuredConnectionCallbacksRemoved
            != OnUnsecuredConnectionCallbacksRemoved)
    {
        if (oldOnUnsecuredConnectionReceived != OnUnsecuredConnectionReceived)
            WeaveLogError(ExchangeManager, "bad arg: OnUnsecuredConnectionReceived");
        else
            WeaveLogError(ExchangeManager, "bad arg: OnUnsecuredConnectionCallbacksRemoved");
        err = WEAVE_ERROR_INVALID_ARGUMENT;
        ExitNow();
    }

    if (UnsecuredListenEnabled() == true)
    {
        err = DisableUnsecuredListen();
        SuccessOrExit(err);
    }

    OnUnsecuredConnectionReceived = NULL;
    OnUnsecuredConnectionCallbacksRemoved = NULL;
    UnsecuredConnectionReceivedAppState = NULL;

exit:
    return err;
}

WEAVE_ERROR WeaveMessageLayer::SelectDestNodeIdAndAddress(uint64_t& destNodeId, IPAddress& destAddr)
{
    // If the destination address has not been supplied, attempt to determine it from the node id.
    // Fail if this can't be done.
    if (destAddr == IPAddress::Any)
    {
        destAddr = FabricState->SelectNodeAddress(destNodeId);
        if (destAddr == IPAddress::Any)
            return WEAVE_ERROR_INVALID_ADDRESS;
    }

    // If the destination address is a fabric address for the local fabric, and the caller
    // didn't specify the destination node id, extract it from the destination address.
    if (FabricState->IsFabricAddress(destAddr) && destNodeId == kNodeIdNotSpecified)
        destNodeId = IPv6InterfaceIdToWeaveNodeId(destAddr.InterfaceId());

    return WEAVE_NO_ERROR;
}

// Encode and return message header field value.
static uint16_t EncodeHeaderField(const WeaveMessageInfo *msgInfo)
{
    return ((((uint16_t)msgInfo->Flags) << kMsgHeaderField_FlagsShift) & kMsgHeaderField_FlagsMask) |
           ((((uint16_t)msgInfo->EncryptionType) << kMsgHeaderField_EncryptionTypeShift) & kMsgHeaderField_EncryptionTypeMask) |
           ((((uint16_t)msgInfo->MessageVersion) << kMsgHeaderField_MessageVersionShift) & kMsgHeaderField_MessageVersionMask);
}

// Decode message header field value.
static void DecodeHeaderField(const uint16_t headerField, WeaveMessageInfo *msgInfo)
{
    msgInfo->Flags = (uint16_t)((headerField & kMsgHeaderField_FlagsMask) >> kMsgHeaderField_FlagsShift);
    msgInfo->EncryptionType = (uint8_t)((headerField & kMsgHeaderField_EncryptionTypeMask) >> kMsgHeaderField_EncryptionTypeShift);
    msgInfo->MessageVersion = (uint8_t)((headerField & kMsgHeaderField_MessageVersionMask) >> kMsgHeaderField_MessageVersionShift);
}

/**
 *  Decode a Weave Message layer header from a received Weave message.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the Weave message.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object which will receive information
 *                              about the message.
 *
 *  @param[out]   payloadStart  A pointer to a pointer to the position in the message buffer after
 *                              decoding is complete.
 *
 *  @retval  #WEAVE_NO_ERROR    On successful decoding of the message header.
 *  @retval  #WEAVE_ERROR_INVALID_MESSAGE_LENGTH
 *                              If the message buffer passed is of invalid length.
 *  @retval  #WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION
 *                              If the Weave Message header format version is not supported.
 *
 */
WEAVE_ERROR WeaveMessageLayer::DecodeHeader(PacketBuffer *msgBuf, WeaveMessageInfo *msgInfo, uint8_t **payloadStart)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    uint8_t *msgStart = msgBuf->Start();
    uint16_t msgLen = msgBuf->DataLength();
    uint8_t *msgEnd = msgStart + msgLen;
    uint8_t *p = msgStart;
    uint16_t headerField;

    if (msgLen < 6)
    {
        ExitNow(err = WEAVE_ERROR_INVALID_MESSAGE_LENGTH);
    }

    // Read and verify the header field.
    headerField = LittleEndian::Read16(p);
    VerifyOrExit((headerField & kMsgHeaderField_ReservedFlagsMask) == 0, err = WEAVE_ERROR_INVALID_MESSAGE_FLAG);

    // Decode the header field.
    DecodeHeaderField(headerField, msgInfo);

    // Error if the message version is unsupported.
    if (msgInfo->MessageVersion != kWeaveMessageVersion_V1 &&
        msgInfo->MessageVersion != kWeaveMessageVersion_V2)
    {
        ExitNow(err = WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION);
    }

    // Decode the message id.
    msgInfo->MessageId = LittleEndian::Read32(p);

    // Decode the source node identifier if included in the message.
    if (msgInfo->Flags & kWeaveMessageFlag_SourceNodeId)
    {
        if ((p + 8) > msgEnd)
        {
            ExitNow(err = WEAVE_ERROR_INVALID_MESSAGE_LENGTH);
        }
        msgInfo->SourceNodeId = LittleEndian::Read64(p);
    }

    // Decode the destination node identifier if included in the message.
    if (msgInfo->Flags & kWeaveMessageFlag_DestNodeId)
    {
        if ((p + 8) > msgEnd)
        {
            ExitNow(err = WEAVE_ERROR_INVALID_MESSAGE_LENGTH);
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
    if (msgInfo->EncryptionType != kWeaveEncryptionType_None)
    {
        if ((p + kKeyIdLen) > msgEnd)
        {
            ExitNow(err = WEAVE_ERROR_INVALID_MESSAGE_LENGTH);
        }
        msgInfo->KeyId = LittleEndian::Read16(p);
    }
    else
    {
        // Clear flag, which could have been accidentally set in the older version of code only for unencrypted messages.
        msgInfo->Flags &= ~kWeaveMessageFlag_MsgCounterSyncReq;

        msgInfo->KeyId = WeaveKeyId::kNone;
    }

    if (payloadStart != NULL)
    {
        *payloadStart = p;
    }

exit:
    return err;
}

WEAVE_ERROR WeaveMessageLayer::ReEncodeMessage(PacketBuffer *msgBuf)
{
    WeaveMessageInfo msgInfo;
    WEAVE_ERROR err;
    uint8_t *p;
    WeaveSessionState sessionState;
    uint16_t msgLen = msgBuf->DataLength();
    uint8_t *msgStart = msgBuf->Start();
    uint16_t encryptionLen;

    msgInfo.Clear();
    msgInfo.SourceNodeId = kNodeIdNotSpecified;

    err = DecodeHeader(msgBuf, &msgInfo, &p);
    if (err != WEAVE_NO_ERROR)
        return err;

    encryptionLen = msgLen - (p - msgStart);

    err = FabricState->GetSessionState(msgInfo.SourceNodeId, msgInfo.KeyId, msgInfo.EncryptionType, NULL, sessionState);
    if (err != WEAVE_NO_ERROR)
        return err;

    switch (msgInfo.EncryptionType)
    {
    case kWeaveEncryptionType_None:
        break;

    case kWeaveEncryptionType_AES128CTRSHA1:
        {
            // TODO: re-validate MIC to ensure that no part of the message has been altered since the time it was received.

            // Re-encrypt the payload.
            AES128CTRMode aes128CTR;
            aes128CTR.SetKey(sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey);
            aes128CTR.SetWeaveMessageCounter(msgInfo.SourceNodeId, msgInfo.MessageId);
            aes128CTR.EncryptData(p, encryptionLen, p);
        }
        break;
    default:
        return WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // signature remains untouched -- we have not modified it.

    return WEAVE_NO_ERROR;
}

/**
 *  Encode a WeaveMessageLayer header into an PacketBuffer.
 *
 *  @param[in]    msgInfo       A pointer to a WeaveMessageInfo object containing information
 *                              about the message to be encoded.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer object that would hold the Weave message.
 *
 *  @param[in]    con           A pointer to the WeaveConnection object.
 *
 *  @param[in]    maxLen        The maximum length of the encoded Weave message.
 *
 *  @param[in]    reserve       The reserved space before the payload to hold the Weave message header.
 *
 *  @retval  #WEAVE_NO_ERROR    on successful encoding of the message.
 *  @retval  #WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION  if the Weave Message header format version is
 *                                                     not supported.
 *  @retval  #WEAVE_ERROR_INVALID_MESSAGE_LENGTH       if the payload length in the message buffer is zero.
 *  @retval  #WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE  if the encryption type in the message header is not
 *                                                     supported.
 *  @retval  #WEAVE_ERROR_MESSAGE_TOO_LONG             if the encoded message would be longer than the
 *                                                     requested maximum.
 *  @retval  #WEAVE_ERROR_BUFFER_TOO_SMALL             if there is not enough space before or after the
 *                                                     message payload.
 *  @retval  other errors generated by the fabric state object when fetching the session state.
 *
 */
WEAVE_ERROR WeaveMessageLayer::EncodeMessage(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf, WeaveConnection *con,
        uint16_t maxLen, uint16_t reserve)
{
    WEAVE_ERROR err;
    uint8_t *p1;
    // Error if an unsupported message version requested.
    if (msgInfo->MessageVersion != kWeaveMessageVersion_V1 &&
        msgInfo->MessageVersion != kWeaveMessageVersion_V2)
        return WEAVE_ERROR_UNSUPPORTED_MESSAGE_VERSION;

    // Message already encoded, don't do anything
    if (msgInfo->Flags & kWeaveMessageFlag_MessageEncoded)
    {
        WeaveMessageInfo existingMsgInfo;
        existingMsgInfo.Clear();
        err = DecodeHeader(msgBuf, &existingMsgInfo, &p1);
        if (err != WEAVE_NO_ERROR)
        {
            return err;
        }
        msgInfo->DestNodeId = existingMsgInfo.DestNodeId;
        return WEAVE_NO_ERROR;
    }

    // Compute the number of bytes that will appear before and after the message payload
    // in the final encoded message.
    uint16_t headLen = 6;
    uint16_t tailLen = 0;
    uint16_t payloadLen = msgBuf->DataLength();
    if (msgInfo->Flags & kWeaveMessageFlag_SourceNodeId)
        headLen += 8;
    if (msgInfo->Flags & kWeaveMessageFlag_DestNodeId)
        headLen += 8;
    switch (msgInfo->EncryptionType)
    {
    case kWeaveEncryptionType_None:
        break;
    case kWeaveEncryptionType_AES128CTRSHA1:
        // Can only encrypt non-zero length payloads.
        if (payloadLen == 0)
            return WEAVE_ERROR_INVALID_MESSAGE_LENGTH;
        headLen += 2;
        tailLen += HMACSHA1::kDigestLength;
        break;
    default:
        return WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // Error if the encoded message would be longer than the requested maximum.
    if ((headLen + msgBuf->DataLength() + tailLen) > maxLen)
        return WEAVE_ERROR_MESSAGE_TOO_LONG;

    // Ensure there's enough room before the payload to hold the message header.
    // Return an error if there's not enough room in the buffer.
    if (!msgBuf->EnsureReservedSize(headLen + reserve))
        return WEAVE_ERROR_BUFFER_TOO_SMALL;

    // Error if not enough space after the message payload.
    if ((msgBuf->DataLength() + tailLen) > msgBuf->MaxDataLength())
        return WEAVE_ERROR_BUFFER_TOO_SMALL;

    uint8_t *payloadStart = msgBuf->Start();

    // Get the session state for the given destination node and encryption key.
    WeaveSessionState sessionState;

    if (msgInfo->DestNodeId == kAnyNodeId)
    {
        err = FabricState->GetSessionState(msgInfo->SourceNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    }
    else
    {
        err = FabricState->GetSessionState(msgInfo->DestNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    }
    if (err != WEAVE_NO_ERROR)
        return err;

    // Starting encoding at the appropriate point in the buffer before the payload data.
    uint8_t *p = payloadStart - headLen;

    // Allocate a new message identifier and write the message identifier field.
    if ((msgInfo->Flags & kWeaveMessageFlag_ReuseMessageId) == 0)
        msgInfo->MessageId = sessionState.NewMessageId();

#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    // Request message counter synchronization if peer group key counter is not synchronized.
    if (sessionState.MessageIdNotSynchronized() && WeaveKeyId::IsAppGroupKey(msgInfo->KeyId))
    {
        // Set the flag.
        msgInfo->Flags |= kWeaveMessageFlag_MsgCounterSyncReq;

        // Update fabric state.
        FabricState->OnMsgCounterSyncReqSent(msgInfo->MessageId);
    }
#endif

    // Adjust the buffer so that the start points to the start of the encoded message.
    msgBuf->SetStart(p);

    // Encode and verify the header field.
    uint16_t headerField = EncodeHeaderField(msgInfo);
    if ((headerField & kMsgHeaderField_ReservedFlagsMask) != 0)
        return WEAVE_ERROR_INVALID_ARGUMENT;

    // Write the header field.
    LittleEndian::Write16(p, headerField);

    if (msgInfo->DestNodeId == kAnyNodeId)
    {
        sessionState.IsDuplicateMessage(msgInfo->MessageId);
    }

    LittleEndian::Write32(p, msgInfo->MessageId);

    // If specified, encode the source node id.
    if (msgInfo->Flags & kWeaveMessageFlag_SourceNodeId)
    {
        LittleEndian::Write64(p, msgInfo->SourceNodeId);
    }

    // If specified, encode the destination node id.
    if (msgInfo->Flags & kWeaveMessageFlag_DestNodeId)
    {
        LittleEndian::Write64(p, msgInfo->DestNodeId);
    }

    switch (msgInfo->EncryptionType)
    {
    case kWeaveEncryptionType_None:
        // If no encryption requested, skip over the payload in the message buffer.
        p += payloadLen;
        break;

    case kWeaveEncryptionType_AES128CTRSHA1:
        // Encode the key id.
        LittleEndian::Write16(p, msgInfo->KeyId);

        // At this point we've completed encoding the head of the message (and therefore p == payloadStart),
        // so skip over the payload data.
        p += payloadLen;

        // Compute the integrity check value and store it immediately after the payload data.
        ComputeIntegrityCheck_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.IntegrityKey,
                                            payloadStart, payloadLen, p);
        p += HMACSHA1::kDigestLength;

        // Encrypt the message payload and the integrity check value that follows it, in place, in the message buffer.
        Encrypt_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey,
                              payloadStart, payloadLen + HMACSHA1::kDigestLength, payloadStart);

        break;
    }

    msgInfo->Flags |= kWeaveMessageFlag_MessageEncoded;
    // Update the buffer length to reflect the entire encoded message.
    msgBuf->SetDataLength(headLen + payloadLen + tailLen);

    // We update the cursor (p) out of good hygiene,
    // such that if the code is extended in the future such that the cursor is used,
    // it will be in the correct position for such code.
    IgnoreUnusedVariable(p);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR WeaveMessageLayer::DecodeMessage(PacketBuffer *msgBuf, uint64_t sourceNodeId, WeaveConnection *con,
        WeaveMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen) // TODO: use references
{
    WEAVE_ERROR err;
    uint8_t *msgStart = msgBuf->Start();
    uint16_t msgLen = msgBuf->DataLength();
    uint8_t *msgEnd = msgStart + msgLen;
    uint8_t *p = msgStart;
    msgInfo->SourceNodeId = sourceNodeId;
    err = DecodeHeader(msgBuf, msgInfo, &p);
    sourceNodeId = msgInfo->SourceNodeId;

    if (err != WEAVE_NO_ERROR)
        return err;

    // Get the session state for the given source node and encryption key.
    WeaveSessionState sessionState;

    err = FabricState->GetSessionState(sourceNodeId, msgInfo->KeyId, msgInfo->EncryptionType, con, sessionState);
    if (err != WEAVE_NO_ERROR)
        return err;

    switch (msgInfo->EncryptionType)
    {
    case kWeaveEncryptionType_None:
        // Return the position and length of the payload within the message.
        *rPayloadLen = msgLen - (p - msgStart);
        *rPayload = p;

        // Skip over the payload.
        p += *rPayloadLen;
        break;

    case kWeaveEncryptionType_AES128CTRSHA1:
    {
        // Error if the message is short given the expected fields.
        if ((p + kMinPayloadLen + HMACSHA1::kDigestLength) > msgEnd)
            return WEAVE_ERROR_INVALID_MESSAGE_LENGTH;

        // Return the position and length of the payload within the message.
        uint16_t payloadLen = msgLen - ((p - msgStart) + HMACSHA1::kDigestLength);
        *rPayloadLen = payloadLen;
        *rPayload = p;

        // Decrypt the message payload and the integrity check value that follows it, in place, in the message buffer.
        Encrypt_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.DataKey,
                              p, payloadLen + HMACSHA1::kDigestLength, p);

        // Compute the expected integrity check value from the decrypted payload.
        uint8_t expectedIntegrityCheck[HMACSHA1::kDigestLength];
        ComputeIntegrityCheck_AES128CTRSHA1(msgInfo, sessionState.MsgEncKey->EncKey.AES128CTRSHA1.IntegrityKey,
                                            p, payloadLen, expectedIntegrityCheck);
        // Error if the expected integrity check doesn't match the integrity check in the message.
        if (!ConstantTimeCompare(p + payloadLen, expectedIntegrityCheck, HMACSHA1::kDigestLength))
            return WEAVE_ERROR_INTEGRITY_CHECK_FAILED;
        // Skip past the payload and the integrity check value.
        p += payloadLen + HMACSHA1::kDigestLength;

        break;
    }

    default:
        return WEAVE_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
    }

    // Set flag in the message header indicating that the message is a duplicate if:
    //  - A message with the same message identifier has already been received from that peer.
    //  - This is the first message from that peer encrypted with application keys.
    if (sessionState.IsDuplicateMessage(msgInfo->MessageId))
        msgInfo->Flags |= kWeaveMessageFlag_DuplicateMessage;

#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    // Set flag if peer group key message counter is not synchronized.
    if (sessionState.MessageIdNotSynchronized() && WeaveKeyId::IsAppGroupKey(msgInfo->KeyId))
        msgInfo->Flags |= kWeaveMessageFlag_PeerGroupMsgIdNotSynchronized;
#endif

    // Pass the peer authentication mode back to the application via the weave message header structure.
    msgInfo->PeerAuthMode = sessionState.AuthMode;

    return err;
}

WEAVE_ERROR WeaveMessageLayer::EncodeMessageWithLength(WeaveMessageInfo *msgInfo, PacketBuffer *msgBuf,
        WeaveConnection *con, uint16_t maxLen)
{
    // Encode the message, reserving 2 bytes for the length.
    WEAVE_ERROR err = EncodeMessage(msgInfo, msgBuf, con, maxLen - 2, 2);
    if (err != WEAVE_NO_ERROR)
        return err;

    // Prepend the message length to the beginning of the message.
    uint8_t * newMsgStart = msgBuf->Start() - 2;
    uint16_t msgLen = msgBuf->DataLength();
    msgBuf->SetStart(newMsgStart);
    LittleEndian::Put16(newMsgStart, msgLen);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR WeaveMessageLayer::DecodeMessageWithLength(PacketBuffer *msgBuf, uint64_t sourceNodeId, WeaveConnection *con,
        WeaveMessageInfo *msgInfo, uint8_t **rPayload, uint16_t *rPayloadLen, uint32_t *rFrameLen)
{
    uint8_t *dataStart = msgBuf->Start();
    uint16_t dataLen = msgBuf->DataLength();

    // Error if the message buffer doesn't contain the entire message length field.
    if (dataLen < 2)
    {
        *rFrameLen = 8; // Assume absolute minimum frame length.
        return WEAVE_ERROR_MESSAGE_INCOMPLETE;
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
            return WEAVE_ERROR_MESSAGE_TOO_LONG;
        return WEAVE_ERROR_MESSAGE_INCOMPLETE;
    }

    // Adjust the message buffer to point at the message, not including the message length field that precedes it,
    // and not including any data that may follow it.
    msgBuf->SetStart(dataStart + 2);
    msgBuf->SetDataLength(msgLen);

    // Decode the message.
    WEAVE_ERROR err = DecodeMessage(msgBuf, sourceNodeId, con, msgInfo, rPayload, rPayloadLen);

    // If successful, adjust the message buffer to point at any remaining data beyond the end of the message.
    // (This may in fact represent another message).
    if (err == WEAVE_NO_ERROR)
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

void WeaveMessageLayer::HandleUDPMessage(UDPEndPoint *endPoint, PacketBuffer *msg, const IPPacketInfo *pktInfo)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    WeaveMessageLayer *msgLayer = (WeaveMessageLayer *) endPoint->AppState;
    WeaveMessageInfo msgInfo;
    uint64_t sourceNodeId;
    uint8_t *payload;
    uint16_t payloadLen;

    WEAVE_FAULT_INJECT(FaultInjection::kFault_DropIncomingUDPMsg,
                       PacketBuffer::Free(msg);
                       ExitNow(err = WEAVE_NO_ERROR));

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
        err = WEAVE_ERROR_INVALID_ADDRESS;

    if (err == WEAVE_NO_ERROR)
    {
        // If the source address is a ULA, derive a node identifier from it.  Depending on what's in the
        // message header, this may in fact be the node identifier of the sending node.
        sourceNodeId = (pktInfo->SrcAddress.IsIPv6ULA()) ? IPv6InterfaceIdToWeaveNodeId(pktInfo->SrcAddress.InterfaceId()) : kNodeIdNotSpecified;

        // Attempt to decode the message.
        err = msgLayer->DecodeMessage(msg, sourceNodeId, NULL, &msgInfo, &payload, &payloadLen);

        if (err == WEAVE_NO_ERROR)
        {
            // Set the message buffer to point at the payload data.
            msg->SetStart(payload);
            msg->SetDataLength(payloadLen);
        }
    }

    // Verify that destination node identifier refers to the local node.
    if (err == WEAVE_NO_ERROR)
    {
        if (msgInfo.DestNodeId != msgLayer->FabricState->LocalNodeId && msgInfo.DestNodeId != kAnyNodeId)
            err = WEAVE_ERROR_INVALID_DESTINATION_NODE_ID;
    }

    // If an error occurred, discard the message and call the on receive error handler.
    SuccessOrExit(err);

    // Record whether the message was sent to the local node's ephemeral port.
#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
    SetFlag(msgInfo.Flags, kWeaveMessageFlag_ViaEphemeralUDPPort,
            (endPoint == msgLayer->mIPv6EphemeralUDP
#if INET_CONFIG_ENABLE_IPV4
             || endPoint == msgLayer->mIPv4EphemeralUDP
#endif // INET_CONFIG_ENABLE_IPV4
            ));
#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    //Check if message carries tunneled data and needs to be sent to Tunnel Agent
    if (msgInfo.MessageVersion == kWeaveMessageVersion_V2)
    {
        if (msgInfo.Flags & kWeaveMessageFlag_TunneledData)
        {
#if WEAVE_CONFIG_ENABLE_TUNNELING
            // Policy for handling duplicate tunneled UDP message:
            //  - Eliminate duplicate tunneled encrypted messages to prevent replay of messages by
            //    a malicious man-in-the-middle.
            //  - Handle duplicate tunneled unencrypted message.
            // Dispatch the tunneled data message to the application if it is not a duplicate or unencrypted.
            if (!(msgInfo.Flags & kWeaveMessageFlag_DuplicateMessage) || msgInfo.KeyId == WeaveKeyId::kNone)
            {
                if (msgLayer->OnUDPTunneledMessageReceived)
                {
                    msgLayer->OnUDPTunneledMessageReceived(msgLayer, msg);
                }
                else
                {
                    ExitNow(err = WEAVE_ERROR_NO_MESSAGE_HANDLER);
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
                ExitNow(err = WEAVE_ERROR_NO_MESSAGE_HANDLER);
            }
        }
    }
    else if (msgInfo.MessageVersion == kWeaveMessageVersion_V1)
    {
        // Call the supplied OnMessageReceived callback.
        if (msgLayer->OnMessageReceived != NULL)
            msgLayer->OnMessageReceived(msgLayer, &msgInfo, msg);
        else
        {
            ExitNow(err = WEAVE_ERROR_NO_MESSAGE_HANDLER);
        }
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(MessageLayer, "HandleUDPMessage Error %s", nl::ErrorStr(err));

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

void WeaveMessageLayer::HandleUDPReceiveError(UDPEndPoint *endPoint, INET_ERROR err, const IPPacketInfo *pktInfo)
{
    WeaveLogError(MessageLayer, "HandleUDPReceiveError Error %s", nl::ErrorStr(err));

    WeaveMessageLayer *msgLayer = (WeaveMessageLayer *) endPoint->AppState;
    if (msgLayer->OnReceiveError != NULL)
        msgLayer->OnReceiveError(msgLayer, err, pktInfo);
}

#if CONFIG_NETWORK_LAYER_BLE
void WeaveMessageLayer::HandleIncomingBleConnection(BLEEndPoint *bleEP)
{
    WeaveMessageLayer *msgLayer = (WeaveMessageLayer *) bleEP->mAppState;

    // Immediately close the connection if there's no callback registered.
    if (msgLayer->OnConnectionReceived == NULL && msgLayer->ExchangeMgr == NULL)
    {
        bleEP->Close();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, WEAVE_ERROR_NO_CONNECTION_HANDLER);
        return;
    }

    // Attempt to allocate a connection object. Fail if too many connections.
    WeaveConnection *con = msgLayer->NewConnection();
    if (con == NULL)
    {
        bleEP->Close();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, WEAVE_ERROR_TOO_MANY_CONNECTIONS);
        return;
    }

    // Setup the connection object.
    con->MakeConnectedBle(bleEP);

#if WEAVE_PROGRESS_LOGGING
    {
        WeaveLogProgress(MessageLayer, "WoBle con rcvd");
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

void WeaveMessageLayer::HandleIncomingTcpConnection(TCPEndPoint *listeningEP, TCPEndPoint *conEP, const IPAddress &peerAddr, uint16_t peerPort)
{
    INET_ERROR err;
    IPAddress localAddr;
    uint16_t localPort;
    uint16_t incomingTCPConCount;
    uint16_t incomingTCPConCountFromIP;
    WeaveMessageLayer *msgLayer = (WeaveMessageLayer *) listeningEP->AppState;

    // Immediately close the connection if there's no callback registered.
    if (msgLayer->OnConnectionReceived == NULL && msgLayer->ExchangeMgr == NULL)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, WEAVE_ERROR_NO_CONNECTION_HANDLER);
        return;
    }

    // Fail if too many incoming TCP connections.
    msgLayer->GetIncomingTCPConCount(peerAddr, incomingTCPConCount, incomingTCPConCountFromIP);
    if (incomingTCPConCount == WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS ||
        incomingTCPConCountFromIP == WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, WEAVE_ERROR_TOO_MANY_CONNECTIONS);
        return;
    }

    // Attempt to allocate a connection object. Fail if too many connections.
    WeaveConnection *con = msgLayer->NewConnection();
    if (con == NULL)
    {
        conEP->Free();
        if (msgLayer->OnAcceptError != NULL)
            msgLayer->OnAcceptError(msgLayer, WEAVE_ERROR_TOO_MANY_CONNECTIONS);
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

#if WEAVE_PROGRESS_LOGGING
    {
        char ipAddrStr[64];
        peerAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
        WeaveLogProgress(MessageLayer, "Con %s %04" PRIX16 " %s %d", "rcvd", con->LogId(), ipAddrStr, (int)peerPort);
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
    if (msgLayer->OnUnsecuredConnectionReceived != NULL && conEP->GetLocalInfo(&localAddr, &localPort) ==
            WEAVE_NO_ERROR && localPort == WEAVE_UNSECURED_PORT)
        msgLayer->OnUnsecuredConnectionReceived(msgLayer, con);

}

void WeaveMessageLayer::HandleAcceptError(TCPEndPoint *ep, INET_ERROR err)
{
    WeaveMessageLayer *msgLayer = (WeaveMessageLayer *) ep->AppState;
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
 *  @retval #WEAVE_NO_ERROR on successful refreshing of endpoints.
 *  @retval InetLayer errors based on calls to create TCP/UDP endpoints.
 *
 */
WEAVE_ERROR WeaveMessageLayer::RefreshEndpoints()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    const bool listenIPv6 = IPv6ListenEnabled();
#if INET_CONFIG_ENABLE_IPV4
    const bool listenIPv4 = IPv4ListenEnabled();
#endif // INET_CONFIG_ENABLE_IPV4

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

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

#else // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

    IPAddress & listenIPv6Addr = IPAddress::Any;
    InterfaceId listenIPv6Intf = INET_NULL_INTERFACEID;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress & listenIPv4Addr = IPAddress::Any;
#endif // INET_CONFIG_ENABLE_IPV4

#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

    // ================================================================================
    // Enable / disable TCP listening endpoints...
    // ================================================================================

    {
        const bool listenTCP = TCPListenEnabled();
        const bool listenIPv6TCP = (listenTCP && listenIPv6);

#if INET_CONFIG_ENABLE_IPV4

        const bool listenIPv4TCP = (listenTCP && listenIPv4);

        // Enable / disable the Weave IPv4 TCP listening endpoint
        //
        // The Weave IPv4 TCP listening endpoint is use to listen for incoming IPv4 TCP connections
        // to the local node's Weave port.
        //
        err = RefreshEndpoint(mIPv4TCPListen, listenIPv4TCP,
                "Weave IPv4 TCP listen", kIPAddressType_IPv4, listenIPv4Addr, WEAVE_PORT);
        SuccessOrExit(err);

#endif // INET_CONFIG_ENABLE_IPV4

        // Enable / disable the Weave IPv6 TCP listening endpoint
        //
        // The Weave IPv6 TCP listening endpoint is use to listen for incoming IPv6 TCP connections
        // to the local node's Weave port.
        //
        err = RefreshEndpoint(mIPv6TCPListen, listenIPv6TCP,
                "Weave IPv6 TCP listen", kIPAddressType_IPv6, listenIPv6Addr, WEAVE_PORT);
        SuccessOrExit(err);

#if WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

        // Enable / disable the Unsecured IPv6 TCP listening endpoint
        //
        // The Unsecured IPv6 TCP listening endpoint is use to listen for incoming IPv6 TCP connections
        // to the local node's unsecured Weave port.  This endpoint is only enabled if the unsecured TCP
        // listen feature has been enabled.
        //
        const bool listenUnsecuredIPv6TCP = (listenIPv6TCP && UnsecuredListenEnabled());
        err = RefreshEndpoint(mUnsecuredIPv6TCPListen, listenUnsecuredIPv6TCP,
                "unsecured IPv6 TCP listen", kIPAddressType_IPv6, listenIPv6Addr, WEAVE_UNSECURED_PORT);
        SuccessOrExit(err);

#endif // WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

    }

    // ================================================================================
    // Enable / disable UDP endpoints...
    // ================================================================================

    {
        const bool listenUDP = UDPListenEnabled();

#if INET_CONFIG_ENABLE_IPV4

        // Enabled / disable the Weave IPv4 UDP endpoint as necessary.
        //
        // The Weave IPv4 UDP endpoint is used to listen for unsolicited IPv4 UDP Weave messages sent
        // to the local node's Weave port.  Is is also used by the local node to send IPv4 UDP Weave
        // messages to other nodes, and to receive their replies, unless the outbound ephemeral UDP port
        // feature has been enabled.
        //
        const bool listenIPv4UDP = (listenIPv4 && listenUDP);
        err = RefreshEndpoint(mIPv4UDP, listenIPv4UDP,
                "Weave IPv4 UDP", kIPAddressType_IPv4, listenIPv4Addr, WEAVE_PORT, INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

        // Enabled / disable the ephemeral IPv4 UDP endpoint as necessary.
        //
        // The ephemeral IPv4 UDP endpoint is used to send IPv4 UDP Weave messages to other nodes and to
        // receive their replies.  It is only enabled when the outbound ephemeral UDP port feature has been
        // enabled.
        //
        const bool listenIPv4EphemeralUDP = (listenIPv4UDP && EphemeralUDPPortEnabled());
        err = RefreshEndpoint(mIPv4EphemeralUDP, listenIPv4EphemeralUDP,
                "ephemeral IPv4 UDP", kIPAddressType_IPv4, listenIPv4Addr, 0, INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#endif // INET_CONFIG_ENABLE_IPV4

        // Enabled / disable the Weave IPv6 UDP endpoint as necessary.
        //
        // The Weave IPv6 UDP endpoint is used to listen for unsolicited IPv6 UDP Weave messages sent
        // to the local node's Weave port.  Is is also used by the local node to send IPv6 UDP Weave
        // messages to other nodes, and to receive their replies, unless the outbound ephemeral UDP port
        // feature has been enabled.
        //
        const bool listenIPv6UDP = (listenIPv6 && listenUDP);
        err = RefreshEndpoint(mIPv6UDP, listenIPv6UDP,
                "Weave IPv6 UDP", kIPAddressType_IPv6, listenIPv6Addr, WEAVE_PORT, listenIPv6Intf);
        SuccessOrExit(err);

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

        // Enabled / disable the ephemeral IPv6 UDP endpoint as necessary.
        //
        // The ephemeral IPv6 UDP endpoint is used to send IPv6 UDP Weave messages to other nodes and to
        // receive their replies.  It is only enabled when the outbound ephemeral UDP port feature has been
        // enabled.
        //
        const bool listenIPv6EphemeralUDP = (listenIPv6UDP && EphemeralUDPPortEnabled());
        err = RefreshEndpoint(mIPv6EphemeralUDP, listenIPv6EphemeralUDP,
                "ephemeral IPv6 UDP", kIPAddressType_IPv6, listenIPv6Addr, 0, listenIPv6Intf);
        SuccessOrExit(err);

#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

        // Enable / disable the Weave IPv6 UDP multicast endpoint.
        //
        // The Weave IPv6 UDP multicast endpoint is used to listen for unsolicited IPv6 UDP Weave messages sent
        // to the all-nodes, link-local multicast address. It is only enabled when the message layer has been bound
        // to a specific IPv6 address.  This is required because the Weave IPv6 UDP endpoint will not receive multicast
        // messages in this configuration.
        //
        IPAddress ipv6LinkLocalAllNodes = IPAddress::MakeIPv6WellKnownMulticast(kIPv6MulticastScope_Link, kIPV6MulticastGroup_AllNodes);
        const bool listenWeaveIPv6UDPMulticastEP = (listenIPv6UDP && IsBoundToLocalIPv6Address());
        err = RefreshEndpoint(mIPv6UDPMulticastRcv, listenWeaveIPv6UDPMulticastEP,
                "Weave IPv6 UDP multicast", kIPAddressType_IPv6, ipv6LinkLocalAllNodes, WEAVE_PORT, listenIPv6Intf);
        SuccessOrExit(err);

#if INET_CONFIG_ENABLE_IPV4

        // Enable / disable the Weave IPv4 UDP broadcast endpoint.
        //
        // Similar to the IPv6 UDP multicast endpoint, this endpoint is used to receive IPv4 broadcast messages
        // when the message layer has been bound to a specific IPv4 address.
        //
        IPAddress ipv4Broadcast = IPAddress::MakeIPv4Broadcast();
        const bool listenWeaveIPv4UDPBroadcastEP = (listenIPv4UDP && IsBoundToLocalIPv4Address());
        err = RefreshEndpoint(mIPv4UDPBroadcastRcv, listenWeaveIPv4UDPBroadcastEP,
                "Weave IPv4 UDP broadcast", kIPAddressType_IPv4, ipv4Broadcast, WEAVE_PORT, INET_NULL_INTERFACEID);
        SuccessOrExit(err);

#endif // INET_CONFIG_ENABLE_IPV4

#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

    }

exit:
    if (err != WEAVE_NO_ERROR)
        WeaveBindLog("RefreshEndpoints failed: %s", ErrorStr(err));
    return err;
}

WEAVE_ERROR WeaveMessageLayer::RefreshEndpoint(TCPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

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
        endPoint->AppState = this;
        endPoint->OnConnectionReceived = HandleIncomingTcpConnection;
        endPoint->OnAcceptError = HandleAcceptError;
        err = endPoint->Listen(1);
        SuccessOrExit(err);

#if WEAVE_BIND_DETAIL_LOGGING && WEAVE_DETAIL_LOGGING
        {
            char ipAddrStr[64];
            addr.ToString(ipAddrStr, sizeof(ipAddrStr));
            WeaveBindLog("Listening on %s endpoint ([%s]:%" PRIu16 ")", name, ipAddrStr, port);
        }
#endif // WEAVE_BIND_DETAIL_LOGGING && WEAVE_DETAIL_LOGGING
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        if (endPoint != NULL)
        {
            endPoint->Free();
            endPoint = NULL;
        }
        WeaveLogError(MessageLayer, "Error initializing %s endpoint: %s", name, ErrorStr(err));
    }
    return err;
}

WEAVE_ERROR WeaveMessageLayer::RefreshEndpoint(UDPEndPoint *& endPoint, bool enable, const char * name, IPAddressType addrType, IPAddress addr, uint16_t port, InterfaceId intfId)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

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
        endPoint->AppState = this;
        endPoint->OnMessageReceived = reinterpret_cast<IPEndPointBasis::OnMessageReceivedFunct>(HandleUDPMessage);
        endPoint->OnReceiveError = reinterpret_cast<IPEndPointBasis::OnReceiveErrorFunct>(HandleUDPReceiveError);
        err = endPoint->Listen();
        SuccessOrExit(err);

#if WEAVE_BIND_DETAIL_LOGGING && WEAVE_DETAIL_LOGGING
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
            WeaveBindLog("Listening on %s endpoint ([%s]:%" PRIu16 "%s)", name, ipAddrStr, endPoint->GetBoundPort(), intfStr);
        }
#endif // WEAVE_BIND_DETAIL_LOGGING && WEAVE_DETAIL_LOGGING
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        if (endPoint != NULL)
        {
            endPoint->Free();
            endPoint = NULL;
        }
        WeaveLogError(MessageLayer, "Error initializing %s endpoint: %s", name, ErrorStr(err));
    }
    return err;
}

void WeaveMessageLayer::Encrypt_AES128CTRSHA1(const WeaveMessageInfo *msgInfo, const uint8_t *key,
                                              const uint8_t *inData, uint16_t inLen, uint8_t *outBuf)
{
    AES128CTRMode aes128CTR;
    aes128CTR.SetKey(key);
    aes128CTR.SetWeaveMessageCounter(msgInfo->SourceNodeId, msgInfo->MessageId);
    aes128CTR.EncryptData(inData, inLen, outBuf);
}

void WeaveMessageLayer::ComputeIntegrityCheck_AES128CTRSHA1(const WeaveMessageInfo *msgInfo, const uint8_t *key,
                                                            const uint8_t *inData, uint16_t inLen, uint8_t *outBuf)
{
    HMACSHA1 hmacSHA1;
    uint8_t encodedBuf[2 * sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint32_t)];
    uint8_t *p = encodedBuf;

    // Initialize HMAC Key.
    hmacSHA1.Begin(key, WeaveEncryptionKey_AES128CTRSHA1::IntegrityKeySize);

    // Encode the source and destination node identifiers in a little-endian format.
    Encoding::LittleEndian::Write64(p, msgInfo->SourceNodeId);
    Encoding::LittleEndian::Write64(p, msgInfo->DestNodeId);

    // Hash the message header field and the message Id for the message version V2.
    if (msgInfo->MessageVersion == kWeaveMessageVersion_V2)
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
 *  open WeaveConnections and shutdown any open
 *  WeaveConnectionTunnel objects.
 *
 *  @note
 *    A call to CloseEndpoints() terminates all communication
 *    channels within the WeaveMessageLayer but does not terminate
 *    the WeaveMessageLayer object.
 *
 *  @sa Shutdown().
 *
 */
WEAVE_ERROR WeaveMessageLayer::CloseEndpoints()
{
    // Close all endpoints used for listening.
    CloseListeningEndpoints();

    // Abort any open connections.
    WeaveConnection *con = static_cast<WeaveConnection *>(mConPool);
    for (int i = 0; i < WEAVE_CONFIG_MAX_CONNECTIONS; i++, con++)
        if (con->mRefCount > 0)
            con->Abort();

    // Shut down any open tunnels.
    WeaveConnectionTunnel *tun = static_cast<WeaveConnectionTunnel *>(mTunnelPool);
    for (int i = 0; i < WEAVE_CONFIG_MAX_TUNNELS; i++, tun++)
    {
        if (tun->mMessageLayer != NULL)
        {
            // Suppress callback as we're shutting down the whole stack.
            tun->OnShutdown = NULL;
            tun->Shutdown();
        }
    }

    return WEAVE_NO_ERROR;
}

void WeaveMessageLayer::CloseListeningEndpoints(void)
{
    WeaveBindLog("Closing endpoints");

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

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    if (mIPv6EphemeralUDP != NULL)
    {
        mIPv6EphemeralUDP->Free();
        mIPv6EphemeralUDP = NULL;
    }

#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

    if (mIPv6UDPMulticastRcv != NULL)
    {
        mIPv6UDPMulticastRcv->Free();
        mIPv6UDPMulticastRcv = NULL;
    }

#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

#if WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

    if (mUnsecuredIPv6TCPListen != NULL)
    {
        mUnsecuredIPv6TCPListen->Free();
        mUnsecuredIPv6TCPListen = NULL;
    }

#endif // WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

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

#if WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

    if (mIPv4EphemeralUDP != NULL)
    {
        mIPv4EphemeralUDP->Free();
        mIPv4EphemeralUDP = NULL;
    }

#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

    if (mIPv4UDPBroadcastRcv != NULL)
    {
        mIPv4UDPBroadcastRcv->Free();
        mIPv4UDPBroadcastRcv = NULL;
    }

#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

#endif // INET_CONFIG_ENABLE_IPV4
}

WEAVE_ERROR WeaveMessageLayer::EnableUnsecuredListen()
{
    // Enable reception of connections on the unsecured Weave port. This allows devices to establish
    // a connection while provisionally connected (i.e. without security) at the network layer.
    SetFlag(mFlags, kFlag_ListenUnsecured);
    return RefreshEndpoints();
}

WEAVE_ERROR WeaveMessageLayer::DisableUnsecuredListen()
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
 *  are any ongoing Weave conversations or pending responses.
 *  The handler must be set after the WeaveMessageLayer has been initialized;
 *  shutting down the WeaveMessageLayer will clear out the current handler.
 *
 *  @param[in] messageLayerActivityChangeHandler A pointer to a function to
 *             be called whenever the message layer activity changes.
 *
 *  @retval None.
 */
void WeaveMessageLayer::SetSignalMessageLayerActivityChanged(MessageLayerActivityChangeHandlerFunct messageLayerActivityChangeHandler)
{
    OnMessageLayerActivityChange = messageLayerActivityChangeHandler;
}

bool WeaveMessageLayer::IsMessageLayerActive(void)
{
    return (ExchangeMgr->mContextsInUse != 0)
#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
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
void WeaveMessageLayer::SignalMessageLayerActivityChanged(void)
{
    if (OnMessageLayerActivityChange)
    {
        bool messageLayerIsActive = IsMessageLayerActive();
        OnMessageLayerActivityChange(messageLayerIsActive);
    }
}

/**
 *  Get the max Weave payload size for a message configuration and supplied
 *  PacketBuffer.
 *
 *  The maximum payload size returned will not exceed the available space
 *  for a payload inside the supplied PacketBuffer.
 *
 *  If the message is UDP, the maximum payload size returned will not result in
 *  a Weave message that will not overflow the specified UDP MTU.
 *
 *  Finally, the maximum payload size returned will not result in a Weave
 *  message that will overflow the max Weave message size.
 *
 *  @param[in]    msgBuf        A pointer to the PacketBuffer to which the message
 *                              payload will be written.
 *
 *  @param[in]    isUDP         True if message is a UDP message.
 *
 *  @param[in]    udpMTU        The size of the UDP MTU. Ignored if isUDP is false.
 *
 *  @return the max Weave payload size.
 */
uint32_t WeaveMessageLayer::GetMaxWeavePayloadSize(const PacketBuffer *msgBuf, bool isUDP, uint32_t udpMTU)
{
    uint32_t maxWeaveMessageSize = isUDP ? udpMTU - INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE : UINT16_MAX;
    uint32_t maxWeavePayloadSize = maxWeaveMessageSize - WEAVE_HEADER_RESERVE_SIZE - WEAVE_TRAILER_RESERVE_SIZE;
    uint32_t maxBufferablePayloadSize = msgBuf->AvailableDataLength() - WEAVE_TRAILER_RESERVE_SIZE;

    return maxBufferablePayloadSize < maxWeavePayloadSize
        ? maxBufferablePayloadSize
        : maxWeavePayloadSize;
}

/**
 * Constructs a string describing a peer node and its associated address / connection information.
 *
 * The generated string has the following format:
 *
 *     <node-id> ([<ip-address>]:<port>%<interface>, con <con-id>)
 *
 * @param[in] buf                       A pointer to a buffer into which the string should be written. The supplied
 *                                      buffer should be at least as big as kWeavePeerDescription_MaxLength. If a
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
 * @param[in] con                       A pointer to a WeaveConnection object whose logging id should be printed;
 *                                      or NULL if no connection id should be printed.
 */
void WeaveMessageLayer::GetPeerDescription(char * buf, size_t bufSize, uint64_t nodeId,
    const IPAddress * addr, uint16_t port, InterfaceId interfaceId, const WeaveConnection * con)
{
    enum { kMaxInterfaceNameLength = 20 }; // Arbitrarily capped at 20 characters so long interface
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
            char interfaceName[kMaxInterfaceNameLength+1];
            Inet::GetInterfaceName(interfaceId, interfaceName, sizeof(interfaceName));
            interfaceName[kMaxInterfaceNameLength] = 0;
            len += snprintf(buf + len, bufSize - len, "%%%s", interfaceName);
            VerifyOrExit(len < bufSize, /* no-op */);
        }

        sep = ", ";
    }

    if (con != NULL)
    {
        const char *conType;
        switch (con->NetworkType)
        {
        case WeaveConnection::kNetworkType_BLE:
            conType = "ble ";
            break;
        case WeaveConnection::kNetworkType_IP:
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
 *                                      buffer should be at least as big as kWeavePeerDescription_MaxLength. If a
 *                                      smaller buffer is given the string will be truncated to fit. The output
 *                                      will include a NUL termination character in all cases.
 * @param[in] bufSize                   The size of the buffer pointed at by buf.
 * @param[in] msgInfo                   A pointer to a WeaveMessageInfo structure containing information about the message.
 *
 */
void WeaveMessageLayer::GetPeerDescription(char * buf, size_t bufSize, const WeaveMessageInfo * msgInfo)
{
    GetPeerDescription(buf, bufSize, msgInfo->SourceNodeId,
        (msgInfo->InPacketInfo != NULL) ? &msgInfo->InPacketInfo->SrcAddress : NULL,
        (msgInfo->InPacketInfo != NULL) ? msgInfo->InPacketInfo->SrcPort : 0,
        (msgInfo->InPacketInfo != NULL) ? msgInfo->InPacketInfo->Interface : INET_NULL_INTERFACEID,
        msgInfo->InCon);
}

/**
 * @brief
 *   Generate random Weave node Id.
 *
 * @details
 *   This function generates 64-bit locally unique Weave node Id. This function uses cryptographically strong
 *   random data source to guarantee uniqueness of generated value. Note that bit 57 of the generated Weave
 *   node Id is set to 1 to indicate that generated Weave node Id is locally (not globally) unique.
 *
 * @param nodeId                        A reference to the 64-bit Weave node Id.
 *
 * @retval  #WEAVE_NO_ERROR             If Weave node Id was successfully generated.
 */
NL_DLL_EXPORT WEAVE_ERROR GenerateWeaveNodeId(uint64_t & nodeId)
{
    WEAVE_ERROR err;
    uint64_t id = 0;

    while (id <= kMaxAlwaysLocalWeaveNodeId)
    {
        err = nl::Weave::Platform::Security::GetSecureRandomData(reinterpret_cast<uint8_t*>(&id), sizeof(id));
        SuccessOrExit(err);

        id &= ~kEUI64_UL_Local;
    }

    nodeId = id | kEUI64_UL_Local;

exit:
    return err;
}

} // namespace nl
} // namespace Weave
