/* See Project CHIP LICENSE file for licensing information. */

#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <algorithm>

#include "esp_wifi.h"

#include "ServiceProvisioning.h"

using namespace ::chip::DeviceLayer;

CHIP_ERROR SetWiFiStationProvisioning(const char * ssid, const char * key)
{
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);

    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, ssid, std::min(strlen(ssid) + 1, sizeof(wifiConfig.sta.ssid)));
    memcpy(wifiConfig.sta.password, key, std::min(strlen(key) + 1, sizeof(wifiConfig.sta.password)));
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

    // Configure the ESP WiFi interface.
    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);

exit:
    return err;
}
