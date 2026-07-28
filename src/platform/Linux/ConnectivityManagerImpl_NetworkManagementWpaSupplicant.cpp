/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    Copyright (c) 2025 NXP
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

#include "ConnectivityManagerImpl_NetworkManagementWpaSupplicant.h"

#include <mutex>

#include <ifaddrs.h>

#include <glib.h>

#include <lib/core/Optional.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/NetworkCommissioning.h>

#include "ConnectivityManagerImpl.h"
#include "ConnectivityUtils.h"
#include "NetworkCommissioningDriver.h"
#include "WirelessDefs.h"

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::NetworkCommissioning;
using namespace ::chip::app::Clusters::GeneralDiagnostics;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

namespace chip {
namespace DeviceLayer {
namespace {

// Global Variables

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
static constexpr char kWpaSupplicantBlobUnknown[] = "fi.w1.wpa_supplicant1.BlobUnknown";

// Note: Static blob names assume we're only supporting a single network configuration.
static constexpr char kNetworkIdentityBlobRef[]   = "blob://pdc-ni";
static constexpr char kClientIdentityBlobRef[]    = "blob://pdc-ci";
static constexpr char kClientIdentityKeyBlobRef[] = "blob://pdc-cik";
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

// Function and Method Implementation

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
static CHIP_ERROR AddOrReplaceBlob(WpaSupplicant1Interface * iface, const char * nameOrRef, ByteSpan data) noexcept
{
    // Strip the blob:// prefix off the name (if present), so we don't need as many string constants.
    constexpr auto refPrefix = "blob://"_span;
    const char * name = (strncmp(nameOrRef, refPrefix.data(), refPrefix.size()) == 0) ? nameOrRef + refPrefix.size() : nameOrRef;

    GAutoPtr<GError> err;
    if (!wpa_supplicant_1_interface_call_remove_blob_sync(iface, name, nullptr, &err.GetReceiver()))
    {
        GCharPtr remoteError(g_dbus_error_get_remote_error(err.get()));
        if (!(remoteError && strcmp(remoteError.get(), kWpaSupplicantBlobUnknown) == 0))
        {
            ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to remove blob: %s",
                         err ? err->message : "unknown error");
            return CHIP_ERROR_INTERNAL;
        }
        err.reset();
    }
    if (!wpa_supplicant_1_interface_call_add_blob_sync(
            iface, name, g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, data.data(), data.size(), 1), nullptr, &err.GetReceiver()))
    {
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to add blob: %s", err ? err->message : "unknown error");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

} // namespace

// Function and Method Implementation

// Matter Linux Connectivity Manager Implementation for wpa_supplicant

// Intialization

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::Init(ConnectivityManagerImpl & inConnectivityManagerImpl)
{
    char wifi_ifname[Inet::InterfaceId::kMaxIfNameLength];

    VerifyOrReturnError(mConnectivityManagerImpl == nullptr, CHIP_ERROR_ALREADY_INITIALIZED);

    // Initialize the base network management class.

    ReturnErrorOnFailure(NetworkManagementBasis::Init());

    // Initialize the base wpa_supplicant class.

    ReturnErrorOnFailure(WpaSupplicantClient::Init(inConnectivityManagerImpl));

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    if (ConnectivityUtils::GetEthInterfaceName(mEthernetIfName, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got Ethernet interface: %s", mEthernetIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get Ethernet interface");
        mEthernetIfName[0] = '\0';
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityUtils::GetWiFiInterfaceName(wifi_ifname, Inet::InterfaceId::kMaxIfNameLength) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got WiFi interface: %s", wifi_ifname);

        ReturnErrorOnFailure(WpaSupplicantClient::SetIfName(CharSpan(&wifi_ifname[0], strlen(wifi_ifname))));
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get WiFi interface");
    }
#endif

    mConnectivityManagerImpl = &inConnectivityManagerImpl;

    return CHIP_NO_ERROR;
}

// Event Handling

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) {}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiStationMode()
{
    if (mWiFiStationMode != ConnectivityManager::kWiFiStationMode_ApplicationControlled)
    {
        std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
        mWiFiStationMode = IsWiFiManagementStarted() ? ConnectivityManager::kWiFiStationMode_Enabled
                                                     : ConnectivityManager::kWiFiStationMode_Disabled;
    }

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::SetWiFiStationMode(
    const ConnectivityManager::WiFiStationMode & inWiFiStationMode)
{
    VerifyOrReturnError(inWiFiStationMode != ConnectivityManager::kWiFiStationMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != inWiFiStationMode)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s",
                        ConnectivityManager::WiFiStationModeToStr(mWiFiStationMode),
                        ConnectivityManager::WiFiStationModeToStr(inWiFiStationMode));
    }

