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

#include "WiFiManager.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include <tizen.h>
#include <wifi-manager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformManager.h>

#include "ErrorUtils.h"
#include "NetworkCommissioningDriver.h"

using namespace ::chip::DeviceLayer::NetworkCommissioning;

namespace {

constexpr const char * __WiFiDeviceStateToStr(wifi_manager_device_state_e state)
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

constexpr const char * __WiFiScanStateToStr(wifi_manager_scan_state_e state)
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

constexpr const char * __WiFiConnectionStateToStr(wifi_manager_connection_state_e state)
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

constexpr const char * __WiFiIPConflictStateToStr(wifi_manager_ip_conflict_state_e state)
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

constexpr const char * __WiFiModuleStateToStr(wifi_manager_module_state_e state)
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

constexpr const char * __WiFiSecurityTypeToStr(wifi_manager_security_type_e type)
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

// wifi_manager's scan results don't contains the channel infomation, so we need this lookup table for resolving the band and
// channel infomation.
constexpr std::pair<WiFiBand, int> _GetBandAndChannelFromFrequency(int freq)
{
    std::pair<WiFiBand, int> ret = std::make_pair(WiFiBand::k2g4, 0);
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

constexpr uint8_t _GetNetworkSecurityType(wifi_manager_security_type_e type)
{
    switch (type)
    {
    case WIFI_MANAGER_SECURITY_TYPE_NONE:
        return 0x1;
    case WIFI_MANAGER_SECURITY_TYPE_WEP:
        return 0x2;
    case WIFI_MANAGER_SECURITY_TYPE_WPA_PSK:
        return 0x4;
    case WIFI_MANAGER_SECURITY_TYPE_WPA2_PSK:
        return 0x8;
    case WIFI_MANAGER_SECURITY_TYPE_EAP:
        return 0x10;
    case WIFI_MANAGER_SECURITY_TYPE_WPA_FT_PSK:
        return 0x10;
    case WIFI_MANAGER_SECURITY_TYPE_SAE:
        return 0x10;
    case WIFI_MANAGER_SECURITY_TYPE_OWE:
        return 0x10;
    case WIFI_MANAGER_SECURITY_TYPE_DPP:
        return 0x10;
    default:
        return 0x0;
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
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is activated");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: activate WiFi [%s]", get_error_message(wifiErr));
    }
}

void WiFiManager::_DeactivateCb(wifi_manager_error_e wifiErr, void * userData)
{
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi is deactivated");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: deactivate WiFi [%s]", get_error_message(wifiErr));
    }
}

void WiFiManager::_ScanToConnectFinishedCb(wifi_manager_error_e wifiErr, void * userData)
{
    wifi_manager_ap_h foundAp = nullptr;

    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "WiFi scan finished");

        foundAp = sInstance._WiFiGetFoundAP();
        if (foundAp != nullptr)
        {
            PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiConnect, foundAp);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: scan WiFi [%s]", get_error_message(wifiErr));
    }
}

