/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <cassert>
#include <limits>

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

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/FixedBufferAllocator.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

extern "C" void otSysProcessDrivers(otInstance * aInstance);

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
extern "C" void otAppCliInit(otInstance * aInstance);
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

static_assert(OPENTHREAD_API_VERSION >= 219, "OpenThread version too old");

// Network commissioning
namespace {
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
NetworkCommissioning::GenericThreadDriver sGenericThreadDriver;
app::Clusters::NetworkCommissioning::Instance
    sThreadNetworkCommissioningInstance(CHIP_DEVICE_CONFIG_THREAD_NETWORK_ENDPOINT_ID /* Endpoint Id */, &sGenericThreadDriver);
#endif

void initNetworkCommissioningThreadDriver()
{
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
    sThreadNetworkCommissioningInstance.Init();
#endif
}

void resetGenericThreadDriver()
{
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
    sGenericThreadDriver.ClearNetwork();
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
CHIP_ERROR ReadDomainNameComponent(const char *& in, char * out, size_t outSize)
{
    const char * dotPos = strchr(in, '.');
    VerifyOrReturnError(dotPos != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t componentSize = static_cast<size_t>(dotPos - in);
    VerifyOrReturnError(componentSize < outSize, CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(out, in, componentSize);
    out[componentSize] = '\0';
    in += componentSize + 1;

    return CHIP_NO_ERROR;
}

template <size_t N>
CHIP_ERROR ReadDomainNameComponent(const char *& in, char (&out)[N])
{
    return ReadDomainNameComponent(in, out, N);
}
#endif

NetworkCommissioning::otScanResponseIterator<NetworkCommissioning::ThreadScanResponse> mScanResponseIter;
} // namespace

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
    ChipDeviceEvent event{ .Type              = DeviceEventType::kThreadStateChange,
                           .ThreadStateChange = {
                               .RoleChanged    = (flags & OT_CHANGED_THREAD_ROLE) != 0,
                               .AddressChanged = (flags & (OT_CHANGED_IP6_ADDRESS_ADDED | OT_CHANGED_IP6_ADDRESS_REMOVED)) != 0,
                               .NetDataChanged = (flags & OT_CHANGED_THREAD_NETDATA) != 0,
                               .ChildNodesChanged =
                                   (flags & (OT_CHANGED_THREAD_CHILD_ADDED | OT_CHANGED_THREAD_CHILD_REMOVED)) != 0,
                               .OpenThread = { .Flags = flags } } };

    CHIP_ERROR status = PlatformMgr().PostEvent(&event);
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to post Thread state change: %" CHIP_ERROR_FORMAT, status.Format());
    }

    DeviceLayer::SystemLayer().ScheduleLambda([]() { ThreadStackMgrImpl()._UpdateNetworkStatus(); });
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ProcessThreadActivity()
{
    otTaskletsProcess(mOTInst);
    otSysProcessDrivers(mOTInst);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_HaveRouteToAddress(const Inet::IPAddress & destAddr)
{
    VerifyOrReturnValue(mOTInst, false);
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
                const Inet::IPPrefix prefix = ToIPPrefix(routeConfig.mPrefix);
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
        bool isThreadAttached = Impl()->_IsThreadAttached();
        // Avoid sending muliple events if the attachement state didn't change (Child->router or disable->Detached)
        if (event->ThreadStateChange.RoleChanged && (isThreadAttached != mIsAttached))
        {
            ChipDeviceEvent attachEvent{ .Type                     = DeviceEventType::kThreadConnectivityChange,
                                         .ThreadConnectivityChange = { .Result = (isThreadAttached) ? kConnectivity_Established
                                                                                                    : kConnectivity_Lost } };
            CHIP_ERROR status = PlatformMgr().PostEvent(&attachEvent);
            if (status == CHIP_NO_ERROR)
            {
                mIsAttached = isThreadAttached;
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to post Thread connectivity change: %" CHIP_ERROR_FORMAT, status.Format());
            }

            ThreadDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetThreadDiagnosticsDelegate();

            if (delegate)
            {
                if (mIsAttached)
                {
                    delegate->OnConnectionStatusChanged(app::Clusters::ThreadNetworkDiagnostics::ConnectionStatusEnum::kConnected);
                }
                else
                {
                    delegate->OnConnectionStatusChanged(
                        app::Clusters::ThreadNetworkDiagnostics::ConnectionStatusEnum::kNotConnected);

                    GeneralFaults<kMaxNetworkFaults> current;
                    current.add(to_underlying(chip::app::Clusters::ThreadNetworkDiagnostics::NetworkFaultEnum::kLinkDown));
                    delegate->OnNetworkFaultChanged(mNetworkFaults, current);
                    mNetworkFaults = current;
                }
            }
        }

#if CHIP_DETAIL_LOGGING
        Impl()->LockThreadStack();
        LogOpenThreadStateChange(mOTInst, event->ThreadStateChange.OpenThread.Flags);
        Impl()->UnlockThreadStack();
#endif // CHIP_DETAIL_LOGGING
    }
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadEnabled()
{
    VerifyOrReturnValue(mOTInst, false);
    otDeviceRole curRole;

    Impl()->LockThreadStack();
    curRole = otThreadGetDeviceRole(mOTInst);
    Impl()->UnlockThreadStack();

    return (curRole != OT_DEVICE_ROLE_DISABLED);
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadEnabled(bool val)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
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
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    otError otErr = OT_ERROR_FAILED;
    otOperationalDatasetTlvs tlvs;

    assert(netInfo.size() <= Thread::kSizeOperationalDataset);
    tlvs.mLength = static_cast<uint8_t>(netInfo.size());
    memcpy(tlvs.mTlvs, netInfo.data(), netInfo.size());

    // Set the dataset as the active dataset for the node.
    Impl()->LockThreadStack();
    otErr = otDatasetSetActiveTlvs(mOTInst, &tlvs);
    Impl()->UnlockThreadStack();
    if (otErr != OT_ERROR_NONE)
    {
        return MapOpenThreadError(otErr);
    }

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event{ .Type                      = DeviceEventType::kServiceProvisioningChange,
                           .ServiceProvisioningChange = { .IsServiceProvisioned = true } };
    return PlatformMgr().PostEvent(&event);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadProvisioned()
{
    VerifyOrReturnValue(mOTInst, false);
    bool provisioned;

    Impl()->LockThreadStack();
    provisioned = otDatasetIsCommissioned(mOTInst);
    Impl()->UnlockThreadStack();

    return provisioned;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetThreadProvision(Thread::OperationalDataset & dataset)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(Impl()->IsThreadProvisioned(), CHIP_ERROR_INCORRECT_STATE);
    otOperationalDatasetTlvs datasetTlv;

    Impl()->LockThreadStack();
    otError otErr = otDatasetGetActiveTlvs(mOTInst, &datasetTlv);
    Impl()->UnlockThreadStack();
    if (otErr != OT_ERROR_NONE)
    {
        return MapOpenThreadError(otErr);
    }

    ReturnErrorOnFailure(dataset.Init(ByteSpan(datasetTlv.mTlvs, datasetTlv.mLength)));

    return CHIP_NO_ERROR;
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_IsThreadAttached()
{
    VerifyOrReturnValue(mOTInst, false);
    otDeviceRole curRole;

    Impl()->LockThreadStack();
    curRole = otThreadGetDeviceRole(mOTInst);
    Impl()->UnlockThreadStack();

    return (curRole != OT_DEVICE_ROLE_DISABLED && curRole != OT_DEVICE_ROLE_DETACHED);
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_AttachToThreadNetwork(
    const Thread::OperationalDataset & dataset, NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback)
{
    Thread::OperationalDataset current_dataset;
    // Validate the dataset change with the current state
    ThreadStackMgrImpl().GetThreadProvision(current_dataset);
    if (dataset.AsByteSpan().data_equal(current_dataset.AsByteSpan()) && callback == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    // Reset the previously set callback since it will never be called in case incorrect dataset was supplied.
    mpConnectCallback = nullptr;
    ReturnErrorOnFailure(Impl()->SetThreadEnabled(false));
    ReturnErrorOnFailure(Impl()->SetThreadProvision(dataset.AsByteSpan()));

    if (dataset.IsCommissioned())
    {
        ReturnErrorOnFailure(Impl()->SetThreadEnabled(true));
        mpConnectCallback = callback;
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnThreadAttachFinished()
{
    if (mpConnectCallback != nullptr)
    {
        DeviceLayer::SystemLayer().ScheduleLambda([this]() {
            VerifyOrReturn(mpConnectCallback != nullptr);
            mpConnectCallback->OnResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);
            mpConnectCallback = nullptr;
        });
    }
}

template <class ImplClass>
CHIP_ERROR
GenericThreadStackManagerImpl_OpenThread<ImplClass>::_StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    otLinkModeConfig linkMode;
#endif
    // If there is another ongoing scan request, reject the new one.
    VerifyOrReturnError(mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);

    mpScanCallback = callback;

    Impl()->LockThreadStack();

    // Ensure that IPv6 interface is up when MLE Discovery is performed.
    if (!otIp6IsEnabled(mOTInst))
    {
        SuccessOrExit(error = MapOpenThreadError(otIp6SetEnabled(mOTInst, true)));
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Thread network discovery makes Sleepy End Devices detach from a network, so temporarily disable the SED mode.
    linkMode = otThreadGetLinkMode(mOTInst);

    if (!linkMode.mRxOnWhenIdle)
    {
        mTemporaryRxOnWhenIdle = true;
        linkMode.mRxOnWhenIdle = true;
        otThreadSetLinkMode(mOTInst, linkMode);
    }
#endif

    error = MapOpenThreadError(otThreadDiscover(mOTInst, 0,                       /* all channels */
                                                OT_PANID_BROADCAST, false, false, /* disable PAN ID, EUI64 and Joiner filtering */
                                                _OnNetworkScanFinished, this));

exit:
    Impl()->UnlockThreadStack();

    if (error != CHIP_NO_ERROR)
    {
        mpScanCallback = nullptr;
    }

    return error;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnNetworkScanFinished(otActiveScanResult * aResult, void * aContext)
{
    reinterpret_cast<GenericThreadStackManagerImpl_OpenThread *>(aContext)->_OnNetworkScanFinished(aResult);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnNetworkScanFinished(otActiveScanResult * aResult)
{
    if (aResult == nullptr) // scan completed
    {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        if (mTemporaryRxOnWhenIdle)
        {
            otLinkModeConfig linkMode = otThreadGetLinkMode(mOTInst);
            linkMode.mRxOnWhenIdle    = false;
            mTemporaryRxOnWhenIdle    = false;
            otThreadSetLinkMode(mOTInst, linkMode);
        }
#endif

        // If Thread scanning was done before commissioning, turn off the IPv6 interface.
        if (otThreadGetDeviceRole(mOTInst) == OT_DEVICE_ROLE_DISABLED && !otDatasetIsCommissioned(mOTInst))
        {
            DeviceLayer::SystemLayer().ScheduleLambda([this]() {
                Impl()->LockThreadStack();
                otIp6SetEnabled(mOTInst, false);
                Impl()->UnlockThreadStack();
            });
        }

        if (mpScanCallback != nullptr)
        {
            DeviceLayer::SystemLayer().ScheduleLambda([this]() {
                mpScanCallback->OnFinished(NetworkCommissioning::Status::kSuccess, CharSpan(), &mScanResponseIter);
                mpScanCallback = nullptr;
            });
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Thread Network: %s Panid 0x%x Channel %u RSSI %d LQI %u Version %u", aResult->mNetworkName.m8,
                        aResult->mPanId, aResult->mChannel, aResult->mRssi, aResult->mLqi, aResult->mVersion);

        NetworkCommissioning::ThreadScanResponse scanResponse = { 0 };

        scanResponse.panId           = aResult->mPanId;   // why is scanResponse.panID 64b
        scanResponse.channel         = aResult->mChannel; // why is scanResponse.channel 16b
        scanResponse.version         = aResult->mVersion;
        scanResponse.rssi            = aResult->mRssi;
        scanResponse.lqi             = aResult->mLqi;
        scanResponse.extendedAddress = Encoding::BigEndian::Get64(aResult->mExtAddress.m8);
        scanResponse.extendedPanId   = Encoding::BigEndian::Get64(aResult->mExtendedPanId.m8);
        static_assert(OT_NETWORK_NAME_MAX_SIZE <= UINT8_MAX, "Network name length won't fit");
        scanResponse.networkNameLen = static_cast<uint8_t>(strnlen(aResult->mNetworkName.m8, OT_NETWORK_NAME_MAX_SIZE));
        memcpy(scanResponse.networkName, aResult->mNetworkName.m8, scanResponse.networkNameLen);

        mScanResponseIter.Add(&scanResponse);
    }
}

template <class ImplClass>
ConnectivityManager::ThreadDeviceType GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetThreadDeviceType()
{
    VerifyOrReturnValue(mOTInst, ConnectivityManager::kThreadDeviceType_NotSupported);
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

#if CHIP_DEVICE_CONFIG_THREAD_SSED
#if OPENTHREAD_API_VERSION >= 347
    if (otLinkGetCslPeriod(mOTInst) != 0)
#else
    if (otLinkCslGetPeriod(mOTInst) != 0)
#endif // OPENTHREAD_API_VERSION
        ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
#endif // CHIP_DEVICE_CONFIG_THREAD_SSED

    ExitNow(deviceType = ConnectivityManager::kThreadDeviceType_SleepyEndDevice);

exit:
    Impl()->UnlockThreadStack();

    return deviceType;
}

template <class ImplClass>
CHIP_ERROR
GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
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
#if CHIP_DEVICE_CONFIG_THREAD_SSED
    case ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice:
#endif
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
#if CHIP_DEVICE_CONFIG_THREAD_SSED
        case ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice:
            deviceTypeStr = "SYNCHRONIZED SLEEPY END DEVICE";
            break;
#endif
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
    case ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice:
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
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::_HaveMeshConnectivity()
{
    VerifyOrReturnValue(mOTInst, false);
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
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadStatsCounters()
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr;
    otOperationalDataset activeDataset;

    Impl()->LockThreadStack();
#if CHIP_PROGRESS_LOGGING
    {
        otDeviceRole role;

        role = otThreadGetDeviceRole(mOTInst);
        ChipLogProgress(DeviceLayer, "Thread Role:                  %d\n", role);
    }
#endif // CHIP_PROGRESS_LOGGING

    if (otDatasetIsCommissioned(mOTInst))
    {
        otErr = otDatasetGetActive(mOTInst, &activeDataset);
        VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

        if (activeDataset.mComponents.mIsChannelPresent)
        {
            ChipLogProgress(DeviceLayer, "Thread Channel:               %d\n", activeDataset.mChannel);
        }
    }

#if CHIP_PROGRESS_LOGGING
    {
        const otIpCounters * ipCounters;
        const otMacCounters * macCounters;

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
    }
#endif // CHIP_PROGRESS_LOGGING
exit:
    Impl()->UnlockThreadStack();
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyMinimal()
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_PROGRESS_LOGGING
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
                    "Partition ID:     %" PRIu32 "\n",
                    rloc16, routerId, leaderRouterId, parentAverageRssi, parentLastRssi, partitionId);

    ChipLogProgress(DeviceLayer,
                    "Extended Address: %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Instant RSSI:     %d\n",
                    extAddress->m8[0], extAddress->m8[1], extAddress->m8[2], extAddress->m8[3], extAddress->m8[4],
                    extAddress->m8[5], extAddress->m8[6], extAddress->m8[7], instantRssi);

exit:
    Impl()->UnlockThreadStack();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyMinimul failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif // CHIP_PROGRESS_LOGGING
    return err;
}

#define TELEM_NEIGHBOR_TABLE_SIZE (64)
#define TELEM_PRINT_BUFFER_SIZE (64)

#if CHIP_DEVICE_CONFIG_THREAD_FTD
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetAndLogThreadTopologyFull()
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_PROGRESS_LOGGING
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
    char printBuf[TELEM_PRINT_BUFFER_SIZE] = { 0 };
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

    snprintf(printBuf, TELEM_PRINT_BUFFER_SIZE, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
             leaderAddr->mFields.m8[0], leaderAddr->mFields.m8[1], leaderAddr->mFields.m8[2], leaderAddr->mFields.m8[3],
             leaderAddr->mFields.m8[4], leaderAddr->mFields.m8[5], leaderAddr->mFields.m8[6], leaderAddr->mFields.m8[7],
             leaderAddr->mFields.m8[8], leaderAddr->mFields.m8[9], leaderAddr->mFields.m8[10], leaderAddr->mFields.m8[11],
             leaderAddr->mFields.m8[12], leaderAddr->mFields.m8[13], leaderAddr->mFields.m8[14], leaderAddr->mFields.m8[15]);

    ChipLogProgress(DeviceLayer,
                    "Thread Topology:\n"
                    "RLOC16:                        %04X\n"
                    "Router ID:                     %u\n"
                    "Leader Router ID:              %u\n"
                    "Leader Address:                %s\n"
                    "Leader Weight:                 %d\n"
                    "Local Leader Weight:           %d\n"
                    "Network Data Len:              %d\n"
                    "Network Data Version:          %d\n"
                    "Stable Network Data Version:   %d\n",
                    rloc16, routerId, leaderRouterId, printBuf, leaderWeight, leaderLocalWeight, networkDataLen, networkDataVersion,
                    stableNetworkDataVersion);

    memset(printBuf, 0x00, TELEM_PRINT_BUFFER_SIZE);

    ChipLogProgress(DeviceLayer,
                    "Extended Address:              %02X%02X:%02X%02X:%02X%02X:%02X%02X\n"
                    "Partition ID:                  %" PRIx32 "\n"
                    "Instant RSSI:                  %d\n"
                    "Neighbor Table Length:         %d\n"
                    "Child Table Length:            %d\n",
                    extAddress->m8[0], extAddress->m8[1], extAddress->m8[2], extAddress->m8[3], extAddress->m8[4],
                    extAddress->m8[5], extAddress->m8[6], extAddress->m8[7], partitionId, instantRssi, neighborTableSize,
                    childTableSize);

    // Handle each neighbor event seperatly.
    for (uint32_t i = 0; i < neighborTableSize; i++)
    {
        otNeighborInfo * neighbor = &neighborInfo[i];

        if (neighbor->mIsChild)
        {
            otChildInfo * child = NULL;
            otErr               = otThreadGetChildInfoById(mOTInst, neighbor->mRloc16, child);
            VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

            snprintf(printBuf, TELEM_PRINT_BUFFER_SIZE, ", Timeout: %10" PRIu32 " NetworkDataVersion: %3u", child->mTimeout,
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
                        "LastRSSI:          %3d\n",
                        i, neighbor->mExtAddress.m8[0], neighbor->mExtAddress.m8[1], neighbor->mExtAddress.m8[2],
                        neighbor->mExtAddress.m8[3], neighbor->mExtAddress.m8[4], neighbor->mExtAddress.m8[5],
                        neighbor->mExtAddress.m8[6], neighbor->mExtAddress.m8[7], neighbor->mRloc16, neighbor->mAge,
                        neighbor->mLinkQualityIn, neighbor->mAverageRssi, neighbor->mLastRssi);

        ChipLogProgress(DeviceLayer,
                        "LinkFrameCounter:  %10" PRIu32 "\n"
                        "MleFrameCounter:   %10" PRIu32 "\n"
                        "RxOnWhenIdle:      %c\n"
                        "FullFunction:      %c\n"
                        "FullNetworkData:   %c\n"
                        "IsChild:           %c%s\n",
                        neighbor->mLinkFrameCounter, neighbor->mMleFrameCounter, neighbor->mRxOnWhenIdle ? 'Y' : 'n',
                        neighbor->mFullThreadDevice ? 'Y' : 'n', neighbor->mFullNetworkData ? 'Y' : 'n',
                        neighbor->mIsChild ? 'Y' : 'n', printBuf);
    }

exit:
    Impl()->UnlockThreadStack();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetAndLogThreadTopologyFull failed: %s", ErrorStr(err));
    }
#endif // CHIP_PROGRESS_LOGGING
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
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    const otExtAddress * extendedAddr = otLinkGetExtendedAddress(mOTInst);
    memcpy(buf, extendedAddr, sizeof(otExtAddress));
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
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
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetThreadVersion(uint16_t & version)
{
    version = otThreadGetVersion();
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ResetThreadNetworkDiagnosticsCounts()
{
    // Based on the spec, only OverrunCount should be resetted.
    mOverrunCount = 0;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_GetPollPeriod(uint32_t & buf)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    Impl()->LockThreadStack();
    buf = otLinkGetPollPeriod(mOTInst);
    Impl()->UnlockThreadStack();
    return CHIP_NO_ERROR;
}

/**
 * @brief Helper that sets callbacks for OpenThread state changes and configures the Thread stack.
 * Assigns mOTInst to an instance, and configures the OT stack on a device by setting state change callbacks enabling features
 * for IPv6 address configuration, enabling the Thread network if necessary, and handling SRP if enabled.
 * Allows for the configuration of the Thread stack on a device where the instance and the otCLI are already initialised.
 *
 * @param otInst Pointer to the OT instance
 * @return CHIP_ERROR OpenThread error mapped to CHIP_ERROR
 */
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::ConfigureThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    otError otErr  = OT_ERROR_NONE;

    mOTInst = otInst;

    // Arrange for OpenThread to call the OnOpenThreadStateChange method whenever a
    // state change occurs.  Note that we reference the OnOpenThreadStateChange method
    // on the concrete implementation class so that that class can override the default
    // method implementation if it chooses to.
    otErr = otSetStateChangedCallback(otInst, ImplClass::OnOpenThreadStateChange, mOTInst);
    VerifyOrExit(otErr == OT_ERROR_NONE, err = MapOpenThreadError(otErr));

    // Enable automatic assignment of Thread advertised addresses.
#if defined(OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE) && OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE
    otIp6SetSlaacEnabled(otInst, true);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    otSrpClientSetCallback(mOTInst, &OnSrpClientNotification, nullptr);
    otSrpClientEnableAutoStartMode(mOTInst, &OnSrpClientStateChange, nullptr);
    memset(&mSrpClient, 0, sizeof(mSrpClient));
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_AUTOSTART
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
#endif

exit:

    ChipLogProgress(DeviceLayer, "OpenThread started: %s", otThreadErrorToString(otErr));
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::DoInit(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Arrange for OpenThread errors to be translated to text.
    RegisterOpenThreadErrorFormatter();

    mOTInst = NULL;

    // If an OpenThread instance hasn't been supplied, call otInstanceInitSingle() to
    // create or acquire a singleton instance of OpenThread.
    if (otInst == NULL)
    {
        otInst = otInstanceInitSingle();
        VerifyOrExit(otInst != NULL, err = MapOpenThreadError(OT_ERROR_FAILED));
    }

#if !defined(PW_RPC_ENABLED) && CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
    otAppCliInit(otInst);
#endif

    err = ConfigureThreadStack(otInst);

    initNetworkCommissioningThreadDriver();

exit:
    return err;
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::IsThreadAttachedNoLock()
{
    otDeviceRole curRole = otThreadGetDeviceRole(mOTInst);
    return (curRole != OT_DEVICE_ROLE_DISABLED && curRole != OT_DEVICE_ROLE_DETACHED);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::IsThreadInterfaceUpNoLock()
{
    return otIp6IsEnabled(mOTInst);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = CHIP_NO_ERROR;
    Impl()->LockThreadStack();

// For Thread devices, the intervals are defined as:
// * poll period for SED devices that poll the parent for data
// * CSL period for SSED devices that listen for messages in scheduled time slots.
#if CHIP_DEVICE_CONFIG_THREAD_SSED
#if OPENTHREAD_API_VERSION >= 347
    // Get CSL period in units of us and divide by 1000 to get milliseconds.
    uint32_t curIntervalMS = otLinkGetCslPeriod(mOTInst) / 1000;
#else
    // Get CSL period in units of 10 symbols, convert it to microseconds and divide by 1000 to get milliseconds.
    uint32_t curIntervalMS = otLinkCslGetPeriod(mOTInst) * OT_US_PER_TEN_SYMBOLS / 1000;
#endif // OPENTHREAD_API_VERSION
#else
    uint32_t curIntervalMS = otLinkGetPollPeriod(mOTInst);
#endif
    otError otErr = OT_ERROR_NONE;
    if (pollingInterval.count() != curIntervalMS)
    {
#if CHIP_DEVICE_CONFIG_THREAD_SSED
#if OPENTHREAD_API_VERSION >= 347
        // Set CSL period in units of us and divide by 1000 to get milliseconds.
        otErr         = otLinkSetCslPeriod(mOTInst, pollingInterval.count() * 1000);
        curIntervalMS = otLinkGetCslPeriod(mOTInst) / 1000;
#else
        // Set CSL period in units of 10 symbols, convert it to microseconds and divide by 1000 to get milliseconds.
        otErr         = otLinkCslSetPeriod(mOTInst, pollingInterval.count() * 1000 / OT_US_PER_TEN_SYMBOLS);
        curIntervalMS = otLinkCslGetPeriod(mOTInst) * OT_US_PER_TEN_SYMBOLS / 1000;
#endif // OPENTHREAD_API_VERSION
#else
        otErr         = otLinkSetPollPeriod(mOTInst, pollingInterval.count());
        curIntervalMS = otLinkGetPollPeriod(mOTInst);
#endif
        err = MapOpenThreadError(otErr);
    }

    Impl()->UnlockThreadStack();

    if (otErr != OT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Failed to set SED interval to %" PRId32 "ms. Defaulting to %" PRId32 "ms",
                     pollingInterval.count(), curIntervalMS);
    }

    return err;
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ErasePersistentInfo()
{
    VerifyOrReturn(mOTInst);
    ChipLogProgress(DeviceLayer, "Erasing Thread persistent info...");
    Impl()->LockThreadStack();
    otThreadSetEnabled(mOTInst, false);
    otIp6SetEnabled(mOTInst, false);
    otInstanceErasePersistentInfo(mOTInst);
    resetGenericThreadDriver();
    Impl()->UnlockThreadStack();
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::_UpdateNetworkStatus()
{
    VerifyOrReturn(mOTInst);
    // Thread is not enabled, then we are not trying to connect to the network.
    VerifyOrReturn(ThreadStackMgrImpl().IsThreadEnabled() && mpStatusChangeCallback != nullptr);

    ByteSpan datasetTLV;
    Thread::OperationalDataset dataset;
    ByteSpan extpanid;

    // If we have not provisioned any Thread network, return the status from last network scan,
    // If we have provisioned a network, we assume the ot-br-posix is activitely connecting to that network.
    ReturnOnFailure(ThreadStackMgrImpl().GetThreadProvision(dataset));
    // The Thread network is not enabled, but has a different extended pan id.
    ReturnOnFailure(dataset.GetExtendedPanIdAsByteSpan(extpanid));
    // If we don't have a valid dataset, we are not attempting to connect the network.

    // We have already connected to the network, thus return success.
    if (ThreadStackMgrImpl().IsThreadAttached())
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(NetworkCommissioning::Status::kSuccess, MakeOptional(extpanid),
                                                         NullOptional);
    }
    else
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(NetworkCommissioning::Status::kNetworkNotFound, MakeOptional(extpanid),
                                                         MakeOptional(static_cast<int32_t>(OT_ERROR_DETACHED)));
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnSrpClientNotification(otError aError,
                                                                                  const otSrpClientHostInfo * aHostInfo,
                                                                                  const otSrpClientService * aServices,
                                                                                  const otSrpClientService * aRemovedServices,
                                                                                  void * aContext)
{
    const char * errorStr = nullptr;

    switch (aError)
    {
    case OT_ERROR_NONE: {
        ChipLogDetail(DeviceLayer, "SRP update succeeded");

        if (aHostInfo)
        {
            if (aHostInfo->mState == OT_SRP_CLIENT_ITEM_STATE_REMOVED)
            {
                // Clear memory for removed host
                memset(ThreadStackMgrImpl().mSrpClient.mHostName, 0, sizeof(ThreadStackMgrImpl().mSrpClient.mHostName));

                ThreadStackMgrImpl().mSrpClient.mIsInitialized = true;
                ThreadStackMgrImpl().mSrpClient.mInitializedCallback(ThreadStackMgrImpl().mSrpClient.mCallbackContext,
                                                                     CHIP_NO_ERROR);

                if (ThreadStackMgrImpl().mIsSrpClearAllRequested)
                {
                    ThreadStackMgrImpl().NotifySrpClearAllComplete();
                    ThreadStackMgrImpl().mIsSrpClearAllRequested = false;
                }
            }
        }

        if (aRemovedServices)
        {
            otSrpClientService * otService = const_cast<otSrpClientService *>(aRemovedServices);
            otSrpClientService * next      = nullptr;
            using Service                  = typename SrpClient::Service;

            // Clear memory for all removed services.
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
        errorStr = "parsing operation failed";
        break;
    case OT_ERROR_NOT_FOUND:
        errorStr = "domain name or RRset does not exist";
        break;
    case OT_ERROR_NOT_IMPLEMENTED:
        errorStr = "server does not support query type";
        break;
    case OT_ERROR_SECURITY:
        errorStr = "operation refused for security reasons";
        break;
    case OT_ERROR_DUPLICATED:
        errorStr = "domain name or RRset is duplicated";
        break;
    case OT_ERROR_RESPONSE_TIMEOUT:
        errorStr = "timed out waiting on server response";
        break;
    case OT_ERROR_INVALID_ARGS:
        errorStr = "invalid service structure detected";
        break;
    case OT_ERROR_NO_BUFS:
        errorStr = "insufficient buffer to handle message";
        break;
    case OT_ERROR_FAILED:
        errorStr = "internal server error";
        break;
    default:
        errorStr = "unknown error";
        break;
    }

    if (errorStr != nullptr)
    {
        ChipLogError(DeviceLayer, "SRP update error: %s", errorStr);
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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
        // Set DNS server config to be set at the SRP server address
        otDnsQueryConfig dnsConfig         = *otDnsClientGetDefaultConfig(ThreadStackMgrImpl().OTInstance());
        dnsConfig.mServerSockAddr.mAddress = aServerSockAddr->mAddress;
        otDnsClientSetDefaultConfig(ThreadStackMgrImpl().OTInstance(), &dnsConfig);
#endif
    }
    else
    {
        ChipLogProgress(DeviceLayer, "SRP Client was stopped, because current server is no longer detected.");
    }
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::SrpClient::Service::Matches(const char * instanceName,
                                                                                      const char * name) const
{
    return IsUsed() && (strcmp(mService.mInstanceName, instanceName) == 0) && (strcmp(mService.mName, name) == 0);
}

template <class ImplClass>
bool GenericThreadStackManagerImpl_OpenThread<ImplClass>::SrpClient::Service::Matches(
    const char * instanceName, const char * name, uint16_t port, const Span<const char * const> & subTypes,
    const Span<const Dnssd::TextEntry> & txtEntries) const
{
    size_t myNumSubTypes = 0;

    for (const char * const * mySubType = mService.mSubTypeLabels; (mySubType != nullptr) && (*mySubType != nullptr); ++mySubType)
    {
        myNumSubTypes++;
    }

    VerifyOrReturnError(Matches(instanceName, name) && mService.mPort == port, false);
    VerifyOrReturnError(myNumSubTypes == subTypes.size() && mService.mNumTxtEntries == txtEntries.size(), false);

    const char * const * mySubType = mService.mSubTypeLabels;

    for (const char * subType : subTypes)
    {
        VerifyOrReturnError(strcmp(*mySubType, subType) == 0, false);
        ++mySubType;
    }

    const otDnsTxtEntry * myTxtEntry = mService.mTxtEntries;

    for (const Dnssd::TextEntry & txtEntry : txtEntries)
    {
        VerifyOrReturnError(strcmp(myTxtEntry->mKey, txtEntry.mKey) == 0, false);
        VerifyOrReturnError(
            ByteSpan(myTxtEntry->mValue, myTxtEntry->mValueLength).data_equal(ByteSpan(txtEntry.mData, txtEntry.mDataSize)), false);
        ++myTxtEntry;
    }

    return true;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_AddSrpService(const char * aInstanceName, const char * aName,
                                                                               uint16_t aPort,
                                                                               const Span<const char * const> & aSubTypes,
                                                                               const Span<const Dnssd::TextEntry> & aTxtEntries,
                                                                               uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval)
{
    CHIP_ERROR error                         = CHIP_NO_ERROR;
    typename SrpClient::Service * srpService = nullptr;
    size_t entryId                           = 0;
    FixedBufferAllocator alloc;

    VerifyOrReturnError(mSrpClient.mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(aInstanceName, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aName, CHIP_ERROR_INVALID_ARGUMENT);

    Impl()->LockThreadStack();

    // Try to find an empty slot in array for the new service and
    // remove the possible existing entry from anywhere in the list
    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        if (service.Matches(aInstanceName, aName, aPort, aSubTypes, aTxtEntries))
        {
            // Re-adding existing service without any changes
            service.mIsInvalid = false;
            ExitNow();
        }

        if (service.Matches(aInstanceName, aName))
        {
            // Updating existing service
            SuccessOrExit(error = MapOpenThreadError(otSrpClientClearService(mOTInst, &service.mService)));
            // Clear memory immediately, as OnSrpClientNotification will not be called.
            memset(&service, 0, sizeof(service));
        }

        if ((srpService == nullptr) && !service.IsUsed())
        {
            // Assign first empty slot found in array for a new service.
            srpService = &service;

            // Keep looping to remove possible existing entry further in the list
        }
    }

    // Verify there is a slot found for the new service.
    VerifyOrExit(srpService != nullptr, error = CHIP_ERROR_BUFFER_TOO_SMALL);
    alloc.Init(srpService->mServiceBuffer);

    otSrpClientSetLeaseInterval(mOTInst, aLeaseInterval);
    otSrpClientSetKeyLeaseInterval(mOTInst, aKeyLeaseInterval);

    srpService->mService.mInstanceName = alloc.Clone(aInstanceName);
    srpService->mService.mName         = alloc.Clone(aName);
    srpService->mService.mPort         = aPort;

    VerifyOrExit(aSubTypes.size() < ArraySize(srpService->mSubTypes), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    entryId = 0;

    for (const char * subType : aSubTypes)
    {
        srpService->mSubTypes[entryId++] = alloc.Clone(subType);
    }

    srpService->mSubTypes[entryId]      = nullptr;
    srpService->mService.mSubTypeLabels = srpService->mSubTypes;

    // Initialize TXT entries
    VerifyOrExit(aTxtEntries.size() <= ArraySize(srpService->mTxtEntries), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    entryId = 0;

    for (const chip::Dnssd::TextEntry & entry : aTxtEntries)
    {
        using OtTxtValueLength = decltype(srpService->mTxtEntries[entryId].mValueLength);
        static_assert(SrpClient::kServiceBufferSize <= std::numeric_limits<OtTxtValueLength>::max(),
                      "DNS TXT value length may not fit in otDnsTxtEntry structure");

        // TXT entry keys are constants, so they don't need to be cloned
        srpService->mTxtEntries[entryId].mKey         = entry.mKey;
        srpService->mTxtEntries[entryId].mValue       = alloc.Clone(entry.mData, entry.mDataSize);
        srpService->mTxtEntries[entryId].mValueLength = static_cast<OtTxtValueLength>(entry.mDataSize);
        ++entryId;
    }

    using OtNumTxtEntries = decltype(srpService->mService.mNumTxtEntries);
    static_assert(ArraySize(srpService->mTxtEntries) <= std::numeric_limits<OtNumTxtEntries>::max(),
                  "Number of DNS TXT entries may not fit in otSrpClientService structure");
    srpService->mService.mNumTxtEntries = static_cast<OtNumTxtEntries>(aTxtEntries.size());
    srpService->mService.mTxtEntries    = srpService->mTxtEntries;

    VerifyOrExit(!alloc.AnyAllocFailed(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    ChipLogProgress(DeviceLayer, "advertising srp service: %s.%s", srpService->mService.mInstanceName, srpService->mService.mName);
    error = MapOpenThreadError(otSrpClientAddService(mOTInst, &(srpService->mService)));

exit:
    if (srpService != nullptr && error != CHIP_NO_ERROR)
    {
        memset(srpService, 0, sizeof(*srpService));
    }

    Impl()->UnlockThreadStack();
    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_RemoveSrpService(const char * aInstanceName, const char * aName)
{
    CHIP_ERROR error                         = CHIP_NO_ERROR;
    typename SrpClient::Service * srpService = nullptr;

    VerifyOrReturnError(mSrpClient.mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(aInstanceName, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aName, CHIP_ERROR_INVALID_ARGUMENT);

    Impl()->LockThreadStack();

    // Check if service to remove exists.
    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        if (service.Matches(aInstanceName, aName))
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
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_InvalidateAllSrpServices()
{
    Impl()->LockThreadStack();

    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        if (service.IsUsed())
        {
            service.mIsInvalid = true;
        }
    }

    Impl()->UnlockThreadStack();
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_RemoveInvalidSrpServices()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mSrpClient.mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    Impl()->LockThreadStack();

    for (typename SrpClient::Service & service : mSrpClient.mServices)
    {
        if (service.IsUsed() && service.mIsInvalid)
        {
            ChipLogProgress(DeviceLayer, "removing srp service: %s.%s", service.mService.mInstanceName, service.mService.mName);
            error = MapOpenThreadError(otSrpClientRemoveService(mOTInst, &service.mService));
            SuccessOrExit(error);
        }
    }

exit:
    Impl()->UnlockThreadStack();

    return error;
}

/*
 * @brief This is a utility function to remove all Thread client srp host and services
 * established between the device and the srp server (in most cases the OTBR).
 * The calling task is blocked until OnSrpClientNotification which confims the client received the request.
 * The blocking mechanism is defined by the platform implementation of `WaitOnSrpClearAllComplete` and `NotifySrpClearAllComplete`
 *
 * Note: This function is meant to be used during the factory reset sequence.
 *
 */
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ClearAllSrpHostAndServices()
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;
    Impl()->LockThreadStack();
    if (!mIsSrpClearAllRequested)
    {
        error =
            MapOpenThreadError(otSrpClientRemoveHostAndServices(mOTInst, true /*aRemoveKeyLease*/, true /*aSendUnregToServer*/));
        mIsSrpClearAllRequested = true;
        Impl()->UnlockThreadStack();
        Impl()->WaitOnSrpClearAllComplete();
    }
    else
    {
        Impl()->UnlockThreadStack();
    }
    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetupSrpHost(const char * aHostName)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mSrpClient.mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    Impl()->LockThreadStack();

    VerifyOrExit(aHostName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aHostName) <= Dnssd::kHostNameMaxLength, error = CHIP_ERROR_INVALID_STRING_LENGTH);

    // Avoid adding the same host name multiple times
    if (strcmp(mSrpClient.mHostName, aHostName) != 0)
    {
        strcpy(mSrpClient.mHostName, aHostName);
        error = MapOpenThreadError(otSrpClientSetHostName(mOTInst, mSrpClient.mHostName));
        SuccessOrExit(error);

        error = MapOpenThreadError(otSrpClientEnableAutoHostAddress(mOTInst));
    }

exit:
    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_ClearSrpHost(const char * aHostName)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;

    Impl()->LockThreadStack();

    VerifyOrExit(aHostName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(aHostName) <= Dnssd::kHostNameMaxLength, error = CHIP_ERROR_INVALID_STRING_LENGTH);
    VerifyOrExit(mSrpClient.mInitializedCallback, error = CHIP_ERROR_INCORRECT_STATE);

    // Add host and remove it with notifying SRP server to clean old information related to the host.
    // Avoid adding the same host name multiple times
    if (strcmp(mSrpClient.mHostName, aHostName) != 0)
    {
        strcpy(mSrpClient.mHostName, aHostName);
        error = MapOpenThreadError(otSrpClientSetHostName(mOTInst, mSrpClient.mHostName));
        SuccessOrExit(error);
    }
    error = MapOpenThreadError(otSrpClientRemoveHostAndServices(mOTInst, false, true));

exit:
    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_SetSrpDnsCallbacks(DnsAsyncReturnCallback aInitCallback,
                                                                                    DnsAsyncReturnCallback aErrorCallback,
                                                                                    void * aContext)
{
    mSrpClient.mInitializedCallback = aInitCallback;
    mSrpClient.mCallbackContext     = aContext;
    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::FromOtDnsResponseToMdnsData(
    otDnsServiceInfo & serviceInfo, const char * serviceType, chip::Dnssd::DnssdService & mdnsService,
    DnsServiceTxtEntries & serviceTxtEntries, otError error)
{
    char protocol[chip::Dnssd::kDnssdProtocolTextMaxSize + 1];

    ReturnErrorOnFailure(ReadDomainNameComponent(serviceType, mdnsService.mType));
    ReturnErrorOnFailure(ReadDomainNameComponent(serviceType, protocol));

    if (strncmp(protocol, "_udp", chip::Dnssd::kDnssdProtocolTextMaxSize) == 0)
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUdp;
    }
    else if (strncmp(protocol, "_tcp", chip::Dnssd::kDnssdProtocolTextMaxSize) == 0)
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolTcp;
    }
    else
    {
        mdnsService.mProtocol = chip::Dnssd::DnssdServiceProtocol::kDnssdProtocolUnknown;
    }

    mdnsService.mInterface     = Inet::InterfaceId::Null();
    mdnsService.mSubTypeSize   = 0;
    mdnsService.mTextEntrySize = 0;

    // Check if SRV record was included in DNS response.
    // If not, return partial information about the service and exit early.
    if (error != OT_ERROR_NONE)
    {
        return CHIP_NO_ERROR;
    }

    const char * host = serviceInfo.mHostNameBuffer;
    ReturnErrorOnFailure(ReadDomainNameComponent(host, mdnsService.mHostName));
    mdnsService.mPort = serviceInfo.mPort;

    // Check if AAAA record was included in DNS response.

    if (!otIp6IsAddressUnspecified(&serviceInfo.mHostAddress))
    {
        mdnsService.mAddressType = Inet::IPAddressType::kIPv6;
        mdnsService.mAddress     = std::make_optional(ToIPAddress(serviceInfo.mHostAddress));
    }

    // Check if TXT record was included in DNS response.
    if (serviceInfo.mTxtDataSize != 0)
    {
        otDnsTxtEntryIterator iterator;
        otDnsInitTxtEntryIterator(&iterator, serviceInfo.mTxtData, serviceInfo.mTxtDataSize);

        otDnsTxtEntry txtEntry;
        FixedBufferAllocator alloc(serviceTxtEntries.mBuffer);

        uint8_t entryIndex = 0;
        while ((otDnsGetNextTxtEntry(&iterator, &txtEntry) == OT_ERROR_NONE) && entryIndex < kMaxDnsServiceTxtEntriesNumber)
        {
            if (txtEntry.mKey == nullptr || txtEntry.mValue == nullptr)
                continue;

            serviceTxtEntries.mTxtEntries[entryIndex].mKey      = alloc.Clone(txtEntry.mKey);
            serviceTxtEntries.mTxtEntries[entryIndex].mData     = alloc.Clone(txtEntry.mValue, txtEntry.mValueLength);
            serviceTxtEntries.mTxtEntries[entryIndex].mDataSize = txtEntry.mValueLength;
            entryIndex++;
        }

        VerifyOrReturnError(!alloc.AnyAllocFailed(), CHIP_ERROR_BUFFER_TOO_SMALL);

        mdnsService.mTextEntries   = serviceTxtEntries.mTxtEntries;
        mdnsService.mTextEntrySize = entryIndex;
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::ResolveAddress(intptr_t context, otDnsAddressCallback callback)
{
    VerifyOrReturnError(ThreadStackMgrImpl().OTInstance(), CHIP_ERROR_INCORRECT_STATE);
    DnsResult * dnsResult = reinterpret_cast<DnsResult *>(context);

    ThreadStackMgrImpl().LockThreadStack();

    char fullHostName[chip::Dnssd::kHostNameMaxLength + 1 + SrpClient::kDefaultDomainNameSize + 1];
    snprintf(fullHostName, sizeof(fullHostName), "%s.%s", dnsResult->mMdnsService.mHostName, SrpClient::kDefaultDomainName);

    CHIP_ERROR error = MapOpenThreadError(otDnsClientResolveAddress(ThreadStackMgrImpl().OTInstance(), fullHostName, callback,
                                                                    reinterpret_cast<void *>(dnsResult), NULL));

    ThreadStackMgrImpl().UnlockThreadStack();

    return error;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchAddressResolve(intptr_t context)
{
    CHIP_ERROR error = ResolveAddress(context, OnDnsAddressResolveResult);

    // In case of address resolve failure, fill the error code field and dispatch method to end resolve process.
    if (error != CHIP_NO_ERROR)
    {
        DnsResult * dnsResult = reinterpret_cast<DnsResult *>(context);
        dnsResult->error      = error;

        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(dnsResult));
    }
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchResolve(intptr_t context)
{
    DnsResult * dnsResult         = reinterpret_cast<DnsResult *>(context);
    Dnssd::DnssdService & service = dnsResult->mMdnsService;
    Span<Inet::IPAddress> ipAddrs;

    if (service.mAddress.has_value())
    {
        ipAddrs = Span<Inet::IPAddress>(&*service.mAddress, 1);
    }

    ThreadStackMgrImpl().mDnsResolveCallback(dnsResult->context, &service, ipAddrs, dnsResult->error);
    Platform::Delete<DnsResult>(dnsResult);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchResolveNoMemory(intptr_t context)
{
    Span<Inet::IPAddress> ipAddrs;
    ThreadStackMgrImpl().mDnsResolveCallback(reinterpret_cast<void *>(context), nullptr, ipAddrs, CHIP_ERROR_NO_MEMORY);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchBrowseEmpty(intptr_t context)
{
    auto * dnsResult = reinterpret_cast<DnsResult *>(context);
    ThreadStackMgrImpl().mDnsBrowseCallback(dnsResult->context, nullptr, 0, true, dnsResult->error);
    Platform::Delete<DnsResult>(dnsResult);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchBrowse(intptr_t context)
{
    auto * dnsResult = reinterpret_cast<DnsResult *>(context);
    ThreadStackMgrImpl().mDnsBrowseCallback(dnsResult->context, &dnsResult->mMdnsService, 1, false, dnsResult->error);
    Platform::Delete<DnsResult>(dnsResult);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::DispatchBrowseNoMemory(intptr_t context)
{
    ThreadStackMgrImpl().mDnsBrowseCallback(reinterpret_cast<void *>(context), nullptr, 0, true, CHIP_ERROR_NO_MEMORY);
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnDnsBrowseResult(otError aError, const otDnsBrowseResponse * aResponse,
                                                                            void * aContext)
{
    CHIP_ERROR error;
    // type buffer size is kDnssdTypeAndProtocolMaxSize + . + kMaxDomainNameSize + . + termination character
    char type[Dnssd::kDnssdTypeAndProtocolMaxSize + SrpClient::kMaxDomainNameSize + 3];
    // hostname buffer size is kHostNameMaxLength + . + kMaxDomainNameSize + . + termination character
    char hostname[Dnssd::kHostNameMaxLength + SrpClient::kMaxDomainNameSize + 3];
    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxDnsServiceTxtEntriesNumber + kTotalDnsServiceTxtBufferSize];
    otDnsServiceInfo serviceInfo;
    uint16_t index = 0;

    if (ThreadStackMgrImpl().mDnsBrowseCallback == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid dns browse callback");
        return;
    }

    VerifyOrExit(aError == OT_ERROR_NONE, error = MapOpenThreadError(aError));

    error = MapOpenThreadError(otDnsBrowseResponseGetServiceName(aResponse, type, sizeof(type)));

    VerifyOrExit(error == CHIP_NO_ERROR, );

    char serviceName[Dnssd::Common::kInstanceNameMaxLength + 1];
    while (otDnsBrowseResponseGetServiceInstance(aResponse, index, serviceName, sizeof(serviceName)) == OT_ERROR_NONE)
    {
        serviceInfo.mHostNameBuffer     = hostname;
        serviceInfo.mHostNameBufferSize = sizeof(hostname);
        serviceInfo.mTxtData            = txtBuffer;
        serviceInfo.mTxtDataSize        = sizeof(txtBuffer);

        otError err = otDnsBrowseResponseGetServiceInfo(aResponse, serviceName, &serviceInfo);
        error       = MapOpenThreadError(err);

        VerifyOrExit(err == OT_ERROR_NOT_FOUND || err == OT_ERROR_NONE, );

        DnsResult * dnsResult = Platform::New<DnsResult>(aContext, CHIP_NO_ERROR);

        VerifyOrExit(dnsResult != nullptr, error = CHIP_ERROR_NO_MEMORY);

        error = FromOtDnsResponseToMdnsData(serviceInfo, type, dnsResult->mMdnsService, dnsResult->mServiceTxtEntry, err);
        if (CHIP_NO_ERROR == error)
        {
            // Invoke callback for every service one by one instead of for the whole
            // list due to large memory size needed to allocate on stack.
            static_assert(ArraySize(dnsResult->mMdnsService.mName) >= ArraySize(serviceName),
                          "The target buffer must be big enough");
            Platform::CopyString(dnsResult->mMdnsService.mName, serviceName);
            DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowse, reinterpret_cast<intptr_t>(dnsResult));
        }
        else
        {
            Platform::Delete<DnsResult>(dnsResult);
        }
        index++;
    }

exit:
    // Invoke callback to notify about end-of-browse or failure
    DnsResult * dnsResult = Platform::New<DnsResult>(aContext, error);

    if (dnsResult == nullptr)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseNoMemory, reinterpret_cast<intptr_t>(aContext));
    }
    else
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchBrowseEmpty, reinterpret_cast<intptr_t>(dnsResult));
    }
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback,
                                                                           void * aContext)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;

    Impl()->LockThreadStack();

    VerifyOrExit(aServiceName, error = CHIP_ERROR_INVALID_ARGUMENT);

    mDnsBrowseCallback = aCallback;

    // Append default SRP domain name to the service name.
    // fullServiceName buffer size is kDnssdFullTypeAndProtocolMaxSize + . + kDefaultDomainNameSize + null-terminator.
    char fullServiceName[Dnssd::kDnssdFullTypeAndProtocolMaxSize + 1 + SrpClient::kDefaultDomainNameSize + 1];
    snprintf(fullServiceName, sizeof(fullServiceName), "%s.%s", aServiceName, SrpClient::kDefaultDomainName);

    error = MapOpenThreadError(otDnsClientBrowse(mOTInst, fullServiceName, OnDnsBrowseResult, aContext, /* config */ nullptr));

exit:

    Impl()->UnlockThreadStack();

    return error;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnDnsAddressResolveResult(otError aError,
                                                                                    const otDnsAddressResponse * aResponse,
                                                                                    void * aContext)
{
    CHIP_ERROR error;
    DnsResult * dnsResult = reinterpret_cast<DnsResult *>(aContext);
    otIp6Address address;

    error = MapOpenThreadError(otDnsAddressResponseGetAddress(aResponse, 0, &address, nullptr));
    if (error == CHIP_NO_ERROR)
    {
        dnsResult->mMdnsService.mAddress = std::make_optional(ToIPAddress(address));
    }

    dnsResult->error = error;

    DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(dnsResult));
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread<ImplClass>::OnDnsResolveResult(otError aError, const otDnsServiceResponse * aResponse,
                                                                             void * aContext)
{
    CHIP_ERROR error;
    otError otErr;
    DnsResult * dnsResult = Platform::New<DnsResult>(aContext, MapOpenThreadError(aError));

    VerifyOrExit(dnsResult != nullptr, error = CHIP_ERROR_NO_MEMORY);

    // type buffer size is kDnssdTypeAndProtocolMaxSize + . + kMaxDomainNameSize + . + termination character
    char type[Dnssd::kDnssdTypeAndProtocolMaxSize + SrpClient::kMaxDomainNameSize + 3];
    // hostname buffer size is kHostNameMaxLength + . + kMaxDomainNameSize + . + termination character
    char hostname[Dnssd::kHostNameMaxLength + SrpClient::kMaxDomainNameSize + 3];
    // secure space for the raw TXT data in the worst-case scenario relevant for Matter:
    // each entry consists of txt_entry_size (1B) + txt_entry_key + "=" + txt_entry_data
    uint8_t txtBuffer[kMaxDnsServiceTxtEntriesNumber + kTotalDnsServiceTxtBufferSize];
    otDnsServiceInfo serviceInfo;

    if (ThreadStackMgrImpl().mDnsResolveCallback == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid dns resolve callback");
        return;
    }

    VerifyOrExit(aError == OT_ERROR_NONE, error = MapOpenThreadError(aError));

    error = MapOpenThreadError(otDnsServiceResponseGetServiceName(aResponse, dnsResult->mMdnsService.mName,
                                                                  sizeof(dnsResult->mMdnsService.mName), type, sizeof(type)));

    VerifyOrExit(error == CHIP_NO_ERROR, );

    serviceInfo.mHostNameBuffer     = hostname;
    serviceInfo.mHostNameBufferSize = sizeof(hostname);
    serviceInfo.mTxtData            = txtBuffer;
    serviceInfo.mTxtDataSize        = sizeof(txtBuffer);

    otErr = otDnsServiceResponseGetServiceInfo(aResponse, &serviceInfo);
    error = MapOpenThreadError(otErr);

    VerifyOrExit(error == CHIP_NO_ERROR, );

    error = FromOtDnsResponseToMdnsData(serviceInfo, type, dnsResult->mMdnsService, dnsResult->mServiceTxtEntry, otErr);

exit:
    if (dnsResult == nullptr)
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolveNoMemory, reinterpret_cast<intptr_t>(aContext));
        return;
    }

    dnsResult->error = error;

    // If IPv6 address in unspecified (AAAA record not present), send additional DNS query to obtain IPv6 address.
    if (otIp6IsAddressUnspecified(&serviceInfo.mHostAddress))
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchAddressResolve, reinterpret_cast<intptr_t>(dnsResult));
    }
    else
    {
        DeviceLayer::PlatformMgr().ScheduleWork(DispatchResolve, reinterpret_cast<intptr_t>(dnsResult));
    }
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread<ImplClass>::_DnsResolve(const char * aServiceName, const char * aInstanceName,
                                                                            DnsResolveCallback aCallback, void * aContext)
{
    VerifyOrReturnError(mOTInst, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;

    Impl()->LockThreadStack();
    const otDnsQueryConfig * defaultConfig = otDnsClientGetDefaultConfig(mOTInst);

    VerifyOrExit(aServiceName && aInstanceName, error = CHIP_ERROR_INVALID_ARGUMENT);

    mDnsResolveCallback = aCallback;

    // Append default SRP domain name to the service name.
    // fullServiceName buffer size is kDnssdTypeAndProtocolMaxSize + . separator + kDefaultDomainNameSize + termination character.
    char fullServiceName[Dnssd::kDnssdTypeAndProtocolMaxSize + 1 + SrpClient::kDefaultDomainNameSize + 1];
    snprintf(fullServiceName, sizeof(fullServiceName), "%s.%s", aServiceName, SrpClient::kDefaultDomainName);

    error = MapOpenThreadError(
        otDnsClientResolveService(mOTInst, aInstanceName, fullServiceName, OnDnsResolveResult, aContext, defaultConfig));

exit:

    Impl()->UnlockThreadStack();

    return error;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_THREAD_STACK_MANAGER_IMPL_OPENTHREAD_IPP
