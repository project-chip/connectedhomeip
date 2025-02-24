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

/**
 * Note: ThreadStackManager requires ConnectivityManager to be defined
 *       beforehand, otherwise we will face circular dependency between them. */
#include <platform/ConnectivityManager.h>

/**
 * Note: Use public include for ThreadStackManager which includes our local
 *       platform/<PLATFORM>/ThreadStackManagerImpl.h after defining interface
 *       class. */
#include <platform/ThreadStackManager.h>

#include <endian.h>

#include <cstring>

#include <thread.h>
#include <tizen_error.h>

#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>

#include <platform/Tizen/ErrorUtils.h>
#include <platform/Tizen/ThreadStackManagerImpl.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using chip::DeviceLayer::Internal::TizenToChipError;

namespace chip {
namespace DeviceLayer {

namespace {
// Return human readable error message for given Thread API error code.
//
// This is a temporary workaround for the fact that Tizen common API
// does not return messages for all Thread API errors.
//
// TODO (a.bokowy): To be removed once Tizen fixes this issue.
const char * get_error_message(int code)
{
    switch (code)
    {
    case THREAD_ERROR_NOT_INITIALIZED:
        return "Not initialized";
    case THREAD_ERROR_NOT_IN_PROGRESS:
        return "Operation not in progress";
    case THREAD_ERROR_ALREADY_DONE:
        return "Operation already done";
    case THREAD_ERROR_OPERATION_FAILED:
        return "Operation failed";
    case THREAD_ERROR_NOT_READY:
        return "Resource not ready";
    case THREAD_ERROR_NOT_ENABLED:
        return "Not enabled";
    case THREAD_ERROR_NOT_FOUND:
        return "Not found";
    case THREAD_ERROR_ALREADY_REGISTERED:
        return "Already registered";
    case THREAD_ERROR_DB_FAILED:
        return "DB operation failed";
    case THREAD_ERROR_NOT_REGISTERED:
        return "Not registered";
    default:
        return ::get_error_message(code);
    }
}
}; // namespace

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
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Initialize Thread: %s", get_error_message(threadErr)));
    ChipLogProgress(DeviceLayer, "Thread initialized");

    threadErr = thread_enable(&mThreadInstance);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Enable Thread: %s", get_error_message(threadErr)));
    ChipLogProgress(DeviceLayer, "Thread enabled");

    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Get Thread device role: %s", get_error_message(threadErr)));
    ThreadDeviceRoleChangedHandler(deviceRole);

    /* Set callback for change of device role */
    threadErr = thread_set_device_role_changed_cb(mThreadInstance, _ThreadDeviceRoleChangedCb, nullptr);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Set Thread device role changed cb: %s", get_error_message(threadErr)));

