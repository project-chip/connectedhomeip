/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <openthread/netdiag.h>

using chip::MutableByteSpan;
using chip::MutableCharSpan;
using chip::Span;

namespace chip {
namespace app {
namespace Clusters {

namespace ThreadBorderRouterManagement {

class ESP32ThreadBorderRouterDelegate : public Delegate
{
public:
    static constexpr char kThreadBorderRourterName[]        = "Espressif-ThreadBR";
    static constexpr uint8_t kOpenThreadMaxRouterId         = OT_NETWORK_MAX_ROUTER_ID;
    static constexpr uint8_t kOpenThreadMaxChildCount       = 10;
    static constexpr uint8_t kOpenThreadMaxIPv6AddressCount = 8;
    static constexpr uint8_t kMaxThreadNodesCount           = 64;
    static constexpr char kMulticastAddrAllRouters[]        = "ff03::2";
    static constexpr uint8_t kDiagTLVTypes[] = { OT_NETWORK_DIAGNOSTIC_TLV_EXT_ADDRESS,   OT_NETWORK_DIAGNOSTIC_TLV_SHORT_ADDRESS,
                                                 OT_NETWORK_DIAGNOSTIC_TLV_MODE,          OT_NETWORK_DIAGNOSTIC_TLV_ROUTE,
                                                 OT_NETWORK_DIAGNOSTIC_TLV_IP6_ADDR_LIST, OT_NETWORK_DIAGNOSTIC_TLV_CHILD_TABLE };

    ESP32ThreadBorderRouterDelegate()  = default;
    ~ESP32ThreadBorderRouterDelegate() = default;

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetPanChangeSupported(bool & panChangeSupported) override
    {
        panChangeSupported = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        if (borderRouterName.size() < strlen(kThreadBorderRourterName))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        strcpy(borderRouterName.data(), kThreadBorderRourterName);
        borderRouterName.reduce_size(strlen(kThreadBorderRourterName));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override;

    CHIP_ERROR GetThreadVersion(uint16_t & threadVersion) override;

    CHIP_ERROR GetInterfaceEnabled(bool & interfaceEnabled) override;

    CHIP_ERROR GetThreadNode(ThreadNode & threadNode) override;

    CHIP_ERROR GetActiveDataset(chip::Thread::OperationalDataset & activeDataset) override;

    CHIP_ERROR GetPendingDataset(chip::Thread::OperationalDataset & pendingDataset) override;

    CHIP_ERROR SetActiveDataset(const chip::Thread::OperationalDataset & activeDataset) override;

    CHIP_ERROR SetPendingDataset(const chip::Thread::OperationalDataset & pendingDataset) override;

    CHIP_ERROR GetTopology(uint8_t snapshot, Callback * callback) override;

private:
    CHIP_ERROR GetExtAddress(uint64_t & extAddr);
    CHIP_ERROR GetRolc16(uint16_t & rloc16);
    CHIP_ERROR GetRoutingRole(RoutingRoleEnum & routingRole);
    CHIP_ERROR GetIpv6s(Span<Inet::IPAddress> & ipv6Addrs);
    CHIP_ERROR GetRouteTable(Span<RouteTableEntry> & routeTable);
    CHIP_ERROR GetChildTable(Span<ChildTableEntry> & childTable);
    CHIP_ERROR SetThreadEnabled(bool enabled);

    static void DiagnosticReceiveHandler(otError error, otMessage * message, const otMessageInfo * messageInfo, void * aContext);
    static void OnTopologyRequestCompleted(chip::System::Layer * systemLayer, void * appState);

    uint8_t mCurrentSnapshot;
    size_t mThreadNodesCount = 0;
    ThreadNode mThreadNodesBuffer[kMaxThreadNodesCount];
    Callback * mCallback    = nullptr;
    uint8_t mLeaderRouterId = 0;
};
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
