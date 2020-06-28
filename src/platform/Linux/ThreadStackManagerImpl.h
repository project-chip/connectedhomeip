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

#ifndef CHIP_PLATFORM_LINUX_THREAD_STACK_MANAGER_IMPL_H
#define CHIP_PLATFORM_LINUX_THREAD_STACK_MANAGER_IMPL_H

#include <memory>

#include "platform/internal/CHIPDeviceLayerInternal.h"

#include "dbus/client/thread_api_dbus.hpp"
#include "platform/internal/DeviceNetworkInfo.h"

namespace chip {
namespace DeviceLayer {

class ThreadStackManagerImpl : public ThreadStackManager
{
public:
    ThreadStackManagerImpl(DBusConnection * aConnection);

    CHIP_ERROR _InitThreadStack();
    CHIP_ERROR _ProcessThreadActivity();

    CHIP_ERROR _StartThreadTask() { return CHIP_NO_ERROR; }    // Intentionally left blank
    CHIP_ERROR _LockThreadStack() { return CHIP_NO_ERROR; }    // Intentionally left blank
    CHIP_ERROR _TryLockThreadStack() { return CHIP_NO_ERROR; } // Intentionally left blank
    CHIP_ERROR _UnlockThreadStack() { return CHIP_NO_ERROR; }  // Intentionally left blank

    bool _HaveRouteToAddress(const Inet::IPAddress & destAddr);

    void _OnPlatformEvent(const ChipDeviceEvent * event);

    CHIP_ERROR _GetThreadProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);

    CHIP_ERROR _SetThreadProvision(const Internal::DeviceNetworkInfo & netInfo);

    void _ClearThreadProvision();

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

    void _OnCHIPoBLEAdvertisingStart();

    void _OnCHIPoBLEAdvertisingStop();

    CHIP_ERROR _GetAndLogThreadStatsCounters();

    CHIP_ERROR _GetAndLogThreadTopologyMinimal();

    CHIP_ERROR _GetAndLogThreadTopologyFull();

    CHIP_ERROR _GetPrimary802154MACAddress(uint8_t * buf);

    void _FactoryReset();

    ~ThreadStackManagerImpl() = default;

private:
    void _ThreadDevcieRoleChangedHandler(otbr::DBus::DeviceRole role);

    std::unique_ptr<otbr::DBus::ThreadApiDBus> mThreadApi;
    DBusConnection * mConnection;
    Internal::DeviceNetworkInfo mNetworkInfo;
    bool mAttached;
};

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_PLATFORM_LINUX_THREAD_STACK_MANAGER_IMPL_H
