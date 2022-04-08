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

#include <inet/EndPointStateNetworkFramework.h>

namespace chip {
namespace Inet {

class UDPEndPointImplNetworkFramework : public UDPEndPoint, public EndPointStateNetworkFramework
{
public:
    UDPEndPointImplNetworkFramework(EndPointManager<UDPEndPoint> & endPointManager) : UDPEndPoint(endPointManager) {}

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

    nw_listener_t mListener;
    dispatch_semaphore_t mListenerSemaphore;
    dispatch_queue_t mListenerQueue;
    nw_connection_t mConnection;
    dispatch_semaphore_t mConnectionSemaphore;
    dispatch_queue_t mDispatchQueue;
    dispatch_semaphore_t mSendSemaphore;

    CHIP_ERROR ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters);
    CHIP_ERROR StartListener();
    CHIP_ERROR GetConnection(const IPPacketInfo * aPktInfo);
    CHIP_ERROR GetEndPoint(nw_endpoint_t & aEndpoint, const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort);
    CHIP_ERROR StartConnection(nw_connection_t & aConnection);
    void GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(const nw_connection_t & aConnection);
    CHIP_ERROR ReleaseListener();
    CHIP_ERROR ReleaseConnection();
    void ReleaseAll();
};

using UDPEndPointImpl = UDPEndPointImplNetworkFramework;

} // namespace Inet
} // namespace chip
