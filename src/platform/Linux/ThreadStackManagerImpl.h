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

#include <lib/support/ThreadOperationalDataset.h>
#include <platform/Linux/GlibTypeDeleter.h>
#include <platform/Linux/dbus/openthread/introspect.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/DeviceNetworkInfo.h>

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

    CHIP_ERROR _GetThreadProvision(ByteSpan & netInfo);

    CHIP_ERROR _SetThreadProvision(ByteSpan netInfo);

    void _ErasePersistentInfo();

    bool _IsThreadProvisioned();

    bool _IsThreadEnabled();

    bool _IsThreadAttached();

    CHIP_ERROR _SetThreadEnabled(bool val);

    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType();

    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);

    void _GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig);

    CHIP_ERROR _SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig);

    bool _HaveMeshConnectivity();

    void _OnMessageLayerActivityChanged(bool messageLayerIsActive);

    CHIP_ERROR _GetAndLogThreadStatsCounters();

    CHIP_ERROR _GetAndLogThreadTopologyMinimal();

    CHIP_ERROR _GetAndLogThreadTopologyFull();

    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);

    CHIP_ERROR _GetExternalIPv6Address(chip::Inet::IPAddress & addr);

    CHIP_ERROR _GetPollPeriod(uint32_t & buf);

    CHIP_ERROR _JoinerStart();

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

    std::unique_ptr<OpenthreadIoOpenthreadBorderRouter, GObjectDeleter> mProxy;

    static void OnDbusPropertiesChanged(OpenthreadIoOpenthreadBorderRouter * proxy, GVariant * changed_properties,
                                        const gchar * const * invalidated_properties, gpointer user_data);
    void ThreadDevcieRoleChangedHandler(const gchar * role);

    Thread::OperationalDataset mDataset = {};

    bool mAttached;
};

} // namespace DeviceLayer
} // namespace chip
