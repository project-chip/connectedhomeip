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
#include <openthread/dataset_ftd.h>
#include <openthread/joiner.h>
#include <openthread/link.h>
#include <openthread/netdata.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

extern "C" void otSysProcessDrivers(otInstance * aInstance);

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
                prefix.IPAddr.ToString(addrStr, sizeof(addrStr));
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
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadProvision(const DeviceNetworkInfo & netInfo)
{
    otError otErr = OT_ERROR_FAILED;
    otOperationalDataset newDataset;

    // Form a Thread operational dataset from the given network parameters.
    memset(&newDataset, 0, sizeof(newDataset));
    newDataset.mComponents.mIsActiveTimestampPresent  = true;
    newDataset.mComponents.mIsPendingTimestampPresent = true;
    if (netInfo.ThreadNetworkName[0] != 0)
    {
        strncpy((char *) newDataset.mNetworkName.m8, netInfo.ThreadNetworkName, sizeof(newDataset.mNetworkName.m8));
        newDataset.mComponents.mIsNetworkNamePresent = true;
    }
    if (netInfo.FieldPresent.ThreadExtendedPANId)
    {
        memcpy(newDataset.mExtendedPanId.m8, netInfo.ThreadExtendedPANId, sizeof(newDataset.mExtendedPanId.m8));
        newDataset.mComponents.mIsExtendedPanIdPresent = true;
    }
    if (netInfo.FieldPresent.ThreadMeshPrefix)
    {
        memcpy(newDataset.mMeshLocalPrefix.m8, netInfo.ThreadMeshPrefix, sizeof(newDataset.mMeshLocalPrefix.m8));
        newDataset.mComponents.mIsMeshLocalPrefixPresent = true;
    }
    memcpy(newDataset.mMasterKey.m8, netInfo.ThreadNetworkKey, sizeof(newDataset.mMasterKey.m8));
    newDataset.mComponents.mIsMasterKeyPresent = true;
    if (netInfo.FieldPresent.ThreadPSKc)
    {
        memcpy(newDataset.mPskc.m8, netInfo.ThreadPSKc, sizeof(newDataset.mPskc.m8));
        newDataset.mComponents.mIsPskcPresent = true;
    }
    if (netInfo.ThreadPANId != kThreadPANId_NotSpecified)
    {
        newDataset.mPanId                      = netInfo.ThreadPANId;
        newDataset.mComponents.mIsPanIdPresent = true;
    }
    if (netInfo.ThreadChannel != kThreadChannel_NotSpecified)
    {
        newDataset.mChannel                      = netInfo.ThreadChannel;
        newDataset.mComponents.mIsChannelPresent = true;
    }

    // Set the dataset as the active dataset for the node.
    Impl()->LockThreadStack();
    otErr = otDatasetSetActive(mOTInst, &newDataset);
    Impl()->UnlockThreadStack();

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
    otLinkModeConfig linkMode;
    ConnectivityManager::ThreadDeviceType deviceType;

    Impl()->LockThreadStack();

    linkMode = otThreadGetLinkMode(mOTInst);

    if (linkMode.mDeviceType)
    {
        if (otThreadIsRouterEligible(mOTInst))
        {
            deviceType = ConnectivityManager::kThreadDeviceType_Router;
        }
        else
        {
            deviceType = ConnectivityManager::kThreadDeviceType_FullEndDevice;
        }
    }
    else
    {
        if (linkMode.mRxOnWhenIdle)
        {
            deviceType = ConnectivityManager::kThreadDeviceType_MinimalEndDevice;
        }
        else
        {
            deviceType = ConnectivityManager::kThreadDeviceType_SleepyEndDevice;
        }
    }

    Impl()->UnlockThreadStack();

    return deviceType;
}

