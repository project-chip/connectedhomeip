/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides an implementation of the ThreadStackManager singleton object
 *          for Tizen platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include "ThreadStackManagerImpl.h"
#include <lib/dnssd/platform/Dnssd.h>

using namespace ::chip::DeviceLayer::Internal;
using namespace chip::DeviceLayer::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleDisabled[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleDetached[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleChild[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleRouter[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleLeader[];

constexpr char ThreadStackManagerImpl::kOpenthreadDeviceTypeNotSupported[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceTypeRouter[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceTypeFullEndDevice[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceTypeMinimalEndDevice[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceTypeSleepyEndDevice[];

ThreadStackManagerImpl::ThreadStackManagerImpl() : mIsAttached(false), mIsInitialized(false), mThreadInstance(nullptr) {}

const char * ThreadStackManagerImpl::_ThreadRoleToStr(thread_device_role_e role)
{
    switch (role)
    {
    case THREAD_DEVICE_ROLE_DISABLED:
        return kOpenthreadDeviceRoleDisabled;
    case THREAD_DEVICE_ROLE_DETACHED:
        return kOpenthreadDeviceRoleDetached;
    case THREAD_DEVICE_ROLE_CHILD:
        return kOpenthreadDeviceRoleChild;
    case THREAD_DEVICE_ROLE_ROUTER:
        return kOpenthreadDeviceRoleRouter;
    case THREAD_DEVICE_ROLE_LEADER:
        return kOpenthreadDeviceRoleLeader;
    default:
        return nullptr;
    }
}

const char * ThreadStackManagerImpl::_ThreadTypeToStr(thread_device_type_e type)
{
    switch (type)
    {
    case THREAD_DEVICE_TYPE_NOT_SUPPORTED:
        return kOpenthreadDeviceTypeNotSupported;
    case THREAD_DEVICE_TYPE_ROUTER:
        return kOpenthreadDeviceTypeRouter;
    case THREAD_DEVICE_TYPE_FULL_END_DEVICE:
        return kOpenthreadDeviceTypeFullEndDevice;
    case THREAD_DEVICE_TYPE_MINIMAL_END_DEVICE:
        return kOpenthreadDeviceTypeMinimalEndDevice;
    case THREAD_DEVICE_TYPE_SLEEPY_END_DEVICE:
        return kOpenthreadDeviceTypeSleepyEndDevice;
    default:
        return nullptr;
    }
}

void ThreadStackManagerImpl::_ThreadDeviceRoleChangedCb(thread_device_role_e deviceRole, void * userData)
{
    ChipLogProgress(DeviceLayer, "Thread device role [%s]", sInstance._ThreadRoleToStr(deviceRole));
    sInstance.ThreadDeviceRoleChangedHandler(deviceRole);
}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    int threadErr = THREAD_ERROR_NONE;
    thread_device_role_e deviceRole;

    if (mIsInitialized)
    {
        ChipLogProgress(DeviceLayer, "Thread stack manager already initialized");
        return CHIP_NO_ERROR;
    }

    threadErr = thread_initialize();
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: initialize thread"));
    ChipLogProgress(DeviceLayer, "Thread initialized");

    threadErr = thread_enable(&mThreadInstance);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: enable thread"));
    ChipLogProgress(DeviceLayer, "Thread enabled");

    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get device role"));
    ThreadDeviceRoleChangedHandler(deviceRole);

    /* Set callback for change of device role */
    threadErr = thread_set_device_role_changed_cb(mThreadInstance, _ThreadDeviceRoleChangedCb, nullptr);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: set device role changed cb"));

    mIsInitialized = true;
    ChipLogProgress(DeviceLayer, "Thread stack manager initialized");
    return CHIP_NO_ERROR;

exit:
    thread_deinitialize();
    ChipLogError(DeviceLayer, "FAIL: initialize thread stack");
    return CHIP_ERROR_INTERNAL;
}

void ThreadStackManagerImpl::ThreadDeviceRoleChangedHandler(thread_device_role_e role)
{
    ChipLogProgress(DeviceLayer, "Thread device role [%s]", _ThreadRoleToStr(role));

    bool isAttached = (role != THREAD_DEVICE_ROLE_DISABLED) && (role != THREAD_DEVICE_ROLE_DETACHED);

    ChipDeviceEvent event = ChipDeviceEvent{};

    if (isAttached != mIsAttached)
    {
        ChipLogProgress(DeviceLayer, "Thread connectivity state changed [%s]", isAttached ? "Established" : "Lost");
        event.Type = DeviceEventType::kThreadConnectivityChange;
        event.ThreadConnectivityChange.Result =
            isAttached ? ConnectivityChange::kConnectivity_Established : ConnectivityChange::kConnectivity_Lost;
        PlatformMgr().PostEventOrDie(&event);
    }

    mIsAttached = isAttached;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    int threadErr = THREAD_ERROR_NONE;

    if (role == THREAD_DEVICE_ROLE_DISABLED)
    {
        /* start srp client */
        threadErr = thread_srp_client_start(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: thread_srp_client_start");
    }
    else if (role == THREAD_DEVICE_ROLE_ROUTER || role == THREAD_DEVICE_ROLE_CHILD)
    {
        threadErr = thread_srp_server_stop(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: thread_srp_server_stop");

        threadErr = thread_srp_client_start(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: thread_srp_client_start");
    }
    else if (role == THREAD_DEVICE_ROLE_LEADER)
    {
        threadErr = thread_srp_client_stop(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: thread_srp_client_stop");

        threadErr = thread_srp_server_start(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: thread_srp_server_start");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    ChipLogProgress(DeviceLayer, "Thread role state changed [%s]", mIsAttached ? "Attached" : "Detached");
    event.Type                          = DeviceEventType::kThreadStateChange;
    event.ThreadStateChange.RoleChanged = true;
    PlatformMgr().PostEventOrDie(&event);
}

void ThreadStackManagerImpl::_ProcessThreadActivity() {}

bool ThreadStackManagerImpl::_HaveRouteToAddress(const Inet::IPAddress & destAddr)
{
    return false;
}

void ThreadStackManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogProgress(DeviceLayer, "Thread platform event type [%d]", event->Type);
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadProvision(ByteSpan netInfo)
{
    int threadErr = THREAD_ERROR_NONE;

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(Thread::OperationalDataset::IsValid(netInfo), CHIP_ERROR_INVALID_ARGUMENT);

    threadErr = thread_network_set_active_dataset_tlvs(mThreadInstance, netInfo.data(), netInfo.size());
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: set active dataset tlvs"));

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event;
    event.Type                                           = DeviceEventType::kServiceProvisioningChange;
    event.ServiceProvisioningChange.IsServiceProvisioned = true;
    PlatformMgr().PostEventOrDie(&event);

    ChipLogProgress(DeviceLayer, "Thread set active dtaset tlvs");

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: set thread provision");
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(Thread::OperationalDataset & dataset)
{
    int threadErr      = THREAD_ERROR_NONE;
    uint8_t * tlvsData = nullptr;
    int tlvsLen;

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);

    threadErr = thread_network_get_active_dataset_tlvs(mThreadInstance, &tlvsData, &tlvsLen);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get active dataset tlvs"));

    ChipLogProgress(DeviceLayer, "Thread get active dataset tlvs size [%u]", tlvsLen);
    mDataset.Init(ByteSpan(tlvsData, tlvsLen));
    dataset.Init(mDataset.AsByteSpan());

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: get thread provision");
    return CHIP_ERROR_INTERNAL;
}

bool ThreadStackManagerImpl::_IsThreadProvisioned()
{
    return mDataset.IsCommissioned();
}

void ThreadStackManagerImpl::_ErasePersistentInfo()
{
    mDataset.Clear();
}

bool ThreadStackManagerImpl::_IsThreadEnabled()
{
    int threadErr = THREAD_ERROR_NONE;
    thread_device_role_e deviceRole;

    VerifyOrReturnError(mIsInitialized, false);

    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, false);

    ChipLogProgress(DeviceLayer, "Thread device role [%s]", _ThreadRoleToStr(deviceRole));
    return deviceRole != THREAD_DEVICE_ROLE_DISABLED;
}

bool ThreadStackManagerImpl::_IsThreadAttached()
{
    return mIsAttached;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    int threadErr = THREAD_ERROR_NONE;

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    bool isEnabled = sInstance._IsThreadEnabled();

    if (val && !isEnabled)
    {
        threadErr = thread_network_attach(mThreadInstance);
        VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: attach thread network"));

        threadErr = thread_start(mThreadInstance);
        VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: start thread network"));
    }
    else if (!val && isEnabled)
    {
        threadErr = thread_stop(mThreadInstance);
        VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: thread stop"));
    }

    thread_device_role_e deviceRole;
    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get device role"));
    ThreadDeviceRoleChangedHandler(deviceRole);

    ChipLogProgress(DeviceLayer, "Thread set enabled [%s]", val ? "attach" : "reset");
    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: set thread enabled [%d]", val);
    return CHIP_ERROR_INTERNAL;
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    int threadErr = THREAD_ERROR_NONE;
    thread_device_type_e devType;
    ConnectivityManager::ThreadDeviceType deviceType;

    VerifyOrExit(mIsInitialized, ChipLogError(DeviceLayer, "Thread stack not initialized"));

    threadErr = thread_get_device_type(mThreadInstance, &devType);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get device type"));

    ChipLogProgress(DeviceLayer, "Thread device type [%s]", _ThreadTypeToStr(devType));

    switch (devType)
    {
    case THREAD_DEVICE_TYPE_NOT_SUPPORTED:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
        break;
    case THREAD_DEVICE_TYPE_ROUTER:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
        break;
    case THREAD_DEVICE_TYPE_FULL_END_DEVICE:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice;
        break;
    case THREAD_DEVICE_TYPE_MINIMAL_END_DEVICE:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice;
        break;
    case THREAD_DEVICE_TYPE_SLEEPY_END_DEVICE:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice;
        break;
    default:
        deviceType = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
        break;
    }

    return deviceType;

exit:
    ChipLogError(DeviceLayer, "FAIL: get thread device type");
    return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    int threadErr = THREAD_ERROR_NONE;
    thread_device_type_e devType;

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);

    switch (deviceType)
    {
    case ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported:
        devType = THREAD_DEVICE_TYPE_NOT_SUPPORTED;
        break;
    case ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router:
        devType = THREAD_DEVICE_TYPE_ROUTER;
        break;
    case ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice:
        devType = THREAD_DEVICE_TYPE_FULL_END_DEVICE;
        break;
    case ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice:
        devType = THREAD_DEVICE_TYPE_MINIMAL_END_DEVICE;
        break;
    case ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice:
        devType = THREAD_DEVICE_TYPE_SLEEPY_END_DEVICE;
        break;
    default:
        devType = THREAD_DEVICE_TYPE_NOT_SUPPORTED;
        break;
    }

    threadErr = thread_set_device_type(mThreadInstance, devType);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: set device type"));

    ChipLogProgress(DeviceLayer, "Thread set device type [%s]", _ThreadTypeToStr(devType));
    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: set thread device type [%d]", deviceType);
    return CHIP_ERROR_INTERNAL;
}

bool ThreadStackManagerImpl::_HaveMeshConnectivity()
{
    return false;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadStatsCounters()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyMinimal()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyFull()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPrimary802154MACAddress(uint8_t * buf)
{
    uint64_t extAddr;
    int threadErr;

    threadErr = thread_get_extended_address(mThreadInstance, &extAddr);
    VerifyOrReturnError(
        threadErr == THREAD_ERROR_NONE,
        (ChipLogError(DeviceLayer, "thread_get_extended_address() failed. ret: %d", threadErr), CHIP_ERROR_INTERNAL));

    extAddr = htobe64(extAddr);
    memcpy(buf, &extAddr, sizeof(extAddr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_JoinerStart()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(ThreadDriver::ScanCallback * callback)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ThreadStackManagerImpl::_ResetThreadNetworkDiagnosticsCounts() {}

CHIP_ERROR ThreadStackManagerImpl::_WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId,
                                                                               app::AttributeValueEncoder & encoder)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR
ThreadStackManagerImpl::_AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                               NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback)
{
    // Reset the previously set callback since it will never be called in case incorrect dataset was supplied.
    mpConnectCallback = nullptr;
    ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
    ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadProvision(dataset.AsByteSpan()));

    if (dataset.IsCommissioned())
    {
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(true));
        mpConnectCallback = callback;
    }

    return CHIP_NO_ERROR;
}

ThreadStackManager & ThreadStackMgr()
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

ThreadStackManagerImpl & ThreadStackMgrImpl()
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
CHIP_ERROR ThreadStackManagerImpl::_AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                                                  const Span<const char * const> & aSubTypes,
                                                  const Span<const Dnssd::TextEntry> & aTxtEntries, uint32_t aLeaseInterval,
                                                  uint32_t aKeyLeaseInterval)
{
    ChipLogDetail(DeviceLayer, "%s +", __func__);
    CHIP_ERROR error = CHIP_NO_ERROR;
    int threadErr    = THREAD_ERROR_NONE;

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrExit(aInstanceName, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(aName, error = CHIP_ERROR_INVALID_ARGUMENT);

    threadErr = thread_srp_client_register_service(mThreadInstance, aInstanceName, aName, aPort);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE || threadErr == THREAD_ERROR_ALREADY_DONE, error = CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: thread_srp_client_register_service");
    return error;
}

CHIP_ERROR ThreadStackManagerImpl::_RemoveSrpService(const char * aInstanceName, const char * aName)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_InvalidateAllSrpServices()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_RemoveInvalidSrpServices()
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ThreadStackManagerImpl::_ThreadIpAddressCb(int index, char * ipAddr, thread_ipaddr_type_e ipAddrType, void * userData)
{
    int threadErr = THREAD_ERROR_NONE;

    VerifyOrExit(ipAddr, ChipLogError(DeviceLayer, "FAIL: invalid argument, ipAddr not found"));
    VerifyOrExit(strlen(ipAddr) >= 6, ChipLogError(DeviceLayer, "FAIL: invalid ipAddr"));

    ChipLogProgress(DeviceLayer, "_ThreadIpAddressCb index:[%d] ipAddr:[%s] type:[%d]", index, ipAddr, ipAddrType);

    if (ipAddrType != THREAD_IPADDR_TYPE_MLEID)
        return;
    threadErr = thread_srp_client_set_host_address(sInstance.mThreadInstance, ipAddr);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE || threadErr == THREAD_ERROR_ALREADY_DONE,
                 ChipLogError(DeviceLayer, "FAIL: Thread library API failed"));

    return;

exit:
    ChipLogError(DeviceLayer, "FAIL: thread_srp_client_set_host_address");
}

CHIP_ERROR ThreadStackManagerImpl::_SetupSrpHost(const char * aHostName)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(aHostName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aHostName) <= Dnssd::kHostNameMaxLength, CHIP_ERROR_INVALID_STRING_LENGTH);

    int threadErr;

    threadErr = thread_srp_client_set_host_name(mThreadInstance, aHostName);
    if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
        ChipLogError(DeviceLayer, "thread_srp_client_set_host_name() failed. ret: %d", threadErr);

    /* Get external ip address */
    threadErr = thread_get_ipaddr(mThreadInstance, _ThreadIpAddressCb, THREAD_IPADDR_TYPE_MLEID, nullptr);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE,
                        (ChipLogError(DeviceLayer, "thread_get_ipaddr() failed. ret: %d", threadErr), CHIP_ERROR_INTERNAL));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_ClearSrpHost(const char * aHostName)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback,
                                               void * aContext)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
} // namespace DeviceLayer
} // namespace chip