void WiFiManager::_UpdateScanCallback(void * scanCbData)
{
    auto networkScanned = static_cast<std::vector<WiFiScanResponse> *>(scanCbData);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    if (sInstance.mpScanCallback != nullptr)
    {
        TizenScanResponseIterator<WiFiScanResponse> iter(networkScanned);
        sInstance.mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
        sInstance.mpScanCallback = nullptr;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

void WiFiManager::_SpecificScanFinishedCb(wifi_manager_error_e wifiErr, void * userData)
{
    VerifyOrReturn(wifiErr == WIFI_MANAGER_ERROR_NONE,
                   ChipLogError(DeviceLayer, "FAIL: scan WiFi [%s]", get_error_message(wifiErr)));

    std::vector<WiFiScanResponse> networkScanned;
    int err = wifi_manager_foreach_found_specific_ap(sInstance.mWiFiManagerHandle, _FoundAPOnScanCb, &networkScanned);

    VerifyOrReturn(err == WIFI_MANAGER_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get scan list [%s]", get_error_message(err)));
    _UpdateScanCallback(&networkScanned);
}

void WiFiManager::_FullScanFinishedCb(wifi_manager_error_e wifiErr, void * userData)
{
    VerifyOrReturn(wifiErr == WIFI_MANAGER_ERROR_NONE,
                   ChipLogError(DeviceLayer, "FAIL: scan WiFi [%s]", get_error_message(wifiErr)));

    std::vector<WiFiScanResponse> networkScanned;
    int err = wifi_manager_foreach_found_ap(sInstance.mWiFiManagerHandle, _FoundAPOnScanCb, &networkScanned);

    VerifyOrReturn(err == WIFI_MANAGER_ERROR_NONE, ChipLogError(DeviceLayer, "FAIL: get scan list [%s]", get_error_message(err)));
    _UpdateScanCallback(&networkScanned);
}

bool WiFiManager::_FoundAPOnScanCb(wifi_manager_ap_h ap, void * userData)
{
    bool cbRet  = true;
    int wifiErr = WIFI_MANAGER_ERROR_NONE;
    GAutoPtr<char> essid;
    GAutoPtr<char> bssid;
    int rssi = 0;
    int freq = 0;

    auto networkScanned = static_cast<std::vector<WiFiScanResponse> *>(userData);
    std::pair<WiFiBand, int> bandInfo;

    wifi_manager_security_type_e type;
    WiFiScanResponse scannedAP;

    wifiErr = wifi_manager_ap_get_essid(ap, &essid.GetReceiver());
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: get AP essid [%s]", get_error_message(wifiErr)));
    ChipLogProgress(DeviceLayer, "Essid Found: %s\n", essid.get());
    scannedAP.ssidLen = static_cast<uint8_t>(std::min(strlen(essid.get()), sizeof(scannedAP.ssid)));
    memcpy(scannedAP.ssid, essid.get(), scannedAP.ssidLen);

    wifiErr = wifi_manager_ap_get_bssid(ap, &bssid.GetReceiver());
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Fail: get AP bssid [%s]", get_error_message(wifiErr)));
    memcpy(scannedAP.bssid, bssid.get(), std::min(strlen(bssid.get()), sizeof(scannedAP.bssid)));

    wifiErr = wifi_manager_ap_get_rssi(ap, &rssi);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Fail: get rssi value [%s]", get_error_message(wifiErr)));
    scannedAP.rssi = static_cast<int8_t>(rssi);

    wifiErr = wifi_manager_ap_get_security_type(ap, &type);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Fail: get AP security type [%s]", get_error_message(wifiErr)));
    scannedAP.security.SetRaw(_GetNetworkSecurityType(type));

    wifiErr = wifi_manager_ap_get_frequency(ap, &freq);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Fail: get AP frequency [%s]", get_error_message(wifiErr)));
    bandInfo           = _GetBandAndChannelFromFrequency(freq);
    scannedAP.wiFiBand = bandInfo.first;
    scannedAP.channel  = static_cast<uint16_t>(bandInfo.second);

    networkScanned->push_back(scannedAP);

exit:
    return cbRet;
}

bool WiFiManager::_FoundAPCb(wifi_manager_ap_h ap, void * userData)
{
    bool cbRet  = true;
    int wifiErr = WIFI_MANAGER_ERROR_NONE;
    GAutoPtr<char> essid;
    bool isPassphraseRequired = false;
    auto clonedAp             = reinterpret_cast<wifi_manager_ap_h *>(userData);

    wifiErr = wifi_manager_ap_get_essid(ap, &essid.GetReceiver());
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: get AP essid [%s]", get_error_message(wifiErr)));

    VerifyOrExit(strcmp(sInstance.mWiFiSSID, essid.get()) == 0, );

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

    memset(sInstance.mWiFiSSID, 0, sizeof(sInstance.mWiFiSSID));
    memset(sInstance.mWiFiKey, 0, sizeof(sInstance.mWiFiKey));
    cbRet = false;

exit:
    return cbRet;
}

void WiFiManager::_ConnectedCb(wifi_manager_error_e wifiErr, void * userData)
{
    if (wifiErr == WIFI_MANAGER_ERROR_NONE || wifiErr == WIFI_MANAGER_ERROR_ALREADY_EXISTS)
    {
        ChipLogProgress(DeviceLayer, "WiFi is connected");
        if (sInstance.mpConnectCallback != nullptr)
        {
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            sInstance.mpConnectCallback->OnResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);
            sInstance.mpConnectCallback = nullptr;
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: connect WiFi [%s]", get_error_message(wifiErr));
        if (sInstance.mpConnectCallback != nullptr)
        {
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            sInstance.mpConnectCallback->OnResult(NetworkCommissioning::Status::kUnknownError, CharSpan(), 0);
            sInstance.mpConnectCallback = nullptr;
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        }
    }
}

bool WiFiManager::_ConfigListCb(const wifi_manager_config_h config, void * userData)
{
    int wifiErr = WIFI_MANAGER_ERROR_NONE;
    GAutoPtr<char> name;
    wifi_manager_security_type_e securityType = WIFI_MANAGER_SECURITY_TYPE_NONE;

    wifi_manager_config_get_name(config, &name.GetReceiver());
    wifi_manager_config_get_security_type(config, &securityType);

    wifiErr = wifi_manager_config_remove(sInstance.mWiFiManagerHandle, config);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Remove config [%s:%s]", name.get(), __WiFiSecurityTypeToStr(securityType));
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: remove config [%s]", get_error_message(wifiErr));
    }

    return true;
}

CHIP_ERROR WiFiManager::_WiFiInitialize(gpointer userData)
{
    int wifiErr;

    wifiErr = wifi_manager_initialize(&sInstance.mWiFiManagerHandle);
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: initialize WiFi [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi is initialized");

    sInstance._WiFiSetStates();
    sInstance._WiFiSetCallbacks();

    return CHIP_NO_ERROR;
}

void WiFiManager::_WiFiDeinitialize()
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

CHIP_ERROR WiFiManager::_WiFiActivate(gpointer userData)
{
    int wifiErr;

    wifiErr = wifi_manager_activate(sInstance.mWiFiManagerHandle, _ActivateCb, nullptr);
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: request WiFi activation [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi activation is requested");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::_WiFiDeactivate(gpointer userData)
{
    int wifiErr;

    wifiErr = wifi_manager_deactivate(sInstance.mWiFiManagerHandle, _DeactivateCb, nullptr);
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: request WiFi deactivation [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi deactivation is requested");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::_WiFiScanToConnect(gpointer userData)
{
    int wifiErr;

    wifiErr = wifi_manager_scan(sInstance.mWiFiManagerHandle, _ScanToConnectFinishedCb, nullptr);
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: request WiFi scan [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi scan is requested");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::_WiFiSpecificScan(gpointer userData)
{
    int wifiErr =
        wifi_manager_scan_specific_ap(sInstance.mWiFiManagerHandle, sInstance.mInterestedSSID, _SpecificScanFinishedCb, nullptr);

    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: request WiFi scan [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi specific scan is requested");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::_WiFiFullScan(gpointer userData)
{
    int wifiErr = wifi_manager_scan(sInstance.mWiFiManagerHandle, _FullScanFinishedCb, nullptr);

    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: request WiFi scan [%s]", get_error_message(wifiErr)));

    ChipLogProgress(DeviceLayer, "WiFi full scan is requested");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::_WiFiConnect(wifi_manager_ap_h ap)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int wifiErr;

    wifiErr = wifi_manager_connect(sInstance.mWiFiManagerHandle, ap, _ConnectedCb, nullptr);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE,
                 ChipLogError(DeviceLayer, "FAIL: request WiFi connect [%s]", get_error_message(wifiErr));
                 err = CHIP_ERROR_INTERNAL);

    ChipLogProgress(DeviceLayer, "WiFi connect is requested");

exit:
    wifi_manager_ap_destroy(ap);
    return err;
}

void WiFiManager::_WiFiSetStates()
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

void WiFiManager::_WiFiSetCallbacks()
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

void WiFiManager::_WiFiUnsetCallbacks()
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

wifi_manager_ap_h WiFiManager::_WiFiGetConnectedAP()
{
    int wifiErr                   = WIFI_MANAGER_ERROR_NONE;
    wifi_manager_ap_h connectedAp = nullptr;

    wifiErr = wifi_manager_get_connected_ap(mWiFiManagerHandle, &connectedAp);
    if (wifiErr == WIFI_MANAGER_ERROR_NONE)
    {
        char * ssidStr = nullptr;
        if (connectedAp != nullptr)
        {
            wifiErr = wifi_manager_ap_get_essid(connectedAp, &ssidStr);
            if (wifiErr != WIFI_MANAGER_ERROR_NONE)
            {
                ChipLogError(DeviceLayer, "FAIL: get ssid of connected AP [%s]", get_error_message(wifiErr));
            }
        }
        ChipLogProgress(DeviceLayer, "Get connected AP [%s]", ssidStr);
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: get connected AP [%s]", get_error_message(wifiErr));
    }

    return connectedAp;
}

wifi_manager_ap_h WiFiManager::_WiFiGetFoundAP()
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

void WiFiManager::Init()
{
    sInstance.mDeviceState     = WIFI_MANAGER_DEVICE_STATE_DEACTIVATED;
    sInstance.mModuleState     = WIFI_MANAGER_MODULE_STATE_DETACHED;
    sInstance.mConnectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;

    PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiInitialize, static_cast<void *>(nullptr));
}

void WiFiManager::Deinit()
{
    sInstance._WiFiDeinitialize();
}

CHIP_ERROR WiFiManager::IsActivated(bool * isWiFiActivated)
{
    int wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, isWiFiActivated);
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "FAIL: Check whether WiFi is activated: %s", get_error_message(wifiErr)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::Activate()
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    bool isWiFiActivated = false;

    VerifyOrExit((err = IsActivated(&isWiFiActivated)) == CHIP_NO_ERROR, );
    VerifyOrExit(isWiFiActivated == false, ChipLogProgress(DeviceLayer, "WiFi is already activated"));

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiActivate, static_cast<void *>(nullptr));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR WiFiManager::Deactivate()
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    bool isWiFiActivated = false;

    VerifyOrExit((err = IsActivated(&isWiFiActivated)) == CHIP_NO_ERROR, );
    VerifyOrExit(isWiFiActivated == true, ChipLogProgress(DeviceLayer, "WiFi is already deactivated"));

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiDeactivate, static_cast<void *>(nullptr));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR WiFiManager::Connect(const char * ssid, const char * key,
                                DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    bool isWiFiActivated      = false;
    wifi_manager_ap_h foundAp = nullptr;

    g_strlcpy(sInstance.mWiFiSSID, ssid, sizeof(sInstance.mWiFiSSID));
    g_strlcpy(sInstance.mWiFiKey, key, sizeof(sInstance.mWiFiKey));

    VerifyOrExit((err = IsActivated(&isWiFiActivated)) == CHIP_NO_ERROR, );
    VerifyOrExit(isWiFiActivated == true, {
        ChipLogProgress(DeviceLayer, "WiFi is not activated");
        err = CHIP_ERROR_INCORRECT_STATE;
    });

    sInstance.mpConnectCallback = apCallback;

    foundAp = sInstance._WiFiGetFoundAP();
    if (foundAp != nullptr)
    {
        err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiConnect, foundAp);
        SuccessOrExit(err);
    }
    else
    {
        err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiScanToConnect, static_cast<void *>(nullptr));
        SuccessOrExit(err);
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

    VerifyOrExit((err = IsActivated(&isWiFiActivated)) == CHIP_NO_ERROR, );
    VerifyOrExit(isWiFiActivated == true, {
        ChipLogProgress(DeviceLayer, "WiFi is not activated");
        err = CHIP_ERROR_INCORRECT_STATE;
    });

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

CHIP_ERROR WiFiManager::StartWiFiScan(ByteSpan ssid, DeviceLayer::NetworkCommissioning::WiFiDriver::ScanCallback * callback)
{
    // If there is another ongoing scan request, reject the new one.
    VerifyOrReturnError(sInstance.mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(ssid.size() <= sizeof(sInstance.mInterestedSSID) - 1, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err       = CHIP_NO_ERROR;
    int wifiErr          = WIFI_MANAGER_ERROR_NONE;
    bool isWiFiActivated = false;

    wifiErr = wifi_manager_is_activated(sInstance.mWiFiManagerHandle, &isWiFiActivated);
    VerifyOrExit(wifiErr == WIFI_MANAGER_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE;
                 ChipLogError(DeviceLayer, "FAIL: check whether WiFi is activated [%s]", get_error_message(wifiErr)));

    VerifyOrExit(isWiFiActivated == true, ChipLogProgress(DeviceLayer, "WiFi is deactivated"));
    sInstance.mpScanCallback = callback;

    if (ssid.size() > 0)
    {
        memcpy(sInstance.mInterestedSSID, ssid.data(), ssid.size());
        sInstance.mInterestedSSID[ssid.size()] = '\0';
        err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiSpecificScan, static_cast<void *>(nullptr));
    }
    else
    {
        err = PlatformMgrImpl().GLibMatterContextInvokeSync(_WiFiFullScan, static_cast<void *>(nullptr));
    }

    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR WiFiManager::RemoveAllConfigs()
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

CHIP_ERROR WiFiManager::GetBssId(MutableByteSpan & value)
{
    VerifyOrReturnError(value.size() >= kWiFiBSSIDLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    wifi_manager_ap_h connectedAp = _WiFiGetConnectedAP();
    VerifyOrReturnError(connectedAp != nullptr, CHIP_ERROR_INCORRECT_STATE);

    GAutoPtr<char> bssIdStr;
    int wifiErr = wifi_manager_ap_get_bssid(connectedAp, &bssIdStr.GetReceiver());
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, TizenToChipError(wifiErr),
                        ChipLogError(DeviceLayer, "FAIL: Get AP BSSID: %s", get_error_message(wifiErr)));

    uint8_t * data = value.data();
    int rv = sscanf(bssIdStr.get(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
    VerifyOrReturnError(rv == kWiFiBSSIDLength, CHIP_ERROR_READ_FAILED, ChipLogError(DeviceLayer, "FAIL: Parse AP BSSID"));

    value.reduce_size(kWiFiBSSIDLength);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::GetSecurityType(wifi_manager_security_type_e * securityType)
{
    wifi_manager_ap_h connectedAp = _WiFiGetConnectedAP();
    if (connectedAp == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    wifi_manager_security_type_e secType;
    int wifiErr = wifi_manager_ap_get_security_type(connectedAp, &secType);
    if (wifiErr != WIFI_MANAGER_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "FAIL: get security type [%s]", get_error_message(wifiErr));
        return CHIP_ERROR_READ_FAILED;
    }

    *securityType = secType;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::GetConfiguredNetwork(NetworkCommissioning::Network & network)
{
    wifi_manager_ap_h connectedAp = _WiFiGetConnectedAP();
    if (connectedAp == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    GAutoPtr<char> essid;
    int wifiErr = wifi_manager_ap_get_essid(connectedAp, &essid.GetReceiver());
    VerifyOrReturnError(wifiErr == WIFI_MANAGER_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "FAIL: get essid [%s]", get_error_message(wifiErr)));
    network.networkIDLen = static_cast<uint8_t>(std::min(strlen(essid.get()), sizeof(network.networkID)));
    memcpy(network.networkID, essid.get(), network.networkIDLen);

    return CHIP_NO_ERROR;
}

bool WiFiManager::IsWiFiStationConnected()
{
    CHIP_ERROR err                                  = CHIP_NO_ERROR;
    wifi_manager_connection_state_e connectionState = WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED;
    bool isWiFiStationConnected                     = false;

    err = GetConnectionState(&connectionState);
    VerifyOrReturnError(err == CHIP_NO_ERROR, isWiFiStationConnected);

    if (connectionState == WIFI_MANAGER_CONNECTION_STATE_CONNECTED)
        isWiFiStationConnected = true;

    return isWiFiStationConnected;
}
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
