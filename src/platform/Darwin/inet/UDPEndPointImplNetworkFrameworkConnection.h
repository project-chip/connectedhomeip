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

#include <inet/IPPacketInfo.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Inet {
namespace Darwin {
class UDPEndPointImplNetworkFrameworkConnection
{
public:
    virtual ~UDPEndPointImplNetworkFrameworkConnection(){};

    CHIP_ERROR Configure(nw_parameters_t parameters, IPAddressType addressType);
    CHIP_ERROR StartConnection(nw_connection_t connection);
    nw_connection_t RetrieveOrStartConnection(const IPPacketInfo & pktInfo);
    bool RefreshConnectionTimeout(nw_connection_t connection);
    void StopAll();

    virtual uint16_t GetBoundPort() const                                            = 0;
    virtual nw_endpoint_t GetEndPoint(const IPAddressType addressType, const IPAddress & address, uint16_t port,
                                      InterfaceId interfaceId = InterfaceId::Null()) = 0;

private:
    class ConnectionWrapper
    {
    public:
        ConnectionWrapper(nw_connection_t connection, dispatch_queue_t timeoutQueue, dispatch_block_t onTimeout);

        bool Matches(const IPPacketInfo & pktInfo) const;
        void RefreshTimeout();

        nw_connection_t mConnection    = nullptr;
        dispatch_source_t mTimer       = nullptr;
        dispatch_queue_t mTimeoutQueue = nullptr;
        dispatch_block_t mTimeoutBlock = nullptr;
    };
    CHIP_ERROR AddConnectionWrapper(nw_connection_t connection);
    void RemoveConnectionWrapper(nw_connection_t connection);
    static void ReleaseConnectionWrapperCallback(CFAllocatorRef allocator, const void * value);

    CHIP_ERROR WaitForConnectionStateReady(nw_connection_t connection);
    CHIP_ERROR WaitForConnectionStateCancelled(nw_connection_t connection);
    CHIP_ERROR WaitForAllConnectionStateCancelled();

    CHIP_ERROR GetConnection(const IPPacketInfo & pktInfo);
    void PrepareConnections();
    void ReleaseConnections();
    void SetConnectionInterface(nw_parameters_t parameters, InterfaceId interfaceId);

    bool HasConnection(nw_connection_t connection);
    nw_connection_t FindConnection(const IPPacketInfo & pktInfo);

    CFMutableDictionaryRef mConnections = nullptr;
    dispatch_queue_t mConnectionQueue   = nullptr;
    nw_parameters_t mLocalParameters    = nullptr;
    IPAddressType mAddressType          = IPAddressType::kAny;
};

} // namespace Darwin
} // namespace Inet
} // namespace chip
