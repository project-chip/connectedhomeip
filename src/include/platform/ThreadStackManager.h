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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/basic-types.h>
#include <lib/support/Span.h>
#include <platform/NetworkCommissioning.h>

namespace chip {

namespace Dnssd {
struct TextEntry;
struct DnssdService;
} // namespace Dnssd

namespace Thread {
class OperationalDataset;
} // namespace Thread

namespace DeviceLayer {

class PlatformManagerImpl;
class ThreadStackManagerImpl;
class ConfigurationManagerImpl;
class DeviceControlServer;

namespace Internal {
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
// Declaration of callback types corresponding to DnssdResolveCallback and DnssdBrowseCallback to avoid circular including.
using DnsResolveCallback = void (*)(void * context, chip::Dnssd::DnssdService * result, const Span<Inet::IPAddress> & addresses,
                                    CHIP_ERROR error);
using DnsBrowseCallback  = void (*)(void * context, chip::Dnssd::DnssdService * services, size_t servicesSize, bool finalBrowse,
                                   CHIP_ERROR error);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
using DnsAsyncReturnCallback = void (*)(void * context, CHIP_ERROR error);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

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
    bool IsThreadEnabled();
    bool IsThreadProvisioned();
    bool IsThreadAttached();
    CHIP_ERROR GetThreadProvision(Thread::OperationalDataset & dataset);
    CHIP_ERROR GetAndLogThreadStatsCounters();
    CHIP_ERROR GetAndLogThreadTopologyMinimal();
    CHIP_ERROR GetAndLogThreadTopologyFull();
    CHIP_ERROR GetPrimary802154MACAddress(uint8_t * buf);
    CHIP_ERROR GetExternalIPv6Address(chip::Inet::IPAddress & addr);
    CHIP_ERROR GetPollPeriod(uint32_t & buf);

    CHIP_ERROR JoinerStart();
    CHIP_ERROR SetThreadProvision(ByteSpan aDataset);
    CHIP_ERROR SetThreadEnabled(bool val);
    CHIP_ERROR AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                     NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback);
    CHIP_ERROR StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback);
    void OnThreadAttachFinished(void);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    CHIP_ERROR AddSrpService(const char * aInstanceName, const char * aName, uint16_t aPort,
                             const Span<const char * const> & aSubTypes, const Span<const Dnssd::TextEntry> & aTxtEntries,
                             uint32_t aLeaseInterval, uint32_t aKeyLeaseInterval);
    CHIP_ERROR RemoveSrpService(const char * aInstanceName, const char * aName);
    CHIP_ERROR InvalidateAllSrpServices(); ///< Mark all SRP services as invalid
    CHIP_ERROR RemoveInvalidSrpServices(); ///< Remove SRP services marked as invalid
    CHIP_ERROR SetupSrpHost(const char * aHostName);
    CHIP_ERROR ClearSrpHost(const char * aHostName);
    CHIP_ERROR SetSrpDnsCallbacks(DnsAsyncReturnCallback aInitCallback, DnsAsyncReturnCallback aErrorCallback, void * aContext);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    CHIP_ERROR DnsBrowse(const char * aServiceName, DnsBrowseCallback aCallback, void * aContext);
    CHIP_ERROR DnsResolve(const char * aServiceName, const char * aInstanceName, DnsResolveCallback aCallback, void * aContext);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    void ResetThreadNetworkDiagnosticsCounts(void);
    CHIP_ERROR WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

private:
    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    friend class ConfigurationManagerImpl;
    friend class DeviceControlServer;
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    friend class Internal::BLEManagerImpl;
#endif
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
    void ErasePersistentInfo();
    ConnectivityManager::ThreadDeviceType GetThreadDeviceType();
    CHIP_ERROR SetThreadDeviceType(ConnectivityManager::ThreadDeviceType threadRole);

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    CHIP_ERROR GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig);

    /**
     * Sets Sleepy End Device intervals configuration and posts kSEDIntervalChange event to inform other software
     * modules about the change.
     *
     * @param[in]  intervalsConfig  intervals configuration to be set
     */
    CHIP_ERROR SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig);

    /**
     * Requests setting Sleepy End Device active interval on or off.
     * Every method call with onOff parameter set to true or false results in incrementing or decrementing the active mode
     * consumers counter. Active mode is set if the consumers counter is bigger than 0.
     *
     * @param[in]  onOff  true if active mode should be enabled and false otherwise.
     */
    CHIP_ERROR RequestSEDActiveMode(bool onOff);
