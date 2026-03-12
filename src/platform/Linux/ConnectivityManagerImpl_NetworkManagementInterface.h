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

#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/ConnectivityManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  An abstract interface for controlling and interacting with network
 *  management infrastructure on a Linux platform.
 *
 *  Concrete implementations are expected to provide the platform- and
 *  driver-specific integration required to manage one or more
 *  IP-bearing network interfaces, especially Wi-Fi but also
 *  Ethernet. Thread, however, is excluded as it is managed through an
 *  independent, parallel stack interface.
 *
 *  Implementations may include full-featured network management
 *  software such as GNOME Network Manager or Connection Manager (also
 *  known as connman) or may be Wi-Fi-specific backends such as iwd or
 *  wpa_supplicant with adjunct handling for Ethernet, if any.
 *
 */
class NetworkManagementInterface
{
public:
    // Destruction

    virtual ~NetworkManagementInterface(void) = default;

    // Initialization

    /**
     *  @brief
     *    Perform explicit class initialization.
     *
     *  This initializer is invoked during stack initialization and
     *  should perform any actions necessary to support subsequent
     *  observation, mutation, event handling, and worker operations
     *  on the managed network interfaces.
     *
     *  @param[in]  inConnectivityManagerImpl
     *    A reference to the mutable platform connectivity manager
     *    implementation for which the concrete interface
     *    implementation is being initialized.
     *
     */
    virtual CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) = 0;

    // Observation

    /**
     *  @brief
     *    Get the currently configured (provisioned) operational network.
     *
     *  This returns the currently configured network used for operational
     *  connectivity (for example, the Wi-Fi network to which the device
     *  will attempt to connect when Wi-Fi station mode is enabled).
     *
     *  @param[out]  outNetwork
     *    A reference to the mutable network structure to populate on
     *    success.
     *
     */
    virtual CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) = 0;

    // Event Handling

    /**
     *  @brief
     *    Handle a platform event.
     *
     *  This provides an opportunity for the concrete interface
     *  implementation to handle a platform event.
     *
     *  @param[in]  inDeviceEvent
     *    A reference to the immutable platform event to handle.
     *
     */
    virtual void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) = 0;

    /**
     *  @brief
     *    Commit any network configuration to non-volatile storage
     *    such that the configuration may be restored and connectivity
     *    resumed across system restarts.
     *
     *  @sa ClearWiFiStationProvision
     *
     */
    virtual CHIP_ERROR CommitConfig(void) = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Ethernet Control Plane Management

    // Observation

    /**
     *  @brief
     *    Return the Ethernet network interface name.
     *
     *  This returns, if available, the Ethernet station network
     *  interface name for the implicit "main" or default Ethernet
     *  network interface as a null-terminated C string.
     *
     *  The returned pointer follows the "Get" / "Create" or "Copy"
     *  rule and is owned by the interface instance returning it. The
     *  pointer is guaranteed to be valid for the lifetime of the
     *  interface instance.
     *
     *  @retval
     *    If present and known, the Ethernet network interface name
     *    as an immutable, null-terminated C string; otherwise, null.
     *
     */
    virtual const char * GetEthernetIfName(void) = 0;

    // Worker

    /**
     *  @brief
     *    Update cached Ethernet networking status.
     *
     *  Implementations should refresh any cached Ethernet link and IP
     *  state used by the Connectivity Manager (for example, to drive
     *  DeviceLayer events or diagnostic attributes).
     *
     *  This method is typically invoked from a platform work context
     *  and should not block for extended periods.
     *
     */
    virtual void UpdateEthernetNetworkingStatus(void) = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Observation

    /**
     *  @brief
     *    Return the Wi-Fi station network interface name.
     *
     *  This returns, if available, the Wi-Fi station network
     *  interface name for the implicit "main" or default Wi-Fi
     *  network interface as a null-terminated C string.
     *
     *  The returned pointer follows the "Get" / "Create" or "Copy"
     *  rule and is owned by the interface instance returning it. The
     *  pointer is guaranteed to be valid for the lifetime of the
     *  interface instance.
     *
     *  @returns
     *    If present and known, the Wi-Fi station network interface
     *    name as an immutable, null-terminated C string; otherwise,
     *    null.
     *
     */
    virtual const char * GetWiFiIfName(void) = 0;

    // Control

    /**
     *  @brief
     *    Start Wi-Fi management in a non-concurrent mode.
     *
     *  Start the Wi-Fi management infrastructure in a mode that does
     *  @b not permit concurrent Wi-Fi station and Bluetooth Low
     *  Energy (BLE) functions, where the latter is being used as the
     *  commissioning channel (if supported by the platform or
     *  implementation), biasing towards Wi-Fi station functionality
     *  only. Concrete implementations may interpret this as
     *  "station-only" or as a reduced-feature startup used during
     *  commissioning or early boot.
     *
     *  This may be invoked by the BLE engine subsystem when Wi-Fi is
     *  enabled as an operational network and the
     *  #CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
     *  configuration option is deasserted.
     *
     *  @sa IsWiFiManagementStarted
     *  @sa StartWiFiManagement
     *
     */
    virtual void StartNonConcurrentWiFiManagement(void) = 0;

    /**
     *  @brief
     *    Start Wi-Fi management.
     *
     *  Start the Wi-Fi management infrastructure for the device.
     *  This may include attaching to, configuring, or starting an
     *  external network manager and establishing any required event
     *  subscriptions.
     *
     *  @sa IsWiFiManagementStarted
     *  @sa StartNonConcurrentWiFiManagement
     *
     */
    virtual void StartWiFiManagement(void) = 0;

    // Wi-Fi Station Control Plane Management

    // Introspection

    /**
     *  @brief
     *    Return whether the network management infrastructure for the
     *    Wi-Fi network interface is started running.
     *
     *  @returns
     *    True if the network management infrastructure for the Wi-Fi
     *    network interface is started and running; otherwise, false.
     *
     *  @sa StartNonConcurrentWiFiManagement
     *  @sa StartWiFiManagement
     *
     */
    virtual bool IsWiFiManagementStarted(void) = 0;

    /**
     *  @brief
     *    Return whether the Wi-Fi station network interface is
     *    application-controlled.
     *
     *  This returns whether the Wi-Fi station network interface is
     *  managed by the application rather than by the platform network
     *  management infrastructure.
     *
     *  When true, the Connectivity Manager should avoid taking
     *  autonomous actions that would conflict with application
     *  control (for example, automatically enabling/disabling station
     *  mode or initiating reconnect behavior).
     *
     *  @returns
     *    True if the Wi-Fi station network interface is
     *    application-controlled; otherwise, false.
     *
     *  @sa GetWiFiStationMode
     *  @sa IsWiFiStationConnected
     *  @sa IsWiFiStationEnabled
     *  @sa IsWiFiStationProvisioned
     *  @sa SetWiFiStationMode
     *
     */
    virtual bool IsWiFiStationApplicationControlled(void) = 0;

    /**
     *  @brief
     *    Return whether the Wi-Fi station network interface is
     *    connected.
     *
     *  This returns whether the Wi-Fi station network interface is
     *  connected, that is, whether it as been assigned a valid IPv4
     *  and/or IPv6 address and is locally- and/or Internet-reachable.
     *
     *  @returns
     *    True if the Wi-Fi station network interface is connected;
     *    otherwise, false.
     *
     *  @sa IsWiFiStationApplicationControlled
     *  @sa IsWiFiStationEnabled
     *  @sa IsWiFiStationProvisioned
     *
     */
    virtual bool IsWiFiStationConnected(void) = 0;

    /**
     *  @brief
     *    Return whether the Wi-Fi station network interface is
     *    enabled.
     *
     *  @returns
     *    True if the Wi-Fi station network interface is enabled;
     *    otherwise, false.
     *
     *  @sa GetWiFiStationMode
     *  @sa IsWiFiStationApplicationControlled
     *  @sa IsWiFiStationConnected
     *  @sa IsWiFiStationProvisioned
     *  @sa SetWiFiStationMode
     *
     */
    virtual bool IsWiFiStationEnabled(void) = 0;

    /**
     *  @brief
     *    Return whether the Wi-Fi station network interface is
     *    provisioned.
     *
     *  Returns whether the Wi-Fi station network interface is
     *  provisioned to connect to a particular Wi-Fi network when that
     *  network is encountered through an active, directed or passive,
     *  broadcast Wi-Fi network scan.
     *
     *  @returns
     *    True if the Wi-Fi station network interface is provisioned;
     *    otherwise, false.
     *
     *  @sa ClearWiFiStationProvision
     *  @sa CommitConfig
     *  @sa IsWiFiStationApplicationControlled
     *  @sa IsWiFiStationConnected
     *  @sa IsWiFiStationEnabled
     *
     */
    virtual bool IsWiFiStationProvisioned(void) = 0;

    // Observation

    /**
     *  @brief
     *    Get the Wi-Fi access point Basic Service Set Identifier
     *    (BSSID) the Wi-Fi station network interface is currently
     *    associated with.
     *
     *  This specifically is the remote Wi-Fi access point Basic
     *  Service Set Identifier (BSSID), not the Wi-Fi access point
     *  Basic Service Set Identifier (BSSID) associated with any local
     *  Wi-Fi access point used for the purposes of "Soft Access Point
     *  (AP)" functionality.
     *
     *  @param[out]  outBssId
     *    A reference to the mutable byte span into which the Wi-Fi
     *    remote access point Basic Service Set Identifier (BSSID) the
     *    Wi-Fi station network interface is currently associated
     *    with, if any, is copied.
     *
     */
    virtual CHIP_ERROR GetWiFiBssId(MutableByteSpan & outBssId) = 0;

    /**
     *  @brief
     *    Get the Wi-Fi security type associated with the current link
     *    between the station and remote access point.
     *
     *  @param[out]  outSecurityType
     *    A reference to mutable storage for the Wi-Fi security type
     *    associated with the current link between the station and
     *    remote access point.
     *
     */
    virtual CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType) = 0;

    /**
     *  @brief
     *    Get the current operational mode of the Wi-Fi station
     *    network interface.
     *
     *  @returns
     *    The current operational mode of the Wi-Fi station network
     *    interface.
     *
     *  @sa IsWiFiStationApplicationControlled
     *  @sa IsWiFiStationEnabled
     *  @sa SetWiFiStationMode
     *
     */
    virtual ConnectivityManager::WiFiStationMode GetWiFiStationMode(void) = 0;

    /**
     *  @brief
     *    Get the Wi-Fi station reconnect interval.
     *
     *  Return the interval used by the Connectivity Manager and/or
     *  the concrete implementation to throttle automatic reconnect
     *  attempts when Wi-Fi station mode is enabled but the station is
     *  not connected.
     *
     *  A value of 0 may be used to indicate that reconnect attempts
     *  should occur without an additional software-imposed delay
     *  beyond any delay inherent to the underlying network manager.
     *
     *  @returns
     *    The reconnect interval as a System::Clock::Timeout.
     *
     *  @sa SetWiFiStationReconnectInterval
     *
     */
    virtual System::Clock::Timeout GetWiFiStationReconnectInterval(void) = 0;

    /**
     *  @brief
     *    Get the Wi-Fi specification or standard version associated
     *    with the current link between the station and remote access
     *    point.
     *
     *  @param[out]  outVersion
     *    A reference to mutable storage for the Wi-Fi specification
     *    or standard version associated with the current link between
     *    the station and remote access point.
     *
     */
    virtual CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion) = 0;

    // Mutation

    /**
     *  @brief
     *    Set the current operational mode of the Wi-Fi station
     *    network interface.
     *
     *  Request a change to the Wi-Fi station operational
     *  mode. Concrete implementations should apply the requested mode
     *  via the underlying network manager (or, if
     *  application-controlled, update their internal state
     *  accordingly).
     *
     *  @param[in]  inWiFiStationMode
     *    A reference to the immutable Wi-Fi station
     *    operational mode to set.
     *
     *  @sa GetWiFiStationMode
     *  @sa IsWiFiStationApplicationControlled
     *  @sa IsWiFiStationEnabled
     *
     */
    virtual CHIP_ERROR SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode) = 0;

    /**
     *  @brief
     *    Set the Wi-Fi station reconnect interval.
     *
     *  Set the interval used to throttle automatic reconnect attempts
     *  while the Wi-Fi station operational mode is enabled and the
     *  station is not connected.
     *
     *  Concrete implementations may apply this interval internally,
     *  may forward it to an underlying network manager if supported,
     *  or may treat it as a best-effort hint.
     *
     *  @param[in]  inInterval
     *    A reference to the immutable Wi-Fi station reconnect
     *    interval to set.
     *
     *  @sa GetWiFiStationReconnectInterval
     *
     */
    virtual CHIP_ERROR SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval) = 0;

    // Worker

    /**
     *  @brief
     *    Clear any provisioning or other non-volatile configuration
     *    information associated with the current Wi-Fi station
     *    network interface.
     *
     *  @sa CommitConfig
     *  @sa IsWiFiStationProvisioned
     *
     */
    virtual void ClearWiFiStationProvision(void) = 0;

    /**
     *  @brief
     *    Asynchronously connect the Wi-Fi station network interface
     *    to a Wi-Fi remote access point with preshared key (PSK)
     *    security.
     *
     *  This attempts to connect the Wi-Fi station network interface
     *  to the Wi-Fi remote access point with optional preshared key
     *  (PSK) security with the Service Set Identifier (SSID) @a
     *  inSsid using the optional PSK credential @a inCredentials.
     *
     *  @param[in]  inSsid
     *    A byte span for the Service Set Identifier (SSID) of the
     *    Wi-Fi remote access point to connect to. While @a inSsid is
     *    required and is always specified by the caller, the
     *    underlying Wi-Fi control plane may not yet be aware of this
     *    network. Implementations may need to perform a scan to
     *    discover the SSID and its associated BSSIDs before
     *    attempting association.
     *
     *  @param[in]  inCredentials
     *    An optional byte span for the preshared key (PSK)
     *    credentials associated with @a inSsid, if any.
     *
     *  @param[in]  inConnectCallback
     *    A pointer to the callback to invoke when the connection
     *    completes, either on failure or success.
     *
     *  @sa ConnectWiFiNetworkWithPDCAsync
     *  @sa IsWiFiStationConnected
     *
     */
    virtual CHIP_ERROR
    ConnectWiFiNetworkAsync(ByteSpan inSsid, ByteSpan inCredentials,
                            NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) = 0;

    /**
     *  @brief
     *    Asynchronously connect the Wi-Fi station network interface
     *    to a Wi-Fi remote access point with per-device credential
     *    (PDC) certificate-based security.
     *
     *  This attempts to connect the Wi-Fi station network interface
     *  to the Wi-Fi remote access point with per-device credential
     *  (PDC) certificate-based security.
     *
     *  The concrete implementation is expected to use the provided
     *  network identity and client identity material to perform a
     *  certificate-based authentication exchange appropriate for the
     *  underlying Wi-Fi security method (for example, EAP-TLS).
     *
     *  Unless otherwise specified by the concrete implementation,
     *  this operation is asynchronous and may complete after this
     *  method returns.
     *
     *  @param[in]  inSsid
     *    A byte span for the Service Set Identifier (SSID) of the
     *    Wi-Fi remote access point to connect to. While @a inSsid is
     *    required and is always specified by the caller, the
     *    underlying Wi-Fi control plane may not yet be aware of this
     *    network. Implementations may need to perform a scan to
     *    discover the SSID and its associated BSSIDs before
     *    attempting association.
     *
     *  @param[in]  inNetworkIdentity
     *    The network identity used to select the credential and/or
     *    trust context for the target network.  This is typically an
     *    identifier meaningful to the underlying control plane (for
     *    example, an EAP "realm", an SSID-scoped identity label, or
     *    other network-identifier string encoded as bytes).
     *
     *  @param[in]  inClientIdentity
     *    The client identity presented by the device during
     *    certificate-based authentication (for example, an EAP
     *    identity / username, often encoded as a UTF-8 string).
     *
     *  @param[in]  inClientIdentityKeypair
     *    A reference to the immutable client authentication keypair
     *    associated with the device's per-device credential. The
     *    concrete implementation should use the private key material
     *    for client authentication (and may use the public key to
     *    construct or validate associated certificate material).
     *
     *  @param[in]  inConnectCallback
     *    A pointer to the callback to invoke when the connection
     *    completes, either on failure or success.
     *
     *  @sa ConnectWiFiNetworkAsync
     *  @sa IsWiFiStationConnected
     *
     */
    virtual CHIP_ERROR
    ConnectWiFiNetworkWithPDCAsync(ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity,
                                   const Crypto::P256Keypair & inClientIdentityKeypair,
                                   NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) = 0;

    /**
     *  @brief
     *    Asynchronously scan for a Wi-Fi remote access point.
     *
     *  This attempts to scan for the Wi-Fi remote access point with
     *  the Service Set Identifier (SSID) @a inSsid.
     *
     *  @param[in]  inSsid
     *    The byte span for the Service Set Identifier (SSID) of the
     *    Wi-Fi remote access point to scan for.
     *
     *  @param[in]  inScanCallback
     *    A pointer to the callback to invoke when the scan completes,
     *    either on failure or success.
     *
     *  @sa ConnectWiFiNetworkAsync
     *
     */
    virtual CHIP_ERROR StartWiFiScan(ByteSpan inSsid, NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback) = 0;

    // Wi-Fi Soft Access Point (AP) Control Plane Management

    // Observation

    /**
     *  @brief
     *    Get the current operational mode of the local Wi-Fi access
     *    point network interface.
     *
     *  @returns
     *    The current operational mode of the local Wi-Fi access point
     *    network interface.
     *
     *  @sa SetWiFiApMode
     *
     */
    virtual ConnectivityManager::WiFiAPMode GetWiFiApMode(void) = 0;

    // Mutation

    /**
     *  @brief
     *    Set the current operational mode of the local Wi-Fi access
     *    point network interface.
     *
     *  Request a change to the Wi-Fi local soft access point (AP)
     *  operational mode. Concrete implementations should apply the
     *  requested mode via the underlying network manager and/or
     *  access point subsystem.
     *
     *  @param[in]  inWiFiApMode
     *    A reference to the immutable W-Fi local soft access point
     *    operational mode to set.
     *
     *  @sa GetWiFiApMode
     *
     */
    virtual CHIP_ERROR SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode) = 0;

    /**
     *  @brief
     *    Set the Wi-Fi local soft access point (AP) idle timeout used
     *    for on-demand operation.
     *
     *  Set the amount of time the the Wi-Fi local soft access point
     *  (AP) may remain idle (for example, with no associated
     *  stations) before the implementation may automatically stop it
     *  when operating in on-demand mode.
     *
     *  @param[in]  inTimeout
     *    A reference to the immutable idle timeout to set.
     *
     */
    virtual void SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout) = 0;

    // Control

    /**
     *  @brief
     *    Demand-start the Wi-Fi local soft access point (AP) for
     *    on-demand use.
     *
     *  Request that the Wi-Fi local soft access point (AP) be started
     *  (or kept started) for commissioning or other on-demand
     *  use. Implementations may start the the Wi-Fi local soft access
     *  point (AP) immediately or schedule startup according to
     *  platform constraints.
     *
     *  @sa MaintainOnDemandWiFiAp
     *  @sa StopOnDemandWiFiAp
     *
     */
    virtual void DemandStartWiFiAp(void) = 0;

    /**
     *  @brief
     *    Stop the on-demand Wi-Fi local soft access point (AP).
     *
     *  Request that the the on-demand Wi-Fi local soft access point
     *  (AP) be stopped if it was started for on-demand
     *  use. Implementations should stop the on-demand Wi-Fi local
     *  soft access point (AP) promptly, subject to platform
     *  constraints.
     *
     *  @sa DemandStartWiFiAp
     *  @sa MaintainOnDemandWiFiAp
     */
    virtual void StopOnDemandWiFiAp(void) = 0;

    /**
     *  @brief
     *    Maintain the on-demand Wi-Fi local soft access point (AP).
     *
     *  Inform the implementation that on-demand on-demand Wi-Fi local
     *  soft access point (AP) availability is still desired (for
     *  example, due to ongoing commissioning).  Implementations may
     *  use this as a keep-alive signal to reset or extend any idle
     *  timeout configured via #SetWiFiApIdleTimeout.
     *
     *  @sa DemandStartWiFiAp
     *  @sa StopOnDemandWiFiAp
     *
     */
    virtual void MaintainOnDemandWiFiAp(void) = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
