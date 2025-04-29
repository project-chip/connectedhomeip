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

#include <inet/Darwin/InterfacesMonitor.h>
#include <inet/EndPointStateNetworkFramework.h>

#include <map>

namespace chip {
namespace Inet {

class UDPEndPointImplNetworkFramework : public UDPEndPoint, public EndPointStateNetworkFramework, public Darwin::InterfacesMonitor
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

    dispatch_queue_t mListenerQueue                = nullptr;
    dispatch_queue_t mConnectionQueue              = nullptr;
    dispatch_queue_t mSystemQueue                  = nullptr;
    nw_connection_group_t mConnectionGroup         = nullptr;
    dispatch_semaphore_t mConnectionGroupSemaphore = nullptr;
    dispatch_queue_t mConnectionGroupQueue         = nullptr;

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
                              InterfaceId interfaceIndex = InterfaceId::Null());
    CHIP_ERROR GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(nw_connection_t aConnection);
    void ReleaseAll();

    CHIP_ERROR StartConnectionGroup(nw_group_descriptor_t groupDescriptor);
    CHIP_ERROR ReleaseConnectionGroup();
    CHIP_ERROR IPAnyJoinLeaveMulticastGroup(nw_endpoint_t endpoint, bool join);

    CFMutableDictionaryRef mConnections = nullptr;
    CHIP_ERROR GetConnection(const IPPacketInfo * aPktInfo);
    CHIP_ERROR StartConnection(nw_connection_t aConnection);
    CHIP_ERROR StartConnectionFromListener(nw_connection_t connection);
    void PrepareConnections();
    CHIP_ERROR ReleaseConnections();
    bool RefreshConnectionTimeout(nw_connection_t connection);
    bool CreateConnectionWrapper(nw_connection_t connection);
    bool ClearConnectionWrapper(nw_connection_t connection);
    nw_connection_t FindConnection(const IPPacketInfo & pktInfo);

    std::map<IPAddress, nw_listener_t> mListeners;
    CHIP_ERROR StartListeners();
    CHIP_ERROR StartListenerOnAddress(const IPAddress & address, InterfaceId intfId = InterfaceId::Null());
    CHIP_ERROR ReleaseListener(nw_listener_t listener);
    CHIP_ERROR ReleaseListeners();

    template <typename InterfaceVec>
    void RegisterInterfaces(const InterfaceVec & interfaces)
    {
        for (const auto & iface : interfaces)
        {
            IPAddress addr(iface.second);
            if (mListeners.find(addr) == mListeners.end())
            {
                LogErrorOnFailure(StartListenerOnAddress(addr, static_cast<chip::Inet::InterfaceId>(iface.first)));
            }
        }
    }

    IPAddress mAddr         = IPAddress::Any;
    uint16_t mRequestedPort = 0;
};

using UDPEndPointImpl = UDPEndPointImplNetworkFramework;

} // namespace Inet
} // namespace chip