    mWiFiStationMode = inWiFiStationMode;

    return CHIP_NO_ERROR;
}

System::Clock::Timeout ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiStationReconnectInterval()
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementWpaSupplicant::SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval)
{
    mWiFiStationReconnectInterval = inInterval;

    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl_NetworkManagementWpaSupplicant::IsWiFiStationEnabled()
{
    return GetWiFiStationMode() == ConnectivityManager::kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl_NetworkManagementWpaSupplicant::IsWiFiStationConnected()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnValue(mWpaSupplicant.iface, false);
    GCharPtr state(wpa_supplicant_1_interface_dup_state(mWpaSupplicant.iface.get()));
    // The "completed" state indicates that we are associated with the access point.
    return g_strcmp0(state.get(), "completed") == 0;
}

bool ConnectivityManagerImpl_NetworkManagementWpaSupplicant::IsWiFiStationApplicationControlled()
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

bool ConnectivityManagerImpl_NetworkManagementWpaSupplicant::IsWiFiStationProvisioned()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnValue(mWpaSupplicant.iface, false);

    GAutoPtr<GError> err;
    // WPA supplicant does not emit PropertiesChanged signal for the Networks property, so we can not use our
    // proxy observer to get the property value. Instead, we need to get it directly using the D-Bus call.
    const char * ifaceName = g_dbus_proxy_get_interface_name(reinterpret_cast<GDBusProxy *>(mWpaSupplicant.iface.get()));
    GAutoPtr<GVariant> response(g_dbus_proxy_call_sync(
        reinterpret_cast<GDBusProxy *>(mWpaSupplicant.iface.get()), "org.freedesktop.DBus.Properties.Get",
        g_variant_new("(ss)", ifaceName, "Networks"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err.GetReceiver()));
    VerifyOrReturnValue(
        response, false,
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to get Networks property: %s", err->message));

    // Check whether we have at least one network provisioned.
    GAutoPtr<GVariant> responseValue(g_variant_get_child_value(response.get(), 0));
    GAutoPtr<GVariant> networks(g_variant_get_child_value(responseValue.get(), 0));
    return g_variant_n_children(networks.get()) > 0;
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ClearWiFiStationProvision()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturn(mWpaSupplicant.iface);
    VerifyOrReturn(mWiFiStationMode != ConnectivityManager::kWiFiStationMode_ApplicationControlled);

    GAutoPtr<GError> err;
    if (!wpa_supplicant_1_interface_call_remove_all_networks_sync(mWpaSupplicant.iface.get(), nullptr, &err.GetReceiver()))
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to remove all networks: %s", err->message);
    }
}

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiApMode()
{
    return mWiFiAPMode;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementWpaSupplicant::SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode)
{
    VerifyOrReturnError(inWiFiApMode != ConnectivityManager::kWiFiAPMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != inWiFiApMode)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", ConnectivityManager::WiFiAPModeToStr(mWiFiAPMode),
                        ConnectivityManager::WiFiAPModeToStr(inWiFiApMode));

        mWiFiAPMode = inWiFiApMode;

        return DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveApState(); });
    }

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::DemandStartWiFiAp()
{
    if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand ||
        mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Demand start WiFi AP");
        mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveApState(); });
    }
    else
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Demand start WiFi AP ignored, mode: %s",
                        ConnectivityManager::WiFiAPModeToStr(mWiFiAPMode));
    }
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::StopOnDemandWiFiAp()
{
    if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand ||
        mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision)
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Demand stop WiFi AP");
        mLastAPDemandTime = System::Clock::kZero;
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveApState(); });
    }
    else
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Demand stop WiFi AP ignored, mode: %s",
                        ConnectivityManager::WiFiAPModeToStr(mWiFiAPMode));
    }
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::MaintainOnDemandWiFiAp()
{
    if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand ||
        mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision)
    {
        if (mWiFiAPState == ConnectivityManager::kWiFiAPState_Active)
        {
            mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        }
    }
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout)
{
    mWiFiAPIdleTimeout = inTimeout;
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveApState(); });
}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
// Ethernet Control Plane Management

