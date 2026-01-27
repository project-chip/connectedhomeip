/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <mutex>
#include <utility>

#include <gio/gio.h>

#include <lib/core/Optional.h>
#include <lib/support/FixedBuffer.h>
#include <lib/support/Span.h>
#include <platform/Linux/dbus/connman/DBusConnManAgent.h>
#include <platform/Linux/dbus/connman/DBusConnManManager.h>
#include <platform/Linux/dbus/connman/DBusConnManService.h>
#include <platform/Linux/dbus/connman/DBusConnManTechnology.h>
#include <system/SystemMutex.h>

#include "ConnectivityManagerImpl_NetworkManagementBasis.h"
#include "ConnectivityManagerImpl_NetworkManagementInterface.h"

namespace chip {

template <>
struct GAutoPtrDeleter<ConnManAgent>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<ConnManManager>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<ConnManService>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<ConnManTechnology>
{
    using deleter = GObjectDeleter;
};

namespace DeviceLayer {

// Forward Declarations

struct ChipDeviceEvent;

/**
 *  Provides an implementation of the ConnectionManager object
 *  for Linux platforms where Connection Manager (aka connman)
 *  is the high-level network manager, that owns network
 *  policy and state and orchestrates connectivity decisions
 *  and cross-interface policy.
 *
 */
class ConnectivityManagerImpl_NetworkManagementConnMan final : public Internal::NetworkManagementBasis,
                                                               public Internal::NetworkManagementInterface
{
public:
    // Construction

    ConnectivityManagerImpl_NetworkManagementConnMan(void) = default;

    // Destruction

    virtual ~ConnectivityManagerImpl_NetworkManagementConnMan(void) = default;

    // Initialization

    CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) override final;

    // Observation

    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) override final;

    // Event Handling

    void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) override final;

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Ethernet Control Plane Management

    const char * GetEthernetIfName(void) override final;
    void UpdateEthernetNetworkingStatus(void) override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Observation

    const char * GetWiFiIfName(void) override final;

    // Control

    void StartNonConcurrentWiFiManagement(void) override final;
    void StartWiFiManagement(void) override final;

    // Wi-Fi Station Control Plane Management

    // Introspection

    bool IsWiFiManagementStarted(void) override final;
    bool IsWiFiStationApplicationControlled(void) override final;
    bool IsWiFiStationConnected(void) override final;
    bool IsWiFiStationEnabled(void) override final;
    bool IsWiFiStationProvisioned(void) override final;

    // Observation

    CHIP_ERROR GetWiFiBssId(MutableByteSpan & outBssId) override final;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType) override final;
    ConnectivityManager::WiFiStationMode GetWiFiStationMode(void) override final;
    System::Clock::Timeout GetWiFiStationReconnectInterval(void) override final;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion) override final;

    // Mutation

    CHIP_ERROR SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode) override final;
    CHIP_ERROR SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval) override final;

    // Worker

    void ClearWiFiStationProvision(void) override final;
    CHIP_ERROR CommitConfig(void) override final;
    CHIP_ERROR
    ConnectWiFiNetworkAsync(const ByteSpan & inSsid, const ByteSpan & inCredentials,
                            NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR ConnectWiFiNetworkWithPDCAsync(
        const ByteSpan & inSsid, const ByteSpan & inNetworkIdentity, const ByteSpan & inClientIdentity,
        const Crypto::P256Keypair & inClientIdentityKeypair,
        NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR StartWiFiScan(const ByteSpan & inSsid,
                             NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback) override final;

    // Wi-Fi Soft Access Point (AP) Control Plane Management

    // Observation

    ConnectivityManager::WiFiAPMode GetWiFiApMode(void) override final;

    // Mutation

    CHIP_ERROR SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode) override final;
    void SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout) override final;

    // Control

    void DemandStartWiFiAp(void) override final;
    void StopOnDemandWiFiAp(void) override final;
    void MaintainOnDemandWiFiAp(void) override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

