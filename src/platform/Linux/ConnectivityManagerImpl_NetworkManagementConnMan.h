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

#include <array>
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
#include <platform/NetworkCommissioning.h>
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

    ConnectivityManagerImpl_NetworkManagementConnMan() = default;

    // Destruction

    virtual ~ConnectivityManagerImpl_NetworkManagementConnMan() = default;

    // Initialization

    CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) override final;

    // Observation

    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) override final;

    // Event Handling

    void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) override final;

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Ethernet Control Plane Management

    const char * GetEthernetIfName() override final;
    void UpdateEthernetNetworkingStatus() override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Observation

    const char * GetWiFiIfName() override final;

    // Control

    void StartNonConcurrentWiFiManagement() override final;
    void StartWiFiManagement() override final;

    // Wi-Fi Station Control Plane Management

    // Introspection

    bool IsWiFiManagementStarted() override final;
    bool IsWiFiStationApplicationControlled() override final;
    bool IsWiFiStationConnected() override final;
    bool IsWiFiStationEnabled() override final;
    bool IsWiFiStationProvisioned() override final;

    // Observation

    CHIP_ERROR GetWiFiBssId(MutableByteSpan & outBssId) override final;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType) override final;
    ConnectivityManager::WiFiStationMode GetWiFiStationMode() override final;
    System::Clock::Timeout GetWiFiStationReconnectInterval() override final;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion) override final;

    // Mutation

    CHIP_ERROR SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode) override final;
    CHIP_ERROR SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval) override final;

    // Worker

    void ClearWiFiStationProvision() override final;
    CHIP_ERROR CommitConfig() override final;
    ;
    CHIP_ERROR
    ConnectWiFiNetworkAsync(ByteSpan inSsid, ByteSpan inCredentials,
                            NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR ConnectWiFiNetworkWithPDCAsync(
        ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity, const Crypto::P256Keypair & inClientIdentityKeypair,
        NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR StartWiFiScan(ByteSpan inSsid, NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback) override final;

    // Wi-Fi Soft Access Point (AP) Control Plane Management

    // Observation

    ConnectivityManager::WiFiAPMode GetWiFiApMode() override final;

    // Mutation

    CHIP_ERROR SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode) override final;
    void SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout) override final;

    // Control

    void DemandStartWiFiAp() override final;
    void StopOnDemandWiFiAp() override final;
    void MaintainOnDemandWiFiAp() override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

