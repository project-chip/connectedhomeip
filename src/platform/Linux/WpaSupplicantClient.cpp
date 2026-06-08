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

#include "WpaSupplicantClient.h"

#include <mutex>

#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// MARK: Global Variables

static constexpr char kWpaSupplicantServiceName[] = "fi.w1.wpa_supplicant1";

} // namespace

void WpaSupplicantClient::GDBusWpaSupplicant::Reset()
{
    iface.reset();
    proxy.reset();
    interfacePath.reset();
    networkPath.reset();
}

CHIP_ERROR WpaSupplicantClient::Init(ConnectivityManagerImpl & inConnectivityManagerImpl) noexcept
{
    // Initially, and again at some future point, this should have an
    // assertion that mConnectivityManagerImpl is null and otherwise
    // return CHIP_ERROR_ALREADY_INITIALIZED to indicate to the caller
    // that 'Init' has been double-tripped without a bookending call
    // to Shutdown. However, the ConnectivityManager stack of which
    // this is a part has no peer 'Shutdown' method to go with its
    // 'Init' and subsequently, there is not yet a reasonable place to
    // hook the 'Shutdown' for this class.
    //
    // At some point, revisit the top-down Init/Shutdown in
    // ConnectivityManager. When that happens, plumb the call
    // propagation through to this client 'Shutdown' and re-add the
    // assertion.

    mWiFiIfName[0] = '\0';

    mConnectivityManagerImpl = &inConnectivityManagerImpl;

    return CHIP_NO_ERROR;
}

void WpaSupplicantClient::Shutdown() noexcept
{
    Reset();

    mConnectivityManagerImpl = nullptr;
}

void WpaSupplicantClient::Reset() noexcept
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    mWpaSupplicant.Reset();
}

bool WpaSupplicantClient::IsStarted() const noexcept
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    return !!mWpaSupplicant.iface;
}

bool WpaSupplicantClient::IsWiFiInterfaceEnabled() const noexcept
{
    VerifyOrReturnValue(mWpaSupplicant.iface, false);
    // Check if the interface is not disabled (for example, due to rfkill or some other reasons).
    return g_strcmp0(wpa_supplicant_1_interface_get_state(mWpaSupplicant.iface.get()), "interface_disabled") != 0;
}

CHIP_ERROR WpaSupplicantClient::GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) noexcept
{
    // This function can be called without g_main_context_get_thread_default() being set.
    // The network proxy object is created in a synchronous manner, so the D-Bus call will
    // be completed before this function returns. Also, no external callbacks are registered
    // with the proxy object.

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    GAutoPtr<GError> err;

    if (!mWpaSupplicant.iface)
    {
        ChipLogDetail(DeviceLayer, "Wifi network not currently connected");

        return CHIP_ERROR_INCORRECT_STATE;
    }

    const char * networkPath = wpa_supplicant_1_interface_get_current_network(mWpaSupplicant.iface.get());
    // wpa_supplicant DBus API: if network path of current network is "/", means no networks is currently selected.
    if ((networkPath == nullptr) || (strcmp(networkPath, "/") == 0))
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    GAutoPtr<WpaSupplicant1Network> networkInfo(wpa_supplicant_1_network_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, networkPath, nullptr, &err.GetReceiver()));
    VerifyOrReturnError(
        networkInfo, CHIP_ERROR_INTERNAL,
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to create network proxy: %s", err->message));

    outNetwork.connected  = wpa_supplicant_1_network_get_enabled(networkInfo.get());
    GVariant * properties = wpa_supplicant_1_network_get_properties(networkInfo.get());
    VerifyOrReturnError(properties != nullptr, CHIP_ERROR_KEY_NOT_FOUND);

    GAutoPtr<GVariant> ssid(g_variant_lookup_value(properties, "ssid", nullptr));
    VerifyOrReturnError(ssid, CHIP_ERROR_KEY_NOT_FOUND);
    gsize length;
    const gchar * ssidStr = g_variant_get_string(ssid.get(), &length);
    // TODO: wpa_supplicant will return ssid with quotes! We should have a better way to get the actual ssid in bytes.
    gsize length_actual = length - 2;
    VerifyOrReturnError(length_actual <= sizeof(outNetwork.networkID), CHIP_ERROR_INTERNAL);
    ChipLogDetail(DeviceLayer, "Current connected network: %s", StringOrNullMarker(ssidStr));
    memcpy(outNetwork.networkID, ssidStr + 1, length_actual);
    outNetwork.networkIDLen = length_actual;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WpaSupplicantClient::GetIfName(CharSpan & outIfName) const noexcept
{
    outIfName = CharSpan::fromCharString(mWiFiIfName);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WpaSupplicantClient::SetIfName(const CharSpan & inIfName) noexcept
{
    VerifyOrReturnError(inIfName.size() < std::size(mWiFiIfName), CHIP_ERROR_BUFFER_TOO_SMALL);

    Platform::CopyString(mWiFiIfName, inIfName);

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