private:
    // Initialization

    static CHIP_ERROR InitAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf, GDBusConnection * inConnection,
                                      const char * inPath, ConnManAgent *& outSkeleton, GError ** outError) noexcept;
    void ShutdownAgentLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath) noexcept;
    static void ShutdownAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf, GDBusConnection * inConnection,
                                    const char * inPath, ConnManAgent * inSkeleton) noexcept;

    // Helper Methods

    using ManagerAgentOpFunc          = gboolean (*)(ConnManManager * inManager, const gchar * inPath, GCancellable * inCancellable,
                                            GError ** inOutError);
    using ManagerGetObjectsFinishFunc = gboolean (*)(ConnManManager * inManager, GVariant ** outObjects,
                                                     GAsyncResult * inAsyncResult, GError ** outError);
    using ObjectActionFinishFunc      = gboolean (*)(GObject * inObject, GAsyncResult * inAsyncResult, GError ** outError);

    using HandleManagerGetObjectsMethod = void (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        ConnManManager * inManager, GVariant * inObjects, const GError * inError) noexcept;
    using HandleObjectActionCompleteMethod =
        void (ConnectivityManagerImpl_NetworkManagementConnMan::*)(GDBusProxy * inProxy, const GError * inError) noexcept;
    using ObjectPropertiesChangedAnyLockedMethod = CHIP_ERROR (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        GDBusProxy * inProxy, const char * inKey, GVariant * inMaybeVariant) noexcept;
    using ObjectPropertiesChangedLockedMethod = CHIP_ERROR (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        GDBusProxy * inProxy, const char * inKey, GVariant * inValue) noexcept;
    using TypedObjectPropertiesChangedAnyLockedMethod = CHIP_ERROR (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey, GVariant * inMaybeVariant) noexcept;
    using TypedObjectPropertyChangedLockedMethod = CHIP_ERROR (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey, GVariant * inValue) noexcept;

    const char * GetInterfaceName(const char * inDescription, const char * inType, char * inOutInterfaceName) noexcept;
    void HandleManagerGetProperties(ConnManManager * inManager, GVariant * inProperties, const GError * inError) noexcept;
    void HandleManagerGetServices(ConnManManager * inManager, GVariant * inServices, const GError * inError) noexcept;
    void HandleManagerGetTechnologies(ConnManManager * inManager, GVariant * inTechnologies, const GError * inError) noexcept;
    CHIP_ERROR HandleManagerPropertiesChangedLocked(ConnManManager * inManager, GVariant * inProperties) noexcept;
    void HandleManagerPropertyChanged(ConnManManager * inManager, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleManagerPropertyChangedLocked(ConnManManager * inManager, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleManagerPropertyChangedAnyLocked(GDBusProxy * inManager, const char * inKey,
                                                     GVariant * inMaybeVariant) noexcept;
    void HandleManagerServicesChanged(ConnManManager * inManager, GVariant * inServicesChanged,
                                      const char * const * inServicesRemoved) noexcept;
    void HandleManagerServicesChangedLocked(std::unique_lock<std::mutex> & inOutLock, ConnManManager * inManager,
                                            GVariant * inServicesChanged) noexcept;
    void HandleManagerServicesRemovedLocked(std::unique_lock<std::mutex> & inOutLock, ConnManManager * inManager,
                                            const char * const * inServicesRemoved) noexcept;
    void HandleManagerTechnologyAdded(ConnManManager * inManager, const char * inPath, GVariant * inProperties) noexcept;
    void HandleManagerTechnologyRemoved(ConnManManager * inManager, const char * inPath) noexcept;
    CHIP_ERROR
    HandleObjectPropertiesChangedLocked(const char * inDescription, GDBusProxy * inProxy, GVariant * inProperties,
                                        ObjectPropertiesChangedAnyLockedMethod inObjectPropertiesChangedAnyLockedMethod) noexcept;
    CHIP_ERROR HandleObjectPropertiesChangedLocked(
        const char * inDescription, GDBusProxy * inProxy, const char * inPath, const char * inType, GVariant * inProperties,
        TypedObjectPropertiesChangedAnyLockedMethod inTypedObjectPropertiesChangedAnyLockedMethod) noexcept;
    CHIP_ERROR
    HandleObjectPropertyChangedAnyLocked(GDBusProxy * inProxy, const char * inPath, const char * inType, const char * inKey,
                                         GVariant * inMaybeVariant,
                                         TypedObjectPropertyChangedLockedMethod inTypedObjectPropertyChangedLockedMethod) noexcept;
    void HandleServiceConnectComplete(GDBusProxy * inService, const GError * inError) noexcept;
    CHIP_ERROR HandleServiceConnectRequestLocked(std::unique_lock<std::mutex> & inOutLock, ConnManService * inService) noexcept;
    CHIP_ERROR HandleServicePropertiesChangedLocked(GDBusProxy * inProxy, const char * inPath, const char * inType,
                                                    GVariant * inProperties) noexcept;
    void HandleServicePropertyChanged(ConnManService * inService, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleServicePropertyChangedLocked(GDBusProxy * inProxy, const char * inPath, const char * inType,
                                                  const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleServicePropertyChangedAnyLocked(GDBusProxy * inService, const char * inPath, const char * inType,
                                                     const char * inKey, GVariant * inMaybeVariant) noexcept;
    CHIP_ERROR HandleTechnologyPropertiesChangedLocked(GDBusProxy * inProxy, const char * inPath, const char * inType,
                                                       GVariant * inProperties) noexcept;
    void HandleTechnologyPropertyChanged(ConnManTechnology * inTechnology, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleTechnologyPropertyChangedLocked(GDBusProxy * inProxy, const char * inPath, const char * inType,
                                                     const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleTechnologyPropertyChangedAnyLocked(GDBusProxy * inTechnology, const char * inPath, const char * inType,
                                                        const char * inKey, GVariant * inMaybeVariant) noexcept;
    void HandleTechnologyScanComplete(GDBusProxy * inTechnology, const GError * inError) noexcept;
    CHIP_ERROR HandleWiFiPendingConnectLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                              size_t & inOutScanCount, Internal::WiFiSSIDFixedBuffer & inOutSsid) noexcept;
    CHIP_ERROR HandleWiFiPendingScanLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                           size_t & inOutScanCount, Internal::WiFiSSIDFixedBuffer & inOutSsid) noexcept;
    CHIP_ERROR HandleWiFiScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology) noexcept;
    CHIP_ERROR HandleWiFiScanRetryLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                         const char * inReason, size_t & inOutScanCount,
                                         const Internal::WiFiSSIDFixedBuffer & inSsid) noexcept;
    CHIP_ERROR ManagerAgentOpLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath,
                                    ManagerAgentOpFunc inManagerAgentOpFunc, const char * inAction,
                                    const bool & inRegister) noexcept;
    void MaybeClearInterfaceNameLocked(const char * inPath) noexcept;
    CHIP_ERROR MaybeSetInterfaceNameLocked(const char * inType, const char * inInterface) noexcept;
    CHIP_ERROR RemoveServiceLocked(const char * inPath) noexcept;
    CHIP_ERROR RemoveTechnologyLocked(const char * inPath) noexcept;
    CHIP_ERROR StartNetworkManagementOnGLib(void) noexcept;
    CHIP_ERROR UpdateManagerPropertiesLocked(GVariant * inProperties) noexcept;
    CHIP_ERROR UpdateServicesLocked(GVariant * inServices) noexcept;
    CHIP_ERROR UpdateServiceProxyLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath,
                                        ConnManService *& outService) noexcept;
    CHIP_ERROR UpdateTechnologiesLocked(GVariant * inTechnologies) noexcept;
    CHIP_ERROR UpdateTechnologyProxyLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath,
                                           ConnManTechnology *& outTechnology) noexcept;

    // Worker Methods

    CHIP_ERROR ManagerRegisterAgent(const char * inPath) noexcept;
    CHIP_ERROR ManagerRegisterAgentLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath) noexcept;
    CHIP_ERROR ManagerUnregisterAgent(const char * inPath) noexcept;
    CHIP_ERROR ManagerUnregisterAgentLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath) noexcept;
    CHIP_ERROR ServiceConnectLocked(std::unique_lock<std::mutex> & inOutLock, ConnManService * inService) noexcept;
    void ServiceRegisterPropertyChangedOnGLib(ConnManService * inService) noexcept;
    void TechnologyRegisterPropertyChangedOnGLib(ConnManTechnology * inTechnology) noexcept;
    CHIP_ERROR TechnologyScanLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology) noexcept;
    CHIP_ERROR TechnologySetPropertyLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                           const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR TechnologySetPoweredLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                          const bool & inPowered) noexcept;

    // D-Bus / glib Asynchronous Completion Methods

    void OnManagerReady(GObject * inObject, GAsyncResult * inResult);
    void OnManagerGetObjectsReady(GObject * inObject, GAsyncResult * inResult,
                                  ManagerGetObjectsFinishFunc inManagerGetObjectsFinishFunc,
                                  HandleManagerGetObjectsMethod inHandleManagerGetObjectsMethod) noexcept;
    void OnManagerGetPropertiesReady(GObject * inObject, GAsyncResult * inResult);
    void OnManagerGetServicesReady(GObject * inObject, GAsyncResult * inResult);
    void OnManagerGetTechnologiesReady(GObject * inObject, GAsyncResult * inResult);
    void OnObjectActionReady(GObject * inObject, GAsyncResult * inResult, ObjectActionFinishFunc inObjectActionFinishFunc,
                             HandleObjectActionCompleteMethod inHandleObjectActionCompleteMethod) noexcept;
    void OnServiceConnectReady(GObject * inObject, GAsyncResult * inResult);
    void OnTechnologyScanReady(GObject * inObject, GAsyncResult * inResult);

    // D-Bus / glib Signal Callback Methods

    gboolean OnAgentCancel(ConnManAgent * inAgent, GDBusMethodInvocation * inInvocation) noexcept;
    gboolean OnAgentRelease(ConnManAgent * inAgent, GDBusMethodInvocation * inInvocation) noexcept;
    gboolean OnAgentRequestInput(ConnManAgent * inAgent, GDBusMethodInvocation * inInvocation, const gchar * inPath,
                                 GVariant * inProperties) noexcept;
    gboolean OnAgentReportError(ConnManAgent * inAgent, GDBusMethodInvocation * inInvocation, const gchar * inPath,
                                const gchar * inError) noexcept;
    void OnManagerPropertyChanged(ConnManManager * inManager, const char * inKey, GVariant * inValue);
    void OnManagerTechnologyAdded(ConnManManager * inManager, const char * inPath, GVariant * inProperties);
    void OnManagerTechnologyRemoved(ConnManManager * inManager, const char * inPath);
    void OnManagerServicesChanged(ConnManManager * inManager, GVariant * inServicesChanged, const char * const * inServicesRemoved);
    void OnServicePropertyChanged(ConnManService * inService, const char * inKey, GVariant * inValue);
    void OnTechnologyPropertyChanged(ConnManTechnology * inTechnology, const char * inKey, GVariant * inValue);

