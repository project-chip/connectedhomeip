/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#pragma once

#include <support/Span.h>

namespace chip {

namespace Mdns {
struct TextEntry;
struct MdnsService;
} // namespace Mdns

namespace DeviceLayer {

class PlatformManagerImpl;
class ThreadStackManagerImpl;
class ConfigurationManagerImpl;

namespace Internal {
class DeviceControlServer;
class BLEManagerImpl;
template <class>
class GenericPlatformManagerImpl;
template <class>
class GenericConfigurationManagerImpl;
template <class>
class GenericPlatformManagerImpl_FreeRTOS;
template <class>
class GenericConnectivityManagerImpl_Thread;
template <class>
class GenericThreadStackManagerImpl_OpenThread;
template <class>
class GenericThreadStackManagerImpl_OpenThread_LwIP;
template <class>
class GenericThreadStackManagerImpl_FreeRTOS;
} // namespace Internal

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
// Declaration of callback types corresponding to MdnsResolveCallback and MdnsBrowseCallback to avoid circular including.
using DnsResolveCallback = void (*)(void * context, chip::Mdns::MdnsService * result, CHIP_ERROR error);
using DnsBrowseCallback  = void (*)(void * context, chip::Mdns::MdnsService * services, size_t servicesSize, CHIP_ERROR error);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT

/**
 * Provides features for initializing and interacting with the Thread stack on
 * a chip-enabled device.
 */
class ThreadStackManager
{
    using ImplClass = ThreadStackManagerImpl;

public:
    // ===== Members that define the public interface of the ThreadStackManager

    CHIP_ERROR InitThreadStack();
    void ProcessThreadActivity();
    CHIP_ERROR StartThreadTask();
    void LockThreadStack();
    bool TryLockThreadStack();
    void UnlockThreadStack();
    bool HaveRouteToAddress(const chip::Inet::IPAddress & destAddr);
    CHIP_ERROR GetAndLogThreadStatsCounters();
    CHIP_ERROR GetAndLogThreadTopologyMinimal();
    CHIP_ERROR GetAndLogThreadTopologyFull();
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR GetExternalIPv6Address(chip::Inet::IPAddress & addr);
    CHIP_ERROR GetPollPeriod(uint32_t & buf);

    CHIP_ERROR JoinerStart();
    CHIP_ERROR SetThreadProvision(ByteSpan aDataset);
    CHIP_ERROR SetThreadEnabled(bool val);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    CHIP_ERROR AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                             const Span<const char * const> & aSubTypes, const Span<const Mdns::TextEntry> & aTxtEntries,
                             uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR RemoveAllSrpServices();
    CHIP_ERROR SetupSrpHost(const char * aHostName);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    CHIP_ERROR DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext);
    CHIP_ERROR DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback, void * aContext);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

private:
    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConfigurationManagerImpl;
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    friend class Internal::BLEManagerImpl;
#endif
    friend class Internal::DeviceControlServer;
    template <class>
    friend class Internal::GenericPlatformManagerImpl;
    template <class>
    friend class Internal::GenericConfigurationManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    template <class>
    friend class Internal::GenericConnectivityManagerImpl_Thread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_OpenThread_LwIP;
    template <class>
    friend class Internal::GenericThreadStackManagerImpl_FreeRTOS;

    void OnPlatformEvent(const ChipDeviceEvent * event);
    bool IsThreadEnabled();
    bool IsThreadProvisioned();
    bool IsThreadAttached();
    CHIP_ERROR GetThreadProvision(ByteSpan & netInfo);
    void ErasePersistentInfo();
    ConnectivityManager::ThreadDeviceType GetThreadDeviceType();
    CHIP_ERROR SetThreadDeviceType(ConnectivityManager::ThreadDeviceType threadRole);
    void GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig);
    CHIP_ERROR SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig);
    bool HaveMeshConnectivity();
    void OnMessageLayerActivityChanged(bool messageLayerIsActive);

protected:
    // Construction/destruction limited to subclasses.
    ThreadStackManager()  = default;
    ~ThreadStackManager() = default;

    // No copy, move or assignment.
    ThreadStackManager(const ThreadStackManager &)  = delete;
    ThreadStackManager(const ThreadStackManager &&) = delete;
    ThreadStackManager & operator=(const ThreadStackManager &) = delete;
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * chip applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
extern ThreadStackManager & ThreadStackMgr();

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * chip applications can use this to gain access to features of the ThreadStackManager
 * that are specific to the selected platform.
 */
extern ThreadStackManagerImpl & ThreadStackMgrImpl();

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the ThreadStackManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_THREADSTACKMANAGERIMPL_HEADER
#include EXTERNAL_THREADSTACKMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define THREADSTACKMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/ThreadStackManagerImpl.h>
#include THREADSTACKMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {

inline CHIP_ERROR ThreadStackManager::InitThreadStack()
{
    return static_cast<ImplClass *>(this)->_InitThreadStack();
}

inline void ThreadStackManager::ProcessThreadActivity()
{
    static_cast<ImplClass *>(this)->_ProcessThreadActivity();
}

inline CHIP_ERROR ThreadStackManager::StartThreadTask()
{
    return static_cast<ImplClass *>(this)->_StartThreadTask();
}

inline void ThreadStackManager::LockThreadStack()
{
    static_cast<ImplClass *>(this)->_LockThreadStack();
}

inline bool ThreadStackManager::TryLockThreadStack()
{
    return static_cast<ImplClass *>(this)->_TryLockThreadStack();
}

inline void ThreadStackManager::UnlockThreadStack()
{
    static_cast<ImplClass *>(this)->_UnlockThreadStack();
}

