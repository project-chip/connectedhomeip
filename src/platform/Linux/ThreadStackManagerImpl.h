/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <memory>
#include <vector>

#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/openthread/DBusOpenthread.h>
#include <platform/NetworkCommissioning.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {

template <>
struct GAutoPtrDeleter<OpenthreadBorderRouter>
{
    using deleter = GObjectDeleter;
};

namespace DeviceLayer {

class ThreadStackManagerImpl : public ThreadStackManager
{
public:
    ThreadStackManagerImpl();

    void
    SetNetworkStatusChangeCallback(NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback)
    {
        mpStatusChangeCallback = statusChangeCallback;
    }

    CHIP_ERROR _InitThreadStack();
    void _ProcessThreadActivity();

    CHIP_ERROR _StartThreadTask() { return CHIP_NO_ERROR; } // Intentionally left blank
    void _LockThreadStack() {}                              // Intentionally left blank
    bool _TryLockThreadStack() { return false; }            // Intentionally left blank
    void _UnlockThreadStack() {}                            // Intentionally left blank

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    void _WaitOnSrpClearAllComplete() {}
    void _NotifySrpClearAllComplete() {}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    bool _HaveRouteToAddress(const Inet::IPAddress & destAddr);

    void _OnPlatformEvent(const ChipDeviceEvent * event);

    CHIP_ERROR _GetThreadProvision(Thread::OperationalDataset & dataset);

    CHIP_ERROR _SetThreadProvision(ByteSpan netInfo);

    void _OnNetworkScanFinished(GAsyncResult * res);
    static void _OnNetworkScanFinished(GObject * source_object, GAsyncResult * res, gpointer user_data);

    CHIP_ERROR GetExtendedPanId(uint8_t extPanId[Thread::kSizeExtendedPanId]);

    void _ErasePersistentInfo();

    bool _IsThreadProvisioned();

    bool _IsThreadEnabled();

    bool _IsThreadAttached() const;

    CHIP_ERROR _AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                      NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback);

    CHIP_ERROR _SetThreadEnabled(bool val);

    void _OnThreadAttachFinished(void);

    void _UpdateNetworkStatus();

    static void _OnThreadBrAttachFinished(GObject * source_object, GAsyncResult * res, gpointer user_data);

    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType();

    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR _SetPollingInterval(System::Clock::Milliseconds32 pollingInterval);
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER */

    bool _HaveMeshConnectivity();

    CHIP_ERROR _GetAndLogThreadStatsCounters();

    CHIP_ERROR _GetAndLogThreadTopologyMinimal();

    CHIP_ERROR _GetAndLogThreadTopologyFull();

    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);

    CHIP_ERROR _GetExternalIPv6Address(chip::Inet::IPAddress & addr);
    CHIP_ERROR _GetThreadVersion(uint16_t & version);
    CHIP_ERROR _GetPollPeriod(uint32_t & buf);

    void _ResetThreadNetworkDiagnosticsCounts();

    CHIP_ERROR _StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback);

    ~ThreadStackManagerImpl() = default;

    static ThreadStackManagerImpl sInstance;

private:
    static constexpr char kDBusOpenThreadService[]    = "io.openthread.BorderRouter.wpan0";
    static constexpr char kDBusOpenThreadObjectPath[] = "/io/openthread/BorderRouter/wpan0";

    static constexpr char kOpenthreadDeviceRoleDisabled[] = "disabled";
    static constexpr char kOpenthreadDeviceRoleDetached[] = "detached";
    static constexpr char kOpenthreadDeviceRoleChild[]    = "child";
    static constexpr char kOpenthreadDeviceRoleRouter[]   = "router";
    static constexpr char kOpenthreadDeviceRoleLeader[]   = "leader";

    static constexpr char kPropertyDeviceRole[] = "DeviceRole";

    struct ThreadNetworkScanned
    {
        uint16_t panId;
        uint64_t extendedPanId;
        uint8_t networkName[16];
        uint8_t networkNameLen;
        uint16_t channel;
        uint8_t version;
        uint64_t extendedAddress;
        int8_t rssi;
        uint8_t lqi;
    };

    GAutoPtr<OpenthreadBorderRouter> mProxy;

    static CHIP_ERROR GLibMatterContextInitThreadStack(ThreadStackManagerImpl * self);
    static CHIP_ERROR GLibMatterContextCallAttach(ThreadStackManagerImpl * self);
    static CHIP_ERROR GLibMatterContextCallScan(ThreadStackManagerImpl * self);
    static void OnDbusPropertiesChanged(OpenthreadBorderRouter * proxy, GVariant * changed_properties,
                                        const gchar * const * invalidated_properties, gpointer user_data);
    void ThreadDeviceRoleChangedHandler(const gchar * role);

    Thread::OperationalDataset mDataset = {};

    NetworkCommissioning::ThreadDriver::ScanCallback * mpScanCallback;
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;
    NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;

    bool mAttached;
};

inline void ThreadStackManagerImpl::_OnThreadAttachFinished(void)
{
    // stub for ThreadStackManager.h
}

} // namespace DeviceLayer
} // namespace chip