private:
    struct GDBusConnManClient
    {
        GAutoPtr<ConnManManager> mManagerProxy;
        GAutoPtr<GHashTable> mServiceProxies;
        GAutoPtr<GHashTable> mTechnologyProxies;
        GAutoPtr<GHashTable> mProperties;
        GAutoPtr<GHashTable> mServices;
        GAutoPtr<GHashTable> mTechnologies;
    };

    struct GDBusConnManAgent
    {
        using ConnectCallback = NetworkCommissioning::Internal::WirelessDriver::ConnectCallback *;

        GAutoPtr<GDBusConnection> mConnection;
        GAutoPtr<ConnManAgent> mSkeleton;
        bool mExported   = false;
        bool mRegistered = false;
        GAutoPtr<ConnManService> mPendingService;
        ConnectCallback mConnectCallback = nullptr;
    };

    // Access to mConnManClient and mConnManAgentServer have to be
    // protected by a mutex because they are accessed from both the
    // Matter event loop thread (down calls) and dedicated GLib D-Bus thread
    // started by Platform Manager (up calls).

    std::mutex mConnManMutex;
    GDBusConnManClient mConnManClient CHIP_GUARDED_BY(mConnManMutex);
    GDBusConnManAgent mConnManAgentServer CHIP_GUARDED_BY(mConnManMutex);
    ConnectivityManagerImpl * mConnectivityManagerImpl;
    char mEthernetIfName[Inet::InterfaceId::kMaxIfNameLength];
    Internal::WiFiSSIDFixedBuffer mWiFiActiveScanSsid;
    size_t mWiFiActiveScanCount;
    Internal::WiFiKeyFixedBuffer mWiFiConnectPassphrase;
    Internal::WiFiSSIDFixedBuffer mWiFiConnectSsid;
    size_t mWiFiConnectScanCount;
    char mWiFiIfName[Inet::InterfaceId::kMaxIfNameLength];
    bool mWiFiStationConnected;
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    System::Clock::Timeout mWiFiStationReconnectInterval;
};

} // namespace DeviceLayer
} // namespace chip
