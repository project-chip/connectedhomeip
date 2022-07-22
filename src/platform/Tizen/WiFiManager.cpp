/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <memory>
#include <wifi-manager.h>

#include "MainLoop.h"
#include "WiFiManager.h"

namespace {
static constexpr const char * __WiFiDeviceStateToStr(wifi_manager_device_state_e state)
{
    switch (state)
    {
    case WIFI_MANAGER_DEVICE_STATE_DEACTIVATED:
        return "Deactivated";
    case WIFI_MANAGER_DEVICE_STATE_ACTIVATED:
        return "Activated";
    default:
        return "(unknown)";
    }
}

static constexpr const char * __WiFiScanStateToStr(wifi_manager_scan_state_e state)
{
    switch (state)
    {
    case WIFI_MANAGER_SCAN_STATE_NOT_SCANNING:
        return "Not scanning";
    case WIFI_MANAGER_SCAN_STATE_SCANNING:
        return "Scanning";
    default:
        return "(unknown)";
    }
}

static constexpr const char * __WiFiConnectionStateToStr(wifi_manager_connection_state_e state)
{
    switch (state)
    {
    case WIFI_MANAGER_CONNECTION_STATE_FAILURE:
        return "Failure";
    case WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED:
        return "Disconnected";
    case WIFI_MANAGER_CONNECTION_STATE_ASSOCIATION:
        return "Association";
    case WIFI_MANAGER_CONNECTION_STATE_CONNECTED:
        return "Connected";
    case WIFI_MANAGER_CONNECTION_STATE_CONFIGURATION:
        return "Configuration";
    default:
        return "(unknown)";
    }
}

static constexpr const char * __WiFiIPConflictStateToStr(wifi_manager_ip_conflict_state_e state)
{
    switch (state)
    {
    case WIFI_MANAGER_IP_CONFLICT_STATE_CONFLICT_NOT_DETECTED:
        return "Removed";
    case WIFI_MANAGER_IP_CONFLICT_STATE_CONFLICT_DETECTED:
        return "Detacted";
    default:
        return "(unknown)";
    }
}

static constexpr const char * __WiFiModuleStateToStr(wifi_manager_module_state_e state)
{
    switch (state)
    {
    case WIFI_MANAGER_MODULE_STATE_DETACHED:
        return "Detached";
    case WIFI_MANAGER_MODULE_STATE_ATTACHED:
        return "Attached";
    default:
        return "(unknown)";
    }
}

static constexpr const char * __WiFiSecurityTypeToStr(wifi_manager_security_type_e type)
{
    switch (type)
    {
    case WIFI_MANAGER_SECURITY_TYPE_NONE:
        return "None";
    case WIFI_MANAGER_SECURITY_TYPE_WEP:
        return "WEP";
    case WIFI_MANAGER_SECURITY_TYPE_WPA_PSK:
        return "WPA";
    case WIFI_MANAGER_SECURITY_TYPE_WPA2_PSK:
        return "WPA2";
    case WIFI_MANAGER_SECURITY_TYPE_EAP:
        return "EAP";
    case WIFI_MANAGER_SECURITY_TYPE_WPA_FT_PSK:
        return "FT_PSK";
    case WIFI_MANAGER_SECURITY_TYPE_SAE:
        return "WPA3";
    case WIFI_MANAGER_SECURITY_TYPE_OWE:
        return "OWE";
    case WIFI_MANAGER_SECURITY_TYPE_DPP:
        return "DPP";
    default:
        return "(unknown)";
    }
}
} // namespace

