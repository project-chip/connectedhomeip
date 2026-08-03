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

#include <algorithm>
#include <mutex>
#include <utility>
#include <vector>

#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/NetworkCommissioning.h>
#include <platform/PlatformManager.h>
#include <system/SystemLayer.h>

#include "ConnectivityManagerImpl.h"
#include "NetworkCommissioningDriver.h"
#include "WirelessDefs.h"

using namespace ::chip::DeviceLayer::NetworkCommissioning;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static constexpr char kWpaSupplicantServiceName[] = "fi.w1.wpa_supplicant1";
static constexpr char kWpaSupplicantObjectPath[]  = "/fi/w1/wpa_supplicant1";

// wpa_supplicant's scan results don't contains the channel infomation, so we need this lookup table for resolving the band and
// channel infomation.
static std::pair<WiFiBand, uint16_t> GetBandAndChannelFromFrequency(const uint32_t & freq) noexcept
{
    std::pair<WiFiBand, uint16_t> ret = std::make_pair(WiFiBand::k2g4, 0);
    if (freq <= 931)
    {
        ret.first = WiFiBand::k1g;
        if (freq >= 916)
        {
            ret.second = ((freq - 916) * 2) - 1;
        }
        else if (freq >= 902)
        {
            ret.second = (freq - 902) * 2;
        }
        else if (freq >= 863)
        {
            ret.second = (freq - 863) * 2;
        }
        else
        {
            ret.second = 1;
        }
    }
    else if (freq <= 2472)
    {
        ret.second = static_cast<uint16_t>((freq - 2412) / 5 + 1);
    }
    else if (freq == 2484)
    {
        ret.second = 14;
    }
    else if (freq >= 3600 && freq <= 3700)
    {
        // Note: There are not many devices supports this band, and this band contains rational frequency in MHz, need to figure out
        // the behavior of wpa_supplicant in this case.
        ret.first = WiFiBand::k3g65;
    }
    else if (freq >= 5035 && freq <= 5945)
    {
        ret.first  = WiFiBand::k5g;
        ret.second = static_cast<uint16_t>((freq - 5000) / 5);
    }
    else if (freq == 5960 || freq == 5980)
    {
        ret.first  = WiFiBand::k5g;
        ret.second = static_cast<uint16_t>((freq - 5000) / 5);
    }
    else if (freq >= 5955)
    {
        ret.first  = WiFiBand::k6g;
        ret.second = static_cast<uint16_t>((freq - 5950) / 5);
    }
    else if (freq >= 58000)
    {
        ret.first = WiFiBand::k60g;
        // Note: Some channel has the same center frequency but different bandwidth. Should figure out wpa_supplicant's behavior in
        // this case. Also, wpa_supplicant's frequency property is uint16 infact.
        switch (freq)
        {
        case 58'320:
            ret.second = 1;
            break;
        case 60'480:
            ret.second = 2;
            break;
        case 62'640:
            ret.second = 3;
            break;
        case 64'800:
            ret.second = 4;
            break;
        case 66'960:
            ret.second = 5;
            break;
        case 69'120:
            ret.second = 6;
            break;
        case 59'400:
            ret.second = 9;
            break;
        case 61'560:
            ret.second = 10;
            break;
        case 63'720:
            ret.second = 11;
            break;
        case 65'880:
            ret.second = 12;
            break;
        case 68'040:
            ret.second = 13;
            break;
        }
    }
    return ret;
}

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

    mAssociationStarted = false;

    mAssociationRetriesLeft = 0;

    mWiFiIfName[0] = '\0';

    mInterestedSSID.reset();

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

