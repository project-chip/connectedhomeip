/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *          Contains non-inline method definitions for the
 *          GenericThreadStackManagerImpl_OpenThread<> template.
 */

#ifndef GENERIC_THREAD_STACK_MANAGER_IMPL_OPENTHREAD_IPP
#define GENERIC_THREAD_STACK_MANAGER_IMPL_OPENTHREAD_IPP

#include <openthread/cli.h>
#include <openthread/dataset.h>
#include <openthread/joiner.h>
#include <openthread/link.h>
#include <openthread/netdata.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#if CHIP_DEVICE_CONFIG_THREAD_FTD
#include <openthread/dataset_ftd.h>
#include <openthread/thread_ftd.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
#include <openthread/srp_client.h>
#endif

#include <core/CHIPEncoding.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/CodeUtils.h>
#include <support/ThreadOperationalDataset.h>
#include <support/logging/CHIPLogging.h>

extern "C" void otSysProcessDrivers(otInstance * aInstance);

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
extern "C" void otAppCliInit(otInstance * aInstance);
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;

/**
 * Called by OpenThread to alert the ThreadStackManager of a change in the state of the Thread stack.
 *
 * By default, applications never need to call this method directly.  However, applications that
 * wish to receive OpenThread state change call-backs directly from OpenThread (e.g. by calling
 * otSetStateChangedCallback() with their own callback function) can call this method to pass
 * state change events to the ThreadStackManager.
 */
