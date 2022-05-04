/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