const char * ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetEthernetIfName()
{
    return (mEthernetIfName[0] == '\0') ? nullptr : mEthernetIfName;
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::UpdateEthernetNetworkingStatus()
{
    VerifyOrReturn(mConnectivityManagerImpl != nullptr);

    if (mEthernetIfName[0] != '\0')
    {
        ByteSpan ifNameSpan(reinterpret_cast<unsigned char *>(mEthernetIfName),
                            strnlen(mEthernetIfName, Inet::InterfaceId::kMaxIfNameLength));
        mConnectivityManagerImpl->OnStatusChange(Status::kSuccess, MakeOptional(ifNameSpan), NullOptional);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
// Wi-Fi Control Plane Management

// Observation

const char * ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiIfName()
{
    CharSpan ifname;
    CHIP_ERROR status;
    const char * retval = nullptr;

    status = WpaSupplicantClient::GetIfName(ifname);
    SuccessOrExit(status);

    retval = ifname.data();

exit:
    return retval;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

// Observation

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::StartWiFiManagement()
{
    WpaSupplicantClient::Start();
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::StopWiFiManagement()
{
    WpaSupplicantClient::Reset();
}

bool ConnectivityManagerImpl_NetworkManagementWpaSupplicant::IsWiFiManagementStarted()
{
    return WpaSupplicantClient::IsStarted();
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::StartNonConcurrentWiFiManagement()
{
    StartWiFiManagement();

    for (int cnt = 0; cnt < WIFI_START_CHECK_ATTEMPTS; cnt++)
    {
        if (IsWiFiManagementStarted())
        {
            TEMPORARY_RETURN_IGNORED DeviceControlServer::DeviceControlSvr().PostOperationalNetworkStartedEvent();
            ChipLogProgress(DeviceLayer, "Non-concurrent mode Wi-Fi Management Started.");
            return;
        }
        usleep(WIFI_START_CHECK_TIME_USEC);
    }
    ChipLogError(Ble, "Non-concurrent mode Wi-Fi Management taking too long to start.");
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::DriveApState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ConnectivityManager::WiFiAPState targetState;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    // If the AP interface is not under application control...
    if (mWiFiAPMode != ConnectivityManager::kWiFiAPMode_ApplicationControlled)
    {
        // Determine the target (desired) state for AP interface...

        // The target state is 'NotActive' if the application has expressly disabled the AP interface.
        if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_Disabled)
        {
            targetState = ConnectivityManager::kWiFiAPState_NotActive;
        }

        // The target state is 'Active' if the application has expressly enabled the AP interface.
        else if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_Enabled)
        {
            targetState = ConnectivityManager::kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is 'On demand, when no station is available'
        // and the station interface is not provisioned or the application has disabled the station
        // interface.
        else if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision &&
                 (!IsWiFiStationProvisioned() || GetWiFiStationMode() == ConnectivityManager::kWiFiStationMode_Disabled))
        {
            targetState = ConnectivityManager::kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is one of the 'On demand' modes and there
        // has been demand for the AP within the idle timeout period.
        else if (mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand ||
                 mWiFiAPMode == ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision)
        {
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

            if (mLastAPDemandTime != System::Clock::kZero && now < (mLastAPDemandTime + mWiFiAPIdleTimeout))
            {
                targetState = ConnectivityManager::kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                System::Clock::Timeout apTimeout = (mLastAPDemandTime + mWiFiAPIdleTimeout) - now;
                err                              = DeviceLayer::SystemLayer().StartTimer(apTimeout, DriveApState, this);
                SuccessOrExit(err);
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " s",
                                std::chrono::duration_cast<System::Clock::Seconds32>(apTimeout).count());
            }
            else
            {
                targetState = ConnectivityManager::kWiFiAPState_NotActive;
            }
        }

        // Otherwise the target state is 'NotActive'.
        else
        {
            targetState = ConnectivityManager::kWiFiAPState_NotActive;
        }

        // If the current AP state does not match the target state...
        if (mWiFiAPState != targetState)
        {
            if (targetState == ConnectivityManager::kWiFiAPState_Active)
            {
                err = ConfigureWiFiAp();
                SuccessOrExit(err);

                ChangeWiFiApState(ConnectivityManager::kWiFiAPState_Active);
            }
            else
            {
                if (mWpaSupplicant.networkPath)
                {
                    GAutoPtr<GError> error;
                    if (wpa_supplicant_1_interface_call_remove_network_sync(
                            mWpaSupplicant.iface.get(), mWpaSupplicant.networkPath.get(), nullptr, &error.GetReceiver()))
                    {
                        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "removed network: %s",
                                        mWpaSupplicant.networkPath.get());
                        ChangeWiFiApState(ConnectivityManager::kWiFiAPState_NotActive);
                        mWpaSupplicant.networkPath.reset();
                    }
                    else
                    {
                        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to stop AP mode with error: %s",
                                        error ? error->message : "unknown error");
                        err = CHIP_ERROR_INTERNAL;
                    }
                }
            }
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        TEMPORARY_RETURN_IGNORED SetWiFiApMode(ConnectivityManager::kWiFiAPMode_Disabled);
        ChipLogError(DeviceLayer, "Drive AP state failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ConfigureWiFiAp()
{
    CHIP_ERROR ret = CHIP_NO_ERROR;
    GAutoPtr<GError> err;
    GVariant * args = nullptr;
    GVariantBuilder builder;

    uint16_t channel       = 1;
    uint16_t discriminator = 0;
    char ssid[32];

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    channel = ConnectivityUtils::MapChannelToFrequency(kWiFi_BAND_2_4_GHZ, CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL);

    if (GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator) != CHIP_NO_ERROR)
        discriminator = 0;

    snprintf(ssid, 32, "%s%04u", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, discriminator);

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "ConfigureWiFiAP, ssid: %s, channel: %d", ssid, channel);

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssid));
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("NONE"));
    g_variant_builder_add(&builder, "{sv}", "mode", g_variant_new_int32(2));
    g_variant_builder_add(&builder, "{sv}", "frequency", g_variant_new_int32(channel));
    args = g_variant_builder_end(&builder);

    if (wpa_supplicant_1_interface_call_add_network_sync(mWpaSupplicant.iface.get(), args,
                                                         &mWpaSupplicant.networkPath.GetReceiver(), nullptr, &err.GetReceiver()))
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "added network: SSID: %s: %s", ssid,
                        mWpaSupplicant.networkPath.get());

        GAutoPtr<GError> error;
        if (wpa_supplicant_1_interface_call_select_network_sync(mWpaSupplicant.iface.get(), mWpaSupplicant.networkPath.get(),
                                                                nullptr, &error.GetReceiver()))
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "succeeded to start softAP: SSID: %s", ssid);
        }
        else
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to start softAP: SSID: %s: %s", ssid,
                            error ? error->message : "unknown error");
            ret = CHIP_ERROR_INTERNAL;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "failed to add network: %s: %s", ssid,
                        err ? err->message : "unknown error");
        mWpaSupplicant.networkPath.reset();
        ret = CHIP_ERROR_INTERNAL;
    }

    return ret;
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ChangeWiFiApState(const ConnectivityManager::WiFiAPState & newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", ConnectivityManager::WiFiAPStateToStr(mWiFiAPState),
                        ConnectivityManager::WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::DriveApState(::chip::System::Layer * aLayer, void * aAppState)
{
    reinterpret_cast<ConnectivityManagerImpl_NetworkManagementWpaSupplicant *>(aAppState)->DriveApState();
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ConnectWiFiNetworkAsyncLocked(
    GVariant * inArguments, NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) noexcept
{
    GAutoPtr<GVariant> argsDeleter(g_variant_ref_sink(inArguments)); // args may be floating, ensure we don't leak it
    GAutoPtr<GError> err;

    VerifyOrReturnError(WpaSupplicantClient::IsWiFiInterfaceEnabled(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "WiFi interface is disabled (blocked)"));

    const char * networkPath = wpa_supplicant_1_interface_get_current_network(mWpaSupplicant.iface.get());
    // wpa_supplicant DBus API: if network path of current network is not "/", means we have already selected some network.
    if (networkPath != nullptr && strcmp(networkPath, "/") != 0)
    {
        if (!wpa_supplicant_1_interface_call_remove_network_sync(mWpaSupplicant.iface.get(), networkPath, nullptr,
                                                                 &err.GetReceiver()))
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to stop AP mode with error: %s", err->message);
            return CHIP_ERROR_INTERNAL;
        }

        if (mWpaSupplicant.networkPath)
        {
            ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Removed network: %s", mWpaSupplicant.networkPath.get());
            mWpaSupplicant.networkPath.reset();
        }
    }

    OnWiFiMediumAvailable(*this, false);

    if (!wpa_supplicant_1_interface_call_add_network_sync(mWpaSupplicant.iface.get(), inArguments,
                                                          &mWpaSupplicant.networkPath.GetReceiver(), nullptr, &err.GetReceiver()))
    {
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to add network: %s", err->message);
        mWpaSupplicant.networkPath.reset();
        OnWiFiMediumAvailable(*this, true);
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Added network: %s", mWpaSupplicant.networkPath.get());

    // Note: wpa_supplicant will return immediately if the network is already connected, but it will still try reconnect in the
    // background. The client still need to wait for a few seconds for this reconnect operation. So we always disconnect from
    // the network we are connected and ignore any errors.
    wpa_supplicant_1_interface_call_disconnect_sync(mWpaSupplicant.iface.get(), nullptr, nullptr);

    mAssociationRetriesLeft = kWpaAssocMaxRetries;
    if (!wpa_supplicant_1_interface_call_select_network_sync(mWpaSupplicant.iface.get(), mWpaSupplicant.networkPath.get(), nullptr,
                                                             &err.GetReceiver()))
    {
        ChipLogError(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to select network: %s", err->message);
        return CHIP_ERROR_INTERNAL;
    }

    // Network was provisioned successfully - emit a connectivity change event so the application can update its state.
    WpaSupplicantClient::NotifyWiFiConnectivityChange(kConnectivity_NoChange);

    mConnectivityManagerImpl->SetOneShotConnectCallback(inConnectCallback);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ConnectWiFiNetworkAsync(
    ByteSpan ssid, ByteSpan credentials, NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback)
{
    char ssidStr[kMaxWiFiSSIDLength + 1] = { 0 };
    char keyStr[kMaxWiFiKeyLength + 1]   = { 0 };

    VerifyOrReturnError(ssid.size() <= kMaxWiFiSSIDLength, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(credentials.size() <= kMaxWiFiKeyLength, CHIP_ERROR_INVALID_ARGUMENT);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    // There is another ongoing connect request, reject the new one.
    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationConnecting(), CHIP_ERROR_INCORRECT_STATE);

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    memcpy(ssidStr, ssid.data(), ssid.size());
    memcpy(keyStr, credentials.data(), credentials.size());
    g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssidStr));
    g_variant_builder_add(&builder, "{sv}", "psk", g_variant_new_string(keyStr));
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("SAE WPA-PSK"));
    GVariant * args = g_variant_builder_end(&builder);
    return ConnectWiFiNetworkAsyncLocked(args, connectCallback);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::ConnectWiFiNetworkWithPDCAsync(
    ByteSpan ssid, ByteSpan networkIdentity, ByteSpan clientIdentity, const Crypto::P256Keypair & clientIdentityKeypair,
    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    VerifyOrReturnError(mConnectivityManagerImpl != nullptr, CHIP_ERROR_UNINITIALIZED);

    VerifyOrReturnError(ssid.size() <= kMaxWiFiSSIDLength, CHIP_ERROR_INVALID_ARGUMENT);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    // There is another ongoing connect request, reject the new one.
    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationConnecting(), CHIP_ERROR_INCORRECT_STATE);

    // Convert identities and our key pair to DER and add them to wpa_supplicant as blobs
    {
        constexpr size_t bufferSize = std::max(kMaxDERCertLength, kP256ECPrivateKeyDERLength);
        Platform::ScopedMemoryBuffer<uint8_t> buffer;
        VerifyOrReturnError(buffer.Alloc(bufferSize), CHIP_ERROR_NO_MEMORY);

        MutableByteSpan networkIdentityDER(buffer.Get(), bufferSize);
        ReturnErrorOnFailure(ConvertChipCertToX509Cert(networkIdentity, networkIdentityDER));
        ReturnErrorOnFailure(AddOrReplaceBlob(mWpaSupplicant.iface.get(), kNetworkIdentityBlobRef, networkIdentityDER));

        MutableByteSpan clientIdentityDER(buffer.Get(), bufferSize);
        ReturnErrorOnFailure(ConvertChipCertToX509Cert(clientIdentity, clientIdentityDER));
        ReturnErrorOnFailure(AddOrReplaceBlob(mWpaSupplicant.iface.get(), kClientIdentityBlobRef, clientIdentityDER));

        Crypto::P256SerializedKeypair serializedKeypair;
        MutableByteSpan clientIdentityKeypairDER(buffer.Get(), bufferSize);
        ReturnErrorOnFailure(clientIdentityKeypair.Serialize(serializedKeypair));
        ReturnErrorOnFailure(ConvertECDSAKeypairRawToDER(serializedKeypair, clientIdentityKeypairDER));
        ReturnErrorOnFailure(AddOrReplaceBlob(mWpaSupplicant.iface.get(), kClientIdentityKeyBlobRef, clientIdentityKeypairDER));
    }

    // Build the network configuration
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    {
        char ssidStr[kMaxWiFiSSIDLength + 1] = { 0 };
        memcpy(ssidStr, ssid.data(), ssid.size());
        g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssidStr));
    }

    {
        CertificateKeyIdStorage keyId;
        ReturnErrorOnFailure(ExtractIdentifierFromChipNetworkIdentity(networkIdentity, keyId));

        static constexpr char kNAIDomain[] = ".pdc.csa-iot.org";
        static constexpr auto keyIdHexSize = keyId.size() * 2;
        char identityStr[1 + keyIdHexSize + sizeof(kNAIDomain)]; // sizeof(kNAIDomain) includes null terminator

        identityStr[0] = '@';
        ReturnErrorOnFailure(Encoding::BytesToUppercaseHexBuffer(keyId.data(), keyId.size(), &identityStr[1], keyIdHexSize));
        strcpy(&identityStr[1 + keyIdHexSize], kNAIDomain);
        g_variant_builder_add(&builder, "{sv}", "identity", g_variant_new_string(identityStr));
    }

    // The configuration will become simpler once we add explicit Matter support to wpa_supplicant
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("WPA-EAP-SHA256"));
    g_variant_builder_add(&builder, "{sv}", "fallback_key_mgmt", g_variant_new_string("WPA-EAP-SHA256"));
    g_variant_builder_add(&builder, "{sv}", "pairwise", g_variant_new_string("CCMP"));
    g_variant_builder_add(&builder, "{sv}", "group", g_variant_new_string("CCMP"));
    g_variant_builder_add(&builder, "{sv}", "ieee80211w", g_variant_new_int32(2));
    g_variant_builder_add(&builder, "{sv}", "eap", g_variant_new_string("TLS"));
    g_variant_builder_add(&builder, "{sv}", "eap_workaround", g_variant_new_int32(0));

    g_variant_builder_add(
        &builder, "{sv}", "phase1",
        g_variant_new_string("tls_disable_tlsv1_0=1,tls_disable_tlsv1_1=1,tls_disable_tlsv1_2=1,tls_disable_tlsv1_3=0"));
    g_variant_builder_add(&builder, "{sv}", "openssl_ciphers", g_variant_new_string("TLS_AES_128_CCM_SHA256"));
    g_variant_builder_add(&builder, "{sv}", "openssl_ecdh_curves", g_variant_new_string("P-256"));

    g_variant_builder_add(&builder, "{sv}", "ca_cert", g_variant_new_string(kNetworkIdentityBlobRef));
    g_variant_builder_add(&builder, "{sv}", "client_cert", g_variant_new_string(kClientIdentityBlobRef));
    g_variant_builder_add(&builder, "{sv}", "private_key", g_variant_new_string(kClientIdentityKeyBlobRef));
    GVariant * args = g_variant_builder_end(&builder);
    return ConnectWiFiNetworkAsyncLocked(args, connectCallback);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::PostNetworkConnect()
{
    CharSpan wiFiIfName;
    CHIP_ERROR status;

    status = WpaSupplicantClient::GetIfName(wiFiIfName);
    ReturnOnFailure(status);

    // Iterate on the network interface to see if we already have beed assigned addresses.
    // The temporary hack for getting IP address change on linux for network provisioning in the rendezvous session.
    // This should be removed or find a better place once we deprecate the rendezvous session.
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)) &&
            strncmp(ifName, wiFiIfName.data(), sizeof(ifName)) == 0)
        {
            chip::Inet::IPAddress addr;
            if (it.GetAddress(addr) != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to got IP address on interface: %s", ifName);
                continue;
            }
            ChipDeviceEvent event{ .Type                       = DeviceEventType::kInternetConnectivityChange,
                                   .InternetConnectivityChange = { .ipAddress = addr } };

            if (addr.IsIPv4())
            {
                event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
                event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
            }
            else if (addr.IsIPv6())
            {
                event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
                event.InternetConnectivityChange.IPv6 = kConnectivity_Established;
            }

            char ipStrBuf[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
            addr.ToString(ipStrBuf);
            ChipLogDetail(DeviceLayer, "Got IP address on interface: %s IP: %s", ifName, ipStrBuf);
            PlatformMgr().PostEventOrDie(&event);
        }
    }

