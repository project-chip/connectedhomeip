/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <Network/Network.h>
#include <inet/InetConfig.h>

#include <inet/IANAConstants.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Inet {
namespace Darwin {
class UDPEndPointImplNetworkFrameworkListenerGroup
{
public:
    virtual ~UDPEndPointImplNetworkFrameworkListenerGroup() = default;

    CHIP_ERROR Configure(nw_parameters_t parameters);
    void Unlisten();

    CHIP_ERROR SetMulticastLoopback(IPVersion ipVersion, bool loopback) { return CHIP_ERROR_NOT_IMPLEMENTED; }

#if INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR IPv4JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join);
    ;
#endif // INET_CONFIG_ENABLE_IPV4

    CHIP_ERROR IPv6JoinLeaveMulticastGroup(InterfaceId interfaceId, const IPAddress & address, bool join);

    virtual uint16_t GetBoundPort() const                                            = 0;
    virtual nw_endpoint_t GetEndPoint(const IPAddressType addressType, const IPAddress & address, uint16_t port,
                                      InterfaceId interfaceId = InterfaceId::Null()) = 0;

private:
    void StopListeners();
    CHIP_ERROR StartListeners(nw_group_descriptor_t groupDescriptor);
    CHIP_ERROR JoinLeaveMulticastGroup(nw_endpoint_t endpoint, bool join);
    CHIP_ERROR JoinMulticastGroup(nw_endpoint_t endpoint);
    CHIP_ERROR LeaveMulticastGroup(nw_endpoint_t endpoint);
    bool IsSameEndPoints(nw_endpoint_t a, nw_endpoint_t b);

    CHIP_ERROR WaitForConnectionGroupReadyState(nw_connection_group_t connectionGroup);
    CHIP_ERROR WaitForConnectionGroupCancelledState(nw_connection_group_t connectionGroup);

    nw_connection_group_t mConnectionGroup   = nullptr;
    dispatch_queue_t mConnectionGroupQueue   = nullptr;
    dispatch_semaphore_t mCancelledSemaphore = nullptr;
    nw_parameters_t mLocalParameters         = nullptr;
};

} // namespace Darwin
} // namespace Inet
} // namespace chip
