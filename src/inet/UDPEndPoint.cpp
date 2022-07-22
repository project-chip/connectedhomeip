/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file implements the <tt>Inet::UDPEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with UDP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP UDP protocol
 *      control blocks, as the system is configured accordingly.
 *
 */

#include <inet/UDPEndPoint.h>

#include <inet/IPPacketInfo.h>
#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <cstring>
#include <utility>

namespace chip {
namespace Inet {

CHIP_ERROR UDPEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    if (mState != State::kReady && mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ((addr != IPAddress::Any) && (addr.Type() != IPAddressType::kAny) && (addr.Type() != addrType))
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    ReturnErrorOnFailure(BindImpl(addrType, addr, port, intfId));

    mState = State::kBound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterface(IPAddressType addrType, InterfaceId intfId)
{
    if (mState != State::kReady && mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(BindInterfaceImpl(addrType, intfId));

    mState = State::kBound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::Listen(OnMessageReceivedFunct onMessageReceived, OnReceiveErrorFunct onReceiveError, void * appState)
{
    if (mState == State::kListening)
    {
        return CHIP_NO_ERROR;
    }

    if (mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    OnMessageReceived = onMessageReceived;
    OnReceiveError    = onReceiveError;
    mAppState         = appState;

    ReturnErrorOnFailure(ListenImpl());

    mState = State::kListening;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::SendTo(const IPAddress & addr, uint16_t port, chip::System::PacketBufferHandle && msg, InterfaceId intfId)
{
    IPPacketInfo pktInfo;
    pktInfo.Clear();
    pktInfo.DestAddress = addr;
    pktInfo.DestPort    = port;
    pktInfo.Interface   = intfId;
    return SendMsg(&pktInfo, std::move(msg));
}

CHIP_ERROR UDPEndPoint::SendMsg(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    INET_FAULT_INJECT(FaultInjection::kFault_Send, return INET_ERROR_UNKNOWN_INTERFACE;);
    INET_FAULT_INJECT(FaultInjection::kFault_SendNonCritical, return CHIP_ERROR_NO_MEMORY;);

    ReturnErrorOnFailure(SendMsgImpl(pktInfo, std::move(msg)));

    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return CHIP_NO_ERROR;
}

void UDPEndPoint::Close()
{
    if (mState != State::kClosed)
    {
        mState = State::kClosed;
        CloseImpl();
    }
}

CHIP_ERROR UDPEndPoint::JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    VerifyOrReturnError(aAddress.IsMulticast(), INET_ERROR_WRONG_ADDRESS_TYPE);

    switch (aAddress.Type())
    {

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        return IPv4JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, true);
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6:
        return IPv6JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, true);

    default:
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
}

CHIP_ERROR UDPEndPoint::LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    VerifyOrReturnError(aAddress.IsMulticast(), INET_ERROR_WRONG_ADDRESS_TYPE);

    switch (aAddress.Type())
    {

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        return IPv4JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, false);
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6:
        return IPv6JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, false);

    default:
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
}

} // namespace Inet
} // namespace chip