namespace chip {
namespace DeviceLayer {
namespace Internal {

WiFiManager WiFiManager::sInstance;

void WiFiManager::_DeviceStateChangedCb(wifi_manager_device_state_e deviceState, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi device state changed [%s]", __WiFiDeviceStateToStr(deviceState));
    sInstance._WiFiSetDeviceState(deviceState);
}

void WiFiManager::_ModuleStateChangedCb(wifi_manager_module_state_e moduleState, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi module state changed [%s]", __WiFiModuleStateToStr(moduleState));
    sInstance._WiFiSetModuleState(moduleState);
}

void WiFiManager::_ConnectionStateChangedCb(wifi_manager_connection_state_e connectionState, wifi_manager_ap_h ap, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi connection state changed [%s]", __WiFiConnectionStateToStr(connectionState));
    sInstance._WiFiSetConnectionState(connectionState);
}

void WiFiManager::_ScanStateChangedCb(wifi_manager_scan_state_e scanState, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi scan state changed [%s]", __WiFiScanStateToStr(scanState));
}

void WiFiManager::_RssiLevelChangedCb(wifi_manager_rssi_level_e rssiLevel, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi rssi level changed [%d]", rssiLevel);
}

void WiFiManager::_BackgroundScanCb(wifi_manager_error_e wifiErr, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi background scan completed [%s]", get_error_message(wifiErr));
}

void WiFiManager::_IPConflictCb(char * mac, wifi_manager_ip_conflict_state_e ipConflictState, void * userData)
{
    ChipLogProgress(DeviceLayer, "WiFi ip conflict [%s %s]", mac, __WiFiIPConflictStateToStr(ipConflictState));
}

void WiFiManager::_ActivateCb(wifi_manager_error_e wifiErr, void * userData)
{
    auto loop = reinterpret_cast<GMainLoop *>(userData);

    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is activated");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: activate WiFi [%s]", get_error_message(wifiErr));
    }

    g_main_loop_quit(loop);
}

void WiFiManager::_DeactivateCb(wifi_manager_error_e wifiErr, void * userData)
{
    auto loop = reinterpret_cast<GMainLoop *>(userData);

    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is deactivated");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: deactivate WiFi [%s]", get_error_message(wifiErr));
    }

    g_main_loop_quit(loop);
}

void WiFiManager::_ScanFinishedCb(wifi_manager_error_e wifiErr, void * userData)
{
    auto loop                 = reinterpret_cast<GMainLoop *>(userData);
    wifi_manager_ap_h foundAp = nullptr;

    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi scan finished");

        foundAp = sInstance._WiFiGetFoundAP();
        if (foundAp != nullptr)
        {
            MainLoop::Instance().AsyncRequest(_WiFiConnect, static_cast<gpointer>(foundAp));
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: scan WiFi [%s]", get_error_message(wifiErr));
    }

    g_main_loop_quit(loop);
}

bool WiFiManager::_FoundAPCb(wifi_manager_ap_h ap, void * userData)
{
    bool cbRet                = true;
    int wifiErr               = WIFI_MANAGER_ERROR_NONE;
    char * essid              = nullptr;
    bool isPassphraseRequired = false;
    auto clonedAp             = reinterpret_cast<wifi_manager_ap_h *>(userData);

    wifiErr = wifi_manager_ap_get_essid(ap, &essid);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: get AP essid [%s]", get_error_message(wifiErr)));

    VerifyOrExit(strcmp(sInstance.mWiFiSSID, essid) == 0, );

    wifiErr = wifi_manager_ap_is_passphrase_required(ap, &isPassphraseRequired);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: get AP passphrase required [%s]", get_error_message(wifiErr)));

    if (isPassphraseRequired)
    {
        wifiErr = wifi_manager_ap_set_passphrase(ap, sInstance.mWiFiKey);
        VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                     ChipLogError(DeviceLayer, "FAIL: set AP passphrase [%s]", get_error_message(wifiErr)));
    }

    wifiErr = wifi_manager_ap_clone(clonedAp, ap);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: clone AP [%s]", get_error_message(wifiErr)));

    cbRet = false;

exit:
    memset(sInstance.mWiFiSSID, 0, sizeof(sInstance.mWiFiSSID));
    memset(sInstance.mWiFiKey, 0, sizeof(sInstance.mWiFiKey));
    g_free(essid);
    return cbRet;
}