    mIsInitialized = true;
    ChipLogProgress(DeviceLayer, "Thread stack manager initialized");
    return CHIP_NO_ERROR;

exit:
    thread_deinitialize();
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
            ChipLogError(DeviceLayer, "FAIL: Start Thread SRP client: %s", get_error_message(threadErr));
    }
    else if (role == THREAD_DEVICE_ROLE_ROUTER || role == THREAD_DEVICE_ROLE_CHILD)
    {
        threadErr = thread_srp_server_stop(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: Stop Thread SRP server: %s", get_error_message(threadErr));
        threadErr = thread_srp_client_start(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: Start Thread SRP client: %s", get_error_message(threadErr));
    }
    else if (role == THREAD_DEVICE_ROLE_LEADER)
    {
        threadErr = thread_srp_client_stop(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: Stop Thread SRP client: %s", get_error_message(threadErr));
        threadErr = thread_srp_server_start(mThreadInstance);
        if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
            ChipLogError(DeviceLayer, "FAIL: Start Thread SRP server: %s", get_error_message(threadErr));
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
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(Thread::OperationalDataset::IsValid(netInfo), CHIP_ERROR_INVALID_ARGUMENT);

    int threadErr = THREAD_ERROR_NONE;

    threadErr = thread_network_set_active_dataset_tlvs(mThreadInstance, netInfo.data(), netInfo.size());
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Thread set active dataset TLVs: %s", get_error_message(threadErr)));

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event;
    event.Type                                           = DeviceEventType::kServiceProvisioningChange;
    event.ServiceProvisioningChange.IsServiceProvisioned = true;
    PlatformMgr().PostEventOrDie(&event);

    ChipLogProgress(DeviceLayer, "Thread set active dataset TLVs");

    return CHIP_NO_ERROR;

exit:
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(Thread::OperationalDataset & dataset)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    int threadErr      = THREAD_ERROR_NONE;
    uint8_t * tlvsData = nullptr;
    int tlvsLen;

    threadErr = thread_network_get_active_dataset_tlvs(mThreadInstance, &tlvsData, &tlvsLen);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Thread get active dataset TLVs: %s", get_error_message(threadErr)));

    ChipLogProgress(DeviceLayer, "Thread get active dataset TLVs size [%u]", tlvsLen);
    mDataset.Init(ByteSpan(tlvsData, tlvsLen));
    dataset.Init(mDataset.AsByteSpan());

    return CHIP_NO_ERROR;

exit:
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
    VerifyOrReturnError(mIsInitialized, false);

    int threadErr = THREAD_ERROR_NONE;
    thread_device_role_e deviceRole;

    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, false,
                        ChipLogError(DeviceLayer, "FAIL: Get Thread device role: %s", get_error_message(threadErr)));

    ChipLogProgress(DeviceLayer, "Thread device role [%s]", _ThreadRoleToStr(deviceRole));
    return deviceRole != THREAD_DEVICE_ROLE_DISABLED;
}

bool ThreadStackManagerImpl::_IsThreadAttached()
{
    return mIsAttached;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    int threadErr  = THREAD_ERROR_NONE;
    bool isEnabled = sInstance._IsThreadEnabled();

    if (val && !isEnabled)
    {
        threadErr = thread_network_attach(mThreadInstance);
        DeviceLayer::SystemLayer().ScheduleLambda([&, threadErr]() {
            if (this->mpConnectCallback != nullptr && threadErr != THREAD_ERROR_NONE)
            {
                this->mpConnectCallback->OnResult(NetworkCommissioning::Status::kUnknownError, CharSpan(), 0);
                this->mpConnectCallback = nullptr;
            }
        });
        VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                     ChipLogError(DeviceLayer, "FAIL: Attach Thread network: %s", get_error_message(threadErr)));

        threadErr = thread_start(mThreadInstance);
        DeviceLayer::SystemLayer().ScheduleLambda([&, threadErr]() {
            if (this->mpConnectCallback != nullptr)
            {
                this->mpConnectCallback->OnResult(threadErr == THREAD_ERROR_NONE ? NetworkCommissioning::Status::kSuccess
                                                                                 : NetworkCommissioning::Status::kUnknownError,
                                                  CharSpan(), 0);
                this->mpConnectCallback = nullptr;
            }
        });
        VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                     ChipLogError(DeviceLayer, "FAIL: Start Thread network: %s", get_error_message(threadErr)));
    }
    else if (!val && isEnabled)
    {
        threadErr = thread_stop(mThreadInstance);
        VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                     ChipLogError(DeviceLayer, "FAIL: Stop Thread: %s", get_error_message(threadErr)));
    }

    thread_device_role_e deviceRole;
    threadErr = thread_get_device_role(mThreadInstance, &deviceRole);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Get Thread device role: %s", get_error_message(threadErr)));
    ThreadDeviceRoleChangedHandler(deviceRole);

    ChipLogProgress(DeviceLayer, "Thread set enabled [%s]", val ? "attach" : "reset");
    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "FAIL: Set Thread enabled [%s]", val ? "attach" : "reset");
    return CHIP_ERROR_INTERNAL;
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    VerifyOrReturnError(mIsInitialized, ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported,
                        ChipLogError(DeviceLayer, "Thread stack not initialized"));

    int threadErr = THREAD_ERROR_NONE;
    thread_device_type_e devType;

    threadErr = thread_get_device_type(mThreadInstance, &devType);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Get Thread device type: %s", get_error_message(threadErr)));

    ChipLogProgress(DeviceLayer, "Thread device type [%s]", _ThreadTypeToStr(devType));

    switch (devType)
    {
    case THREAD_DEVICE_TYPE_NOT_SUPPORTED:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    case THREAD_DEVICE_TYPE_ROUTER:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
    case THREAD_DEVICE_TYPE_FULL_END_DEVICE:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice;
    case THREAD_DEVICE_TYPE_MINIMAL_END_DEVICE:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice;
    case THREAD_DEVICE_TYPE_SLEEPY_END_DEVICE:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice;
    default:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }

