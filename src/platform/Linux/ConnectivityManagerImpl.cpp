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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <new>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#include <DBusWpa.h>
#include <DBusWpaInterface.h>
#include <DBusWpaNetwork.h>

#include <functional>
#include <memory>
#include <thread>
#include <cassert>

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

// ==================== ConnectivityManager Platform WiFi Station Interface ====================

template<typename Interface, typename Callback>
gpointer _MakeAsyncCallback(Callback && callback)
{
    return new std::function<Interface>(std::forward<Callback>(callback));
}

template<typename F, bool Free = true>
class _CallAsyncCallback;

template<typename R, typename ...ARGS, bool Free>
class _CallAsyncCallback<R(ARGS...), Free>
{
public:
    static void v(ARGS... args, gpointer user_data)
    {
        std::unique_ptr<std::function<R(ARGS...)>> callback((std::function<R(ARGS...)>*) user_data);
        // TODO(mingjiez): lock weave here
        (*callback)(args...);
        if (!Free) callback.release();
    }

};

using AsyncCallbackType = void(GObject*, GAsyncResult*);
using PropertiesChangeCallbackType = void(GDBusProxy *, GVariant *, const gchar* const *);

/*
 * The WiFi state machine, _WiFiState can only be modified in this function.
 *
 * This function is reentrancy, 
 */