template <class ImplClass>
CHIP_ERROR
GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otLinkModeConfig linkMode;

    VerifyOrExit(deviceType == ConnectivityManager::kThreadDeviceType_Router ||
                     deviceType == ConnectivityManager::kThreadDeviceType_FullEndDevice ||
                     deviceType == ConnectivityManager::kThreadDeviceType_MinimalEndDevice ||
                     deviceType == ConnectivityManager::kThreadDeviceType_SleepyEndDevice,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

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
    case ConnectivityManager::kThreadDeviceType_Router:
    case ConnectivityManager::kThreadDeviceType_FullEndDevice:
        linkMode.mDeviceType   = true;
        linkMode.mRxOnWhenIdle = true;
        otThreadSetRouterEligible(mOTInst, deviceType == ConnectivityManager::kThreadDeviceType_Router);
        break;
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
                    "PHY Rx Total:                 %d\n"
                    "MAC Rx Unicast:               %d\n"
                    "MAC Rx Broadcast:             %d\n"
                    "MAC Rx Data:                  %d\n"
                    "MAC Rx Data Polls:            %d\n"
                    "MAC Rx Beacons:               %d\n"
                    "MAC Rx Beacon Reqs:           %d\n"
                    "MAC Rx Other:                 %d\n"
                    "MAC Rx Filtered Whitelist:    %d\n"
                    "MAC Rx Filtered DestAddr:     %d\n",
                    macCounters->mRxTotal, macCounters->mRxUnicast, macCounters->mRxBroadcast, macCounters->mRxData,
                    macCounters->mRxDataPoll, macCounters->mRxBeacon, macCounters->mRxBeaconRequest, macCounters->mRxOther,
                    macCounters->mRxAddressFiltered, macCounters->mRxDestAddrFiltered);

    ChipLogProgress(DeviceLayer,
                    "Tx Counters:\n"
                    "PHY Tx Total:                 %d\n"
                    "MAC Tx Unicast:               %d\n"
                    "MAC Tx Broadcast:             %d\n"
                    "MAC Tx Data:                  %d\n"
                    "MAC Tx Data Polls:            %d\n"
                    "MAC Tx Beacons:               %d\n"
                    "MAC Tx Beacon Reqs:           %d\n"
                    "MAC Tx Other:                 %d\n"
                    "MAC Tx Retry:                 %d\n"
                    "MAC Tx CCA Fail:              %d\n",
                    macCounters->mTxTotal, macCounters->mTxUnicast, macCounters->mTxBroadcast, macCounters->mTxData,
                    macCounters->mTxDataPoll, macCounters->mTxBeacon, macCounters->mTxBeaconRequest, macCounters->mTxOther,
                    macCounters->mTxRetry, macCounters->mTxErrCca);

    ChipLogProgress(DeviceLayer,
                    "Failure Counters:\n"
                    "MAC Rx Decrypt Fail:          %d\n"
                    "MAC Rx No Frame Fail:         %d\n"
                    "MAC Rx Unknown Neighbor Fail: %d\n"
                    "MAC Rx Invalid Src Addr Fail: %d\n"
                    "MAC Rx FCS Fail:              %d\n"
                    "MAC Rx Other Fail:            %d\n",
                    macCounters->mRxErrSec, macCounters->mRxErrNoFrame, macCounters->mRxErrUnknownNeighbor,
                    macCounters->mRxErrInvalidSrcAddr, macCounters->mRxErrFcs, macCounters->mRxErrOther);

    ipCounters = otThreadGetIp6Counters(mOTInst);

    ChipLogProgress(DeviceLayer,
                    "IP Counters:\n"
                    "IP Tx Success:                %d\n"
                    "IP Rx Success:                %d\n"
                    "IP Tx Fail:                   %d\n"
                    "IP Rx Fail:                   %d\n",
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
                    "Partition ID:     %d\n"
                    "Extended Address: %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Instant RSSI:     %d\n",
                    rloc16, routerId, leaderRouterId, parentAverageRssi, parentLastRssi, partitionId, extAddress->m8[0],
                    extAddress->m8[1], extAddress->m8[2], extAddress->m8[3], extAddress->m8[4], extAddress->m8[5],
                    extAddress->m8[6], extAddress->m8[7], instantRssi);

    Impl()->UnlockThreadStack();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyMinimul failed: %d", err);
    }

    return err;
}