template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnOpenThreadStateChange(uint32_t flags, void * context)
{
    ChipDeviceEvent event;
    event.Type                                = DeviceEventType::kThreadStateChange;
    event.ThreadStateChange.RoleChanged       = (flags & OT_CHANGED_THREAD_ROLE) != 0;
    event.ThreadStateChange.AddressChanged    = (flags & (OT_CHANGED_IP6_ADDRESS_ADDED | OT_CHANGED_IP6_ADDRESS_REMOVED)) != 0;
    event.ThreadStateChange.NetDataChanged    = (flags & OT_CHANGED_THREAD_NETDATA) != 0;
    event.ThreadStateChange.ChildNodesChanged = (flags & (OT_CHANGED_THREAD_CHILD_ADDED | OT_CHANGED_THREAD_CHILD_REMOVED)) != 0;
    event.ThreadStateChange.OpenThread.Flags  = flags;

    PlatformMgr().PostEvent(&event);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ProcessThreadActivity(void)
{
    otTaskletsProcess(mOTInst);
    otSysProcessDrivers(mOTInst);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_HaveRouteToAddress(const IPAddress & destAddr)
{
    bool res = false;

    // Lock OpenThread
    Impl()->LockThreadStack();

    // No routing of IPv4 over Thread.
    VerifyOrExit(!destAddr.IsIPv4(), res = false);

    // If the device is attached to a Thread network...
    if (IsThreadAttachedNoLock())
    {
        // Link-local addresses are always presumed to be routable, provided the device is attached.
        if (destAddr.IsIPv6LinkLocal())
        {
            ExitNow(res = true);
        }

        // Iterate over the routes known to the OpenThread stack looking for a route that covers the
        // destination address.  If found, consider the address routable.
        // Ignore any routes advertised by this device.
        // If the destination address is a ULA, ignore default routes. Border routers advertising
        // default routes are not expected to be capable of routing CHIP fabric ULAs unless they
        // advertise those routes specifically.
        {
            otError otErr;
            otNetworkDataIterator routeIter = OT_NETWORK_DATA_ITERATOR_INIT;
            otExternalRouteConfig routeConfig;
            const bool destIsULA = destAddr.IsIPv6ULA();

            while ((otErr = otNetDataGetNextRoute(Impl()->OTInstance(), &routeIter, &routeConfig)) == OT_ERROR_NONE)
            {
                const IPPrefix prefix = ToIPPrefix(routeConfig.mPrefix);
                char addrStr[64];
                prefix.IPAddr.ToString(addrStr);
                if (!routeConfig.mNextHopIsThisDevice && (!destIsULA || routeConfig.mPrefix.mLength > 0) &&
                    ToIPPrefix(routeConfig.mPrefix).MatchAddress(destAddr))
                {
                    ExitNow(res = true);
                }
            }
        }
    }

exit:

    // Unlock OpenThread
    Impl()->UnlockThreadStack();

    return res;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    if (event->Type == DeviceEventType::kThreadStateChange)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
        if (event->ThreadStateChange.AddressChanged)
        {
            const otSrpClientHostInfo * hostInfo = otSrpClientGetHostInfo(Impl()->OTInstance());
            if (hostInfo && hostInfo->mName)
            {
                Impl()->_SetupSrpHost(hostInfo->mName);
            }
        }
#endif
        Impl()->LockThreadStack();

#if CHIP_DETAIL_LOGGING

        LogOpenThreadStateChange(mOTInst, event->ThreadStateChange.OpenThread.Flags);

#endif // CHIP_DETAIL_LOGGING

        Impl()->UnlockThreadStack();
    }
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadEnabled(void)
{
    otDeviceRole curRole;

    Impl()->LockThreadStack();
    curRole = otThreadGetDeviceRole(mOTInst);
    Impl()->UnlockThreadStack();

    return (curRole != OT_DEVICE_ROLE_DISABLED);
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadEnabled(bool val)
{
    otError otErr = OT_ERROR_NONE;

    Impl()->LockThreadStack();

    bool isEnabled    = (otThreadGetDeviceRole(mOTInst) != OT_DEVICE_ROLE_DISABLED);
    bool isIp6Enabled = otIp6IsEnabled(mOTInst);

    if (val && !isIp6Enabled)
    {
        otErr = otIp6SetEnabled(mOTInst, val);
        VerifyOrExit(otErr == OT_ERROR_NONE, );
    }

    if (val != isEnabled)
    {
        otErr = otThreadSetEnabled(mOTInst, val);
        VerifyOrExit(otErr == OT_ERROR_NONE, );
    }

    if (!val && isIp6Enabled)
    {
        otErr = otIp6SetEnabled(mOTInst, val);
        VerifyOrExit(otErr == OT_ERROR_NONE, );
    }

exit:
    Impl()->UnlockThreadStack();

    return MapOpenThreadError(otErr);
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadProvision(ByteSpan netInfo)
{
    otError otErr = OT_ERROR_FAILED;
    otOperationalDatasetTlvs tlvs;

    assert(netInfo.size() <= Thread::kSizeOperationalDataset);
    tlvs.mLength = static_cast<uint8_t>(netInfo.size());
    memcpy(tlvs.mTlvs, netInfo.data(), netInfo.size());

    // Set the dataset as the active dataset for the node.
    Impl()->LockThreadStack();
    otErr = otDatasetSetActiveTlvs(mOTInst, &tlvs);
    Impl()->UnlockThreadStack();

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event;
    event.Type                                           = DeviceEventType::kServiceProvisioningChange;
    event.ServiceProvisioningChange.IsServiceProvisioned = true;
    PlatformMgr().PostEvent(&event);

    return MapOpenThreadError(otErr);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadProvisioned(void)
{
    bool provisioned;

    Impl()->LockThreadStack();
    provisioned = otDatasetIsCommissioned(mOTInst);
    Impl()->UnlockThreadStack();

    return provisioned;
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadAttached(void)
{
    otDeviceRole curRole;

    Impl()->LockThreadStack();
    curRole = otThreadGetDeviceRole(mOTInst);
    Impl()->UnlockThreadStack();

    return (curRole != OT_DEVICE_ROLE_DISABLED && curRole != OT_DEVICE_ROLE_DETACHED);
}

template <class ImplClass>
ConnectivityManager::ThreadDeviceType GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetThreadDeviceType(void)
{
    ConnectivityManager::ThreadDeviceType deviceType;

    Impl()->LockThreadStack();

    const otLinkModeConfig linkMode = otThreadGetLinkMode(mOTInst);

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    if (linkMode.mDeviceType && otThreadIsRouterEligible(mOTInst))
        ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_Router);
    if (linkMode.mDeviceType)
        ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_FullEndDevice);
#endif
    if (linkMode.mRxOnWhenIdle)
        ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_MinimalEndDevice);

    ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_SleepyEndDevice);

exit:
    Impl()->UnlockThreadStack();

    return deviceType;
}

template <class ImplClass>
CHIP_ERROR
GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otLinkModeConfig linkMode;

    switch (deviceType)
    {
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    case ConnectivityManager::kThreadDeviceType_Router:
    case ConnectivityManager::kThreadDeviceType_FullEndDevice:
#endif
    case ConnectivityManager::kThreadDeviceType_MinimalEndDevice:
    case ConnectivityManager::kThreadDeviceType_SleepyEndDevice:
        break;
    default:
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

#if CHIP_PROGRESS_LOGGING

    {
        const char * deviceTypeStr;
        switch (deviceType)
        {
        case ConnectivityManager::kThreadDeviceType_Router:
            deviceTypeStr = "ROUTER";
            break;
        case ConnectivityManager::kThreadDeviceType_FullEndDevice:
            deviceTypeStr = "FULL END DEVICE";
            break;
        case ConnectivityManager::kThreadDeviceType_MinimalEndDevice:
            deviceTypeStr = "MINIMAL END DEVICE";
            break;
        case ConnectivityManager::kThreadDeviceType_SleepyEndDevice:
            deviceTypeStr = "SLEEPY END DEVICE";
            break;
        default:
            deviceTypeStr = "(unknown)";
            break;
        }
        ChipLogProgress(DeviceLayer, "Setting OpenThread device type to %s", deviceTypeStr);
    }

#endif // CHIP_PROGRESS_LOGGING

    Impl()->LockThreadStack();

    linkMode = otThreadGetLinkMode(mOTInst);

    switch (deviceType)
    {
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    case ConnectivityManager::kThreadDeviceType_Router:
    case ConnectivityManager::kThreadDeviceType_FullEndDevice:
        linkMode.mDeviceType   = true;
        linkMode.mRxOnWhenIdle = true;
        otThreadSetRouterEligible(mOTInst, deviceType == ConnectivityManager::kThreadDeviceType_Router);
        break;
#endif
    case ConnectivityManager::kThreadDeviceType_MinimalEndDevice:
        linkMode.mDeviceType   = false;
        linkMode.mRxOnWhenIdle = true;
        break;
    case ConnectivityManager::kThreadDeviceType_SleepyEndDevice:
        linkMode.mDeviceType   = false;
        linkMode.mRxOnWhenIdle = false;
        break;
    default:
        break;
    }

    otThreadSetLinkMode(mOTInst, linkMode);

    Impl()->UnlockThreadStack();

exit:
    return err;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetThreadPollingConfig(
    ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    pollingConfig = mPollingConfig;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadPollingConfig(
    const ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    mPollingConfig = pollingConfig;
    return Impl()->AdjustPollingInterval();
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_HaveMeshConnectivity(void)
{
    bool res;
    otDeviceRole curRole;

    Impl()->LockThreadStack();

    // Get the current Thread role.
    curRole = otThreadGetDeviceRole(mOTInst);

    // If Thread is disabled, or the node is detached, then the node has no mesh connectivity.
    if (curRole == OT_DEVICE_ROLE_DISABLED || curRole == OT_DEVICE_ROLE_DETACHED)
    {
        res = false;
    }

    // If the node is a child, that implies the existence of a parent node which provides connectivity
    // to the mesh.
    else if (curRole == OT_DEVICE_ROLE_CHILD)
    {
        res = true;
    }

    // Otherwise, if the node is acting as a router, scan the Thread neighbor table looking for at least
    // one other node that is also acting as router.
    else
    {
        otNeighborInfoIterator neighborIter = OT_NEIGHBOR_INFO_ITERATOR_INIT;
        otNeighborInfo neighborInfo;

        res = false;

        while (otThreadGetNextNeighborInfo(mOTInst, &neighborIter, &neighborInfo) == OT_ERROR_NONE)
        {
            if (!neighborInfo.mIsChild)
            {
                res = true;
                break;
            }
        }
    }

    Impl()->UnlockThreadStack();

    return res;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnMessageLayerActivityChanged(bool messageLayerIsActive)
{
    Impl()->AdjustPollingInterval();
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadStatsCounters(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr;
    const otMacCounters * macCounters;
    const otIpCounters * ipCounters;
    otOperationalDataset activeDataset;
    otDeviceRole role;

    Impl()->LockThreadStack();

    role = otThreadGetDeviceRole(mOTInst);
    ChipLogProgress(DeviceLayer, "Thread Role:                  %d\n", role);

    if (otDatasetIsCommissioned(mOTInst))
    {
        otErr = otDatasetGetActive(mOTInst, &activeDataset);
        VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

        if (activeDataset.mComponents.mIsChannelPresent)
        {
            ChipLogProgress(DeviceLayer, "Thread Channel:               %d\n", activeDataset.mChannel);
        }
    }

    macCounters = otLinkGetCounters(mOTInst);

    ChipLogProgress(DeviceLayer,
                    "Rx Counters:\n"
                    "PHY Rx Total:                 %" PRIu32 "\n"
                    "MAC Rx Unicast:               %" PRIu32 "\n"
                    "MAC Rx Broadcast:             %" PRIu32 "\n"
                    "MAC Rx Data:                  %" PRIu32 "\n"
                    "MAC Rx Data Polls:            %" PRIu32 "\n"
                    "MAC Rx Beacons:               %" PRIu32 "\n"
                    "MAC Rx Beacon Reqs:           %" PRIu32 "\n"
                    "MAC Rx Other:                 %" PRIu32 "\n"
                    "MAC Rx Filtered Whitelist:    %" PRIu32 "\n"
                    "MAC Rx Filtered DestAddr:     %" PRIu32 "\n",
                    macCounters->mRxTotal, macCounters->mRxUnicast, macCounters->mRxBroadcast, macCounters->mRxData,
                    macCounters->mRxDataPoll, macCounters->mRxBeacon, macCounters->mRxBeaconRequest, macCounters->mRxOther,
                    macCounters->mRxAddressFiltered, macCounters->mRxDestAddrFiltered);

    ChipLogProgress(DeviceLayer,
                    "Tx Counters:\n"
                    "PHY Tx Total:                 %" PRIu32 "\n"
                    "MAC Tx Unicast:               %" PRIu32 "\n"
                    "MAC Tx Broadcast:             %" PRIu32 "\n"
                    "MAC Tx Data:                  %" PRIu32 "\n"
                    "MAC Tx Data Polls:            %" PRIu32 "\n"
                    "MAC Tx Beacons:               %" PRIu32 "\n"
                    "MAC Tx Beacon Reqs:           %" PRIu32 "\n"
                    "MAC Tx Other:                 %" PRIu32 "\n"
                    "MAC Tx Retry:                 %" PRIu32 "\n"
                    "MAC Tx CCA Fail:              %" PRIu32 "\n",
                    macCounters->mTxTotal, macCounters->mTxUnicast, macCounters->mTxBroadcast, macCounters->mTxData,
                    macCounters->mTxDataPoll, macCounters->mTxBeacon, macCounters->mTxBeaconRequest, macCounters->mTxOther,
                    macCounters->mTxRetry, macCounters->mTxErrCca);

    ChipLogProgress(DeviceLayer,
                    "Failure Counters:\n"
                    "MAC Rx Decrypt Fail:          %" PRIu32 "\n"
                    "MAC Rx No Frame Fail:         %" PRIu32 "\n"
                    "MAC Rx Unknown Neighbor Fail: %" PRIu32 "\n"
                    "MAC Rx Invalid Src Addr Fail: %" PRIu32 "\n"
                    "MAC Rx FCS Fail:              %" PRIu32 "\n"
                    "MAC Rx Other Fail:            %" PRIu32 "\n",
                    macCounters->mRxErrSec, macCounters->mRxErrNoFrame, macCounters->mRxErrUnknownNeighbor,
                    macCounters->mRxErrInvalidSrcAddr, macCounters->mRxErrFcs, macCounters->mRxErrOther);

    ipCounters = otThreadGetIp6Counters(mOTInst);

    ChipLogProgress(DeviceLayer,
                    "IP Counters:\n"
                    "IP Tx Success:                %" PRIu32 "\n"
                    "IP Rx Success:                %" PRIu32 "\n"
                    "IP Tx Fail:                   %" PRIu32 "\n"
                    "IP Rx Fail:                   %" PRIu32 "\n",
                    ipCounters->mTxSuccess, ipCounters->mRxSuccess, ipCounters->mTxFailure, ipCounters->mRxFailure);

    Impl()->UnlockThreadStack();

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyMinimal(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr;
    const otExtAddress * extAddress;
    uint16_t rloc16;
    uint16_t routerId;
    uint16_t leaderRouterId;
    uint32_t partitionId;
    int8_t parentAverageRssi;
    int8_t parentLastRssi;
    int8_t instantRssi;

    Impl()->LockThreadStack();

    rloc16 = otThreadGetRloc16(mOTInst);

    // Router ID is the top 6 bits of the RLOC
    routerId = (rloc16 >> 10) & 0x3f;

    leaderRouterId = otThreadGetLeaderRouterId(mOTInst);

    otErr = otThreadGetParentAverageRssi(mOTInst, &parentAverageRssi);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    otErr = otThreadGetParentLastRssi(mOTInst, &parentLastRssi);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    partitionId = otThreadGetPartitionId(mOTInst);

    extAddress = otLinkGetExtendedAddress(mOTInst);

    instantRssi = otPlatRadioGetRssi(mOTInst);

    ChipLogProgress(DeviceLayer,
                    "Thread Topology:\n"
                    "RLOC16:           %04X\n"
                    "Router ID:        %u\n"
                    "Leader Router ID: %u\n"
                    "Parent Avg RSSI:  %d\n"
                    "Parent Last RSSI: %d\n"
                    "Partition ID:     %" PRIu32 "\n"
                    "Extended Address: %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Instant RSSI:     %d\n",
                    rloc16, routerId, leaderRouterId, parentAverageRssi, parentLastRssi, partitionId, extAddress->m8[0],
                    extAddress->m8[1], extAddress->m8[2], extAddress->m8[3], extAddress->m8[4], extAddress->m8[5],
                    extAddress->m8[6], extAddress->m8[7], instantRssi);

    Impl()->UnlockThreadStack();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyMinimul failed: %" CHIP_ERROR_FORMAT, err);
    }

    return err;
}

#define TELEM_NEIGHBOR_TABLE_SIZE (64)
#define TELEM_PRINT_BUFFER_SIZE (64)

#if CHIP_DEVICE_CONFIG_THREAD_FTD
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyFull()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr;
    otIp6Address * leaderAddr    = NULL;
    uint8_t * networkData        = NULL;
    uint8_t * stableNetworkData  = NULL;
    uint8_t networkDataLen       = 0;
    uint8_t stableNetworkDataLen = 0;
    const otExtAddress * extAddress;
    otNeighborInfo neighborInfo[TELEM_NEIGHBOR_TABLE_SIZE];
    otNeighborInfoIterator iter;
    otNeighborInfoIterator iterCopy;
    char printBuf[TELEM_PRINT_BUFFER_SIZE];
    uint16_t rloc16;
    uint16_t routerId;
    uint16_t leaderRouterId;
    uint8_t leaderWeight;
    uint8_t leaderLocalWeight;
    uint32_t partitionId;
    int8_t instantRssi;
    uint8_t networkDataVersion;
    uint8_t stableNetworkDataVersion;
    uint16_t neighborTableSize = 0;
    uint16_t childTableSize    = 0;

    Impl()->LockThreadStack();

    rloc16 = otThreadGetRloc16(mOTInst);

    // Router ID is the top 6 bits of the RLOC
    routerId = (rloc16 >> 10) & 0x3f;

    leaderRouterId = otThreadGetLeaderRouterId(mOTInst);

    otErr = otThreadGetLeaderRloc(mOTInst, leaderAddr);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    leaderWeight = otThreadGetLeaderWeight(mOTInst);

    leaderLocalWeight = otThreadGetLocalLeaderWeight(mOTInst);

    otErr = otNetDataGet(mOTInst, false, networkData, &networkDataLen);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    networkDataVersion = otNetDataGetVersion(mOTInst);

    otErr = otNetDataGet(mOTInst, true, stableNetworkData, &stableNetworkDataLen);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    stableNetworkDataVersion = otNetDataGetStableVersion(mOTInst);

    extAddress = otLinkGetExtendedAddress(mOTInst);

    partitionId = otThreadGetPartitionId(mOTInst);

    instantRssi = otPlatRadioGetRssi(mOTInst);

    iter              = OT_NEIGHBOR_INFO_ITERATOR_INIT;
    iterCopy          = OT_NEIGHBOR_INFO_ITERATOR_INIT;
    neighborTableSize = 0;
    childTableSize    = 0;

    while (otThreadGetNextNeighborInfo(mOTInst, &iter, &neighborInfo[iter]) == OT_ERROR_NONE)
    {
        neighborTableSize++;
        if (neighborInfo[iterCopy].mIsChild)
        {
            childTableSize++;
        }
        iterCopy = iter;
    }

    ChipLogProgress(DeviceLayer,
                    "Thread Topology:\n"
                    "RLOC16:                        %04X\n"
                    "Router ID:                     %u\n"
                    "Leader Router ID:              %u\n"
                    "Leader Address:                %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Leader Weight:                 %d\n"
                    "Local Leader Weight:           %d\n"
                    "Network Data Len:              %d\n"
                    "Network Data Version:          %d\n"
                    "Stable Network Data Version:   %d\n"
                    "Extended Address:              %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Partition ID:                  %" PRIx32 "\n"
                    "Instant RSSI:                  %d\n"
                    "Neighbor Table Length:         %d\n"
                    "Child Table Length:            %d\n",
                    rloc16, routerId, leaderRouterId, leaderAddr->mFields.m8[0], leaderAddr->mFields.m8[1],
                    leaderAddr->mFields.m8[2], leaderAddr->mFields.m8[3], leaderAddr->mFields.m8[4], leaderAddr->mFields.m8[5],
                    leaderAddr->mFields.m8[6], leaderAddr->mFields.m8[7], leaderAddr->mFields.m8[8], leaderAddr->mFields.m8[9],
                    leaderAddr->mFields.m8[10], leaderAddr->mFields.m8[11], leaderAddr->mFields.m8[12], leaderAddr->mFields.m8[13],
                    leaderAddr->mFields.m8[14], leaderAddr->mFields.m8[15], leaderWeight, leaderLocalWeight, networkDataLen,
                    networkDataVersion, stableNetworkDataVersion, extAddress->m8[0], extAddress->m8[1], extAddress->m8[2],
                    extAddress->m8[3], extAddress->m8[4], extAddress->m8[5], extAddress->m8[6], extAddress->m8[7], partitionId,
                    instantRssi, neighborTableSize, childTableSize);

    // Handle each neighbor event seperatly.
    for (uint32_t i = 0; i < neighborTableSize; i++)
    {
        otNeighborInfo * neighbor = &neighborInfo[i];

        if (neighbor->mIsChild)
        {
            otChildInfo * child = NULL;
            otErr               = otThreadGetChildInfoById(mOTInst, neighbor->mRloc16, child);
            VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

            snprintf(printBuf, TELEM_PRINT_BUFFER_SIZE, ", Timeout: %10" PRIu32 " NetworkDataVersion: %3" PRIu8, child->mTimeout,
                     child->mNetworkDataVersion);
        }
        else
        {
            printBuf[0] = 0;
        }

        ChipLogProgress(DeviceLayer,
                        "TopoEntry[%" PRIu32 "]:     %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                        "RLOC:              %04X\n"
                        "Age:               %3" PRIu32 "\n"
                        "LQI:               %1d\n"
                        "AvgRSSI:           %3d\n"
                        "LastRSSI:          %3d\n"
                        "LinkFrameCounter:  %10" PRIu32 "\n"
                        "MleFrameCounter:   %10" PRIu32 "\n"
                        "RxOnWhenIdle:      %c\n"
                        "FullFunction:      %c\n"
                        "FullNetworkData:   %c\n"
                        "IsChild:           %c%s\n",
                        i, neighbor->mExtAddress.m8[0], neighbor->mExtAddress.m8[1], neighbor->mExtAddress.m8[2],
                        neighbor->mExtAddress.m8[3], neighbor->mExtAddress.m8[4], neighbor->mExtAddress.m8[5],
                        neighbor->mExtAddress.m8[6], neighbor->mExtAddress.m8[7], neighbor->mRloc16, neighbor->mAge,
                        neighbor->mLinkQualityIn, neighbor->mAverageRssi, neighbor->mLastRssi, neighbor->mLinkFrameCounter,
                        neighbor->mMleFrameCounter, neighbor->mRxOnWhenIdle ? 'Y' : 'n', neighbor->mFullThreadDevice ? 'Y' : 'n',
                        neighbor->mFullNetworkData ? 'Y' : 'n', neighbor->mIsChild ? 'Y' : 'n', printBuf);
    }

    Impl()->UnlockThreadStack();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyFull failed: %s", ErrorStr(err));
    }
    return err;
}
#else // CHIP_DEVICE_CONFIG_THREAD_FTD
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyFull()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
#endif

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetPrimary802154MACAddress(uint8_t * buf)
{
    const otExtAddress * extendedAddr = otLinkGetExtendedAddress(mOTInst);
    memcpy(buf, extendedAddr, sizeof(otExtAddress));
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    const otNetifAddress * otAddresses = otIp6GetUnicastAddresses(mOTInst);

    // Look only for the global unicast addresses, not internally assigned by Thread.
    for (const otNetifAddress * otAddress = otAddresses; otAddress != nullptr; otAddress = otAddress->mNext)
    {
        if (otAddress->mValid)
        {
            switch (otAddress->mAddressOrigin)
            {
            case OT_ADDRESS_ORIGIN_THREAD:
                break;
            case OT_ADDRESS_ORIGIN_SLAAC:
            case OT_ADDRESS_ORIGIN_DHCPV6:
            case OT_ADDRESS_ORIGIN_MANUAL:
                addr = ToIPAddress(otAddress->mAddress);
                return CHIP_NO_ERROR;
            default:
                break;
            }
        }
    }

    return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetPollPeriod(uint32_t & buf)
{
    Impl()->LockThreadStack();
    buf = otLinkGetPollPeriod(mOTInst);
    Impl()->UnlockThreadStack();
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::DoInit(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr  = OT_ERROR_NONE;

    // Arrange for OpenThread errors to be translated to text.
    RegisterOpenThreadErrorFormatter();

    mOTInst = NULL;
    mPollingConfig.Clear();

    // If an OpenThread instance hasn't been supplied, call otInstanceInitSingle() to
    // create or acquire a singleton instance of OpenThread.
    if (otInst == NULL)
    {
        otInst = otInstanceInitSingle();
        VerifyOrExit(otInst != NULL, err = MapOpenThreadError(OT_ERROR_FAILED));
    }

#if !defined(__ZEPHYR__) && !defined(ENABLE_CHIP_SHELL) && !defined(PW_RPC_ENABLED) && CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
    otAppCliInit(otInst);
#endif

    mOTInst = otInst;

    // Arrange for OpenThread to call the OnOpenThreadStateChange method whenever a
    // state change occurs.  Note that we reference the OnOpenThreadStateChange method
    // on the concrete implementation class so that that class can override the default
    // method implementation if it chooses to.
    otErr = otSetStateChangedCallback(otInst, ImplClass::OnOpenThreadStateChange, this);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    // Enable automatic assignment of Thread advertised addresses.
#if OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE
    otIp6SetSlaacEnabled(otInst, true);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    otSrpClientSetCallback(mOTInst, &OnSrpClientNotification, nullptr);
    otSrpClientEnableAutoStartMode(mOTInst, &OnSrpClientStateChange, nullptr);
    memset(&mSrpClient, 0, sizeof(mSrpClient));
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    // If the Thread stack has been provisioned, but is not currently enabled, enable it now.
    if (otThreadGetDeviceRole(mOTInst) == OT_DEVICE_ROLE_DISABLED && otDatasetIsCommissioned(otInst))
    {
        // Enable the Thread IPv6 interface.
        otErr = otIp6SetEnabled(otInst, true);
        VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

        otErr = otThreadSetEnabled(otInst, true);
        VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

        ChipLogProgress(DeviceLayer, "OpenThread ifconfig up and thread start");
    }

exit:
    ChipLogProgress(DeviceLayer, "OpenThread started: %s", otThreadErrorToString(otErr));
    return err;
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::IsThreadAttachedNoLock(void)
{
    otDeviceRole curRole = otThreadGetDeviceRole(mOTInst);
    return (curRole != OT_DEVICE_ROLE_DISABLED && curRole != OT_DEVICE_ROLE_DETACHED);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::IsThreadInterfaceUpNoLock(void)
{
    return otIp6IsEnabled(mOTInst);
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::AdjustPollingInterval(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint32_t newPollingIntervalMS = mPollingConfig.InactivePollingIntervalMS;

    if (newPollingIntervalMS != 0)
    {
        Impl()->LockThreadStack();

        uint32_t curPollingIntervalMS = otLinkGetPollPeriod(mOTInst);

        if (newPollingIntervalMS != curPollingIntervalMS)
        {
            otError otErr = otLinkSetPollPeriod(mOTInst, newPollingIntervalMS);
            err           = MapOpenThreadError(otErr);
        }

        Impl()->UnlockThreadStack();

        if (newPollingIntervalMS != curPollingIntervalMS)
        {
            ChipLogProgress(DeviceLayer, "OpenThread polling interval set to %" PRId32 "ms", newPollingIntervalMS);
        }
    }

    return err;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ErasePersistentInfo(void)
{
    ChipLogProgress(DeviceLayer, "Erasing Thread persistent info...");
    Impl()->LockThreadStack();
    otThreadSetEnabled(mOTInst, false);
    otInstanceErasePersistentInfo(mOTInst);
    Impl()->UnlockThreadStack();
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnJoinerComplete(otError aError, void * aContext)
{
    static_cast<GenericThreadStackManagerImpl_OpenThread *>(aContext)->OnJoinerComplete(aError);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnJoinerComplete(otError aError)
{
    ChipLogProgress(DeviceLayer, "Join Thread network: %s", otThreadErrorToString(aError));

    if (aError == OT_ERROR_NONE)
    {
        otError error = otThreadSetEnabled(mOTInst, true);

        ChipLogProgress(DeviceLayer, "Start Thread network: %s", otThreadErrorToString(error));
    }
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_JoinerStart(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    Impl()->LockThreadStack();
    VerifyOrExit(!otDatasetIsCommissioned(mOTInst) && otThreadGetDeviceRole(mOTInst) == OT_DEVICE_ROLE_DISABLED,
                 error = MapOpenThreadError(OT_ERROR_INVALID_STATE));
    VerifyOrExit(otJoinerGetState(mOTInst) == OT_JOINER_STATE_IDLE, error = MapOpenThreadError(OT_ERROR_BUSY));

    if (!otIp6IsEnabled(mOTInst))
    {
        SuccessOrExit(error = MapOpenThreadError(otIp6SetEnabled(mOTInst, true)));
    }

    {
        otJoinerDiscerner discerner;
        uint16_t discriminator;

        SuccessOrExit(error = ConfigurationMgr().GetSetupDiscriminator(discriminator));
        discerner.mLength = 12;
        discerner.mValue  = discriminator;

        ChipLogProgress(DeviceLayer, "Joiner Discerner: %u", discriminator);
        otJoinerSetDiscerner(mOTInst, &discerner);
    }

    {
        otJoinerPskd pskd;
        uint32_t pincode;

        SuccessOrExit(error = ConfigurationMgr().GetSetupPinCode(pincode));
        snprintf(pskd.m8, sizeof(pskd.m8) - 1, "%09" PRIu32, pincode);

        ChipLogProgress(DeviceLayer, "Joiner PSKd: %s", pskd.m8);
        error = MapOpenThreadError(otJoinerStart(mOTInst, pskd.m8, NULL, NULL, NULL, NULL, NULL,
                                                 &GenericThreadStackManagerImpl_OpenThread::OnJoinerComplete, this));
    }

exit:
    Impl()->UnlockThreadStack();

    ChipLogProgress(DeviceLayer, "Joiner start: %s", chip::ErrorStr(error));

    return error;
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

static_assert(OPENTHREAD_API_VERSION >= 120, "SRP Client requires a more recent OpenThread version");

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnSrpClientNotification(otError aError,
                                                                                  const otSrpClientHostInfo * aHostInfo,
                                                                                  const otSrpClientService * aServices,
                                                                                  const otSrpClientService * aRemovedServices,
                                                                                  void * aContext)
{
    switch (aError)
    {
    case OT_ERROR_NONE: {
        ChipLogProgress(DeviceLayer, "OnSrpClientNotification: Last requested operation completed successfully");

        if (aRemovedServices)
        {
            otSrpClientService * otService = const_cast<otSrpClientService *>(aRemovedServices);
            otSrpClientService * next      = nullptr;
            using Service                  = typename SrpClient::Service;

            // Free memory for all removed services.
            do
            {
                next         = otService->mNext;
                auto service = reinterpret_cast<Service *>(reinterpret_cast<size_t>(otService) - offsetof(Service, mService));
                memset(service, 0, sizeof(Service));
                otService = next;
            } while (otService);
        }
        break;
    }
    case OT_ERROR_PARSE:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Parsing operaton failed");
        break;
    case OT_ERROR_NOT_FOUND:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Domain name or RRset does not exist");
        break;
    case OT_ERROR_NOT_IMPLEMENTED:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Server does not support query type");
        break;
    case OT_ERROR_SECURITY:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Operation refused for security reasons");
        break;
    case OT_ERROR_DUPLICATED:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Domain name or RRset is duplicated");
        break;
    case OT_ERROR_RESPONSE_TIMEOUT:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Timed out waiting on server response");
        break;
    case OT_ERROR_INVALID_ARGS:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Invalid service structure detected");
        break;
    case OT_ERROR_NO_BUFS:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Insufficient buffer to handle message");
        break;
    case OT_ERROR_FAILED:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Internal server error occurred");
        break;
    default:
        ChipLogError(DeviceLayer, "OnSrpClientNotification: Unknown error occurred");
        break;
    }
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnSrpClientStateChange(const otSockAddr * aServerSockAddr,
                                                                                 void * aContext)
{
    if (aServerSockAddr)
    {
        ChipLogProgress(DeviceLayer, "SRP Client was started, detected server: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[0]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[1]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[2]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[3]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[4]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[5]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[6]),
                        Encoding::BigEndian::HostSwap16(aServerSockAddr->mAddress.mFields.m16[7]));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "SRP Client was stopped, because current server is no longer detected.");
    }
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_AddSrpService(const char * aInstanceName, const char * aName,
                                                                               uint16_t aPort, chip::Mdns::TextEntry * aTxtEntries,
                                                                               size_t aTxtEntriesSize, uint32_t aLeaseInterval,
                                                                               uint32_t aKeyLeaseInterval)
{
    CHIP_ERROR error                         = CHIP_NO_ERROR;
    typename SrpClient::Service * srpService = nullptr;

    Impl()->LockThreadStack();

    VerifyOrExit(aInstanceName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aInstanceName) <= SrpClient::kMaxInstanceNameSize, error = CHIP_ERROR_INVALID_STRING_LENGTH);
    VerifyOrExit(aName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aName) <= SrpClient::kMaxNameSize, error = CHIP_ERROR_INVALID_STRING_LENGTH);

    // Try to find an empty slot in array for the new service and
    // remove the possible existing entry from anywhere in the list
    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        // Remove possible existing entry
        if ((strcmp(service.mInstanceName, aInstanceName) == 0) && (strcmp(service.mName, aName) == 0))
        {
            VerifyOrExit(MapOpenThreadError(otSrpClientClearService(mOTInst, &(service.mService))) == CHIP_NO_ERROR,
                         error = MapOpenThreadError(OT_ERROR_FAILED));

            // Clear memory immediately, as OnSrpClientNotification will not be called.
            memset(&service, 0, sizeof(service));
        }

        if ((srpService == nullptr) && (strcmp(service.mInstanceName, "") == 0))
        {
            // Assign first empty slot found in array for a new service.
            srpService = &service;

            // Keep looping to remove possible existing entry further in the list
        }
    }

    // Verify there is a slot found for the new service.
    VerifyOrExit(srpService, error = MapOpenThreadError(OT_ERROR_NO_BUFS));

    otSrpClientSetLeaseInterval(mOTInst, aLeaseInterval);
    otSrpClientSetKeyLeaseInterval(mOTInst, aKeyLeaseInterval);

    memcpy(srpService->mInstanceName, aInstanceName, strlen(aInstanceName) + 1);
    srpService->mService.mInstanceName = srpService->mInstanceName;

    memcpy(srpService->mName, aName, strlen(aName) + 1);
    srpService->mService.mName = srpService->mName;

    srpService->mService.mPort = aPort;

    // Check if there are some optional text entries to add.
    if (aTxtEntries && aTxtEntriesSize != 0)
    {
        VerifyOrExit(aTxtEntriesSize <= SrpClient::kMaxTxtEntriesNumber, error = CHIP_ERROR_INVALID_LIST_LENGTH);

        srpService->mService.mNumTxtEntries = static_cast<uint8_t>(aTxtEntriesSize);

        for (uint8_t entryId = 0; entryId < aTxtEntriesSize; entryId++)
        {
            VerifyOrExit(aTxtEntries[entryId].mDataSize <= SrpClient::kMaxTxtValueSize, error = CHIP_ERROR_BUFFER_TOO_SMALL);
            VerifyOrExit((strlen(aTxtEntries[entryId].mKey) + 1) <= SrpClient::kMaxTxtKeySize, error = CHIP_ERROR_BUFFER_TOO_SMALL);

            srpService->mTxtEntries[entryId].mValueLength = static_cast<uint8_t>(aTxtEntries[entryId].mDataSize);
            memcpy(&(srpService->mTxtValueBuffers[entryId][0]), aTxtEntries[entryId].mData, aTxtEntries[entryId].mDataSize);
            srpService->mTxtEntries[entryId].mValue = &(srpService->mTxtValueBuffers[entryId][0]);

            memcpy(&(srpService->mTxtKeyBuffers[entryId][0]), aTxtEntries[entryId].mKey, strlen(aTxtEntries[entryId].mKey) + 1);
            srpService->mTxtEntries[entryId].mKey = &(srpService->mTxtKeyBuffers[entryId][0]);
        }

        srpService->mService.mTxtEntries = srpService->mTxtEntries;
    }

    ChipLogProgress(DeviceLayer, "advertising srp service: %s.%s", srpService->mService.mInstanceName, srpService->mService.mName);
    error = MapOpenThreadError(otSrpClientAddService(mOTInst, &(srpService->mService)));

exit:
    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_RemoveSrpService(const char * aInstanceName, const char * aName)
{
    CHIP_ERROR error                         = CHIP_NO_ERROR;
    typename SrpClient::Service * srpService = nullptr;

    Impl()->LockThreadStack();

    VerifyOrExit(aInstanceName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aInstanceName) <= SrpClient::kMaxInstanceNameSize, error = CHIP_ERROR_INVALID_STRING_LENGTH);
    VerifyOrExit(aName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aName) <= SrpClient::kMaxNameSize, error = CHIP_ERROR_INVALID_STRING_LENGTH);

    // Check if service to remove exists.
    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        if ((strcmp(service.mInstanceName, aInstanceName) == 0) && (strcmp(service.mName, aName) == 0))
        {
            srpService = &service;
            break;
        }
    }

    VerifyOrExit(srpService, error = MapOpenThreadError(OT_ERROR_NOT_FOUND));

    ChipLogProgress(DeviceLayer, "removing srp service: %s.%s", aInstanceName, aName);
    error = MapOpenThreadError(otSrpClientRemoveService(mOTInst, &(srpService->mService)));

exit:
    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_RemoveAllSrpServices()
{
    CHIP_ERROR error;

    Impl()->LockThreadStack();
    const otSrpClientService * services = otSrpClientGetServices(mOTInst);

    // In case of empty list just return with no error
    VerifyOrExit(services != nullptr, error = CHIP_NO_ERROR);

    error = MapOpenThreadError(otSrpClientRemoveHostAndServices(mOTInst, false));

exit:
    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetupSrpHost(const char * aHostName)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    Inet::IPAddress hostAddress;

    Impl()->LockThreadStack();

    VerifyOrExit(aHostName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aHostName) <= SrpClient::kMaxHostNameSize, error = CHIP_ERROR_INVALID_STRING_LENGTH);

    // Avoid adding the same host name multiple times
    if (strcmp(mSrpClient.mHostName, aHostName) != 0)
    {
        strcpy(mSrpClient.mHostName, aHostName);
        error = MapOpenThreadError(otSrpClientSetHostName(mOTInst, mSrpClient.mHostName));
        SuccessOrExit(error);
    }

    // Check if device has any external IPv6 assigned. If not, host will be set without IPv6 addresses
    // and updated later on.
    if (ThreadStackMgr().GetExternalIPv6Address(hostAddress) == CHIP_NO_ERROR)
    {
        memcpy(&mSrpClient.mHostAddress.mFields.m32, hostAddress.Addr, sizeof(hostAddress.Addr));
        error = MapOpenThreadError(otSrpClientSetHostAddresses(mOTInst, &mSrpClient.mHostAddress, 1));
    }

exit:
    Impl()->UnlockThreadStack();

    return error;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_THREAD_STACK_MANAGER_IMPL_OPENTHREAD_IPP