bool WpaSupplicantClient::GetBssInfo(const gchar * inBssPath,
                                     NetworkCommissioning::WiFiScanResponse & outWiFiScanResponse) const noexcept
{
    // This function can be called without g_main_context_get_thread_default() being set.
    // The BSS proxy object is created in a synchronous manner, so the D-Bus call will be
    // completed before this function returns. Also, no external callbacks are registered
    // with the proxy object.

    GAutoPtr<GError> err;
    GAutoPtr<WpaSupplicant1BSS> bss(wpa_supplicant_1_bss_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, inBssPath, nullptr, &err.GetReceiver()));

    if (bss == nullptr)
    {
        return false;
    }

    WpaSupplicant1BSSProxy * bssProxy = WPA_SUPPLICANT_1_BSS_PROXY(bss.get());

    GAutoPtr<GVariant> ssid(g_dbus_proxy_get_cached_property(G_DBUS_PROXY(bssProxy), "SSID"));
    GAutoPtr<GVariant> bssid(g_dbus_proxy_get_cached_property(G_DBUS_PROXY(bssProxy), "BSSID"));

    // Network scan is performed in the background, so the BSS
    // may be gone when we try to get the properties.
    if (ssid == nullptr || bssid == nullptr)
    {
        ChipLogDetail(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "BSS not found: %s", StringOrNullMarker(inBssPath));
        return false;
    }

    gsize ssidLen                = 0;
    gsize bssidLen               = 0;
    char bssidStr[2 * 6 + 5 + 1] = { 0 };
    auto ssidStr      = reinterpret_cast<const uint8_t *>(g_variant_get_fixed_array(ssid.get(), &ssidLen, sizeof(uint8_t)));
    auto bssidBuf     = reinterpret_cast<const uint8_t *>(g_variant_get_fixed_array(bssid.get(), &bssidLen, sizeof(uint8_t)));
    gint16 signal     = wpa_supplicant_1_bss_get_signal(bss.get());
    guint16 frequency = wpa_supplicant_1_bss_get_frequency(bss.get());

    if (bssidLen == 6)
    {
        snprintf(bssidStr, sizeof(bssidStr), "%02x:%02x:%02x:%02x:%02x:%02x", bssidBuf[0], bssidBuf[1], bssidBuf[2], bssidBuf[3],
                 bssidBuf[4], bssidBuf[5]);
    }
    else
    {
        ChipLogError(DeviceLayer,
                     WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Got a network with incorrect BSSID len: %" G_GSIZE_FORMAT " != 6", bssidLen);
        bssidLen = 0;
    }
    ChipLogDetail(DeviceLayer, "Network Found: %s (%s) Signal:%d",
                  NullTerminated(StringOrNullMarker((const gchar *) ssidStr), ssidLen).c_str(), bssidStr, signal);

    // Internal sentinel (bit 7). Not a real WiFiSecurityBitmap value; keeps an EAP-only
    // network from being reported as Open. Masked off before the result is returned.

    // TODO: The following code will mistakenly recognize WEP encryption as OPEN network, this should be fixed by reading
    // IEs (information elements) field instead of reading cooked data.

    static constexpr chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> kEAP{ static_cast<uint8_t>(1 << 7) };

    auto IsNetworkWPAPSK = [](GVariant * wpa) -> chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> {
        chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> res;

        if (wpa == nullptr)
        {
            return res;
        }

        GAutoPtr<GVariant> keyMgmt(g_variant_lookup_value(wpa, "KeyMgmt", nullptr));
        if (keyMgmt == nullptr)
        {
            return res;
        }
        GBorrowedStrvPtr keyMgmts(g_variant_get_strv(keyMgmt.get(), nullptr));
        const gchar ** keyMgmtsHandle = keyMgmts.get();

        VerifyOrReturnError(keyMgmtsHandle != nullptr, res);

        for (auto keyMgmtVal = *keyMgmtsHandle; keyMgmtVal != nullptr; keyMgmtVal = *(++keyMgmtsHandle))
        {
            if (g_strcasecmp(keyMgmtVal, "wpa-psk") == 0 || g_strcasecmp(keyMgmtVal, "wpa-none") == 0)
            {
                res.Set(app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kWpaPersonal);
            }
            else if (g_strcasecmp(keyMgmtVal, "wpa-eap") == 0)
            {
                res.Set(kEAP);
            }
        }

        return res;
    };
    auto IsNetworkWPA2PSK = [](GVariant * rsn) -> chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> {
        chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> res;

        if (rsn == nullptr)
        {
            return res;
        }
        GAutoPtr<GVariant> keyMgmt(g_variant_lookup_value(rsn, "KeyMgmt", nullptr));
        if (keyMgmt == nullptr)
        {
            return res;
        }
        GBorrowedStrvPtr keyMgmts(g_variant_get_strv(keyMgmt.get(), nullptr));
        const gchar ** keyMgmtsHandle = keyMgmts.get();

        VerifyOrReturnError(keyMgmtsHandle != nullptr, res);

        for (auto keyMgmtVal = *keyMgmtsHandle; keyMgmtVal != nullptr; keyMgmtVal = *(++keyMgmtsHandle))
        {
            if (g_strcasecmp(keyMgmtVal, "wpa-psk") == 0 || g_strcasecmp(keyMgmtVal, "wpa-psk-sha256") == 0 ||
                g_strcasecmp(keyMgmtVal, "wpa-ft-psk") == 0)
            {
                res.Set(app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kWpa2Personal);
            }
            else if (g_strcasecmp(keyMgmtVal, "wpa-eap") == 0 || g_strcasecmp(keyMgmtVal, "wpa-eap-sha256") == 0 ||
                     g_strcasecmp(keyMgmtVal, "wpa-ft-eap") == 0)
            {
                res.Set(kEAP);
            }
            else if (g_strcasecmp(keyMgmtVal, "sae") == 0)
            {
                // wpa_supplicant will include "sae" in KeyMgmt field for WPA3 WiFi, this is not included in the wpa_supplicant
                // document.
                res.Set(app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kWpa3Personal);
            }
        }

        return res;
    };
    auto GetNetworkSecurityType =
        [IsNetworkWPAPSK, IsNetworkWPA2PSK](
            WpaSupplicant1BSSProxy * proxy) -> chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> {
        GAutoPtr<GVariant> wpa(g_dbus_proxy_get_cached_property(G_DBUS_PROXY(proxy), "WPA"));
        GAutoPtr<GVariant> rsn(g_dbus_proxy_get_cached_property(G_DBUS_PROXY(proxy), "RSN"));

        chip::BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurityBitmap> res(IsNetworkWPAPSK(wpa.get()),
                                                                                    IsNetworkWPA2PSK(rsn.get()));
        if (!res.HasAny())
        {
            res.Set(app::Clusters::NetworkCommissioning::WiFiSecurityBitmap::kUnencrypted);
        }
        res.Clear(kEAP);
        return res;
    };

    // Drop the network if its SSID or BSSID is illegal.
    VerifyOrReturnError(ssidLen <= DeviceLayer::Internal::kMaxWiFiSSIDLength, false);
    VerifyOrReturnError(bssidLen == kWiFiBSSIDLength, false);
    memcpy(outWiFiScanResponse.ssid, ssidStr, ssidLen);
    memcpy(outWiFiScanResponse.bssid, bssidBuf, bssidLen);
    outWiFiScanResponse.ssidLen         = ssidLen;
    outWiFiScanResponse.signal.type     = WirelessSignalType::kdBm;
    outWiFiScanResponse.signal.strength = static_cast<int8_t>(std::clamp<gint16>(signal, INT8_MIN, INT8_MAX));
    auto bandInfo                       = GetBandAndChannelFromFrequency(frequency);
    outWiFiScanResponse.wiFiBand        = bandInfo.first;
    outWiFiScanResponse.channel         = bandInfo.second;
    outWiFiScanResponse.security        = GetNetworkSecurityType(bssProxy);

    return true;
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

CHIP_ERROR WpaSupplicantClient::ScanNetwork(const ByteSpan & inSsid)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(inSsid.size() <= mInterestedSSID.capacity(), CHIP_ERROR_INVALID_ARGUMENT);

    GAutoPtr<GError> err;
    GVariant * args = nullptr;
    GVariantBuilder builder;

    mInterestedSSID = inSsid;

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "Type", g_variant_new_string("active"));
    args = g_variant_builder_end(&builder);

    if (!wpa_supplicant_1_interface_call_scan_sync(mWpaSupplicant.iface.get(), args, nullptr, &err.GetReceiver()))
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to start network scan: %s",
                        err ? err->message : "unknown error");

        mInterestedSSID.reset();

        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "initialized network scan");

    return CHIP_NO_ERROR;
}

