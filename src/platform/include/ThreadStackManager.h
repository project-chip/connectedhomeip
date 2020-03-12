/*
 *
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
 *          Defines the public interface for the Device Layer ThreadStackManager object.
 */

#ifndef THREAD_STACK_MANAGER_H
#define THREAD_STACK_MANAGER_H

namespace nl {
namespace Weave {
namespace DeviceLayer {

class PlatformManagerImpl;
class ThreadStackManagerImpl;
class ConfigurationManagerImpl;

namespace Internal {
class DeviceNetworkInfo;
class DeviceControlServer;
template<class> class GenericPlatformManagerImpl;
template<class> class GenericPlatformManagerImpl_FreeRTOS;
template<class> class GenericConnectivityManagerImpl_Thread;
template<class> class GenericThreadStackManagerImpl_OpenThread;
template<class> class GenericThreadStackManagerImpl_OpenThread_LwIP;
template<class> class GenericThreadStackManagerImpl_FreeRTOS;
template<class> class GenericNetworkProvisioningServerImpl;
} // namespace Internal

/**
 * Provides features for initializing and interacting with the Thread stack on
 * a Weave-enabled device.
 */
class ThreadStackManager
{
    using ImplClass = ThreadStackManagerImpl;

public:

    // ===== Members that define the public interface of the ThreadStackManager

    WEAVE_ERROR InitThreadStack(void);
    void ProcessThreadActivity(void);
    WEAVE_ERROR StartThreadTask(void);
    void LockThreadStack(void);
    bool TryLockThreadStack(void);
    void UnlockThreadStack(void);
    bool HaveRouteToAddress(const IPAddress & destAddr);
    WEAVE_ERROR GetAndLogThreadStatsCounters(void);
    WEAVE_ERROR GetAndLogThreadTopologyMinimal(void);
    WEAVE_ERROR GetAndLogThreadTopologyFull(void);
    WEAVE_ERROR GetPrimary802154MACAddress(uint8_t *buf);

private:

    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class Internal::BLEManagerImpl;
    friend class Internal::DeviceControlServer;
    template<class> friend class Internal::GenericPlatformManagerImpl;
    template<class> friend class Internal::GenericConfigurationManagerImpl;
    template<class> friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    template<class> friend class Internal::GenericConnectivityManagerImpl_Thread;
    template<class> friend class Internal::GenericThreadStackManagerImpl_OpenThread;
    template<class> friend class Internal::GenericThreadStackManagerImpl_OpenThread_LwIP;
    template<class> friend class Internal::GenericThreadStackManagerImpl_FreeRTOS;
    template<class> friend class Internal::GenericNetworkProvisioningServerImpl;

    void OnPlatformEvent(const WeaveDeviceEvent * event);
    bool IsThreadEnabled(void);
    WEAVE_ERROR SetThreadEnabled(bool val);
    bool IsThreadProvisioned(void);
    bool IsThreadAttached(void);
    WEAVE_ERROR GetThreadProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    WEAVE_ERROR SetThreadProvision(const Internal::DeviceNetworkInfo & netInfo);
    void ClearThreadProvision(void);
    ConnectivityManager::ThreadDeviceType GetThreadDeviceType(void);
    WEAVE_ERROR SetThreadDeviceType(ConnectivityManager::ThreadDeviceType threadRole);
    void GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig);
    WEAVE_ERROR SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig);
    bool HaveMeshConnectivity(void);
    void OnMessageLayerActivityChanged(bool messageLayerIsActive);
    void OnWoBLEAdvertisingStart(void);
    void OnWoBLEAdvertisingStop(void);

protected:

    // Construction/destruction limited to subclasses.
    ThreadStackManager() = default;
    ~ThreadStackManager() = default;