void WiFiManager::_ConnectedCb(wifi_manager_error_e wifiErr, void * userData)
{
    auto loop = reinterpret_cast<GMainLoop *>(userData);

    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is connected");
        if (sInstance.mpConnectCallback != nullptr)
        {
            sInstance.mpConnectCallback->OnResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);
            sInstance.mpConnectCallback = nullptr;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: connect WiFi [%s]", get_error_message(wifiErr));
        if (sInstance.mpConnectCallback != nullptr)
        {
            sInstance.mpConnectCallback->OnResult(NetworkCommissioning::Status::kUnknownError, CharSpan(), 0);
            sInstance.mpConnectCallback = nullptr;
        }
    }

    g_main_loop_quit(loop);
}

bool WiFiManager::_ConfigListCb(const wifi_manager_config_h config, void * userData)
{
    int wifiErr                               = WIFI_MANAGER_ERROR_NONE;
    char * name                               = nullptr;
    wifi_manager_security_type_e securityType = WIFI_MANAGER_SECURITY_TYPE_NONE;

    wifi_manager_config_get_name(config, &name);
    wifi_manager_config_get_security_type(config, &securityType);

    wifiErr = wifi_manager_config_remove(sInstance.mWiFiManagerHandle, config);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Remove config [%s:%s]", name, __WiFiSecurityTypeToStr(securityType));
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: remove config [%s]", get_error_message(wifiErr));
    }

    g_free(name);
    return true;
}

gboolean WiFiManager::_WiFiInitialize(gpointer userData)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_initialize(&(sInstance.mWiFiManagerHandle));
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is initialized");
        sInstance._WiFiSetStates();
        sInstance._WiFiSetCallbacks();
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: initialize WiFi [%s]", get_error_message(wifiErr));
        return false;
    }

    return true;
}

void WiFiManager::_WiFiDeinitialize(void)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    _WiFiUnsetCallbacks();

    wifiErr = wifi_manager_deinitialize(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is deinitialized");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: deinitialize WiFi [%s]", get_error_message(wifiErr));
    }
}

gboolean WiFiManager::_WiFiActivate(GMainLoop * mainLoop, gpointer userData)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_activate(sInstance.mWiFiManagerHandle, _ActivateCb, mainLoop);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi activation is requested");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: request WiFi activation [%s]", get_error_message(wifiErr));
        return false;
    }

    return true;
}

gboolean WiFiManager::_WiFiDeactivate(GMainLoop * mainLoop, gpointer userData)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_deactivate(sInstance.mWiFiManagerHandle, _DeactivateCb, mainLoop);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi deactivation is requested");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: request WiFi deactivation [%s]", get_error_message(wifiErr));
        return false;
    }

    return true;
}

gboolean WiFiManager::_WiFiScan(GMainLoop * mainLoop, gpointer userData)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_scan(sInstance.mWiFiManagerHandle, _ScanFinishedCb, mainLoop);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi scan is requested");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: request WiFi scan [%s]", get_error_message(wifiErr));
        return false;
    }

    return true;
}

gboolean WiFiManager::_WiFiConnect(GMainLoop * mainLoop, gpointer userData)
{
    int wifiErr          = WIFI_MANAGER_ERROR_NONE;
    wifi_manager_ap_h ap = static_cast<wifi_manager_ap_h>(userData);

    wifiErr = wifi_manager_connect(sInstance.mWiFiManagerHandle, ap, _ConnectedCb, mainLoop);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi connect is requested");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: request WiFi connect [%s]", get_error_message(wifiErr));
        wifi_manager_ap_destroy(ap);
        return false;
    }

    wifi_manager_ap_destroy(ap);
    return true;
}