void WpaSupplicantClient::OnWpaPropertiesChanged(WpaSupplicant1Interface * iface, GVariant * changedProperties)
{
    const char * state = nullptr;
    // We are only interested in the "State" property changes.
    VerifyOrReturn(g_variant_lookup(changedProperties, "State", "&s", &state));

    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    ChipLogDetail(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Interface properties changed, state is '%s'", state);

    if (g_strcmp0(state, "associating") == 0)
    {
        mAssociationStarted = true;
    }
    else if (g_strcmp0(state, "disconnected") == 0)
    {
        int reason = wpa_supplicant_1_interface_get_disconnect_reason(iface);

        ChipLogDetail(DeviceLayer,
                      WPA_SUPPLICANT_CLIENT_LOG_PREFIX
                      "Disconnected with reason code=%d, assoc status code=%d, auth status code=%d (associationStarted=%d)",
                      reason, wpa_supplicant_1_interface_get_assoc_status_code(iface),
                      wpa_supplicant_1_interface_get_auth_status_code(iface), mAssociationStarted);

        if (delegate != nullptr)
        {
            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([delegate, reason]() {
                delegate->OnDisconnectionDetected(reason);
                delegate->OnConnectionStatusChanged(static_cast<uint8_t>(ConnectionStatusEnum::kNotConnected));
            });
        }

        if (mAssociationStarted && mConnectivityManagerImpl != nullptr)
        {
            uint8_t associationFailureCause = static_cast<uint8_t>(AssociationFailureCauseEnum::kUnknown);
            uint16_t status                 = WLAN_STATUS_UNSPECIFIED_FAILURE;

            if (wpa_supplicant_1_interface_get_assoc_status_code(iface) == WLAN_STATUS_AUTH_TIMEOUT)
            {
                std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
                /* Handle intermittent association failures */
                if (mAssociationRetriesLeft > 0)
                {
                    mAssociationRetriesLeft--;
                    ChipLogDetail(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Association timeout, %u retries left",
                                  mAssociationRetriesLeft);

                    mAssociationStarted = false;

                    GAutoPtr<GError> err;
                    if (!wpa_supplicant_1_interface_call_select_network_sync(
                            mWpaSupplicant.iface.get(), mWpaSupplicant.networkPath.get(), nullptr, &err.GetReceiver()))
                    {
                        // Fallthrough to existing error handling code as we could not retry.
                        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to select network: '%s'", err->message);
                    }
                    else
                    {
                        // Skip existing error handling code to not report error too early to the network commissioning cluster.
                        return;
                    }
                }
            }

            switch (abs(reason))
            {
            case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
            case WLAN_REASON_DISASSOC_AP_BUSY:
            case WLAN_REASON_DISASSOC_STA_HAS_LEFT:
            case WLAN_REASON_DISASSOC_LOW_ACK:
            case WLAN_REASON_BSS_TRANSITION_DISASSOC:
                associationFailureCause = static_cast<uint8_t>(AssociationFailureCauseEnum::kAssociationFailed);
                status                  = wpa_supplicant_1_interface_get_assoc_status_code(iface);
                break;
            case WLAN_REASON_PREV_AUTH_NOT_VALID:
            case WLAN_REASON_DEAUTH_LEAVING:
            case WLAN_REASON_IEEE_802_1X_AUTH_FAILED:
                associationFailureCause = static_cast<uint8_t>(AssociationFailureCauseEnum::kAuthenticationFailed);
                status                  = wpa_supplicant_1_interface_get_auth_status_code(iface);
                break;
            default:
                break;
            }

            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this, reason]() {
                mConnectivityManagerImpl->OnConnectResult(NetworkCommissioning::Status::kUnknownError, CharSpan(), reason);
            });
            if (delegate != nullptr)
            {
                TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([delegate, associationFailureCause, status]() {
                    delegate->OnAssociationFailureDetected(associationFailureCause, status);
                });
            }
        }

        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this]() { this->UpdateWiFiNetworkStatus(); });
        NotifyWiFiConnectivityChange(kConnectivity_Lost);

        mAssociationStarted = false;
    }
    else if (g_strcmp0(state, "associated") == 0)
    {
        if (delegate != nullptr)
        {
            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate]() { delegate->OnConnectionStatusChanged(static_cast<uint8_t>(ConnectionStatusEnum::kConnected)); });
        }
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this]() { this->UpdateWiFiNetworkStatus(); });
    }
    else if (g_strcmp0(state, "completed") == 0)
    {
        if (mAssociationStarted && mConnectivityManagerImpl != nullptr)
        {
            TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this]() {
                mConnectivityManagerImpl->OnConnectResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);

                PostNetworkConnect();
            });
        }
        NotifyWiFiConnectivityChange(kConnectivity_Established);
        mAssociationStarted = false;
    }
}

