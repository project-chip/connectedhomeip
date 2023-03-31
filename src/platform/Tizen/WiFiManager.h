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

#pragma once

#include <cstddef>
#include <cstdint>

#include <glib.h>
#include <wifi-manager.h>

#include <lib/core/CHIPError.h>
#include <platform/NetworkCommissioning.h>

#include "platform/internal/DeviceNetworkInfo.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class WiFiManager
{
    friend class ConnectivityManagerImpl;

public:
    void Init();
    void Deinit();

    CHIP_ERROR IsActivated(bool * isWiFiActivated);
    CHIP_ERROR Activate();
    CHIP_ERROR Deactivate();
    CHIP_ERROR Connect(const char * ssid, const char * key,
                       NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback = nullptr);
    CHIP_ERROR Disconnect(const char * ssid);
    CHIP_ERROR RemoveAllConfigs();

    CHIP_ERROR GetDeviceMACAddress(uint8_t * macAddress, size_t macAddressLen);
    CHIP_ERROR GetDeviceState(wifi_manager_device_state_e * deviceState);
    CHIP_ERROR SetDeviceState(wifi_manager_device_state_e deviceState);
    CHIP_ERROR GetModuleState(wifi_manager_module_state_e * moduleState);
    CHIP_ERROR GetConnectionState(wifi_manager_connection_state_e * connectionState);

private:
    static void _DeviceStateChangedCb(wifi_manager_device_state_e deviceState, void * userData);
    static void _ModuleStateChangedCb(wifi_manager_module_state_e moduleState, void * userData);
    static void _ConnectionStateChangedCb(wifi_manager_connection_state_e connectionState, wifi_manager_ap_h ap, void * userData);
    static void _ScanStateChangedCb(wifi_manager_scan_state_e scanState, void * userData);
    static void _RssiLevelChangedCb(wifi_manager_rssi_level_e rssiLevel, void * userData);
    static void _BackgroundScanCb(wifi_manager_error_e wifiErr, void * userData);
    static void _IPConflictCb(char * mac, wifi_manager_ip_conflict_state_e ipConflictState, void * userData);
    static void _ActivateCb(wifi_manager_error_e wifiErr, void * userData);
    static void _DeactivateCb(wifi_manager_error_e wifiErr, void * userData);
    static void _ScanFinishedCb(wifi_manager_error_e wifiErr, void * userData);
    static bool _FoundAPCb(wifi_manager_ap_h ap, void * userData);
    static void _ConnectedCb(wifi_manager_error_e wifiErr, void * userData);
    static bool _ConfigListCb(const wifi_manager_config_h config, void * userData);

    static gboolean _WiFiInitialize(gpointer userData);
    static gboolean _WiFiActivate(GMainLoop * mainLoop, gpointer userData);
    static gboolean _WiFiDeactivate(GMainLoop * mainLoop, gpointer userData);
    static gboolean _WiFiScan(GMainLoop * mainLoop, gpointer userData);
    static gboolean _WiFiConnect(GMainLoop * mainLoop, gpointer userData);

    void _WiFiDeinitialize();
    void _WiFiSetStates();
    void _WiFiSetCallbacks();
    void _WiFiUnsetCallbacks();
    void _WiFiSetDeviceState(wifi_manager_device_state_e deviceState);
    void _WiFiSetModuleState(wifi_manager_module_state_e moduleState);
    void _WiFiSetConnectionState(wifi_manager_connection_state_e connectionState);
    wifi_manager_ap_h _WiFiGetFoundAP();

    friend WiFiManager & WiFiMgr();

    static WiFiManager sInstance;

    wifi_manager_h mWiFiManagerHandle;
    wifi_manager_device_state_e mDeviceState;
    wifi_manager_module_state_e mModuleState;
    wifi_manager_connection_state_e mConnectionState;

    char mWiFiSSID[kMaxWiFiSSIDLength + 1];
    char mWiFiKey[kMaxWiFiKeyLength + 1];

    NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;
};

inline WiFiManager & WiFiMgr()
{
    return WiFiManager::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