void WiFiManager::_WiFiSetStates(void)
{
    int wifiErr          = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated = false;

    wifiErr = wifi_manager_is_activated(mWiFiManagerHandle, &isWiFiActivated);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        mDeviceState = isWiFiActivated ? WIFI_MANAGER_DEVICE_STATE_ACTIVATED : WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;
        ChipLogProgress(DeviceLayer, "Set WiFi device state [%s]", __WiFiDeviceStateToStr(mDeviceState));
    }

    wifiErr = wifi_manager_get_module_state(mWiFiManagerHandle, &mModuleState);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi module state [%s]", __WiFiModuleStateToStr(mModuleState));
    }

    wifiErr = wifi_manager_get_connection_state(mWiFiManagerHandle, &mConnectionState);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi connection state [%s]", __WiFiConnectionStateToStr(mConnectionState));
    }
}

void WiFiManager::_WiFiSetCallbacks(void)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_set_device_state_changed_cb(mWiFiManagerHandle, _DeviceStateChangedCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi device state changed callback");
    }

    wifiErr = wifi_manager_set_module_state_changed_cb(mWiFiManagerHandle, _ModuleStateChangedCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi module state changed callback");
    }

    wifiErr = wifi_manager_set_connection_state_changed_cb(mWiFiManagerHandle, _ConnectionStateChangedCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi connection state changed callback");
    }

    wifiErr = wifi_manager_set_scan_state_changed_cb(mWiFiManagerHandle, _ScanStateChangedCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi scan state changed callback");
    }

    wifiErr = wifi_manager_set_rssi_level_changed_cb(mWiFiManagerHandle, _RssiLevelChangedCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi rssi level changed callback");
    }

    wifiErr = wifi_manager_set_background_scan_cb(mWiFiManagerHandle, _BackgroundScanCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi background scan callback");
    }

    wifiErr = wifi_manager_set_ip_conflict_cb(mWiFiManagerHandle, _IPConflictCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set WiFi IP conflict callback");
    }
}

void WiFiManager::_WiFiUnsetCallbacks(void)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_unset_device_state_changed_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi device state changed callback");
    }

    wifiErr = wifi_manager_unset_module_state_changed_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi module state changed callback");
    }

    wifiErr = wifi_manager_unset_connection_state_changed_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi connection state changed callback");
    }

    wifiErr = wifi_manager_unset_scan_state_changed_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi scan state changed callback");
    }

    wifiErr = wifi_manager_unset_rssi_level_changed_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi rssi level changed callback");
    }

    wifiErr = wifi_manager_unset_background_scan_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi background scan callback");
    }

    wifiErr = wifi_manager_unset_ip_conflict_cb(mWiFiManagerHandle);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset WiFi IP conflict callback");
    }
}

void WiFiManager::_WiFiSetDeviceState(wifi_manager_device_state_e deviceState)
{
    mDeviceState = deviceState;
    ChipLogProgress(DeviceLayer, "Set WiFi device state [%s]", __WiFiDeviceStateToStr(mDeviceState));
}

void WiFiManager::_WiFiSetModuleState(wifi_manager_module_state_e moduleState)
{
    mModuleState = moduleState;
    ChipLogProgress(DeviceLayer, "Set WiFi module state [%s]", __WiFiModuleStateToStr(mModuleState));
}

void WiFiManager::_WiFiSetConnectionState(wifi_manager_connection_state_e connectionState)
{
    mConnectionState = connectionState;
    ChipLogProgress(DeviceLayer, "Set WiFi connection state [%s]", __WiFiConnectionStateToStr(mConnectionState));
}

wifi_manager_ap_h WiFiManager::_WiFiGetFoundAP(void)
{
    int wifiErr               = WIFI_MANAGER_ERROR_NONE;
    wifi_manager_ap_h foundAp = nullptr;

    wifiErr = wifi_manager_foreach_found_ap(mWiFiManagerHandle, _FoundAPCb, &foundAp);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Get found AP list finished");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: get found AP list [%s]", get_error_message(wifiErr));
    }

    return foundAp;
}

