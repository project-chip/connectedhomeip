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

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
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

ThreadStackManagerImpl::ThreadStackManagerImpl() :
    mIsAttached(false), mIsInitialized(false), mThreadInstance(nullptr), mScanParam(nullptr)
{}

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

CHIP_ERROR ThreadStackManagerImpl::TriggerInit(void * userData)
{
    int threadErr = THREAD_ERROR_NONE;
    thread_device_role_e deviceRole;

    threadErr = thread_initialize();
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: initialize thread"));
    ChipLogProgress(DeviceLayer, "Thread initialized");

    threadErr = thread_enable(&sInstance.mThreadInstance);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: enable thread"));
    ChipLogProgress(DeviceLayer, "Thread enabled");

    threadErr = thread_get_device_role(sInstance.mThreadInstance, &deviceRole);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get device role"));
    sInstance.ThreadDeviceRoleChangedHandler(deviceRole);

    /* Set callback for change of device role */
    threadErr = thread_set_device_role_changed_cb(sInstance.mThreadInstance, _ThreadDeviceRoleChangedCb, nullptr);
    VerifyOrExit(threadErr == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: set device role changed cb"));

    sInstance.mScanResult    = new std::vector<NetworkCommissioning::ThreadScanResponse>();
    sInstance.mIsInitialized = true;
    ChipLogProgress(DeviceLayer, "Thread stack manager initialized");
    return CHIP_NO_ERROR;

exit:
    thread_deinitialize();
    ChipLogError(DeviceLayer, "FAIL: initialize thread stack");
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    CHIP_ERROR err;

    if (mIsInitialized)
    {
        ChipLogProgress(DeviceLayer, "Thread stack manager already initialized");
        return CHIP_NO_ERROR;
    }

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(TriggerInit, static_cast<void *>(nullptr));
    SuccessOrExit(err);

    return CHIP_NO_ERROR;

exit:
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
        ChipLogProgress(DeviceLayer, "Network attach %s", threadErr == THREAD_ERROR_NONE ? "successfully" : "failed");
        DeviceLayer::SystemLayer().ScheduleLambda([&, threadErr]() {
            if (this->mpConnectCallback != nullptr)
            {
                this->mpConnectCallback->OnResult(threadErr == THREAD_ERROR_NONE ? NetworkCommissioning::Status::kSuccess
                                                                                 : NetworkCommissioning::Status::kUnknownError,
                                                  CharSpan(), 0);
                this->mpConnectCallback = nullptr;
            }
        });
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
    // TODO: Remove Weave legacy APIs
    // For a leader with a child, the child is considered to have mesh connectivity
    // and the leader is not, which is a very confusing definition.
    // This API is Weave legacy and should be removed.

    ChipLogError(DeviceLayer, "HaveMeshConnectivity has confusing behavior and shouldn't be called");
    return false;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadStatsCounters()
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyMinimal()
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyFull()
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPrimary802154MACAddress(uint8_t * buf)
{
    uint64_t extAddr;
    int threadErr;

    threadErr = thread_get_extended_address(mThreadInstance, &extAddr);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "thread_get_extended_address() failed. ret: %d", threadErr));

    extAddr = htobe64(extAddr);
    memcpy(buf, &extAddr, sizeof(extAddr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_JoinerStart()
{
    // TODO: Remove Weave legacy APIs
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ThreadStackManagerImpl::_SetRouterPromotion(bool val)
{
    // Set Router Promotion is not supported on Tizen
}

void ThreadStackManagerImpl::ThreadScanResultCb(int result, thread_network_scanning_state_e state, uint64_t extAddress,
                                                const char * networkName, uint64_t extPanid, const uint8_t * steeringData,
                                                int length, uint16_t panid, uint16_t joinerDdpPort, uint8_t channel, int16_t rssi,
                                                uint8_t lqi, uint8_t version, bool isNative, bool isJoinable, void * userData)
{
    ThreadStackManagerImpl * self = (ThreadStackManagerImpl *) userData;
    if (state == THREAD_SCANNING_STARTED)
    {
        ChipLogProgress(DeviceLayer, "Scanning started");
    }
    else if (state == THREAD_SCANNING_FINISHED)
    {
        ChipLogProgress(DeviceLayer, "Scanning finished");
        DeviceLayer::SystemLayer().ScheduleLambda([self]() {
            // Note: We cannot post a event in ScheduleLambda since std::vector is not trivial copiable. This results in the use of
            // const_cast but should be fine for almost all cases, since we actually handled the ownership of this element to this
            // lambda.
            if (self->mpScanCallback != nullptr)
            {
                TizenScanResponseIterator<NetworkCommissioning::ThreadScanResponse> iter(
                    const_cast<std::vector<ThreadScanResponse> *>(self->mScanResult));
                self->mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
                self->mpScanCallback = nullptr;
            }
            delete const_cast<std::vector<ThreadScanResponse> *>(self->mScanResult);
        });
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Scan device found. panid: %u, channel %u, extAddress: %llu, rssi: %u, lqi: %u", panid,
                        channel, extAddress, rssi, lqi);

        NetworkCommissioning::ThreadScanResponse networkScanned;
        networkScanned.panId         = panid;
        networkScanned.extendedPanId = extPanid;
        size_t networkNameLen        = strlen(networkName);
        if (networkNameLen > 16)
        {
            ChipLogProgress(DeviceLayer, "Network name is too long, ignore it.");
            return;
        }
        networkScanned.networkNameLen = static_cast<uint8_t>(networkNameLen);
        memcpy(networkScanned.networkName, networkName, networkNameLen);
        networkScanned.channel         = channel;
        networkScanned.version         = version;
        networkScanned.extendedAddress = extAddress;
        networkScanned.rssi            = (int8_t) rssi;
        networkScanned.lqi             = lqi;

        self->mScanResult->push_back(networkScanned);
    }
}

CHIP_ERROR ThreadStackManagerImpl::TriggerScan(ThreadStackManagerImpl * self)
{
    ChipLogProgress(DeviceLayer, "%s enter", __func__);
    int ret = thread_scan(self->mThreadInstance, self->mScanParam, ThreadScanResultCb, self);
    if (ret == THREAD_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "scan start successfully");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "scan starting failed");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(ThreadDriver::ScanCallback * callback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;

    // There is another ongoing scan request, reject the new one.
    VerifyOrReturnError(mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ret = thread_scan_param_create(mThreadInstance, 80, &mScanParam);
    VerifyOrExit(ret == THREAD_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: scan param create"));

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(TriggerScan, this);
    SuccessOrExit(err);

    mpScanCallback = callback;
    return CHIP_NO_ERROR;

exit:
    return CHIP_ERROR_INTERNAL;
}

void ThreadStackManagerImpl::_ResetThreadNetworkDiagnosticsCounts() {}

CHIP_ERROR ThreadStackManagerImpl::_WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId,
                                                                               app::AttributeValueEncoder & encoder)
{
    ChipLogError(DeviceLayer, "%s not implemented", __func__);
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

void ThreadStackManagerImpl::_UpdateNetworkStatus()
{
    // Thread is not enabled, then we are not trying to connect to the network.
    VerifyOrReturn(IsThreadEnabled() && mpStatusChangeCallback != nullptr);

    Thread::OperationalDataset dataset;
    uint8_t extpanid[Thread::kSizeExtendedPanId];

    // If we have not provisioned any Thread network, return the status from last network scan,
    // If we have provisioned a network, we assume the ot-br-posix is activitely connecting to that network.
    CHIP_ERROR err = ThreadStackMgrImpl().GetThreadProvision(dataset);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    // The Thread network is not enabled, but has a different extended pan id.
    VerifyOrReturn(dataset.GetExtendedPanId(extpanid) == CHIP_NO_ERROR);

    // We have already connected to the network, thus return success.
    if (ThreadStackMgrImpl().IsThreadAttached())
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(Status::kSuccess, MakeOptional(ByteSpan(extpanid)), NullOptional);
    }
    else
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(Status::kNetworkNotFound, MakeOptional(ByteSpan(extpanid)), NullOptional);
    }
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
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
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
                        ChipLogError(DeviceLayer, "thread_srp_client_register_service() failed. ret: %d", threadErr));

    SrpClientService service;
    Platform::CopyString(service.mInstanceName, aInstanceName);
    Platform::CopyString(service.mName, aName);
    service.mPort = aPort;
    mSrpClientServices.push_back(service);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_RemoveSrpService(const char * aInstanceName, const char * aName)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(aInstanceName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aName != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    int threadErr;

    threadErr = thread_srp_client_remove_service(mThreadInstance, aInstanceName, aName);
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "thread_srp_client_remove_service() failed. ret: %d", threadErr));

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
    VerifyOrReturnError(threadErr == THREAD_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "thread_get_ipaddr() failed. ret: %d", threadErr));

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
