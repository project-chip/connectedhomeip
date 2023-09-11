/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inet/EndPointStateOpenThread.h>
#include <inet/UDPEndPoint.h>

#include <openthread/error.h>
#include <openthread/icmp6.h>
#include <openthread/ip6.h>
#include <openthread/netdata.h>
#include <openthread/thread.h>
#include <openthread/udp.h>

namespace chip {
namespace Inet {

extern otInstance * globalOtInstance;

class UDPEndPointImplOT : public UDPEndPoint, public EndPointStateOpenThread
{
public:
    UDPEndPointImplOT(EndPointManager<UDPEndPoint> & endPointManager) :
        UDPEndPoint(endPointManager), mBoundIntfId(InterfaceId::Null())
    {}

    // UDPEndPoint overrides.
    InterfaceId GetBoundInterface() const override;
    uint16_t GetBoundPort() const override;
    void Free() override;
    void HandleDataReceived(System::PacketBufferHandle && msg);
    void SetNativeParams(void * params);
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback) override;
    CHIP_ERROR BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId) override;

private:
    // UDPEndPoint overrides.
    CHIP_ERROR IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) override;
    CHIP_ERROR BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port, InterfaceId interfaceId) override;
    CHIP_ERROR ListenImpl() override;
    CHIP_ERROR SendMsgImpl(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg) override;
    void CloseImpl() override;
    CHIP_ERROR IPv6Bind(otUdpSocket & socket, const IPAddress & address, uint16_t port, InterfaceId interface);
    static IPPacketInfo * GetPacketInfo(const System::PacketBufferHandle & aBuffer);

    static void handleUdpReceive(void * aContext, otMessage * aMessage, const otMessageInfo * aMessageInfo);

    InterfaceId mBoundIntfId;
    uint16_t mBoundPort;
    otUdpSocket mSocket;
};

using UDPEndPointImpl = UDPEndPointImplOT;

} // namespace Inet
} // namespace chip