private:
    // Network Service Internet-Connectivity Tracking
    //
    // Producer for DeviceEventType::kInternetConnectivityChange,
    // derived from the cached connman network service "IPv4" / "IPv6"
    // properties and debounced against connman property storms.

    /**
     *  Per-IP-address-family Internet connectivity disposition for a
     *  tracked connman network service type.
     */
    enum class NetworkServiceConnectivity : uint8_t
    {
        /**
         *  The connectivity disposition has not yet been observed;
         *  this is the initial state and, uniquely, transitions out
         *  of it to #kLost are absorbed without generating a device
         *  event.
         */
        kUnknown = 0,

        /**
         *  The address family has no completed configuration (no
         *  "Address" member in the service "IPv4"/"IPv6" property
         *  dictionary) on any service of the tracked type.
         */
        kLost = 1,

        /**
         *  The address family has a completed configuration with a
         *  parseable address on the best service of the tracked
         *  type.
         */
        kEstablished = 2,
    };

    /**
     *  @brief
     *    Per-IP-family reduction of tracked connectivity across every
     *    eligible network service type.
     *
     *  One instance accumulates, for a single IP family, the fold
     *  over all `NetworkServiceState` entries performed when the
     *  debounce timer expires: whether any tracked service's family
     *  transition is worth surfacing (`mReportable`), whether the
     *  family is currently established on any tracked service (
     *  `mAnyEstablished`), and the representative address carried by
     *  the resulting event (`mAddress`). Two of these (one for IPv4
     *  and one for IPv6) drive a single
     *  `OnInternetConnectivityChange` notification, whose data model
     *  expresses only a per-family `ConnectivityChange` verdict and
     *  one address per family.
     *
     *  @sa AccumulateFamilyConnectivity
     *  @sa SetConnectivityChangeFromAggregate
     *
     *  @private
     *
     */
    struct FamilyConnectivityAggregate
    {
        FamilyConnectivityAggregate() noexcept;

        /**
         *  True once at least one tracked service type exhibits a
         *  family connectivity change (per
         *  `IsFamilyConnectivityChangeReportable`) that the event
         *  model can express; the sole gate on whether a
         *  `ConnectivityChange` other than `kConnectivity_NoChange`
         *  is emitted for this family.
         */
        bool mReportable;

        /**
         *  True if this IP family is established on any tracked
         *  service type. Selects `kConnectivity_Established` versus
         *  `kConnectivity_Lost` when `mReportable` holds, and gates
         *  capture of `mAddress` to the first-established entry in
         *  table order.
         */
        bool mAnyEstablished;

        /**
         *  Representative address for this family, taken from the
         *  first-established tracked service in table (declaration)
         *  order. Cosmetic: the event data model carries a single
         *  address per family even though several services may be
         *  established at once. Remains `Inet::IPAddress::Any` until
         *  a first established entry supplies it.
         */
        Inet::IPAddress mAddress;
    };

    /**
     *  @brief
     *    Snapshot of one IP family's connectivity on one network service
     *    type.
     *
     *  The per-family unit of the tracked-state table: the
     *  reachability verdict for a single family (IPv4 or IPv6) on a
     *  single service, paired with the address observed for that
     *  family. A `NetworkServiceConnectivityState` holds one of these
     *  per family, and `AccumulateFamilyConnectivity` folds a
     *  (pending, reported) pair of them into a
     *  `FamilyConnectivityAggregate`.
     *
     *  @private
     *
     */
    struct NetworkServiceFamilyConnectivityState
    {
        NetworkServiceFamilyConnectivityState() noexcept;

        /**
         *  Reachability verdict for this family on this service:
         *  established or lost.
         */
        NetworkServiceConnectivity mConnectivity;

        /**
         *  Address observed for this family on this service;
         *  meaningful when `mConnectivity` is `kEstablished`, and the
         *  source of the aggregate's representative address.
         */
        Inet::IPAddress mAddress;

        bool operator==(const NetworkServiceFamilyConnectivityState & inOther) const noexcept;
        bool operator!=(const NetworkServiceFamilyConnectivityState & inOther) const noexcept;
    };

    /**
     *  Per-IP-address-family Internet connectivity state, with the
     *  established address retained so that address *changes* while
     *  established are themselves detectable and reportable (a
     *  consumer such as the DNS-SD advertiser derives one-shot state
     *  from the address and must be re-driven when it changes).
     */
    struct NetworkServiceConnectivityState
    {
        NetworkServiceFamilyConnectivityState mIPv4;
        NetworkServiceFamilyConnectivityState mIPv6;

        bool operator==(const NetworkServiceConnectivityState & inOther) const noexcept;
        bool operator!=(const NetworkServiceConnectivityState & inOther) const noexcept;
    };

    /**
     *  Aggregate tracked state for one eligible connman network
     *  service type. Membership in the tracked-state table *is* the
     *  eligibility policy: only service types on which Matter can
     *  meaningfully advertise and operate (that is, link-local
     *  mDNS-reachable interfaces--Ethernet and Wi-Fi) have entries.
     *  Cellular and VPN services are deliberately excluded, since
     *  their addresses are not valid DNS-SD advertisement targets;
     *  do not conflate this eligibility set with connman's
     *  connection *preference* policy, which lives in connman.conf.
     */
    struct NetworkServiceState
    {
        NetworkServiceState() noexcept;

        /**
         *  connman service "Type" value (for example, "ethernet")
         */
        const char * mType;

        /**
         *  Human-readable description of the service (for example,
         *  "Ethernet").
         */
        const char * mDescription;

        /**
         *  The network interface name underlying the service.
         */
        char mIfName[Inet::InterfaceId::kMaxIfNameLength];

        /**
         *  Most recently computed per-family connectivity for this
         *  service, recomputed from the cached best-service properties on
         *  each connman event. Compared against `mReportedConnectivity`
         *  to decide whether to (re)arm the debounce timer, and adopted as
         *  the new baseline when the timer expires.
         */
        NetworkServiceConnectivityState mPendingConnectivity;

        /**
         *  Per-family connectivity last reflected into a posted
         *  `OnInternetConnectivityChange` event: the debounce baseline.
         *  A pending state that settles back to this value across a storm
         *  expires the timer with nothing to report.
         */
        NetworkServiceConnectivityState mReportedConnectivity;
    };

    /**
     *  Tracks one in-flight, SSID-oriented Wi-Fi scan operation with
     *  its retry budget.
     *
     *  A nonzero `mCount` is the sole liveness sentinel (a scan is
     *  outstanding); `mSsid`'s emptiness is orthogonally the
     *  directed-versus-broadcast selector, meaningful only while
     *  active.
     */
    struct WiFiScanState
    {
        WiFiScanState() noexcept;

        // Introspection

        bool IsActive() const noexcept;
        bool IsDirected() const noexcept;

        // Mutation

        void Reset() noexcept;

        /**
         *  The fixed buffer for the Service Set Identifier (SSID) of
         *  the Wi-Fi remote access point to scan for when the scan is
         *  directed. Otherwise, empty when the scan is non-directed
         *  (that is, broadcast).
         *
         *  Here, the empty state of the member is load-bearing: it
         *  reflects the optional and nullable state of the upstream,
         *  over-the-wire scan networks command SSID parameter, where
         *  both absence and null translate to an empty buffer.
         */
        Internal::WiFiSSIDFixedBuffer mSsid;

        /**
         *  The number of scans performed.
         */
        size_t mCount;
    };

    enum class WiFiScanTerminalKind : uint8_t
    {
        /**
         *  A Wi-Fi scan was for a connect and is terminating
         *  with `OnConnectResult`.
         */
        kConnect,

        /**
         *  A Wi-Fi scan was for an actual scan and is terminating
         *  with `OnScanFinished`.
         */
        kScan
    };

    // Initialization

    static CHIP_ERROR InitAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf, GDBusConnection * inConnection,
                                      const char * inPath, ConnManAgent *& outSkeleton, GError ** outError) noexcept;
    void ShutdownAgentLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath) noexcept;
    static void ShutdownAgentOnGLib(ConnectivityManagerImpl_NetworkManagementConnMan * inSelf, GDBusConnection * inConnection,
                                    const char * inPath, ConnManAgent * inSkeleton) noexcept;

    // Introspection

    static bool IsFamilyConnectivityChangeReportable(const NetworkServiceFamilyConnectivityState & inPending,
                                                     const NetworkServiceFamilyConnectivityState & inReported) noexcept;

    // Observation

    static const char * GetConnectivityChangeString(ConnectivityChange inChange) noexcept;
    static const char * GetNetworkServiceConnectivityString(NetworkServiceConnectivity inNetworkServiceConnectivity) noexcept;

    // Mutation

    static bool SetConnectivityChangeFromAggregate(const FamilyConnectivityAggregate & inAggregate,
                                                   ConnectivityChange & outChange) noexcept;

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
        std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType,
        const char * inKey, GVariant * inMaybeVariant) noexcept;
    using TypedObjectPropertyChangedLockedMethod = CHIP_ERROR (ConnectivityManagerImpl_NetworkManagementConnMan::*)(
        std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath, const char * inType,
        const char * inKey, GVariant * inValue) noexcept;

    void AccumulateFamilyConnectivity(const NetworkServiceFamilyConnectivityState & inPending,
                                      const NetworkServiceFamilyConnectivityState & inReported,
                                      FamilyConnectivityAggregate & inOutAggregate) noexcept;
    static CHIP_ERROR
    BuildWiFiScanResponseFromServicePropertiesLocked(GVariant * inProperties,
                                                     DeviceLayer::NetworkCommissioning::WiFiScanResponse & outResponse) noexcept;
    CHIP_ERROR DispatchWiFiConnectFinishedLocked(std::unique_lock<std::mutex> & inOutLock,
                                                 DeviceLayer::NetworkCommissioning::Status inStatus, const CharSpan & inDebugText,
                                                 int32_t inReason) noexcept;
    CHIP_ERROR DispatchWiFiScanFinishedLocked(
        std::unique_lock<std::mutex> & inOutLock, DeviceLayer::NetworkCommissioning::Status inStatus, const CharSpan & inDebugText,
        std::unique_ptr<std::vector<DeviceLayer::NetworkCommissioning::WiFiScanResponse>> inResponses) noexcept;
    NetworkServiceState * FindNetworkServiceStateLocked(const char * inType) noexcept;
    const char * GetInterfaceName(const char * inType) noexcept;
    static NetworkServiceFamilyConnectivityState GetServiceFamilyConnectivityStateLocked(GVariant * inProperties,
                                                                                         const char * inFamilyKey) noexcept;
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
        std::unique_lock<std::mutex> & inOutLock, const char * inDescription, GDBusProxy * inProxy, const char * inPath,
        const char * inType, GVariant * inProperties,
        TypedObjectPropertiesChangedAnyLockedMethod inTypedObjectPropertiesChangedAnyLockedMethod) noexcept;
    CHIP_ERROR
    HandleObjectPropertyChangedAnyLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy, const char * inPath,
                                         const char * inType, const char * inKey, GVariant * inMaybeVariant,
                                         TypedObjectPropertyChangedLockedMethod inTypedObjectPropertyChangedLockedMethod) noexcept;
    void HandleServiceConnectComplete(GDBusProxy * inService, const GError * inError) noexcept;
    CHIP_ERROR HandleServiceConnectRequestLocked(std::unique_lock<std::mutex> & inOutLock, ConnManService * inService) noexcept;
    CHIP_ERROR HandleServicePropertiesChangedLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy,
                                                    const char * inPath, const char * inType, GVariant * inProperties) noexcept;
    void HandleServicePropertyChanged(ConnManService * inService, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleServicePropertyChangedLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy,
                                                  const char * inPath, const char * inType, const char * inKey,
                                                  GVariant * inValue) noexcept;
    CHIP_ERROR HandleServicePropertyChangedAnyLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inService,
                                                     const char * inPath, const char * inType, const char * inKey,
                                                     GVariant * inMaybeVariant) noexcept;
    CHIP_ERROR HandleTechnologyPropertiesChangedLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy,
                                                       const char * inPath, const char * inType, GVariant * inProperties) noexcept;
    void HandleTechnologyPropertyChanged(ConnManTechnology * inTechnology, const char * inKey, GVariant * inValue) noexcept;
    CHIP_ERROR HandleTechnologyPropertyChangedLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inProxy,
                                                     const char * inPath, const char * inType, const char * inKey,
                                                     GVariant * inValue) noexcept;
    CHIP_ERROR HandleTechnologyPropertyChangedAnyLocked(std::unique_lock<std::mutex> & inOutLock, GDBusProxy * inTechnology,
                                                        const char * inPath, const char * inType, const char * inKey,
                                                        GVariant * inMaybeVariant) noexcept;
    void HandleTechnologyScanComplete(GDBusProxy * inTechnology, const GError * inError) noexcept;
    CHIP_ERROR HandleWiFiBroadcastScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock,
                                                     ConnManTechnology * inTechnology) noexcept;
    CHIP_ERROR HandleWiFiPendingConnectLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                              WiFiScanState & inOutScanState) noexcept;
    CHIP_ERROR HandleWiFiPendingScanLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                           WiFiScanState & inOutScanState) noexcept;
    CHIP_ERROR HandleWiFiScanCompleteLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology) noexcept;
    CHIP_ERROR HandleWiFiScanRetryLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                         const char * inReason, WiFiScanState & inOutScanState) noexcept;
    CHIP_ERROR HandleWiFiUnresolvedAfterScanLocked(std::unique_lock<std::mutex> & inOutLock, ConnManTechnology * inTechnology,
                                                   WiFiScanState & inOutScanState, const size_t & inScanLimit,
                                                   const char * inReason, WiFiScanTerminalKind inKind) noexcept;
    CHIP_ERROR ManagerAgentOpLocked(std::unique_lock<std::mutex> & inOutLock, const char * inPath,
                                    ManagerAgentOpFunc inManagerAgentOpFunc, const char * inAction,
                                    const bool & inRegister) noexcept;
    void MaybeClearInterfaceNameLocked(const char * inPath) noexcept;
    CHIP_ERROR MaybeSetInterfaceNameLocked(const char * inType, const char * inInterface) noexcept;
    CHIP_ERROR RemoveServiceLocked(const char * inPath) noexcept;
    CHIP_ERROR RemoveTechnologyLocked(const char * inPath) noexcept;
    CHIP_ERROR StartNetworkManagementOnGLib() noexcept;
    CHIP_ERROR UpdateManagerPropertiesLocked(GVariant * inProperties) noexcept;
    CHIP_ERROR UpdateNetworkServiceConnectivityLocked() noexcept;
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

    // System Layer Timer Completion Methods

    static void HandleNetworkServiceConnectivityDebounce(System::Layer * inSystemLayer, void * inAppState);
    void ReportNetworkServiceConnectivity(void) noexcept;

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
    /**
     *  The number of tracked network service types, sized by build
     *  configuration.
     */
    static constexpr size_t kNetworkServiceEthernetCount = CHIP_DEVICE_CONFIG_ENABLE_ETHERNET;
    static constexpr size_t kNetworkServiceWiFiCount     = CHIP_DEVICE_CONFIG_ENABLE_WIFI;
    static constexpr size_t kNetworkServiceStateCount    = (kNetworkServiceEthernetCount + kNetworkServiceWiFiCount);

    struct GDBusConnManClient
    {
        // clang-format off
        GAutoPtr<ConnManManager>  mManagerProxy;
        GAutoPtr<GHashTable>      mServiceProxies;
        GAutoPtr<GHashTable>      mTechnologyProxies;
        GAutoPtr<GHashTable>      mProperties;
        GAutoPtr<GHashTable>      mServices;
        GAutoPtr<GHashTable>      mTechnologies;
        // clang-format on
    };

    struct GDBusConnManAgent
    {
        using ConnectCallback = NetworkCommissioning::Internal::WirelessDriver::ConnectCallback *;

        // clang-format off
        GAutoPtr<GDBusConnection> mConnection;
        GAutoPtr<ConnManAgent>    mSkeleton;
        bool                      mExported = false;
        bool                      mRegistered = false;
        GAutoPtr<ConnManService>  mPendingService;
        ConnectCallback           mConnectCallback = nullptr;
        // clang-format on
    };

    using NetworkServiceStateStorage = std::array<NetworkServiceState, kNetworkServiceStateCount>;

    // Access to mConnManClient and mConnManAgentServer have to be
    // protected by a mutex because they are accessed from both the
    // Matter event loop thread (down calls) and dedicated GLib D-Bus thread
    // started by Platform Manager (up calls).

    // clang-format off
    std::mutex                           mConnManMutex;
    GDBusConnManClient                   mConnManClient CHIP_GUARDED_BY(mConnManMutex);
    GDBusConnManAgent                    mConnManAgentServer CHIP_GUARDED_BY(mConnManMutex);
    ConnectivityManagerImpl *            mConnectivityManagerImpl;
    NetworkServiceStateStorage           mNetworkServiceStates CHIP_GUARDED_BY(mConnManMutex);
    WiFiScanState                        mWiFiActiveScanState CHIP_GUARDED_BY(mConnManMutex);
    Internal::WiFiKeyFixedBuffer         mWiFiConnectPassphrase;
    WiFiScanState                        mWiFiConnectScanState CHIP_GUARDED_BY(mConnManMutex);
    bool                                 mWiFiStationConnected;
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    System::Clock::Timeout               mWiFiStationReconnectInterval;
    // clang-format on
};

} // namespace DeviceLayer
} // namespace chip