/**
 * Determines whether a route exists via the Thread interface to the specified destination address.
 */
inline bool ThreadStackManager::HaveRouteToAddress(const chip::Inet::IPAddress & destAddr)
{
    return static_cast<ImplClass *>(this)->_HaveRouteToAddress(destAddr);
}

inline void ThreadStackManager::OnPlatformEvent(const ChipDeviceEvent * event)
{
    static_cast<ImplClass *>(this)->_OnPlatformEvent(event);
}

inline bool ThreadStackManager::IsThreadEnabled()
{
    return static_cast<ImplClass *>(this)->_IsThreadEnabled();
}

inline CHIP_ERROR ThreadStackManager::SetThreadEnabled(bool val)
{
    return static_cast<ImplClass *>(this)->_SetThreadEnabled(val);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
inline CHIP_ERROR ThreadStackManager::AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                                                    const Span<const char * const> & aSubTypes,
                                                    const Span<const Mdns::TextEntry> & aTxtEntries, uint32_t aLeaseInterval = 0,
                                                    uint32_t aKeyLeaseInterval = 0)
{
    return static_cast<ImplClass *>(this)->_AddSrpService(aInstanceName, aName, aPort, aSubTypes, aTxtEntries, aLeaseInterval,
                                                          aKeyLeaseInterval);
}

inline CHIP_ERROR ThreadStackManager::RemoveSrpService(const char * aInstanceName, const char * aName)
{
    return static_cast<ImplClass *>(this)->_RemoveSrpService(aInstanceName, aName);
}

inline CHIP_ERROR ThreadStackManager::RemoveAllSrpServices()
{
    return static_cast<ImplClass *>(this)->_RemoveAllSrpServices();
}

inline CHIP_ERROR ThreadStackManager::SetupSrpHost(const char * aHostName)
{
    return static_cast<ImplClass *>(this)->_SetupSrpHost(aHostName);
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
inline CHIP_ERROR ThreadStackManager::DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext)
{
    return static_cast<ImplClass *>(this)->_DnsBrowse(aServiceName, aCallback, aContext);
}

inline CHIP_ERROR ThreadStackManager::DnsResolve(const char * aServiceName, const char * aInstanceName,
                                                 DnsResolveCallback aCallback, void * aContext)
{
    return static_cast<ImplClass *>(this)->_DnsResolve(aServiceName, aInstanceName, aCallback, aContext);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

inline bool ThreadStackManager::IsThreadProvisioned()
{
    return static_cast<ImplClass *>(this)->_IsThreadProvisioned();
}

inline bool ThreadStackManager::IsThreadAttached()
{
    return static_cast<ImplClass *>(this)->_IsThreadAttached();
}

inline CHIP_ERROR ThreadStackManager::GetThreadProvision(ByteSpan & netInfo)
{
    return static_cast<ImplClass *>(this)->_GetThreadProvision(netInfo);
}

inline CHIP_ERROR ThreadStackManager::SetThreadProvision(ByteSpan netInfo)
{
    return static_cast<ImplClass *>(this)->_SetThreadProvision(netInfo);
}

inline void ThreadStackManager::ErasePersistentInfo()
{
    static_cast<ImplClass *>(this)->_ErasePersistentInfo();
}

inline ConnectivityManager::ThreadDeviceType ThreadStackManager::GetThreadDeviceType()
{
    return static_cast<ImplClass *>(this)->_GetThreadDeviceType();
}

inline CHIP_ERROR ThreadStackManager::SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return static_cast<ImplClass *>(this)->_SetThreadDeviceType(deviceType);
}

inline void ThreadStackManager::GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    static_cast<ImplClass *>(this)->_GetThreadPollingConfig(pollingConfig);
}

inline CHIP_ERROR ThreadStackManager::SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    return static_cast<ImplClass *>(this)->_SetThreadPollingConfig(pollingConfig);
}

inline bool ThreadStackManager::HaveMeshConnectivity()
{
    return static_cast<ImplClass *>(this)->_HaveMeshConnectivity();
}

inline void ThreadStackManager::OnMessageLayerActivityChanged(bool messageLayerIsActive)
{
    return static_cast<ImplClass *>(this)->_OnMessageLayerActivityChanged(messageLayerIsActive);
}

inline CHIP_ERROR ThreadStackManager::GetAndLogThreadStatsCounters()
{
    return static_cast<ImplClass *>(this)->_GetAndLogThreadStatsCounters();
}

inline CHIP_ERROR ThreadStackManager::GetAndLogThreadTopologyMinimal()
{
    return static_cast<ImplClass *>(this)->_GetAndLogThreadTopologyMinimal();
}

inline CHIP_ERROR ThreadStackManager::GetAndLogThreadTopologyFull()
{
    return static_cast<ImplClass *>(this)->_GetAndLogThreadTopologyFull();
}

inline CHIP_ERROR ThreadStackManager::GetPrimary802154MACAddress(uint8_t * buf)
{
    return static_cast<ImplClass *>(this)->_GetPrimary802154MACAddress(buf);
}

inline CHIP_ERROR ThreadStackManager::GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    return static_cast<ImplClass *>(this)->_GetExternalIPv6Address(addr);
}

inline CHIP_ERROR ThreadStackManager::GetPollPeriod(uint32_t & buf)
{
    return static_cast<ImplClass *>(this)->_GetPollPeriod(buf);
}

inline CHIP_ERROR ThreadStackManager::JoinerStart()
{
    return static_cast<ImplClass *>(this)->_JoinerStart();
}

} // namespace DeviceLayer
} // namespace chip