exit:
    return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    int threadErr = THREAD_ERROR_NONE;
    thread_device_type_e devType;

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
    VerifyOrExit(threadErr == THREAD_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: Set Thread device type: %s", get_error_message(threadErr)));

    ChipLogProgress(DeviceLayer, "Thread set device type [%s]", _ThreadTypeToStr(devType));
    return CHIP_NO_ERROR;

exit:
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
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);

    uint64_t extAddr;
    int threadErr;

    threadErr = thread_get_extended_address(mThreadInstance, &extAddr);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Get Thread extended address: %s", get_error_message(threadErr)));

    extAddr = htobe64(extAddr);
    memcpy(buf, &extAddr, sizeof(extAddr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadVersion(uint16_t & version)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);

#if defined(TIZEN_NETWORK_THREAD_VERSION) && TIZEN_NETWORK_THREAD_VERSION >= 0x000900
    int threadErr = thread_get_version(mThreadInstance, &version);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, TizenToChipError(threadErr),
                        ChipLogError(DeviceLayer, "FAIL: Get thread version: %s", get_error_message(threadErr)));
    ChipLogProgress(DeviceLayer, "Thread version [%u]", version);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback)
{
    ChipLogError(DeviceLayer, "Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ThreadStackManagerImpl::_ResetThreadNetworkDiagnosticsCounts() {}

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
    return DeviceLayer::ThreadStackManagerImpl::sInstance;
}

ThreadStackManagerImpl & ThreadStackMgrImpl()
{
    return DeviceLayer::ThreadStackManagerImpl::sInstance;
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
CHIP_ERROR ThreadStackManagerImpl::_AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                                                  const Span<const char * const> & aSubTypes,
                                                  const Span<const Dnssd::TextEntry> & aTxtEntries, uint32_t aLeaseInterval,
                                                  uint32_t aKeyLeaseInterval)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(aInstanceName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    int threadErr;

    std::vector<thread_dns_txt_entry_s> entries;
    entries.reserve(aTxtEntries.size());

    thread_dns_txt_entry_s * ee = entries.data();
    for (auto & entry : aTxtEntries)
    {
        ee->key   = entry.mKey;
        ee->value = entry.mData;
        VerifyOrReturnError(chip::CanCastTo<uint8_t>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);
        ee->value_len = static_cast<uint8_t>(entry.mDataSize);
        ee++;
    }

    VerifyOrReturnError(chip::CanCastTo<uint8_t>(entries.size()), CHIP_ERROR_INVALID_ARGUMENT);
    threadErr = thread_srp_client_register_service_full(mThreadInstance, aInstanceName, aName, aPort, 0, 0, entries.data(),
                                                        static_cast<uint8_t>(entries.size()));
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE || threadErr == THREAD_ERROR_ALREADY_DONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Register Thread SRP client service: %s", get_error_message(threadErr)));

    SrpClientService service;
    Platform::CopyString(service.mInstanceName, aInstanceName);
    Platform::CopyString(service.mName, aName);
    service.mPort = aPort;
    mSrpClientServices.push_back(service);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_RemoveSrpService(const char * aInstanceName, const char * aName)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(aInstanceName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    int threadErr;

    threadErr = thread_srp_client_remove_service(mThreadInstance, aInstanceName, aName);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Remove Thread SRP client service: %s", get_error_message(threadErr)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_InvalidateAllSrpServices()
{
    for (auto & service : mSrpClientServices)
    {
        service.mValid = false;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_RemoveInvalidSrpServices()
{
    for (auto it = mSrpClientServices.begin(); it != mSrpClientServices.end();)
    {
        if (!it->mValid)
        {
            auto err = _RemoveSrpService(it->mInstanceName, it->mName);
            VerifyOrReturnError(err == CHIP_NO_ERROR, err);
            it = mSrpClientServices.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_ClearAllSrpHostAndServices()
{
    for (auto it = mSrpClientServices.begin(); it != mSrpClientServices.end();)
    {
        ReturnErrorOnFailure(_RemoveSrpService(it->mInstanceName, it->mName));
        it = mSrpClientServices.erase(it);
    }
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_ThreadIpAddressCb(int index, char * ipAddr, thread_ipaddr_type_e ipAddrType, void * userData)
{
    VerifyOrReturn(ipAddr != nullptr, ChipLogError(DeviceLayer, "FAIL: Invalid argument: Thread ipAddr not found"));
    VerifyOrReturn(strlen(ipAddr) >= 6, ChipLogError(DeviceLayer, "FAIL: Invalid Thread ipAddr"));

    ChipLogProgress(DeviceLayer, "_ThreadIpAddressCb index:[%d] ipAddr:[%s] type:[%d]", index, ipAddr, ipAddrType);

    if (ipAddrType != THREAD_IPADDR_TYPE_MLEID)
        return;

    auto threadErr = thread_srp_client_set_host_address(sInstance.mThreadInstance, ipAddr);
    VerifyOrReturn(threadErr == THREAD_ERROR_NONE || threadErr == THREAD_ERROR_ALREADY_DONE,
                   ChipLogError(DeviceLayer, "FAIL: Set Thread SRP client host address: %s", get_error_message(threadErr)));
}

CHIP_ERROR ThreadStackManagerImpl::_SetupSrpHost(const char * aHostName)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(aHostName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(aHostName) <= Dnssd::kHostNameMaxLength, CHIP_ERROR_INVALID_STRING_LENGTH);

    int threadErr;

    threadErr = thread_srp_client_set_host_name(mThreadInstance, aHostName);
    if (threadErr != THREAD_ERROR_NONE && threadErr != THREAD_ERROR_ALREADY_DONE)
        ChipLogError(DeviceLayer, "FAIL: Set Thread SRP client host name: %s", get_error_message(threadErr));

    /* Get external IP address */
    threadErr = thread_get_ipaddr(mThreadInstance, _ThreadIpAddressCb, THREAD_IPADDR_TYPE_MLEID, nullptr);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: Get Thread IP address: %s", get_error_message(threadErr)));

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