#endif

    bool HaveMeshConnectivity();

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
                                                    const Span<const Dnssd::TextEntry> & aTxtEntries, uint32_t aLeaseInterval = 0,
                                                    uint32_t aKeyLeaseInterval = 0)
{
    return static_cast<ImplClass *>(this)->_AddSrpService(aInstanceName, aName, aPort, aSubTypes, aTxtEntries, aLeaseInterval,
                                                          aKeyLeaseInterval);
}

inline CHIP_ERROR ThreadStackManager::RemoveSrpService(const char * aInstanceName, const char * aName)
{
    return static_cast<ImplClass *>(this)->_RemoveSrpService(aInstanceName, aName);
}

inline CHIP_ERROR ThreadStackManager::InvalidateAllSrpServices()
{
    return static_cast<ImplClass *>(this)->_InvalidateAllSrpServices();
}

inline CHIP_ERROR ThreadStackManager::RemoveInvalidSrpServices()
{
    return static_cast<ImplClass *>(this)->_RemoveInvalidSrpServices();
}

inline CHIP_ERROR ThreadStackManager::SetupSrpHost(const char * aHostName)
{
    return static_cast<ImplClass *>(this)->_SetupSrpHost(aHostName);
}

inline CHIP_ERROR ThreadStackManager::ClearSrpHost(const char * aHostName)
{
    return static_cast<ImplClass *>(this)->_ClearSrpHost(aHostName);
}

inline CHIP_ERROR ThreadStackManager::SetSrpDnsCallbacks(DnsAsyncReturnCallback aInitCallback,
                                                         DnsAsyncReturnCallback aErrorCallback, void * aContext)
{
    return static_cast<ImplClass *>(this)->_SetSrpDnsCallbacks(aInitCallback, aErrorCallback, aContext);
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

inline CHIP_ERROR ThreadStackManager::GetThreadProvision(Thread::OperationalDataset & dataset)
{
    return static_cast<ImplClass *>(this)->_GetThreadProvision(dataset);
}

inline CHIP_ERROR ThreadStackManager::SetThreadProvision(ByteSpan netInfo)
{
    return static_cast<ImplClass *>(this)->_SetThreadProvision(netInfo);
}

inline CHIP_ERROR
ThreadStackManager::AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                          NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback)
{
    return static_cast<ImplClass *>(this)->_AttachToThreadNetwork(dataset, callback);
}

inline void ThreadStackManager::OnThreadAttachFinished(void)
{
    static_cast<ImplClass *>(this)->_OnThreadAttachFinished();
}

inline CHIP_ERROR ThreadStackManager::StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback)
{
    return static_cast<ImplClass *>(this)->_StartThreadScan(callback);
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

#if CHIP_DEVICE_CONFIG_ENABLE_SED
inline CHIP_ERROR ThreadStackManager::GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return static_cast<ImplClass *>(this)->_GetSEDIntervalsConfig(intervalsConfig);
}

inline CHIP_ERROR ThreadStackManager::SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return static_cast<ImplClass *>(this)->_SetSEDIntervalsConfig(intervalsConfig);
}

inline CHIP_ERROR ThreadStackManager::RequestSEDActiveMode(bool onOff)
{
    return static_cast<ImplClass *>(this)->_RequestSEDActiveMode(onOff);
}
#endif

inline bool ThreadStackManager::HaveMeshConnectivity()
{
    return static_cast<ImplClass *>(this)->_HaveMeshConnectivity();
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

inline void ThreadStackManager::ResetThreadNetworkDiagnosticsCounts()
{
    static_cast<ImplClass *>(this)->_ResetThreadNetworkDiagnosticsCounts();
}

/*
 * @brief Get runtime value from the thread network based on the given attribute ID.
 *        The info is encoded via the AttributeValueEncoder.
 *
 * @param attributeId Id of the attribute for the requested info.
 * @param aEncoder Encoder to encode the attribute value.
 *
 * @return CHIP_NO_ERROR = Succes.
 *         CHIP_ERROR_NOT_IMPLEMENTED = Runtime value for this attribute to yet available to send as reply
 *                                      Use standard read.
 *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE = Is not a Runtime readable attribute. Use standard read
 *         All other errors should be treated as a read error and reported as such.
 */
inline CHIP_ERROR ThreadStackManager::WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId,
                                                                                 app::AttributeValueEncoder & encoder)
{
    return static_cast<ImplClass *>(this)->_WriteThreadNetworkDiagnosticAttributeToTlv(attributeId, encoder);
}

} // namespace DeviceLayer
} // namespace chip