void WpaSupplicantClient::OnWpaInterfaceProxyReady(GObject * sourceObject, GAsyncResult * res)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    GAutoPtr<GError> err;
    mWpaSupplicant.iface.reset(wpa_supplicant_1_interface_proxy_new_for_bus_finish(res, &err.GetReceiver()));
    if (mWpaSupplicant.iface && err == nullptr)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "connected to wpa_supplicant interface proxy");

        g_signal_connect(
            mWpaSupplicant.iface.get(), "g-properties-changed",
            G_CALLBACK(+[](WpaSupplicant1Interface * iface, GVariant * properties, const char * const * invalidatedProps,
                           WpaSupplicantClient * self) { return self->OnWpaPropertiesChanged(iface, properties); }),
            this);

        g_signal_connect(mWpaSupplicant.iface.get(), "scan-done",
                         G_CALLBACK(+[](WpaSupplicant1Interface * iface, gboolean success, WpaSupplicantClient * self) {
                             return self->OnWpaInterfaceScanDone(iface, success);
                         }),
                         this);
    }
    else
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to create wpa_supplicant interface proxy %s: %s",
                        mWpaSupplicant.interfacePath.get(), err ? err->message : "unknown error");
    }

    // We need to stop auto scan or it will block our network scan.
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        CHIP_ERROR errInner = StopAutoScan();
        if (errInner != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to stop auto scan: %" CHIP_ERROR_FORMAT,
                         errInner.Format());
        }
    });
}

