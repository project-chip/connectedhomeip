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
 *          for the Tizen platforms.
 */

#pragma once

#include <cstdint>

#include <thread.h>

#include <app/AttributeAccessInterface.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/NetworkCommissioning.h>

#include "platform/internal/DeviceNetworkInfo.h"

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl : public ThreadStackManager
{
public:
    ThreadStackManagerImpl();

    CHIP_ERROR _InitThreadStack();
    void _ProcessThreadActivity();

    CHIP_ERROR _StartThreadTask() { return CHIP_NO_ERROR; } // Intentionally left blank
    void _LockThreadStack() {}                              // Intentionally left blank
    bool _TryLockThreadStack() { return false; }            // Intentionally left blank
    void _UnlockThreadStack() {}                            // Intentionally left blank

    bool _HaveRouteToAddress(const Inet::IPAddress & destAddr);

    void _OnPlatformEvent(const ChipDeviceEvent * event);

    CHIP_ERROR _GetThreadProvision(Thread::OperationalDataset & dataset);

    CHIP_ERROR _SetThreadProvision(ByteSpan netInfo);

    void _ErasePersistentInfo();

    bool _IsThreadProvisioned();

    bool _IsThreadEnabled();

    bool _IsThreadAttached();

    CHIP_ERROR _AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                      NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback);

    CHIP_ERROR _SetThreadEnabled(bool val);

    void _OnThreadAttachFinished(void);

    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType();

    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);

    bool _HaveMeshConnectivity();

    CHIP_ERROR _GetAndLogThreadStatsCounters();

    CHIP_ERROR _GetAndLogThreadTopologyMinimal();

    CHIP_ERROR _GetAndLogThreadTopologyFull();

    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);

    CHIP_ERROR _GetExternalIPv6Address(chip::Inet::IPAddress & addr);

    CHIP_ERROR _GetPollPeriod(uint32_t & buf);

    CHIP_ERROR _JoinerStart();

    void _SetRouterPromotion(bool val);

    void _ResetThreadNetworkDiagnosticsCounts();

    CHIP_ERROR _WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

    CHIP_ERROR _StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback);

    ~ThreadStackManagerImpl() = default;

    static ThreadStackManagerImpl sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    CHIP_ERROR _AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                              const Span<const char * const> & aSubTypes, const Span<const Dnssd::TextEntry> & aTxtEntries,
                              uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR _RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR _InvalidateAllSrpServices();
    CHIP_ERROR _RemoveInvalidSrpServices();
    CHIP_ERROR _SetupSrpHost(const char * aHostName);
    CHIP_ERROR _ClearSrpHost(const char * aHostName);
    CHIP_ERROR _SetSrpDnsCallbacks(DnsAsyncReturnCallback aInitCallback, DnsAsyncReturnCallback aErrorCallback, void * aContext);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    CHIP_ERROR _DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext);
    CHIP_ERROR _DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback, void * aContext);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

private:
    static constexpr char kOpenthreadDeviceRoleDisabled[] = "disabled";
    static constexpr char kOpenthreadDeviceRoleDetached[] = "detached";
    static constexpr char kOpenthreadDeviceRoleChild[]    = "child";
    static constexpr char kOpenthreadDeviceRoleRouter[]   = "router";
    static constexpr char kOpenthreadDeviceRoleLeader[]   = "leader";

    static constexpr char kOpenthreadDeviceTypeNotSupported[]     = "not supported";
    static constexpr char kOpenthreadDeviceTypeRouter[]           = "router";
    static constexpr char kOpenthreadDeviceTypeFullEndDevice[]    = "full end device";
    static constexpr char kOpenthreadDeviceTypeMinimalEndDevice[] = "minimal end device";
    static constexpr char kOpenthreadDeviceTypeSleepyEndDevice[]  = "sleepy end device";

    void ThreadDeviceRoleChangedHandler(thread_device_role_e role);

    const char * _ThreadRoleToStr(thread_device_role_e role);
    const char * _ThreadTypeToStr(thread_device_type_e type);
    static void _ThreadDeviceRoleChangedCb(thread_device_role_e deviceRole, void * userData);
    static void _ThreadIpAddressCb(int index, char * ipAddr, thread_ipaddr_type_e ipAddrType, void * userData);

    Thread::OperationalDataset mDataset = {};

    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;

    bool mIsAttached;
    bool mIsInitialized;
    thread_instance_h mThreadInstance;
};

} // namespace DeviceLayer
} // namespace chip
