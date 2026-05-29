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

#include <glib.h>

#include <inet/InetInterface.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/wpa/DBusWpa.h>
#include <platform/Linux/dbus/wpa/DBusWpaBss.h>
#include <platform/Linux/dbus/wpa/DBusWpaInterface.h>
#include <platform/Linux/dbus/wpa/DBusWpaNetwork.h>
#include <platform/NetworkCommissioning.h>
#include <system/SystemMutex.h>

#define WPA_SUPPLICANT_CLIENT_LOG_PREFIX "wpa_supplicant: "

namespace chip {

template <>
struct GAutoPtrDeleter<WpaSupplicant1>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1BSS>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1Interface>
{
    using deleter = GObjectDeleter;
};

template <>
struct GAutoPtrDeleter<WpaSupplicant1Network>
{
    using deleter = GObjectDeleter;
};

namespace DeviceLayer {

// Forward Declarations

class ConnectivityManagerImpl;

namespace Internal {

/**
 *  @brief
 *    A D-Bus client for the wpa_supplicant Wi-Fi control plane.
 *
 *  Encapsulates the GLib/D-Bus proxy objects and shared state
 *  needed to interact with the wpa_supplicant daemon over the
 *  D-Bus system bus.
 *
 *  This class is intended as a base for concrete Wi-Fi feature
 *  implementations (for example, network management or Wi-Fi USD NAN
 *  / PAF) that share a common wpa_supplicant back end.
 *
 *  Subclasses inherit the D-Bus proxy state (#mWpaSupplicant) and its
 *  associated mutex (#mWpaSupplicantMutex), and must call @c Init
 *  before using any wpa_supplicant interactions.
 *
 *  The expected lifecycle is:
 *
 *    1. Construct (default, via subclass).
 *    2. Initinitalization (via #Init).
 *       a. Binds to a ConnectivityManagerImpl for event dispatch.
 *    3. Use.
 *    4. Shutdown (via #Shutdown)
 *       a. Tears down proxy state and releases the
 *          ConnectivityManagerImpl back-reference.
 *
 */
class WpaSupplicantClient
{
public:
    virtual ~WpaSupplicantClient() noexcept = default;

protected:
    WpaSupplicantClient() noexcept = default;

    /**
     *  @brief
     *    Bind to a Connectivity Manager and prepare for D-Bus
     *    interactions with wpa_supplicant.
     *
     *  Associates this client with the given platform Connectivity
     *  Manager implementation, which serves as the target for
     *  asynchronous event and status callbacks originating from
     *  wpa_supplicant D-Bus signals (for example, scan results,
     *  connection state changes).
     *
     *  @param[in]  inConnectivityManagerImpl
     *    A reference to the platform Connectivity Manager
     *    implementation to which wpa_supplicant events will be
     *    dispatched.
     *
     *  @sa Shutdown
     *  @sa Reset
     *
     */
    CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) noexcept;

    /**
     *  @brief
     *    Tear down D-Bus proxy state and release the Connectivity
     *    Manager back-reference.
     *
     *  Resets all wpa_supplicant D-Bus proxy objects via Reset and
     *  then clears the ConnectivityManagerImpl association
     *  established by #Init, returning this client to an uninitialized
     *  state suitable for re-initialization or destruction.
     *
     *  @sa Init
     *  @sa Reset
     *
     */
    void Shutdown() noexcept;

    /**
     *  @brief
     *    Reset the D-Bus proxy objects to their default state.
     *
     *  Releases all GLib D-Bus proxy resources held in
     *  #mWpaSupplicant, including the wpa_supplicant daemon proxy,
     *  interface proxy, and any cached interface or network object
     *  paths. The ConnectivityManagerImpl back-reference is @b not
     *  cleared; use Shutdown for a full teardown.
     *
     *  @sa Init
     *  @sa Shutdown
     *
     */
    void Reset() noexcept;

    /**
     *  @brief
     *    Return whether the wpa_supplicant client has been started.
     *
     */
    bool IsStarted() const noexcept;

    /**
     *  @brief
     *    Return whether the Wi-Fi network interface is enabled.
     *
     *  @return
     *    @c True if the Wi-Fi network interface is enabled;
     *    otherwise, @c false.
     *
     */
    bool IsWiFiInterfaceEnabled() const noexcept CHIP_REQUIRES(mWpaSupplicantMutex);

    /**
     *  @brief
     *    Get the Wi-Fi station service set identifier (SSID) and
     *    connected state.
     *
     *  This attempts to get, if any, the currently connected (that
     *  is, associated) Wi-Fi station service set identifier (SSID)
     *  and connected state.
     *
     *  @param[out]  outNetwork
     *    A reference to the mutable network commissioning state to
     *    which to copy the currently connected Wi-Fi station SSID and
     *    connected state.
     *
     *  @sa IsWiFiInterfaceEnabled
     *
     */
    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) noexcept;

    /**
     *  @brief
     *    Get the Wi-Fi network interface name.
     *
     *  @param[out]  outIfName
     *    A reference to the mutable character span to which to assign
     *    the Wi-Fi network interface name extent.
     *
     *  @sa SetIfName
     *
     */
    CHIP_ERROR GetIfName(CharSpan & outIfName) const noexcept;

    /**
     *  @brief
     *    Set the Wi-Fi network interface name.
     *
     *  @param[in]  inIfName
     *    A reference to the immutable character span from which to
     *    copy the Wi-Fi network interface name.
     *
     *  @retval  CHIP_NO_ERROR
     *    If successful.
     *
     *  @retval  CHIP_ERROR_BUFFER_TOO_SMALL
     *    If the length of @a inIfName exceeds the internal buffer
     *    space.
     *
     *  @sa GetIfName
     *
     */
    CHIP_ERROR SetIfName(const CharSpan & inIfName) noexcept;

    struct GDBusWpaSupplicant
    {
        GAutoPtr<WpaSupplicant1> proxy;
        GAutoPtr<WpaSupplicant1Interface> iface;
        GAutoPtr<char> interfacePath;
        GAutoPtr<char> networkPath;

        // Must be called synchronously on the GLib thread while the
        // GLib main loop is still running.

        void Reset();
    };

    GDBusWpaSupplicant mWpaSupplicant CHIP_GUARDED_BY(mWpaSupplicantMutex);

    // Access to mWpaSupplicant has to be protected by a mutex because
    // it is accessed from the CHIP event loop thread and dedicated
    // GLib D-Bus thread started by platform manager.

    mutable std::mutex mWpaSupplicantMutex;

private:
    ConnectivityManagerImpl * mConnectivityManagerImpl = nullptr;
    char mWiFiIfName[Inet::InterfaceId::kMaxIfNameLength];
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