void WpaSupplicantClient::OnWpaProxyReady(GObject * sourceObject, GAsyncResult * res)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    GAutoPtr<GError> err;
    mWpaSupplicant.proxy.reset(wpa_supplicant_1_proxy_new_for_bus_finish(res, &err.GetReceiver()));
    if (mWpaSupplicant.proxy && err.get() == nullptr)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "connected to wpa_supplicant proxy");

        g_signal_connect(mWpaSupplicant.proxy.get(), "interface-added",
                         G_CALLBACK(+[](WpaSupplicant1 * proxy, const char * path, GVariant * properties,
                                        WpaSupplicantClient * self) { return self->OnWpaInterfaceAdded(proxy, path, properties); }),
                         this);
        g_signal_connect(mWpaSupplicant.proxy.get(), "interface-removed",
                         G_CALLBACK(+[](WpaSupplicant1 * proxy, const char * path, WpaSupplicantClient * self) {
                             return self->OnWpaInterfaceRemoved(proxy, path);
                         }),
                         this);

        wpa_supplicant_1_call_get_interface(
            mWpaSupplicant.proxy.get(), mWiFiIfName, nullptr,
            reinterpret_cast<GAsyncReadyCallback>(+[](GObject * sourceObject_, GAsyncResult * res_, WpaSupplicantClient * self) {
                return self->OnWpaInterfaceReady(sourceObject_, res_);
            }),
            this);
    }
    else
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to create wpa_supplicant proxy %s",
                        err ? err->message : "unknown error");
    }
}

