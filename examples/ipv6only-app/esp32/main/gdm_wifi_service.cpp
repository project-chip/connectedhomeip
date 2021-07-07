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
#include "gdm_wifi_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "pw_containers/flat_map.h"

namespace chip {
namespace rpc {

namespace {

const char * TAG = "ipv6only";

constexpr pw::containers::FlatMap<uint8_t, uint32_t, 73> kChannelToFreqMap({ {
    { 1, 2412 },   { 2, 2417 },   { 3, 2422 },   { 4, 2427 },   { 5, 2432 },   { 6, 2437 },   { 7, 2442 },   { 8, 2447 },
    { 9, 2452 },   { 10, 2457 },  { 11, 2462 },  { 12, 2467 },  { 13, 2472 },  { 14, 2484 },  { 32, 5160 },  { 34, 5170 },
    { 36, 5180 },  { 38, 5190 },  { 40, 5200 },  { 42, 5210 },  { 44, 5220 },  { 46, 5230 },  { 48, 5240 },  { 50, 5250 },
    { 52, 5260 },  { 54, 5270 },  { 56, 5280 },  { 58, 5290 },  { 60, 5300 },  { 62, 5310 },  { 64, 5320 },  { 68, 5340 },
    { 96, 5480 },  { 100, 5500 }, { 102, 5510 }, { 104, 5520 }, { 106, 5530 }, { 108, 5540 }, { 110, 5550 }, { 112, 5560 },
    { 114, 5570 }, { 116, 5580 }, { 118, 5590 }, { 120, 5600 }, { 122, 5610 }, { 124, 5620 }, { 126, 5630 }, { 128, 5640 },
    { 132, 5660 }, { 134, 5670 }, { 136, 5680 }, { 138, 5690 }, { 140, 5700 }, { 142, 5710 }, { 144, 5720 }, { 149, 5745 },
    { 151, 5755 }, { 153, 5765 }, { 155, 5775 }, { 157, 5785 }, { 159, 5795 }, { 161, 5805 }, { 165, 5825 }, { 169, 5845 },
    { 173, 5865 }, { 183, 4915 }, { 184, 4920 }, { 185, 4925 }, { 187, 4935 }, { 188, 4940 }, { 189, 4945 }, { 192, 4960 },
    { 196, 4980 },
} });

// Class handles the event handlers needed for station startup.
// Creating the object will register all handlers needed, destroying will
// unregister. The object is only needed during initalization, after the station
// is up it is safe to destroy this object.
class WifiInitStationEventHandler
{
public:
    WifiInitStationEventHandler()
    {
        handler_context_.event_group = xEventGroupCreate();
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler, &handler_context_);
    }

    ~WifiInitStationEventHandler()
    {
        vEventGroupDelete(handler_context_.event_group);
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler);
    }

    pw::Status WaitForStationUp()
    {
        EventBits_t bits = xEventGroupWaitBits(handler_context_.event_group, kWifiStationUpBit, pdFALSE, pdFALSE, portMAX_DELAY);
        if (!(bits & kWifiStationUpBit))
        {
            return pw::Status::Unknown();
        }
        return pw::OkStatus();
    }

private:
    static constexpr uint8_t kWifiStationUpBit = BIT0;
    struct HandlerContext
    {
        size_t retry_count = 0;
        uint8_t error_code = 0;
        EventGroupHandle_t event_group;
    } handler_context_;

    static void EventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data)
    {
        HandlerContext * context = static_cast<HandlerContext *>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        {
            ESP_LOGI(TAG, "EVENT: WIFI_EVENT_STATION_START");
            xEventGroupSetBits(context->event_group, kWifiStationUpBit);
        }
    }
};

