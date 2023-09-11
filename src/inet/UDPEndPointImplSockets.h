/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file declares an implementation of Inet::UDPEndPoint using sockets.
 */

#pragma once

#include <inet/EndPointStateSockets.h>
#include <inet/UDPEndPoint.h>

namespace chip {
namespace Inet {

class UDPEndPointImplSockets : public UDPEndPoint, public EndPointStateSockets
{
public:
    UDPEndPointImplSockets(EndPointManager<UDPEndPoint> & endPointManager) :
        UDPEndPoint(endPointManager), mBoundIntfId(InterfaceId::Null())
    {}

    // UDPEndPoint overrides.
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback) override;
    InterfaceId GetBoundInterface() const override;
    uint16_t GetBoundPort() const override;
    void Free() override;

private:
    // UDPEndPoint overrides.
#if INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) override;
#endif // INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) override;
    CHIP_ERROR BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port, InterfaceId interfaceId) override;
    CHIP_ERROR BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId) override;
    CHIP_ERROR ListenImpl() override;
    CHIP_ERROR SendMsgImpl(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg) override;
    void CloseImpl() override;

    CHIP_ERROR GetSocket(IPAddressType addressType);
    void HandlePendingIO(System::SocketEvents events);
    static void HandlePendingIO(System::SocketEvents events, intptr_t data);

    InterfaceId mBoundIntfId;
    uint16_t mBoundPort;

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
public:
    using MulticastGroupHandler = CHIP_ERROR (*)(InterfaceId, const IPAddress &);
    static void SetJoinMulticastGroupHandler(MulticastGroupHandler handler) { sJoinMulticastGroupHandler = handler; }
    static void SetLeaveMulticastGroupHandler(MulticastGroupHandler handler) { sLeaveMulticastGroupHandler = handler; }

private:
    static MulticastGroupHandler sJoinMulticastGroupHandler;
    static MulticastGroupHandler sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
};

using UDPEndPointImpl = UDPEndPointImplSockets;

} // namespace Inet
} // namespace chip