void WpaSupplicantClient::OnWpaInterfaceRemoved(WpaSupplicant1 * proxy, const char * path)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    if (g_strcmp0(mWpaSupplicant.interfacePath.get(), path) == 0)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "WiFi interface removed: %s", StringOrNullMarker(path));
        mWpaSupplicant.interfacePath.reset();
        mWpaSupplicant.iface.reset();
    }
}

void WpaSupplicantClient::OnWpaInterfaceAdded(WpaSupplicant1 * proxy, const char * path, GVariant * properties)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    if (mWpaSupplicant.interfacePath)
    {
        return;
    }

    mWpaSupplicant.interfacePath.reset(g_strdup(path));
    if (mWpaSupplicant.interfacePath)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "WiFi interface added: %s",
                        mWpaSupplicant.interfacePath.get());
        wpa_supplicant_1_interface_proxy_new_for_bus(
            G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, mWpaSupplicant.interfacePath.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(+[](GObject * sourceObject_, GAsyncResult * res_, WpaSupplicantClient * self) {
                return self->OnWpaInterfaceProxyReady(sourceObject_, res_);
            }),
            this);
    }
}

void WpaSupplicantClient::OnWpaInterfaceScanDone(WpaSupplicant1Interface * iface, gboolean success)
{
    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "network scan done");

    const char * const * bsss = wpa_supplicant_1_interface_get_bsss(iface);
    if (bsss == nullptr)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "no network found");
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda(
            [this]() { mConnectivityManagerImpl->OnScanFinished(Status::kSuccess, CharSpan(), nullptr); });
        return;
    }

    auto networkScanned = std::make_unique<std::vector<WiFiScanResponse>>();
    for (const char * bssPath = (bsss != nullptr ? *bsss : nullptr); bssPath != nullptr; bssPath = *(++bsss))
    {
        WiFiScanResponse network;
        if (GetBssInfo(bssPath, network))
        {
            if (mInterestedSSID.empty() ||
                (network.ssidLen == mInterestedSSID.size() &&
                 memcmp(network.ssid, mInterestedSSID.data(), mInterestedSSID.size()) == 0))
            {
                networkScanned->push_back(network);
            }
        }
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().ScheduleLambda([this, scanned = networkScanned.get()]() {
        // Note: We cannot post an event in ScheduleLambda since std::vector is not trivial copyable.
        LinuxScanResponseIterator<WiFiScanResponse> iter(scanned);

        mConnectivityManagerImpl->OnScanFinished(Status::kSuccess, CharSpan(), &iter);

        delete scanned;
    });

    if (::chip::ChipError::IsSuccess(err))
    {
        networkScanned.release();
    }
    else
    {
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to schedule scan completion: %" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    OnWiFiMediumAvailable(*this, true);
}

void WpaSupplicantClient::OnWpaInterfaceReady(GObject * sourceObject, GAsyncResult * res)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    GAutoPtr<GError> err;
    if (wpa_supplicant_1_call_get_interface_finish(mWpaSupplicant.proxy.get(), &mWpaSupplicant.interfacePath.GetReceiver(), res,
                                                   &err.GetReceiver()))
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "WiFi interface: %s", mWpaSupplicant.interfacePath.get());

        wpa_supplicant_1_interface_proxy_new_for_bus(
            G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, mWpaSupplicant.interfacePath.get(), nullptr,
            reinterpret_cast<GAsyncReadyCallback>(+[](GObject * sourceObject_, GAsyncResult * res_, WpaSupplicantClient * self) {
                return self->OnWpaInterfaceProxyReady(sourceObject_, res_);
            }),
            this);
    }
    else
    {
        GVariant * args = nullptr;
        GVariantBuilder builder;
        const char * const ifname = (mWiFiIfName[0] != '\0') ? mWiFiIfName : CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME;

        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "can't find interface %s: %s", ifname,
                        err ? err->message : "unknown error");

        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "try to create interface %s", ifname);

        g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&builder, "{sv}", "Ifname", g_variant_new_string(ifname));
        args = g_variant_builder_end(&builder);

        err.reset();
        if (wpa_supplicant_1_call_create_interface_sync(mWpaSupplicant.proxy.get(), args,
                                                        &mWpaSupplicant.interfacePath.GetReceiver(), nullptr, &err.GetReceiver()))
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "WiFi interface: %s", mWpaSupplicant.interfacePath.get());

            Platform::CopyString(mWiFiIfName, ifname);

            wpa_supplicant_1_interface_proxy_new_for_bus(
                G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, mWpaSupplicant.interfacePath.get(), nullptr,
                reinterpret_cast<GAsyncReadyCallback>(
                    +[](GObject * sourceObject_, GAsyncResult * res_, WpaSupplicantClient * self) {
                        return self->OnWpaInterfaceProxyReady(sourceObject_, res_);
                    }),
                this);
        }
        else
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to create interface %s: %s",
                            CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, err ? err->message : "unknown error");
            mWpaSupplicant.interfacePath.reset();
        }
    }
}