#if defined(CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD)
    // CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD can be defined to a command pattern
    // to run once the network has been connected, with a %s placeholder for the
    // interface name. E.g. "dhclient -nw %s"
    // Run dhclient for IP on WiFi.
    // TODO: The wifi can be managed by networkmanager on linux so we don't have to care about this.
    char cmdBuffer[128];
    sprintf(cmdBuffer, CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD, sWiFiIfName);
    int dhclientSystemRet = system(cmdBuffer);
    if (dhclientSystemRet != 0)
    {
        ChipLogError(DeviceLayer, "Failed to run dhclient, system() returns %d", dhclientSystemRet);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "dhclient is running on the %s interface.", sWiFiIfName);
    }
#endif // defined(CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD)
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::CommitConfig()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Saving config");

    GAutoPtr<GError> err;
    if (!wpa_supplicant_1_interface_call_save_config_sync(mWpaSupplicant.iface.get(), nullptr, &err.GetReceiver()))
    {
        ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Failed to save config: %s", err->message);
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Save config succeeded!");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(MutableByteSpan & value)
{
    WiFiScanResponse bssInfo;
    static_assert(kMaxHardwareAddrSize >= sizeof(bssInfo.bssid),
                  "We are assuming we can fit a BSSID in a buffer of size kMaxHardwareAddrSize");
    VerifyOrReturnError(value.size() >= sizeof(bssInfo.bssid), CHIP_ERROR_BUFFER_TOO_SMALL);

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    const char * bssPath = wpa_supplicant_1_interface_get_current_bss(mWpaSupplicant.iface.get());
    VerifyOrReturnError(bssPath != nullptr && strcmp(bssPath, "/") != 0, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(_GetBssInfo(bssPath, bssInfo));

    memcpy(value.data(), bssInfo.bssid, sizeof(bssInfo.bssid));
    value.reduce_size(sizeof(bssInfo.bssid));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiSecurityType(SecurityTypeEnum & outSecurityType)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INCORRECT_STATE);

    const char * mode = wpa_supplicant_1_interface_get_current_auth_mode(mWpaSupplicant.iface.get());
    ChipLogProgress(DeviceLayer, WPA_SUPPLICANT_CLIENT_LOG_PREFIX "Current Wi-Fi security type: %s", StringOrNullMarker(mode));
    VerifyOrReturnError(mode != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (strncmp(mode, "WPA-PSK", 7) == 0)
    {
        outSecurityType = SecurityTypeEnum::kWpa;
    }
    else if (strncmp(mode, "WPA2-PSK", 8) == 0)
    {
        outSecurityType = SecurityTypeEnum::kWpa2;
    }
    else if (strncmp(mode, "WPA2-EAP", 8) == 0)
    {
        outSecurityType = SecurityTypeEnum::kWpa2;
    }
    else if (strncmp(mode, "WPA3-PSK", 8) == 0)
    {
        outSecurityType = SecurityTypeEnum::kWpa3;
    }
    else if (strncmp(mode, "WEP", 3) == 0)
    {
        outSecurityType = SecurityTypeEnum::kWep;
    }
    else if (strncmp(mode, "NONE", 4) == 0)
    {
        outSecurityType = SecurityTypeEnum::kNone;
    }
    else if (strncmp(mode, "WPA-NONE", 8) == 0)
    {
        outSecurityType = SecurityTypeEnum::kNone;
    }
    else
    {
        outSecurityType = SecurityTypeEnum::kUnspecified;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetWiFiVersion(WiFiVersionEnum & wiFiVersion)
{
    // We don't have direct API to get the WiFi version yet, return 802.11n on Linux simulation.
    wiFiVersion = WiFiVersionEnum::kN;

    return CHIP_NO_ERROR;
}

int32_t ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetDisconnectReason()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    GAutoPtr<GError> err;

    gint errorValue = wpa_supplicant_1_interface_get_disconnect_reason(mWpaSupplicant.iface.get());
    // wpa_supplicant DBus API: DisconnectReason: The most recent IEEE 802.11 reason code for disconnect. Negative value
    // indicates locally generated disconnection.
    return errorValue;
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork)
{
    return WpaSupplicantClient::GetConfiguredNetwork(outNetwork);
}

CHIP_ERROR ConnectivityManagerImpl_NetworkManagementWpaSupplicant::StartWiFiScan(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    VerifyOrReturnError(mConnectivityManagerImpl != nullptr, CHIP_ERROR_UNINITIALIZED);
    // There is another ongoing scan request, reject the new one.
    VerifyOrReturnError(!mConnectivityManagerImpl->IsWiFiStationScanning(), CHIP_ERROR_INCORRECT_STATE);

    mConnectivityManagerImpl->SetOneShotScanCallback(callback);

    retval = WpaSupplicantClient::ScanNetwork(ssid);
    if (retval != CHIP_NO_ERROR)
    {
        mConnectivityManagerImpl->SetOneShotScanCallback(nullptr);
        return retval;
    }

    return retval;
}

// wpa_supplicant Base Class Method Overrides

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::OnWiFiMediumAvailable(WpaSupplicantClient & inOutWpaSupplicantClient,
                                                                                   bool inAvailable)
{
    NetworkManagementBasis::OnWiFiMediumAvailable(inAvailable);
}

void ConnectivityManagerImpl_NetworkManagementWpaSupplicant::UpdateWiFiNetworkStatus()
{
    Network configuredNetwork;

    VerifyOrReturn(mConnectivityManagerImpl != nullptr);
    VerifyOrReturn(IsWiFiStationEnabled());

    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    // If we have already connected to the WiFi AP, then return null to indicate a success state.
    if (IsWiFiStationConnected())
    {
        mConnectivityManagerImpl->OnStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
    }
    else
    {
        mConnectivityManagerImpl->OnStatusChange(
            Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
            MakeOptional(GetDisconnectReason()));
    }
}

} // namespace DeviceLayer
} // namespace chip
