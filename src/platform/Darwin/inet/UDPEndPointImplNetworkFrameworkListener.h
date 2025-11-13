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

#include "NetworkMonitor.h"

#include <lib/support/CodeUtils.h>

namespace chip {
namespace Inet {
namespace Darwin {
class UDPEndPointImplNetworkFrameworkListener : public NetworkMonitor
{
public:
    virtual ~UDPEndPointImplNetworkFrameworkListener(){};
    CHIP_ERROR Configure(nw_parameters_t parameters, const IPAddressType addressType, const IPAddress & address, uint16_t port,
                         InterfaceId interfaceId = InterfaceId::Null());
    CHIP_ERROR Listen();
    void Unlisten();
    uint16_t GetBoundPort() const;

    virtual void StartConnectionFromListener(nw_connection_t aConnection)               = 0;
    virtual nw_endpoint_t GetEndPoint(const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort,
                                      InterfaceId interfaceIndex = InterfaceId::Null()) = 0;

private:
    bool IsAnyAddress() const { return mLocalAddress == IPAddress::Any; }
    CHIP_ERROR ListenLoopback();
    CHIP_ERROR ListenAddress();
    CHIP_ERROR ListenAddress(const IPAddress & address, InterfaceId intfId = InterfaceId::Null());
    CHIP_ERROR ListenAddress(const IPAddress & address, nw_interface_t interface);
    CHIP_ERROR ListenInterfaces();
    template <typename InterfaceVec>
    void ListenInterfaces(const InterfaceVec & interfaces)
    {
        for (const auto & iface : interfaces)
        {
            nw_interface_t interface = iface.first;
            IPAddress address(iface.second);
            LogErrorOnFailure(ListenAddress(address, interface));
        }
    }

    void StopListeners();
    CHIP_ERROR WaitForListenerReadyState(nw_listener_t listener);
    CHIP_ERROR WaitForListenerCancelledState(nw_listener_t listener);

    CFMutableArrayRef mListeners     = nullptr;
    dispatch_queue_t mListenerQueue  = nullptr;
    nw_parameters_t mLocalParameters = nullptr;
    IPAddress mLocalAddress          = IPAddress::Any;
    uint16_t mLocalPort              = 0;
};

} // namespace Darwin
} // namespace Inet
} // namespace chip
