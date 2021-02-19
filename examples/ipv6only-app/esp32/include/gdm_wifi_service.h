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

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "gdm_service/gdm_wifi_base_rpc.rpc.pb.h"
#include "pw_status/status.h"
#include "pw_status/try.h"

namespace chip {
namespace rpc {

class GDMWifiBase final : public generated::GDMWifiBase<GDMWifiBase>
{
public:
    // Singleton
    static GDMWifiBase & Instance() { return instance_; }

    // Initalize the wifi station
    pw::Status Init();

    // Blocks the calling thread until wifi connection is completed succesfully.
    // NOTE: Currently only supports blocking a single thread.
    void BlockUntilWifiConnected() { xSemaphoreTake(wifi_connected_semaphore_, portMAX_DELAY); }

    // The following functions are the RPC handlers

    pw::Status GetChannel(ServerContext &, const chip_rpc_Empty & request, chip_rpc_Channel & response)
    {
        uint8_t channel = 0;
        wifi_second_chan_t second;
        PW_TRY(EspToPwStatus(esp_wifi_get_channel(&channel, &second)));
        response.channel = channel;
        return pw::OkStatus();
    }

    pw::Status GetSsid(ServerContext &, const chip_rpc_Empty & request, chip_rpc_Ssid & response)
    {
        wifi_config_t config;
        PW_TRY(EspToPwStatus(esp_wifi_get_config(ESP_IF_WIFI_STA, &config)));
        size_t size = std::min(sizeof(response.ssid.bytes), sizeof(config.sta.ssid));
        memcpy(response.ssid.bytes, config.sta.ssid, sizeof(response.ssid.bytes));
        response.ssid.size = size;
        return pw::OkStatus();
    }

    pw::Status GetState(ServerContext &, const chip_rpc_Empty & request, chip_rpc_State & response)
    {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        PW_TRY(EspToPwStatus(err));
        response.connected = (err != ESP_ERR_WIFI_NOT_CONNECT);
        return pw::OkStatus();
    }

    pw::Status GetMacAddress(ServerContext &, const chip_rpc_Empty & request, chip_rpc_MacAddress & response)
    {
        uint8_t mac[6];
        PW_TRY(EspToPwStatus(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac)));
        sprintf(response.mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return pw::OkStatus();
    }

    pw::Status GetWiFiInterface(ServerContext &, const chip_rpc_Empty & request, chip_rpc_WiFiInterface & response)
    {
        wifi_ap_record_t ap_info;
        PW_TRY(EspToPwStatus(esp_wifi_sta_get_ap_info(&ap_info)));
        sprintf(response.interface, "STA");
        return pw::OkStatus();
    }

    pw::Status GetIP4Address(ServerContext &, const chip_rpc_Empty & request, chip_rpc_IP4Address & response)
    {
        esp_netif_ip_info_t ip_info;
        PW_TRY(EspToPwStatus(esp_netif_get_ip_info(esp_netif_, &ip_info)));
        sprintf(response.address, IPSTR, IP2STR(&ip_info.ip));
        return pw::OkStatus();
    }

    pw::Status GetIP6Address(ServerContext &, const chip_rpc_Empty & request, chip_rpc_IP6Address & response)
    {
        esp_ip6_addr_t ip6{ 0 };
        PW_TRY(EspToPwStatus(esp_netif_get_ip6_linklocal(esp_netif_, &ip6)));
        sprintf(response.address, IPV6STR, IPV62STR(ip6));
        return pw::OkStatus();
    }

    // NOTE: Currently this is blocking, it can be made non-blocking if needed
    //       but would require another worker thread to handle the scanning.
    void StartScan(ServerContext &, const chip_rpc_ScanConfig & request, ServerWriter<chip_rpc_ScanResults> & writer);

    pw::Status StopScan(ServerContext &, const chip_rpc_Empty & request, chip_rpc_Empty & response)
    {
        esp_wifi_scan_stop();
        return pw::OkStatus();
    }

    pw::Status Connect(ServerContext &, const chip_rpc_ConnectionData & request, chip_rpc_ConnectionResult & response);

    pw::Status Disconnect(ServerContext &, const chip_rpc_Empty & request, chip_rpc_Empty & response)
    {
        PW_TRY(EspToPwStatus(esp_wifi_disconnect()));
        return pw::OkStatus();
    }

private:
    static GDMWifiBase instance_;
    esp_netif_t * esp_netif_ = nullptr;
    SemaphoreHandle_t wifi_connected_semaphore_;

    static constexpr pw::Status EspToPwStatus(esp_err_t err)
    {
        switch (err)
        {
        case ESP_OK:
            return pw::OkStatus();
        case ESP_ERR_WIFI_NOT_INIT:
            return pw::Status::FailedPrecondition();
        case ESP_ERR_INVALID_ARG:
            return pw::Status::InvalidArgument();
        case ESP_ERR_ESP_NETIF_INVALID_PARAMS:
            return pw::Status::InvalidArgument();
        case ESP_ERR_WIFI_IF:
            return pw::Status::NotFound();
        case ESP_ERR_WIFI_NOT_CONNECT:
            return pw::Status::FailedPrecondition();
        case ESP_ERR_WIFI_NOT_STARTED:
            return pw::Status::FailedPrecondition();
        case ESP_ERR_WIFI_CONN:
            return pw::Status::Internal();
        case ESP_FAIL:
            return pw::Status::Internal();
        default:
            return pw::Status::Unknown();
        }
    }

    static void WifiEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);
};

} // namespace rpc
} // namespace chip