void WiFiManager::Init(void)
{
    sInstance.mDeviceState     = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;
    sInstance.mModuleState     = WIFI_MANAGER_MODULE_STATE_DETACHED;
    sInstance.mConnectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;

    MainLoop::Instance().Init(_WiFiInitialize);
}

void WiFiManager::Deinit(void)
{
    sInstance._WiFiDeinitialize();
    MainLoop::Instance().Deinit();
}

CHIP_ERROR WiFiManager::IsActivated(bool * isWiFiActivated)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int wifiErr    = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, isWiFiActivated);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is %s", *isWiFiActivated ? "activated" : "deactivated");
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr));
    }

    return err;
}

CHIP_ERROR WiFiManager::Activate(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    int wifiErr          = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated = false;
    bool dbusAsyncErr    = false;

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, &isWiFiActivated);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr)));

    VerifyOrExit(isWiFiActivated == false, ChipLogProgress(DeviceLayer, "WiFi is already activated"));

    dbusAsyncErr = MainLoop::Instance().AsyncRequest(_WiFiActivate);
    if (dbusAsyncErr == false)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return err;
}

CHIP_ERROR WiFiManager::Deactivate(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    int wifiErr          = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated = false;
    bool dbusAsyncErr    = false;

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, &isWiFiActivated);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr)));

    VerifyOrExit(isWiFiActivated == true, ChipLogProgress(DeviceLayer, "WiFi is already deactivated"));

    dbusAsyncErr = MainLoop::Instance().AsyncRequest(_WiFiDeactivate);
    if (dbusAsyncErr == false)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return err;
}

CHIP_ERROR WiFiManager::Connect(const char * ssid, const char * key,
                                DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    int wifiErr               = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated      = false;
    bool dbusAsyncErr         = false;
    wifi_manager_ap_h foundAp = nullptr;

    g_strlcpy(sInstance.mWiFiSSID, ssid, sizeof(sInstance.mWiFiSSID));
    g_strlcpy(sInstance.mWiFiKey, key, sizeof(sInstance.mWiFiKey));

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, &isWiFiActivated);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr)));

    VerifyOrExit(isWiFiActivated == true, ChipLogProgress(DeviceLayer, "WiFi is deactivated"));

    sInstance.mpConnectCallback = apCallback;

    foundAp = sInstance._WiFiGetFoundAP();
    if (foundAp != nullptr)
    {
        dbusAsyncErr = MainLoop::Instance().AsyncRequest(_WiFiConnect, static_cast<gpointer>(foundAp));
        if (dbusAsyncErr == false)
        {
            err = CHIP_ERROR_INCORRECT_STATE;
        }
    }
    else
    {
        dbusAsyncErr = MainLoop::Instance().AsyncRequest(_WiFiScan);
        if (dbusAsyncErr == false)
        {
            err = CHIP_ERROR_INCORRECT_STATE;
        }
    }

exit:
    return err;
}

CHIP_ERROR WiFiManager::Disconnect(const char * ssid)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    int wifiErr               = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated      = false;
    wifi_manager_ap_h foundAp = nullptr;

    g_strlcpy(sInstance.mWiFiSSID, ssid, sizeof(sInstance.mWiFiSSID));

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, &isWiFiActivated);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr)));

    VerifyOrExit(isWiFiActivated == true, ChipLogProgress(DeviceLayer, "WiFi is deactivated"));

    foundAp = sInstance._WiFiGetFoundAP();
    VerifyOrExit(foundAp != nullptr, );

    wifiErr = wifi_manager_forget_ap(sInstance.mWiFiManagerHandle, foundAp);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is disconnected");
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(DeviceLayer, "FAIL: disconnect WiFi [%s]", get_error_message(wifiErr));
    }

    wifi_manager_ap_destroy(foundAp);

exit:
    return err;
}