// Class handles the event handlers needed for wifi connection.
// Creating the object will register all handlers needed, destroying will
// unregister. The object is only needed during connection, once connected
// is up it is safe to destroy this object.
class WifiConnectionEventHandler
{
public:
    WifiConnectionEventHandler(esp_netif_t * esp_netif)
    {
        handler_context_.esp_netif   = esp_netif;
        handler_context_.event_group = xEventGroupCreate();
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler, &handler_context_);
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EventHandler, &handler_context_);
        esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &EventHandler, &handler_context_);
    }

    ~WifiConnectionEventHandler()
    {
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler);
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler);
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EventHandler);
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &EventHandler);
        vEventGroupDelete(handler_context_.event_group);
    }

    // Waits for the events to determine if connected succesfully.
    pw::Status WaitForConnection(chip_rpc_ConnectionResult * result)
    {
        EventBits_t bits = xEventGroupWaitBits(handler_context_.event_group, kWifiFailBit | kWifiIpv6ConnectedBit, pdFALSE, pdFALSE,
                                               portMAX_DELAY);
        if (bits & kWifiIpv6ConnectedBit)
        {
            result->error = chip_rpc_CONNECTION_ERROR_OK;
        }
        else if (bits & kWifiFailBit)
        {
            result->error = handler_context_.error_code;
            return pw::Status::Unavailable();
        }
        else
        {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
            return pw::Status::Unknown();
        }
        return pw::OkStatus();
    }

private:
    static constexpr size_t kWiFiConnectRetryMax   = 5;
    static constexpr uint8_t kWifiIpv6ConnectedBit = BIT0;
    static constexpr uint8_t kWifiIpv4ConnectedBit = BIT1;
    static constexpr uint8_t kWifiFailBit          = BIT2;

    struct HandlerContext
    {
        size_t retry_count = 0;
        enum _chip_rpc_CONNECTION_ERROR error_code;
        EventGroupHandle_t event_group;
        esp_netif_t * esp_netif;
    } handler_context_;

    static void EventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data)
    {
        HandlerContext * context = static_cast<HandlerContext *>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGI(TAG, "EVENT: WIFI_EVENT_STA_DISCONNECTED, reason: %d",
                     (static_cast<system_event_sta_disconnected_t *>(event_data))->reason);
            if (context->retry_count < kWiFiConnectRetryMax)
            {
                esp_wifi_connect();
                context->retry_count++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            }
            else
            {
                context->error_code =
                    static_cast<_chip_rpc_CONNECTION_ERROR>((static_cast<system_event_sta_disconnected_t *>(event_data))->reason);
                xEventGroupSetBits(context->event_group, kWifiFailBit);
            }
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
        {
            ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(context->esp_netif));
            ESP_LOGI(TAG, "Connected, link local address created");
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            auto * event = static_cast<ip_event_got_ip_t *>(event_data);
            ESP_LOGI(TAG, "got ip4: " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(context->event_group, kWifiIpv4ConnectedBit);
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6)
        {
            auto * event = static_cast<ip_event_got_ip6_t *>(event_data);
            ESP_LOGI(TAG, "got ip6: " IPV6STR, IPV62STR(event->ip6_info.ip));
            xEventGroupSetBits(context->event_group, kWifiIpv6ConnectedBit);
        }
    }
};

// These are potentially large objects for the scan results.
constexpr size_t kScanRecordsMax = sizeof(chip_rpc_ScanResults().aps) / sizeof(chip_rpc_ScanResult);
chip_rpc_ScanResults out_scan_records;
wifi_ap_record_t scan_records[kScanRecordsMax] = { 0 };

} // namespace

GDMWifiBase GDMWifiBase::instance_;

pw::Status GDMWifiBase::Init()
{
    wifi_connected_semaphore_ = xSemaphoreCreateBinary();
    PW_TRY(EspToPwStatus(esp_netif_init()));
    PW_TRY(EspToPwStatus(esp_event_loop_create_default()));
    esp_netif_ = esp_netif_create_default_wifi_sta();
    PW_TRY(EspToPwStatus(esp_netif_dhcpc_stop(esp_netif_)));

    WifiInitStationEventHandler event_handler;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    PW_TRY(EspToPwStatus(esp_wifi_init(&cfg)));
    PW_TRY(EspToPwStatus(esp_wifi_set_mode(WIFI_MODE_STA)));
    PW_TRY(EspToPwStatus(esp_wifi_start()));

    PW_TRY(event_handler.WaitForStationUp());
    return pw::OkStatus();
}