void ConnectivityManagerImpl::_WiFiStateMachine()
{
    switch (_WiFiState.state)
    {
    case _WiFiState::INIT:
        if (mWiFiStationMode == kWiFiStationMode_ApplicationControlled) break;
        _WiFiState.state = _WiFiState.DBUS_CONNECTING;
        weave_dbus_fi_w1_wpa_supplicant1_proxy_new_for_bus(
            G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            "fi.w1.wpa_supplicant1",
            "/fi/w1/wpa_supplicant1",
            nullptr, /* GCancellable */
            _CallAsyncCallback<AsyncCallbackType>::v,
            _MakeAsyncCallback<AsyncCallbackType>([this] (GObject* source_object, GAsyncResult *res) {
                assert(_WiFiState.state == _WiFiState.DBUS_CONNECTING);
                GError *err = nullptr;
                _WiFiState.wpa = weave_dbus_fi_w1_wpa_supplicant1_proxy_new_for_bus_finish(res, &err);
                if (_WiFiState.wpa != nullptr && err == nullptr)
                {
                    _WiFiState.state = _WiFiState.DBUS_CONNECTED;
                }
                else
                {
                    if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create wpa_supplicant1 proxy %s", err->message);
                    else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create wpa_supplicant1 proxy");
                    _WiFiState.state = _WiFiState.DBUS_ERROR;
                }
                if (err != nullptr) g_error_free (err);
                _WiFiStateMachine();
            }));
        break;
    case _WiFiState::DBUS_CONNECTED:
        ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: connected %p", _WiFiState.wpa);
        _WiFiState.state = _WiFiState.DBUS_GETTING_INTERFACE_PATH;
        weave_dbus_fi_w1_wpa_supplicant1_call_get_interface (_WiFiState.wpa, CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, nullptr,
            _CallAsyncCallback<AsyncCallbackType>::v,
            _MakeAsyncCallback<AsyncCallbackType>([this] (GObject *source_object, GAsyncResult *res) {
                assert(_WiFiState.state == _WiFiState.DBUS_GETTING_INTERFACE_PATH);
                GError *err = nullptr;
                gboolean result = weave_dbus_fi_w1_wpa_supplicant1_call_get_interface_finish(_WiFiState.wpa, &_WiFiState.interfacePath, res, &err);
                if (result)
                {
                    ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: WiFi interface: %s", _WiFiState.interfacePath);
                    _WiFiState.state = _WiFiState.DBUS_GOT_INTERFACE_PATH;
                }
                else
                {
                    if (err != nullptr)
                        ChipLogError(DeviceLayer, "DBus wpa_supplicant: can't find interface %s: %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, err->message);
                    else
                        ChipLogError(DeviceLayer, "DBus wpa_supplicant: can't find interface %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);

                    if (_WiFiState.interfacePath != nullptr) {
                        g_free(_WiFiState.interfacePath);
                        _WiFiState.interfacePath = nullptr;
                    }
                    _WiFiState.state = _WiFiState.DBUS_NO_INTERFACE;
                }
                if (err != nullptr) g_error_free (err);
                _WiFiStateMachine();
            }));
        break;
    case _WiFiState::DBUS_NO_INTERFACE:
        {
            ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: creating interface %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);
            _WiFiState.state = _WiFiState.DBUS_CREATING_INTERFACE;
            GVariantDict *dict = g_variant_dict_new(nullptr);
            g_variant_dict_insert_value(dict, "Ifname", g_variant_new_string(CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME));
            weave_dbus_fi_w1_wpa_supplicant1_call_create_interface(
                _WiFiState.wpa,
                g_variant_dict_end(dict),
                nullptr,
                _CallAsyncCallback<AsyncCallbackType>::v,
                _MakeAsyncCallback<AsyncCallbackType>([this] (GObject *source_object, GAsyncResult *res) {
                    assert(_WiFiState.state == _WiFiState.DBUS_CREATING_INTERFACE);
                    GError *err = nullptr;
                    gboolean result = weave_dbus_fi_w1_wpa_supplicant1_call_create_interface_finish(_WiFiState.wpa, &_WiFiState.interfacePath, res, &err);
                    if (result)
                    {
                        ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: WiFi interface created: %s", _WiFiState.interfacePath);
                        _WiFiState.state = _WiFiState.DBUS_GOT_INTERFACE_PATH;
                    }
                    else
                    {
                        if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create interface %s: %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, err->message);
                        else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create interface %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);
                        _WiFiState.state = _WiFiState.DBUS_ERROR;
                    }
                    if (err != nullptr) g_error_free (err);
                    _WiFiStateMachine();
                }));
            break;
        }
    case _WiFiState::DBUS_GOT_INTERFACE_PATH:
        ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: found WiFi interface %s", _WiFiState.interfacePath);
        _WiFiState.state = _WiFiState.DBUS_GETTING_INTERFACE;
        weave_dbus_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(
            G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            "fi.w1.wpa_supplicant1",
            _WiFiState.interfacePath,
            nullptr, /* GCancellable */
            _CallAsyncCallback<AsyncCallbackType>::v,
            _MakeAsyncCallback<AsyncCallbackType>([this] (GObject* source_object, GAsyncResult *res) {
                assert(_WiFiState.state == _WiFiState.DBUS_GETTING_INTERFACE);
                GError *err = nullptr;
                _WiFiState.iface = weave_dbus_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus_finish(res, &err);
                if (_WiFiState.iface != nullptr && err == nullptr)
                {
                    g_signal_connect (
                        _WiFiState.iface,
                        "g-properties-changed",
                        G_CALLBACK((_CallAsyncCallback<PropertiesChangeCallbackType, false>::v)),
                        _MakeAsyncCallback<PropertiesChangeCallbackType>([this] (GDBusProxy *proxy, GVariant *changed_properties, const gchar* const *invalidated_properties) {
                            ChipLogProgress(DeviceLayer, "DBus wpa_supplicant interface properties changed: %s", invalidated_properties);
                            _WiFiStateMachine();
                        }));
                    _WiFiState.state = _WiFiState.DBUS_GOT_INTERFACE;
                }
                else
                {
                    if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create wpa_supplicant1 interface proxy %s: %s", _WiFiState.interfacePath, err->message);
                    else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: create wpa_supplicant1 interface proxy %s", _WiFiState.interfacePath);
                    _WiFiState.state = _WiFiState.DBUS_ERROR;
                }
                if (err != nullptr) g_error_free (err);
                _WiFiStateMachine();
            }));
        break;
    case _WiFiState::DBUS_GOT_INTERFACE:
        switch (_WiFiState.connectState) {
        case _WiFiState::WIFI_IDLE:
            {
                const gchar *state = weave_dbus_fi_w1_wpa_supplicant1_interface_get_state(_WiFiState.iface);
                if (strcmp(state, "completed") == 0) {
                    ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: WiFi already connected");
                    SetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity, true);
                    SetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity, true);
                    _WiFiState.connectState = _WiFiState::WIFI_CONNECTED;
                }
                else if (wifiProvisioned)
                {
                    _WiFiState.connectState = _WiFiState::WIFI_ADDING_NETWORK;
                    GVariantDict *wifiConfig = g_variant_dict_new(nullptr);
                    g_variant_dict_insert_value(wifiConfig, "ssid", g_variant_new_string(wifiNetworkInfo.mWiFiSSID));
                    g_variant_dict_insert_value(wifiConfig, "scan_ssid", g_variant_new_int32(1));
                    switch (wifiNetworkInfo.mWiFiSecurityType)
                    {
                        case kWiFiSecurityType_None:
                            g_variant_dict_insert_value(wifiConfig, "key_mgmt", g_variant_new_string("NONE"));
                            break;
                        case kWiFiSecurityType_WEP:
                            g_variant_dict_insert_value(wifiConfig, "key_mgmt", g_variant_new_string("NONE"));
                            break;
                        case kWiFiSecurityType_WPAPersonal:
                            g_variant_dict_insert_value(wifiConfig, "key_mgmt", g_variant_new_string("WPA-PSK WPA-PSK-SHA256 FT-PSK"));
                            break;
                        case kWiFiSecurityType_WPA2Personal:
                            g_variant_dict_insert_value(wifiConfig, "key_mgmt", g_variant_new_string("WPA-PSK WPA-PSK-SHA256 FT-PSK"));
                            break;
                        case kWiFiSecurityType_WPA2Enterprise:
                            g_variant_dict_insert_value(wifiConfig, "key_mgmt", g_variant_new_string("WPA-PSK WPA-PSK-SHA256 FT-PSK"));
                            break;
                        default:
                            _WiFiState.connectState = _WiFiState.WIFI_ERROR;
                            return;
                    }

                    if (wifiNetworkInfo.mWiFiKeyLen > 0) g_variant_dict_insert_value(wifiConfig, "psk", g_variant_new_string((const gchar*)wifiNetworkInfo.mWiFiKey));

                    weave_dbus_fi_w1_wpa_supplicant1_interface_call_add_network(
                        _WiFiState.iface,
                        g_variant_dict_end(wifiConfig),
                        nullptr,
                        _CallAsyncCallback<AsyncCallbackType>::v,
                        _MakeAsyncCallback<AsyncCallbackType>([this] (GObject* source_object, GAsyncResult *res) {
                            assert(_WiFiState.connectState == _WiFiState.WIFI_ADDING_NETWORK);
                            GError *err = nullptr;
                            gboolean result = weave_dbus_fi_w1_wpa_supplicant1_interface_call_add_network_finish(_WiFiState.iface, &_WiFiState.networkPath, res, &err);
                            if (result && err == nullptr)
                            {
                                ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: added wifi network");
                                _WiFiState.connectState = _WiFiState.WIFI_ADDED_NETWORK;
                            }
                            else
                            {
                                if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: add wifi network: %s", err->message);
                                else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: add wifi network");
                                _WiFiState.connectState = _WiFiState.WIFI_ERROR;
                            }
                            if (err != nullptr) g_error_free (err);
                            _WiFiStateMachine();
                        }));
                    ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: adding wifi network %s", wifiNetworkInfo.mWiFiSSID);
                }
                break;
            }
        case _WiFiState::WIFI_ADDED_NETWORK:
            if (wifiProvisioned)
            {
                _WiFiState.connectState = _WiFiState::WIFI_SELECTING_NETWORK;
                weave_dbus_fi_w1_wpa_supplicant1_interface_call_select_network(
                    _WiFiState.iface,
                    _WiFiState.networkPath,
                    nullptr,
                    _CallAsyncCallback<AsyncCallbackType>::v,
                    _MakeAsyncCallback<AsyncCallbackType>([this] (GObject* source_object, GAsyncResult *res) {
                        assert(_WiFiState.connectState == _WiFiState.WIFI_SELECTING_NETWORK);
                        GError *err = nullptr;
                        gboolean result = weave_dbus_fi_w1_wpa_supplicant1_interface_call_select_network_finish(_WiFiState.iface, res, &err);
                        if (result && err == nullptr)
                        {
                            ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: selected network %s", _WiFiState.networkPath);
                            _WiFiState.connectState = _WiFiState.WIFI_CONNECTING;
                        }
                        else
                        {
                            if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: select network %s: %s", _WiFiState.networkPath, err->message);
                            else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: select network %s", _WiFiState.networkPath);
                            _WiFiState.connectState = _WiFiState.WIFI_ERROR;
                        }
                        if (err != nullptr) g_error_free (err);
                        _WiFiStateMachine();
                    }));
                ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: selecting wifi network %s", _WiFiState.networkPath);
            }
            break;
        case _WiFiState::WIFI_CONNECTING:
            {
                const gchar *state = weave_dbus_fi_w1_wpa_supplicant1_interface_get_state(_WiFiState.iface);
                if (strcmp(state, "completed") == 0) {
                    ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: WiFi connected");
                    SetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity, true);
                    SetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity, true);
                    _WiFiState.connectState = _WiFiState::WIFI_CONNECTED;
                }
                break;
            }
        case _WiFiState::WIFI_CONNECTED:
            if (!wifiProvisioned)
            {
                _WiFiState.connectState == _WiFiState::WIFI_DISCONNECTING;
                weave_dbus_fi_w1_wpa_supplicant1_interface_call_disconnect(
                    _WiFiState.iface,
                    nullptr,
                    _CallAsyncCallback<AsyncCallbackType>::v,
                    _MakeAsyncCallback<AsyncCallbackType>([this] (GObject* source_object, GAsyncResult *res) {
                        assert(_WiFiState.connectState == _WiFiState.WIFI_DISCONNECTING);
                        GError *err = nullptr;
                        gboolean result = weave_dbus_fi_w1_wpa_supplicant1_interface_call_disconnect_finish(_WiFiState.iface, res, &err);
                        if (result && err == nullptr)
                        {
                            ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: disconnect");
                            _WiFiState.connectState = _WiFiState.WIFI_DISCONNECTING; // still disconnecting
                        }
                        else
                        {
                            if (err != nullptr) ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: disconnect: %s", err->message);
                            else ChipLogError(DeviceLayer, "DBus wpa_supplicant Error: disconnect");
                            _WiFiState.connectState = _WiFiState.WIFI_ERROR;
                        }
                        if (err != nullptr) g_error_free (err);
                        _WiFiStateMachine();
                    }));
                ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: disconnect");
            }
            break;
        case _WiFiState::WIFI_DISCONNECTING:
            {
                const gchar *state = weave_dbus_fi_w1_wpa_supplicant1_interface_get_state(_WiFiState.iface);
                if (strcmp(state, "completed") != 0) {
                    ChipLogProgress(DeviceLayer, "DBus wpa_supplicant: WiFi disconnected");
                    SetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity, false);
                    SetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity, false);
                    _WiFiState.connectState = _WiFiState::WIFI_IDLE;
                }
                break;
            }
        case _WiFiState::WIFI_ERROR:
            break;
        }

        switch (_WiFiState.scanState) {
        case _WiFiState::WIFI_SCANNING_IDLE:
            if (wifiScanPending)
            {

            }
            break;
        }
        break;
    case _WiFiState::DBUS_ERROR:
        break;
    }
}

void ConnectivityManagerImpl::_WiFiThread()
{
    GMainLoop *loop = g_main_loop_new (nullptr, false);
    _WiFiStateMachine();
    g_main_loop_run (loop);
    g_main_loop_unref (loop);
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        bool autoConnect = (val == kWiFiStationMode_Enabled);
        _WiFiStateMachine();
    }

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode), WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return wifiProvisioned;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    wifiProvisioned = false;
    _WiFiStateMachine();
}


// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

    {
        // TODO(mingjiez): do not use thread here
        std::thread wifiThread([this] {
            this->_WiFiThread();
        });
        wifiThread.detach();
    }

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

} // namespace DeviceLayer
} // namespace chip