void WpaSupplicantClient::NotifyWiFiConnectivityChange(const ConnectivityChange & inChange)
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kWiFiConnectivityChange, .WiFiConnectivityChange = { .Result = inChange } };
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR WpaSupplicantClient::StartOnGLib(void)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Start WiFi management");

    wpa_supplicant_1_proxy_new_for_bus(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName, kWpaSupplicantObjectPath, nullptr,
        reinterpret_cast<GAsyncReadyCallback>(+[](GObject * sourceObject_, GAsyncResult * res_, WpaSupplicantClient * self) {
            return self->OnWpaProxyReady(sourceObject_, res_);
        }),
        this);

    return CHIP_NO_ERROR;
}

void WpaSupplicantClient::Start(void)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    mWpaSupplicant = GDBusWpaSupplicant{};

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WpaSupplicantClient * self) { return self->StartOnGLib(); }, this);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to start WiFi management"));
}

CHIP_ERROR WpaSupplicantClient::StartSync(void)
{
    if (IsStarted())
    {
        return CHIP_NO_ERROR;
    }
    ChipLogProgress(DeviceLayer, "Start and sync Wi-Fi Management.");
    static constexpr useconds_t kWiFiStartCheckTimeUsec = WIFI_START_CHECK_TIME_USEC;
    static constexpr uint8_t kWiFiStartCheckAttempts    = WIFI_START_CHECK_ATTEMPTS;
    Start();
    for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
    {
        if (IsStarted())
        {
            break;
        }
        usleep(kWiFiStartCheckTimeUsec);
    }
    if (!IsStarted())
    {
        ChipLogError(DeviceLayer, "Wi-Fi Management can't be started.");
        return CHIP_ERROR_INTERNAL;
    }
    ChipLogProgress(DeviceLayer, "Wi-Fi Management is started");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WpaSupplicantClient::StopAutoScan(void)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "disabling auto scan");

    GAutoPtr<GError> err;
    if (!wpa_supplicant_1_interface_call_auto_scan_sync(mWpaSupplicant.iface.get(), "" /* empty string means disabling auto scan */,
                                                        nullptr, &err.GetReceiver()))
    {
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to stop auto network scan: %s",
                     err ? err->message : "unknown");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