CHIP_ERROR WiFiManager::RemoveAllConfigs(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int wifiErr    = WIFI_MANAGER_ERROR_NONE;

    wifiErr = wifi_manager_config_foreach_configuration(sInstance.mWiFiManagerHandle, _ConfigListCb, nullptr);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Get config list finished");
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(DeviceLayer, "FAIL: get config list [%s]", get_error_message(wifiErr));
    }

    return err;
}

CHIP_ERROR WiFiManager::GetDeviceMACAddress(uint8_t * macAddress, size_t macAddressLen)
{
    VerifyOrReturnError(macAddress != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(macAddressLen >= 6, CHIP_ERROR_INVALID_ARGUMENT);

    char * macAddrStr = nullptr;
    // Make sure that string allocated by wifi_manager_get_mac_address() will be freed
    std::unique_ptr<char, decltype(&::free)> _{ macAddrStr, &::free };

    int wifiErr = wifi_manager_get_mac_address(sInstance.mWiFiManagerHandle, &macAddrStr);
    if (wifiErr == WIFI_MANAGER_ERROR_NOT_SUPPORTED)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (wifiErr != WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "FAIL: get MAC address [%s]", get_error_message(wifiErr));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Parse MAC address
    if (sscanf(macAddrStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &macAddress[0], &macAddress[1], &macAddress[2], &macAddress[3],
               &macAddress[4], &macAddress[5]) != 6)
    {
        ChipLogError(DeviceLayer, "FAIL: parse MAC address");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::GetDeviceState(wifi_manager_device_state_e * deviceState)
{
    *deviceState = sInstance.mDeviceState;
    ChipLogProgress(DeviceLayer, "Get WiFi device state [%s]", __WiFiDeviceStateToStr(*deviceState));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::SetDeviceState(wifi_manager_device_state_e deviceState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(sInstance.mDeviceState != deviceState, );

    if (deviceState == WIFI_MANAGER_DEVICE_STATE_DEACTIVATED)
    {
        err = Deactivate();
    }
    else
    {
        err = Activate();
    }

exit:
    return err;
}

CHIP_ERROR WiFiManager::GetModuleState(wifi_manager_module_state_e * moduleState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int wifiErr    = WIFI_MANAGER_ERROR_NONE;

    if (sInstance.mModuleState != WIFI_MANAGER_MODULE_STATE_DETACHED)
    {
        *moduleState = sInstance.mModuleState;
        ChipLogProgress(DeviceLayer, "Get WiFi module state [%s]", __WiFiModuleStateToStr(*moduleState));
    }
    else
    {
        wifiErr = wifi_manager_get_module_state(sInstance.mWiFiManagerHandle, moduleState);
        if (wifiErr == WIFI_MANAGER_ERROR_NONE)
        {
            ChipLogProgress(DeviceLayer, "Get WiFi module state [%s]", __WiFiModuleStateToStr(*moduleState));
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            ChipLogError(DeviceLayer, "FAIL: get WiFi module state [%s]", get_error_message(wifiErr));
        }
    }

    return err;
}

CHIP_ERROR WiFiManager::GetConnectionState(wifi_manager_connection_state_e * connectionState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int wifiErr    = WIFI_MANAGER_ERROR_NONE;

    if (sInstance.mConnectionState != WIFI_MANAGER_CONNECTION_STATE_FAILURE)
    {
        *connectionState = sInstance.mConnectionState;
        ChipLogProgress(DeviceLayer, "Get WiFi connection state [%s]", __WiFiConnectionStateToStr(*connectionState));
    }
    else
    {
        wifiErr = wifi_manager_get_connection_state(sInstance.mWiFiManagerHandle, connectionState);
        if (wifiErr == WIFI_MANAGER_ERROR_NONE)
        {
            ChipLogProgress(DeviceLayer, "Get WiFi connection state [%s]", __WiFiConnectionStateToStr(*connectionState));
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            ChipLogError(DeviceLayer, "FAIL: get WiFi connection state [%s]", get_error_message(wifiErr));
        }
    }

    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