#define TELEM_NEIGHBOR_TABLE_SIZE (64)
#define TELEM_PRINT_BUFFER_SIZE (64)

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyFull(void)
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
                    "Partition ID:                  %X\n"
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

            snprintf(printBuf, TELEM_PRINT_BUFFER_SIZE, ", Timeout: %10lu NetworkDataVersion: %3d", child->mTimeout,
                     child->mNetworkDataVersion);
        }
        else
        {
            printBuf[0] = 0;
        }

        ChipLogProgress(DeviceLayer,
                        "TopoEntry[%u]:     %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                        "RLOC:              %04X\n"
                        "Age:               %3d\n"
                        "LQI:               %1d\n"
                        "AvgRSSI:           %3d\n"
                        "LastRSSI:          %3d\n"
                        "LinkFrameCounter:  %10d\n"
                        "MleFrameCounter:   %10d\n"
                        "RxOnWhenIdle:      %c\n"
                        "SecureDataRequest: %c\n"
                        "FullFunction:      %c\n"
                        "FullNetworkData:   %c\n"
                        "IsChild:           %c%s\n",
                        i, neighbor->mExtAddress.m8[0], neighbor->mExtAddress.m8[1], neighbor->mExtAddress.m8[2],
                        neighbor->mExtAddress.m8[3], neighbor->mExtAddress.m8[4], neighbor->mExtAddress.m8[5],
                        neighbor->mExtAddress.m8[6], neighbor->mExtAddress.m8[7], neighbor->mRloc16, neighbor->mAge,
                        neighbor->mLinkQualityIn, neighbor->mAverageRssi, neighbor->mLastRssi, neighbor->mLinkFrameCounter,
                        neighbor->mMleFrameCounter, neighbor->mRxOnWhenIdle ? 'Y' : 'n', neighbor->mSecureDataRequest ? 'Y' : 'n',
#if OPENTHREA_API_VERSION
                        neighbor->mFullThreadDevice ? 'Y' : 'n',
#else
                        'n',
#endif

                        neighbor->mFullNetworkData ? 'Y' : 'n', neighbor->mIsChild ? 'Y' : 'n', printBuf);
    }

    Impl()->UnlockThreadStack();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyFull failed: %s", err);
    }
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetPrimary802154MACAddress(uint8_t * buf)
{
    const otExtAddress * extendedAddr = otLinkGetExtendedAddress(mOTInst);
    memcpy(buf, extendedAddr, sizeof(otExtAddress));
    return CHIP_NO_ERROR;
};

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

#ifndef __ZEPHYR__
    otCliUartInit(otInst);
#endif

    mOTInst = otInst;

    // Arrange for OpenThread to call the OnOpenThreadStateChange method whenever a
    // state change occurs.  Note that we reference the OnOpenThreadStateChange method
    // on the concrete implementation class so that that class can override the default
    // method implementation if it chooses to.
    otErr = otSetStateChangedCallback(otInst, ImplClass::OnOpenThreadStateChange, NULL);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    // Enable use of secure data requests.
    {
        otLinkModeConfig linkMode    = otThreadGetLinkMode(otInst);
        linkMode.mSecureDataRequests = true;
        otErr                        = otThreadSetLinkMode(otInst, linkMode);
        VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));
    }

    // Enable automatic assignment of Thread advertised addresses.
#if OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE
    otIp6SetSlaacEnabled(otInst, true);
#endif

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
        uint32_t discriminator;

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
        snprintf(pskd.m8, sizeof(pskd.m8) - 1, "%" PRIu32, pincode);

        ChipLogProgress(DeviceLayer, "Joiner PSKd: %u", pincode);
        error = MapOpenThreadError(otJoinerStart(mOTInst, pskd.m8, NULL, NULL, NULL, NULL, NULL,
                                                 &GenericThreadStackManagerImpl_OpenThread::OnJoinerComplete, this));
    }

exit:
    Impl()->UnlockThreadStack();

    ChipLogProgress(DeviceLayer, "Joiner start: %s", chip::ErrorStr(error));

    return error;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_THREAD_STACK_MANAGER_IMPL_OPENTHREAD_IPP
