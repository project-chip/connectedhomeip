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

#include "ESP32ThreadBorderRouterDelegate.h"

#include <esp_openthread.h>
#include <openthread/border_agent.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/netdiag.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <app/data-model/List.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

using Protocols::InteractionModel::Status;

namespace ThreadBorderRouterManagement {

class ScopedThreadLock
{
public:
    ScopedThreadLock() { DeviceLayer::ThreadStackMgr().LockThreadStack(); }
    ~ScopedThreadLock() { DeviceLayer::ThreadStackMgr().UnlockThreadStack(); }
};

static RoutingRoleEnum MapRoutingRole(otDeviceRole role, bool isFTD, bool rxOnWhenIdle)
{
    if (role == OT_DEVICE_ROLE_DISABLED || role == OT_DEVICE_ROLE_DETACHED)
    {
        return RoutingRoleEnum::kUnassigned;
    }
    else if (role == OT_DEVICE_ROLE_CHILD)
    {
        if (rxOnWhenIdle)
        {
            return isFTD ? RoutingRoleEnum::kReed : RoutingRoleEnum::kEndDevice;
        }
        else
        {
            return RoutingRoleEnum::kSleepyEndDevice;
        }
    }
    else if (role == OT_DEVICE_ROLE_ROUTER)
    {
        return RoutingRoleEnum::kRouter;
    }
    else if (role == OT_DEVICE_ROLE_LEADER)
    {
        return RoutingRoleEnum::kLeader;
    }
    return RoutingRoleEnum::kUnspecified;
}

void ClearThreadNode(Delegate::ThreadNode & threadNode)
{
    threadNode.Routes.Free();
    threadNode.routeTable = DataModel::List<Delegate::RouteTableEntry>();
    threadNode.Ipv6Addresses.Free();
    threadNode.Ipv6AddressesSpans.Free();
    threadNode.IPv6s = DataModel::List<ByteSpan>();
    threadNode.Children.Free();
    threadNode.childTable = DataModel::List<Delegate::ChildTableEntry>();
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetBorderAgentId(MutableByteSpan & borderAgentIdSpan)
{
    otBorderAgentId borderAgentId;
    if (borderAgentIdSpan.size() < sizeof(borderAgentId.mId))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ScopedThreadLock threadLock;
    otError err = otBorderAgentGetId(esp_openthread_get_instance(), &borderAgentId);
    if (err == OT_ERROR_NONE)
    {
        memcpy(borderAgentIdSpan.data(), borderAgentId.mId, sizeof(borderAgentId.mId));
        borderAgentIdSpan.reduce_size(sizeof(borderAgentId.mId));
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetThreadVersion(uint16_t & threadVersion)
{
    threadVersion = otThreadGetVersion();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetInterfaceEnabled(bool & interfaceEnabled)
{
    ScopedThreadLock threadLock;
    interfaceEnabled = otIp6IsEnabled(esp_openthread_get_instance());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetThreadNode(ThreadNode & threadNode)
{
    ScopedThreadLock threadLock;
    ReturnErrorOnFailure(GetExtAddress(threadNode.extAddress));
    ReturnErrorOnFailure(GetRolc16(threadNode.rloc16));
    ReturnErrorOnFailure(GetRoutingRole(threadNode.routingRole));
    threadNode.Ipv6Addresses.Alloc(kOpenThreadMaxIPv6AddressCount);
    threadNode.Ipv6AddressesSpans.Alloc(kOpenThreadMaxIPv6AddressCount);
    ReturnErrorCodeIf((!threadNode.Ipv6Addresses.Get()) || (!threadNode.Ipv6AddressesSpans.Get()), CHIP_ERROR_NO_MEMORY);
    Span<Inet::IPAddress> ipv6AddrsSpan(threadNode.Ipv6Addresses.Get(), kOpenThreadMaxIPv6AddressCount);
    ReturnErrorOnFailure(GetIpv6s(ipv6AddrsSpan));
    for (size_t i = 0; i < ipv6AddrsSpan.size(); ++i)
    {
        threadNode.Ipv6AddressesSpans[i] =
            ByteSpan((uint8_t *) threadNode.Ipv6Addresses[i].Addr, sizeof(threadNode.Ipv6Addresses[i].Addr));
    }
    threadNode.IPv6s = DataModel::List<ByteSpan>(threadNode.Ipv6AddressesSpans.Get(), ipv6AddrsSpan.size());
    threadNode.Routes.Alloc(kOpenThreadMaxRouterId);
    ReturnErrorCodeIf(!threadNode.Routes.Get(), CHIP_ERROR_NO_MEMORY);
    Span<RouteTableEntry> routeTableSpan(threadNode.Routes.Get(), kOpenThreadMaxRouterId);
    ReturnErrorOnFailure(GetRouteTable(routeTableSpan));
    threadNode.routeTable = DataModel::List<RouteTableEntry>(routeTableSpan);
    threadNode.Children.Alloc(kOpenThreadMaxChildCount);
    ReturnErrorCodeIf(!threadNode.Children.Get(), CHIP_ERROR_NO_MEMORY);
    Span<ChildTableEntry> childTableSpan(threadNode.Children.Get(), kOpenThreadMaxChildCount);
    ReturnErrorOnFailure(GetChildTable(childTableSpan));
    threadNode.childTable = DataModel::List<ChildTableEntry>(childTableSpan);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetActiveDataset(chip::Thread::OperationalDataset & activeDataset)
{
    ScopedThreadLock threadLock;
    otOperationalDatasetTlvs datasetTlvs;
    otError otErr = otDatasetGetActiveTlvs(esp_openthread_get_instance(), &datasetTlvs);
    if (otErr == OT_ERROR_NONE)
    {
        return activeDataset.Init(ByteSpan(datasetTlvs.mTlvs, datasetTlvs.mLength));
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetPendingDataset(chip::Thread::OperationalDataset & pendingDataset)
{
    ScopedThreadLock threadLock;
    otOperationalDatasetTlvs datasetTlvs;
    otError otErr = otDatasetGetPendingTlvs(esp_openthread_get_instance(), &datasetTlvs);
    if (otErr == OT_ERROR_NONE)
    {
        return pendingDataset.Init(ByteSpan(datasetTlvs.mTlvs, datasetTlvs.mLength));
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetActiveDataset(const chip::Thread::OperationalDataset & activeDataset)
{
    otInstance * otInst = esp_openthread_get_instance();
    ReturnErrorCodeIf(!otInst, CHIP_ERROR_INCORRECT_STATE);
    otOperationalDatasetTlvs datasetTlvs;
    ScopedThreadLock threadLock;
    if (otDatasetGetActiveTlvs(otInst, &datasetTlvs) != OT_ERROR_NONE || activeDataset.AsByteSpan().size() != datasetTlvs.mLength ||
        memcmp(datasetTlvs.mTlvs, activeDataset.AsByteSpan().data(), datasetTlvs.mLength) != 0)
    {
        memcpy(datasetTlvs.mTlvs, activeDataset.AsByteSpan().data(), activeDataset.AsByteSpan().size());
        datasetTlvs.mLength = activeDataset.AsByteSpan().size();

        // Disable thread before setting active dataset
        ReturnErrorOnFailure(SetThreadEnabled(false));
        ReturnErrorCodeIf(otDatasetSetActiveTlvs(otInst, &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    ReturnErrorOnFailure(SetThreadEnabled(true));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetPendingDataset(const chip::Thread::OperationalDataset & pendingDataset)
{
    ScopedThreadLock threadLock;
    otOperationalDatasetTlvs datasetTlvs;
    memcpy(datasetTlvs.mTlvs, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
    datasetTlvs.mLength = pendingDataset.AsByteSpan().size();
    ReturnErrorCodeIf(otDatasetSetPendingTlvs(esp_openthread_get_instance(), &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

void ESP32ThreadBorderRouterDelegate::OnTopologyRequestCompleted(chip::System::Layer * systemLayer, void * appState)
{
    ESP32ThreadBorderRouterDelegate * delegate = (ESP32ThreadBorderRouterDelegate *) appState;
    if (delegate)
    {
        // Finish the topology request
        delegate->mCurrentSnapshot++;
        if (delegate->mCurrentSnapshot == 0)
        {
            delegate->mCurrentSnapshot = 1;
        }
        auto topology = Span<ThreadNode>(delegate->mThreadNodesBuffer, delegate->mThreadNodesCount);
        delegate->mCallback->OnTopologyRequestFinished(Status::Success, delegate->mCurrentSnapshot, topology);
    }
}

void ESP32ThreadBorderRouterDelegate::DiagnosticReceiveHandler(otError error, otMessage * message,
                                                               const otMessageInfo * messageInfo, void * context)
{
    ESP32ThreadBorderRouterDelegate * delegate = (ESP32ThreadBorderRouterDelegate *) context;
    // This should never happen
    VerifyOrDie(delegate);
    if (error != OT_ERROR_NONE)
    {
        DeviceLayer::SystemLayer().ScheduleLambda(
            [delegate]() { delegate->mCallback->OnTopologyRequestFinished(Status::Failure, 0, Span<ThreadNode>()); });
        // Some error when requesting topology
        return;
    }
    if (delegate->mThreadNodesCount >= kMaxThreadNodesCount)
    {
        DeviceLayer::SystemLayer().ScheduleLambda(
            [delegate]() { delegate->mCallback->OnTopologyRequestFinished(Status::ResourceExhausted, 0, Span<ThreadNode>()); });
        return;
    }
    DeviceLayer::PlatformMgr().LockChipStack();
    // ExtendTimer when receiving a diagnostic message
    DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Milliseconds32(1000), OnTopologyRequestCompleted, delegate);
    DeviceLayer::PlatformMgr().UnlockChipStack();
    otNetworkDiagIterator iterator = OT_NETWORK_DIAGNOSTIC_ITERATOR_INIT;
    otNetworkDiagTlv diagTlv;
    otLinkModeConfig linkMode = {
        .mRxOnWhenIdle = false,
        .mDeviceType   = false,
        .mNetworkData  = false,
    };
    bool linkModeGet = false, rloc16Get = false, isLeader = false;
    auto & currentThreadNode = delegate->mThreadNodesBuffer[delegate->mThreadNodesCount];
    ClearThreadNode(currentThreadNode);
    while ((error = otThreadGetNextDiagnosticTlv(message, &iterator, &diagTlv)) == OT_ERROR_NONE)
    {
        switch (diagTlv.mType)
        {
        case OT_NETWORK_DIAGNOSTIC_TLV_EXT_ADDRESS:
            memcpy(&(currentThreadNode.extAddress), diagTlv.mData.mExtAddress.m8, sizeof(uint64_t));
            break;
        case OT_NETWORK_DIAGNOSTIC_TLV_SHORT_ADDRESS:
            currentThreadNode.rloc16 = diagTlv.mData.mAddr16;
            rloc16Get                = true;
            break;
        case OT_NETWORK_DIAGNOSTIC_TLV_MODE:
            linkMode    = diagTlv.mData.mMode;
            linkModeGet = true;
            break;
        case OT_NETWORK_DIAGNOSTIC_TLV_ROUTE: {
            if (diagTlv.mData.mRoute.mRouteCount == 0)
            {
                DeviceLayer::SystemLayer().ScheduleLambda(
                    [delegate]() { delegate->mCallback->OnTopologyRequestFinished(Status::Failure, 0, Span<ThreadNode>()); });
                return;
            }
            currentThreadNode.Routes.Alloc(diagTlv.mData.mRoute.mRouteCount);
            if (!currentThreadNode.Routes.Get())
            {
                DeviceLayer::SystemLayer().ScheduleLambda([delegate]() {
                    delegate->mCallback->OnTopologyRequestFinished(Status::ResourceExhausted, 0, Span<ThreadNode>());
                });
                return;
            }
            for (size_t routeIndex = 0; routeIndex < diagTlv.mData.mRoute.mRouteCount; ++routeIndex)
            {
                currentThreadNode.Routes[routeIndex].routerId = diagTlv.mData.mRoute.mRouteData[routeIndex].mRouterId;
                currentThreadNode.Routes[routeIndex].pathCost = diagTlv.mData.mRoute.mRouteData[routeIndex].mRouteCost;
                currentThreadNode.Routes[routeIndex].LQIIn    = diagTlv.mData.mRoute.mRouteData[routeIndex].mLinkQualityIn;
                currentThreadNode.Routes[routeIndex].LQIOut   = diagTlv.mData.mRoute.mRouteData[routeIndex].mLinkQualityOut;
                if (currentThreadNode.Routes[routeIndex].routerId == delegate->mLeaderRouterId &&
                    currentThreadNode.Routes[routeIndex].LQIIn == 0 && currentThreadNode.Routes[routeIndex].LQIOut == 0)
                {
                    isLeader = true;
                }
            }
            Span<RouteTableEntry> routeTableSpan =
                Span<RouteTableEntry>(currentThreadNode.Routes.Get(), diagTlv.mData.mRoute.mRouteCount);
            currentThreadNode.routeTable = DataModel::List<RouteTableEntry>(routeTableSpan);
            break;
        }
        case OT_NETWORK_DIAGNOSTIC_TLV_IP6_ADDR_LIST: {
            if (diagTlv.mData.mIp6AddrList.mCount == 0)
            {
                DeviceLayer::SystemLayer().ScheduleLambda(
                    [delegate]() { delegate->mCallback->OnTopologyRequestFinished(Status::Failure, 0, Span<ThreadNode>()); });
                return;
            }
            currentThreadNode.Ipv6Addresses.Alloc(diagTlv.mData.mIp6AddrList.mCount);
            currentThreadNode.Ipv6AddressesSpans.Alloc(diagTlv.mData.mIp6AddrList.mCount);
            if ((!currentThreadNode.Ipv6Addresses.Get()) || (!currentThreadNode.Ipv6AddressesSpans.Get()))
            {
                DeviceLayer::SystemLayer().ScheduleLambda([delegate]() {
                    delegate->mCallback->OnTopologyRequestFinished(Status::ResourceExhausted, 0, Span<ThreadNode>());
                });
                return;
            }
            for (size_t ipAddrIndex = 0; ipAddrIndex < diagTlv.mData.mIp6AddrList.mCount; ++ipAddrIndex)
            {
                memcpy(currentThreadNode.Ipv6Addresses[ipAddrIndex].Addr, diagTlv.mData.mIp6AddrList.mList[ipAddrIndex].mFields.m8,
                       OT_IP6_ADDRESS_SIZE);
                currentThreadNode.Ipv6AddressesSpans[ipAddrIndex] =
                    ByteSpan((uint8_t *) currentThreadNode.Ipv6Addresses[ipAddrIndex].Addr,
                             sizeof(currentThreadNode.Ipv6Addresses[ipAddrIndex].Addr));
            }
            currentThreadNode.IPv6s =
                DataModel::List<ByteSpan>(currentThreadNode.Ipv6AddressesSpans.Get(), diagTlv.mData.mIp6AddrList.mCount);
            break;
        }
        case OT_NETWORK_DIAGNOSTIC_TLV_CHILD_TABLE:
            if (diagTlv.mData.mChildTable.mCount > 0)
            {
                currentThreadNode.Children.Alloc(diagTlv.mData.mChildTable.mCount);
                if (!currentThreadNode.Children.Get())
                {
                    DeviceLayer::SystemLayer().ScheduleLambda(
                        [delegate]() { delegate->mCallback->OnTopologyRequestFinished(Status::Failure, 0, Span<ThreadNode>()); });
                    return;
                }
                for (size_t childIndex = 0; childIndex < diagTlv.mData.mChildTable.mCount; ++childIndex)
                {
                    currentThreadNode.Children[childIndex].linkQuality = diagTlv.mData.mChildTable.mTable[childIndex].mLinkQuality;
                    currentThreadNode.Children[childIndex].rloc16      = diagTlv.mData.mChildTable.mTable[childIndex].mChildId;
                    currentThreadNode.Children[childIndex].routingRole =
                        MapRoutingRole(OT_DEVICE_ROLE_CHILD, diagTlv.mData.mChildTable.mTable[childIndex].mMode.mDeviceType,
                                       diagTlv.mData.mChildTable.mTable[childIndex].mMode.mRxOnWhenIdle);
                }
                currentThreadNode.childTable =
                    DataModel::List<ChildTableEntry>(currentThreadNode.Children.Get(), diagTlv.mData.mChildTable.mCount);
            }
            break;
        default:
            break;
        }
    }
    if (linkModeGet && rloc16Get && currentThreadNode.routeTable.size() > 0)
    {
        if ((currentThreadNode.rloc16 & 0x00FF) == 0)
        {
            currentThreadNode.routingRole = isLeader ? RoutingRoleEnum::kLeader : RoutingRoleEnum::kRouter;
        }
        else
        {
            currentThreadNode.routingRole = MapRoutingRole(OT_DEVICE_ROLE_CHILD, linkMode.mDeviceType, linkMode.mRxOnWhenIdle);
        }
    }
    if (rloc16Get && currentThreadNode.childTable.size() > 0)
    {
        for (size_t i = 0; i < currentThreadNode.childTable.size(); ++i)
        {
            currentThreadNode.Children[i].rloc16 |= (currentThreadNode.rloc16 & 0xFF00);
        }
    }
    delegate->mThreadNodesCount++;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetTopology(uint8_t snapshot, Callback * callback)
{
    if (!callback)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (snapshot != 0)
    {
        if (snapshot != mCurrentSnapshot)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        auto topology = Span<ThreadNode>(mThreadNodesBuffer, mThreadNodesCount);
        callback->OnTopologyRequestFinished(Status::Success, snapshot, topology);
        return CHIP_NO_ERROR;
    }
    // snapshot is zero, we need to create a new snapshot.
    mThreadNodesCount = 0;
    mCallback         = callback;
    ScopedThreadLock threadLock;
    otIp6Address multicastAddress;
    // Get Leader RouterId to distinguish Router and Leader in DiagnosticReceiveHandler
    otLeaderData leaderdata;
    ReturnErrorCodeIf(otThreadGetLeaderData(esp_openthread_get_instance(), &leaderdata) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    mLeaderRouterId = leaderdata.mLeaderRouterId;
    // Send DiagnosticGet message to all the Routers to create a topology snapshot
    ReturnErrorCodeIf(otIp6AddressFromString(kMulticastAddrAllRouters, &multicastAddress) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    ReturnErrorCodeIf(otThreadSendDiagnosticGet(esp_openthread_get_instance(), &multicastAddress, kDiagTLVTypes,
                                                sizeof(kDiagTLVTypes), DiagnosticReceiveHandler, this) != OT_ERROR_NONE,
                      CHIP_ERROR_INTERNAL);
    // If the BR doesn't receive diagnostic message in 1 seconds, we consider that the topology request is done
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(1000), OnTopologyRequestCompleted, this);
    return CHIP_NO_ERROR;
}

// These function should be called when Thread stack is locked
CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetExtAddress(uint64_t & extAddr)
{
    otInstance * otInst             = esp_openthread_get_instance();
    const otExtAddress * extAddress = otLinkGetExtendedAddress(otInst);
    memcpy(&extAddr, extAddress->m8, sizeof(extAddress->m8));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetRolc16(uint16_t & rloc16)
{
    rloc16 = otThreadGetRloc16(esp_openthread_get_instance());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetRoutingRole(RoutingRoleEnum & routingRole)
{
    otInstance * otInst             = esp_openthread_get_instance();
    otDeviceRole role               = otThreadGetDeviceRole(otInst);
    otLinkModeConfig linkModeConfig = otThreadGetLinkMode(otInst);
    routingRole                     = MapRoutingRole(role, linkModeConfig.mDeviceType, linkModeConfig.mRxOnWhenIdle);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetIpv6s(Span<Inet::IPAddress> & ipv6Addrs)
{
    otInstance * otInst               = esp_openthread_get_instance();
    const otNetifAddress * netifAddrs = otIp6GetUnicastAddresses(otInst);
    size_t addrIndex                  = 0;
    for (const otNetifAddress * addr = netifAddrs; addr; addr = addr->mNext)
    {
        if (addrIndex >= ipv6Addrs.size())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(ipv6Addrs[addrIndex].Addr, addr->mAddress.mFields.m8, OT_IP6_ADDRESS_SIZE);
        addrIndex++;
    }
    ipv6Addrs.reduce_size(addrIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetRouteTable(Span<RouteTableEntry> & routeTableSpan)
{
    otInstance * otInst = esp_openthread_get_instance();
    uint8_t maxRouterId;
    otRouterInfo routerInfo;
    maxRouterId             = otThreadGetMaxRouterId(otInst);
    uint8_t routeTableIndex = 0;
    for (uint8_t i = 0; i < maxRouterId && routeTableIndex < routeTableSpan.size(); ++i)
    {
        if (otThreadGetRouterInfo(otInst, i, &routerInfo) == OT_ERROR_NONE)
        {
            routeTableSpan[routeTableIndex].LQIIn    = routerInfo.mLinkQualityIn;
            routeTableSpan[routeTableIndex].LQIOut   = routerInfo.mLinkQualityOut;
            routeTableSpan[routeTableIndex].routerId = routerInfo.mRouterId;
            routeTableSpan[routeTableIndex].pathCost = routerInfo.mPathCost;
            routeTableIndex++;
        }
    }
    routeTableSpan.reduce_size(routeTableIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetChildTable(Span<ChildTableEntry> & childTable)
{
    otInstance * otInst      = esp_openthread_get_instance();
    uint16_t maxChildren     = otThreadGetMaxAllowedChildren(otInst);
    uint16_t childTableIndex = 0;
    otChildInfo childInfo;
    for (uint8_t i = 0; i < maxChildren && childTableIndex < childTable.size(); ++i)
    {
        if (otThreadGetChildInfoByIndex(otInst, i, &childInfo) == OT_ERROR_NONE)
        {
            childTable[childTableIndex].rloc16      = childInfo.mRloc16;
            childTable[childTableIndex].linkQuality = childInfo.mLinkQualityIn;
            childTable[childTableIndex].routingRole =
                MapRoutingRole(OT_DEVICE_ROLE_CHILD, childInfo.mFullThreadDevice, childInfo.mRxOnWhenIdle);
            childTableIndex++;
        }
    }
    childTable.reduce_size(childTableIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetThreadEnabled(bool enabled)
{
    otInstance * instance = esp_openthread_get_instance();
    bool isEnabled        = (otThreadGetDeviceRole(instance) != OT_DEVICE_ROLE_DISABLED);
    bool isIp6Enabled     = otIp6IsEnabled(instance);
    if (enabled && !isIp6Enabled)
    {
        ReturnErrorCodeIf(otIp6SetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    if (enabled != isEnabled)
    {
        ReturnErrorCodeIf(otThreadSetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    if (!enabled && isIp6Enabled)
    {
        ReturnErrorCodeIf(otIp6SetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    return CHIP_NO_ERROR;
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