    // No copy, move or assignment.
    ThreadStackManager(const ThreadStackManager &) = delete;
    ThreadStackManager(const ThreadStackManager &&) = delete;
    ThreadStackManager & operator=(const ThreadStackManager &) = delete;
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * Weave applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
extern ThreadStackManager & ThreadStackMgr(void);

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * Weave applications can use this to gain access to features of the ThreadStackManager
 * that are specific to the selected platform.
 */
extern ThreadStackManagerImpl & ThreadStackMgrImpl(void);

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

/* Include a header file containing the implementation of the ThreadStackManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_THREADSTACKMANAGERIMPL_HEADER
#include EXTERNAL_THREADSTACKMANAGERIMPL_HEADER
#else
#define THREADSTACKMANAGERIMPL_HEADER <Weave/DeviceLayer/WEAVE_DEVICE_LAYER_TARGET/ThreadStackManagerImpl.h>
#include THREADSTACKMANAGERIMPL_HEADER
#endif

namespace nl {
namespace Weave {
namespace DeviceLayer {

inline WEAVE_ERROR ThreadStackManager::InitThreadStack()
{
    return static_cast<ImplClass*>(this)->_InitThreadStack();
}

inline void ThreadStackManager::ProcessThreadActivity()
{
    static_cast<ImplClass*>(this)->_ProcessThreadActivity();
}

inline WEAVE_ERROR ThreadStackManager::StartThreadTask()
{
    return static_cast<ImplClass*>(this)->_StartThreadTask();
}

inline void ThreadStackManager::LockThreadStack()
{
    static_cast<ImplClass*>(this)->_LockThreadStack();
}

inline bool ThreadStackManager::TryLockThreadStack()
{
    return static_cast<ImplClass*>(this)->_TryLockThreadStack();
}

inline void ThreadStackManager::UnlockThreadStack()
{
    static_cast<ImplClass*>(this)->_UnlockThreadStack();
}

/**
 * Determines whether a route exists via the Thread interface to the specified destination address.
 */
inline bool ThreadStackManager::HaveRouteToAddress(const IPAddress & destAddr)
{
    return static_cast<ImplClass*>(this)->_HaveRouteToAddress(destAddr);
}

inline void ThreadStackManager::OnPlatformEvent(const WeaveDeviceEvent * event)
{
    static_cast<ImplClass*>(this)->_OnPlatformEvent(event);
}

inline bool ThreadStackManager::IsThreadEnabled(void)
{
    return static_cast<ImplClass*>(this)->_IsThreadEnabled();
}

inline WEAVE_ERROR ThreadStackManager::SetThreadEnabled(bool val)
{
    return static_cast<ImplClass*>(this)->_SetThreadEnabled(val);
}

inline bool ThreadStackManager::IsThreadProvisioned(void)
{
    return static_cast<ImplClass*>(this)->_IsThreadProvisioned();
}

inline bool ThreadStackManager::IsThreadAttached(void)
{
    return static_cast<ImplClass*>(this)->_IsThreadAttached();
}

inline WEAVE_ERROR ThreadStackManager::GetThreadProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    return static_cast<ImplClass*>(this)->_GetThreadProvision(netInfo, includeCredentials);
}

inline WEAVE_ERROR ThreadStackManager::SetThreadProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    return static_cast<ImplClass*>(this)->_SetThreadProvision(netInfo);
}

inline void ThreadStackManager::ClearThreadProvision(void)
{
    static_cast<ImplClass*>(this)->_ClearThreadProvision();
}

inline ConnectivityManager::ThreadDeviceType ThreadStackManager::GetThreadDeviceType(void)
{
    return static_cast<ImplClass*>(this)->_GetThreadDeviceType();
}

inline WEAVE_ERROR ThreadStackManager::SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return static_cast<ImplClass*>(this)->_SetThreadDeviceType(deviceType);
}

inline void ThreadStackManager::GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    static_cast<ImplClass*>(this)->_GetThreadPollingConfig(pollingConfig);
}

inline WEAVE_ERROR ThreadStackManager::SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    return static_cast<ImplClass*>(this)->_SetThreadPollingConfig(pollingConfig);
}

inline bool ThreadStackManager::HaveMeshConnectivity(void)
{
    return static_cast<ImplClass*>(this)->_HaveMeshConnectivity();
}

inline void ThreadStackManager::OnMessageLayerActivityChanged(bool messageLayerIsActive)
{
    return static_cast<ImplClass*>(this)->_OnMessageLayerActivityChanged(messageLayerIsActive);
}

inline void ThreadStackManager::OnWoBLEAdvertisingStart(void)
{
    static_cast<ImplClass*>(this)->_OnWoBLEAdvertisingStart();
}

inline void ThreadStackManager::OnWoBLEAdvertisingStop(void)
{
    static_cast<ImplClass*>(this)->_OnWoBLEAdvertisingStop();
}

inline WEAVE_ERROR ThreadStackManager::GetAndLogThreadStatsCounters(void)
{
    return static_cast<ImplClass*>(this)->_GetAndLogThreadStatsCounters();
}

inline WEAVE_ERROR ThreadStackManager::GetAndLogThreadTopologyMinimal(void)
{
    return static_cast<ImplClass*>(this)->_GetAndLogThreadTopologyMinimal();
}

inline WEAVE_ERROR ThreadStackManager::GetAndLogThreadTopologyFull(void)
{
    return static_cast<ImplClass*>(this)->_GetAndLogThreadTopologyFull();
}

inline WEAVE_ERROR ThreadStackManager::GetPrimary802154MACAddress(uint8_t * buf)
{
    return static_cast<ImplClass*>(this)->_GetPrimary802154MACAddress(buf);
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // THREAD_STACK_MANAGER_H
