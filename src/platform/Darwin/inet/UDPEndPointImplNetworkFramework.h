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

/**
 * This file declares an implementation of Inet::UDPEndPoint using Network Framework.
 */

#pragma once

#include "EndPointStateNetworkFramework.h"
#include "UDPEndPointImplNetworkFrameworkConnection.h"
#include "UDPEndPointImplNetworkFrameworkListener.h"
#include "UDPEndPointImplNetworkFrameworkListenerGroup.h"
#include <inet/UDPEndPoint.h>

namespace chip {
namespace Inet {

class UDPEndPointImplNetworkFramework : public UDPEndPoint,
                                        public EndPointStateNetworkFramework,
                                        public Darwin::UDPEndPointImplNetworkFrameworkConnection,
                                        public Darwin::UDPEndPointImplNetworkFrameworkListener,
                                        public Darwin::UDPEndPointImplNetworkFrameworkListenerGroup
{
public:
    UDPEndPointImplNetworkFramework(EndPointManager<UDPEndPoint> & endPointManager) : UDPEndPoint(endPointManager) {}

    // UDPEndPoint overrides.
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback) override;
    InterfaceId GetBoundInterface() const override;
    uint16_t GetBoundPort() const override { return UDPEndPointImplNetworkFrameworkListener::GetBoundPort(); }
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

    class WorkFlag
    {
    public:
        void MarkDead() { mAlive = false; }
        bool IsAlive() const { return mAlive; }

    private:
        std::atomic<bool> mAlive{ true };
    };

    Platform::WeakPtr<WorkFlag> mWorkFlagWeak;
    Platform::SharedPtr<WorkFlag> mWorkFlagStrong;

    CHIP_ERROR ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters);
    nw_endpoint_t GetEndPoint(const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort,
                              InterfaceId interfaceIndex = InterfaceId::Null()) override;
    CHIP_ERROR GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(nw_connection_t aConnection);
    void StartConnectionFromListener(nw_connection_t connection) override;
    void ReleaseAll();
};

using UDPEndPointImpl = UDPEndPointImplNetworkFramework;

} // namespace Inet
} // namespace chip