pw::Status GDMWifiBase::Connect(ServerContext &, const chip_rpc_ConnectionData & request, chip_rpc_ConnectionResult & response)
{
    wifi_config_t wifi_config {
        .sta = {
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	        .threshold = {
                .authmode = static_cast<wifi_auth_mode_t>(request.security_type),
            },

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
        };
    memcpy(wifi_config.sta.ssid, request.ssid.bytes,
           std::min(sizeof(wifi_config.sta.ssid), static_cast<size_t>(request.ssid.size)));
    memcpy(wifi_config.sta.password, request.secret.bytes,
           std::min(sizeof(wifi_config.sta.password), static_cast<size_t>(request.secret.size)));

    WifiConnectionEventHandler event_handler(esp_netif_);
    PW_TRY(EspToPwStatus(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config)));
    esp_err_t err = esp_wifi_connect();

    if (ESP_ERR_WIFI_SSID == err)
    {
        ESP_LOGI(TAG, "AP not found SSID:%s", wifi_config.sta.ssid);
        response.error = chip_rpc_CONNECTION_ERROR_NO_AP_FOUND;
        return pw::Status::NotFound();
    }
    if (pw::Status status = event_handler.WaitForConnection(&response); status.ok())
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
        xSemaphoreGive(wifi_connected_semaphore_);
    }
    else
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
        return status;
    }
    return pw::OkStatus();
}

void GDMWifiBase::StartScan(ServerContext &, const chip_rpc_ScanConfig & request, ServerWriter<chip_rpc_ScanResults> & writer)
{
    wifi_scan_config_t scan_config{ 0 };
    if (request.ssid_count != 0)
    {
        scan_config.ssid = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(request.ssid[0].bytes));
    }
    if (request.bssid_count != 0)
    {
        scan_config.bssid = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(request.bssid[0].bytes));
    }
    scan_config.channel     = request.channel;
    scan_config.show_hidden = request.show_hidden;
    scan_config.scan_type   = static_cast<wifi_scan_type_t>(request.active_scan);
    if (request.active_scan)
    {
        scan_config.scan_time.active.min = request.scan_time_min_ms;
        scan_config.scan_time.active.max = request.scan_time_max_ms;
    }
    else
    {
        scan_config.scan_time.passive = request.scan_time_min_ms;
    }

    auto err = esp_wifi_scan_start(&scan_config, true /* block */);
    if (ESP_OK != err)
    {
        ESP_LOGI(TAG, "Error starting scan: %d", err);
        return;
    }

    // Output scan results
    uint16_t num_scan_records = kScanRecordsMax;
    err                       = esp_wifi_scan_get_ap_records(&num_scan_records, scan_records);
    if (ESP_OK != err)
    {
        ESP_LOGI(TAG, "Error getting scanned APs: %d", err);
        num_scan_records = 0;
    }
    ESP_LOGI(TAG, "%d", num_scan_records);
    out_scan_records.aps_count = num_scan_records;

    for (size_t i = 0; i < num_scan_records; ++i)
    {
        memcpy(out_scan_records.aps[i].ssid.bytes, scan_records[i].ssid, sizeof(out_scan_records.aps[i].ssid.bytes));
        out_scan_records.aps[i].ssid.size = sizeof(out_scan_records.aps[i].ssid.bytes);
        memcpy(out_scan_records.aps[i].bssid.bytes, scan_records[i].bssid, sizeof(out_scan_records.aps[i].bssid.bytes));
        out_scan_records.aps[i].bssid.size    = sizeof(out_scan_records.aps[i].bssid.bytes);
        out_scan_records.aps[i].security_type = static_cast<chip_rpc_WIFI_SECURITY_TYPE>(scan_records[i].authmode);
        out_scan_records.aps[i].channel       = scan_records[i].primary;
        auto found_channel                    = kChannelToFreqMap.find(scan_records[i].primary);
        out_scan_records.aps[i].frequency     = (found_channel ? found_channel->second : 0);
        out_scan_records.aps[i].signal        = scan_records[i].rssi;
    }
    writer.Write(out_scan_records);
    writer.Finish();
}

void GDMWifiBase::WifiEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "******** DISCONNECTED FROM AP *********");
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &WifiEventHandler);
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &WifiEventHandler);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6)
    {
        // This is in case not only link-local address is provided
        auto * event = static_cast<ip_event_got_ip6_t *>(event_data);
        ESP_LOGI(TAG, "got ip6 :" IPV6STR, IPV62STR(event->ip6_info.ip));
    }
}

} // namespace rpc
} // namespace chip
