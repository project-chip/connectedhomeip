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

#include "sdkconfig.h"
#if CONFIG_ENABLE_PW_RPC
#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "button_service/button_service.rpc.pb.h"
#include "device_service/device_service.rpc.pb.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pw_log/log.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include <lib/support/logging/CHIPLogging.h>

#include "ScreenManager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "pw_containers/flat_map.h"
#include "pw_status/status.h"
#include "pw_status/try.h"
#include "wifi_service/wifi_service.rpc.pb.h"

#include "ServiceProvisioning.h"

#include "ESP32Utils.h"

static const char * TAG = "RPC";

namespace chip {
namespace rpc {
namespace {

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

// These are potentially large objects for the scan results.
constexpr size_t kScanRecordsMax = sizeof(chip_rpc_ScanResults().aps) / sizeof(chip_rpc_ScanResult);
chip_rpc_ScanResults out_scan_records;
wifi_ap_record_t scan_records[kScanRecordsMax];

class Device final : public generated::Device<Device>
{
public:
    pw::Status FactoryReset(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        chip::DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        return pw::OkStatus();
    }
    pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
    pw::Status TriggerOta(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
    pw::Status GetDeviceInfo(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {
        response.vendor_id        = 1234;
        response.product_id       = 5678;
        response.software_version = 0;
        return pw::OkStatus();
    }
};

class Button final : public generated::Button<Button>
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
#if CONFIG_DEVICE_TYPE_M5STACK
        if (request.pushed)
        {
            ScreenManager::ButtonPressed(1 + request.idx);
        }
        return pw::OkStatus();
#else  // CONFIG_DEVICE_TYPE_M5STACK
        return pw::Status::Unimplemented();
#endif // CONFIG_DEVICE_TYPE_M5STACK
    }
};

class Wifi final : public generated::Wifi<Wifi>
{
public:
    pw::Status GetChannel(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_Channel & response)
    {
        uint8_t channel = 0;
        wifi_second_chan_t second;
        PW_TRY(EspToPwStatus(esp_wifi_get_channel(&channel, &second)));
        response.channel = channel;
        return pw::OkStatus();
    }

    pw::Status GetSsid(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_Ssid & response)
    {
        wifi_config_t config;
        PW_TRY(EspToPwStatus(esp_wifi_get_config(WIFI_IF_STA, &config)));
        size_t size = std::min(sizeof(response.ssid.bytes), sizeof(config.sta.ssid));
        memcpy(response.ssid.bytes, config.sta.ssid, size);
        response.ssid.size = size;
        return pw::OkStatus();
    }

    pw::Status GetState(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_State & response)
    {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        PW_TRY(EspToPwStatus(err));
        response.connected = (err != ESP_ERR_WIFI_NOT_CONNECT);
        return pw::OkStatus();
    }

    pw::Status GetMacAddress(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_MacAddress & response)
    {
        uint8_t mac[6];
        PW_TRY(EspToPwStatus(esp_wifi_get_mac(WIFI_IF_STA, mac)));
        snprintf(response.mac_address, sizeof(response.mac_address), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],
                 mac[3], mac[4], mac[5]);
        return pw::OkStatus();
    }

    pw::Status GetWiFiInterface(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_WiFiInterface & response)
    {
        wifi_ap_record_t ap_info;
        PW_TRY(EspToPwStatus(esp_wifi_sta_get_ap_info(&ap_info)));
        snprintf(response.interface, sizeof(response.interface), "STA");
        return pw::OkStatus();
    }

    pw::Status GetIP4Address(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_IP4Address & response)
    {
        esp_netif_ip_info_t ip_info;
        PW_TRY(EspToPwStatus(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info)));
        snprintf(response.address, sizeof(response.address), IPSTR, IP2STR(&ip_info.ip));
        return pw::OkStatus();
    }

    pw::Status GetIP6Address(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_IP6Address & response)
    {
        esp_ip6_addr_t ip6;
        PW_TRY(EspToPwStatus(esp_netif_get_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip6)));
        snprintf(response.address, sizeof(response.address), IPV6STR, IPV62STR(ip6));
        return pw::OkStatus();
    }

    // NOTE: Currently this is blocking, it can be made non-blocking if needed
    //       but would require another worker thread to handle the scanning.
    void StartScan(ServerContext &, const chip_rpc_ScanConfig & request, ServerWriter<chip_rpc_ScanResults> & writer)
    {
        wifi_scan_config_t scan_config;
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
            out_scan_records.aps[i].ssid.size = std::min(sizeof(scan_records[i].ssid), sizeof(out_scan_records.aps[i].ssid.bytes));
            memcpy(out_scan_records.aps[i].ssid.bytes, scan_records[i].ssid, out_scan_records.aps[i].ssid.size);
            out_scan_records.aps[i].bssid.size =
                std::min(sizeof(scan_records[i].bssid), sizeof(out_scan_records.aps[i].bssid.bytes));
            memcpy(out_scan_records.aps[i].bssid.bytes, scan_records[i].bssid, out_scan_records.aps[i].bssid.size);
            out_scan_records.aps[i].security_type = static_cast<chip_rpc_WIFI_SECURITY_TYPE>(scan_records[i].authmode);
            out_scan_records.aps[i].channel       = scan_records[i].primary;
            auto found_channel                    = kChannelToFreqMap.find(scan_records[i].primary);
            out_scan_records.aps[i].frequency     = (found_channel ? found_channel->second : 0);
            out_scan_records.aps[i].signal        = scan_records[i].rssi;
        }
        writer.Write(out_scan_records);
        writer.Finish();
    }

    pw::Status StopScan(ServerContext &, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        esp_wifi_scan_stop();
        return pw::OkStatus();
    }

    pw::Status Connect(ServerContext &, const chip_rpc_ConnectionData & request, chip_rpc_ConnectionResult & response)
    {
        char ssid[sizeof(wifi_config_t().sta.ssid)];
        char password[sizeof(wifi_config_t().sta.password)];
        size_t ssid_size = std::min(sizeof(ssid) - 1, static_cast<size_t>(request.ssid.size));
        memcpy(ssid, request.ssid.bytes, ssid_size);
        ssid[ssid_size]      = '\0';
        size_t password_size = std::min(sizeof(password) - 1, static_cast<size_t>(request.secret.size));
        memcpy(password, request.secret.bytes, password_size);
        password[password_size] = '\0';
        return SetWiFiStationProvisioning(ssid, password) == CHIP_NO_ERROR ? pw::OkStatus() : pw::Status::Unknown();
    }

    pw::Status Disconnect(ServerContext &, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        chip::DeviceLayer::ConnectivityMgr().ClearWiFiStationProvision();
        chip::DeviceLayer::ConnectivityMgr().SetWiFiStationMode(chip::DeviceLayer::ConnectivityManager::kWiFiStationMode_Disabled);
        return pw::OkStatus();
    }

private:
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
};

constexpr size_t kRpcStackSizeBytes = (8 * 1024);
constexpr uint8_t kRpcTaskPriority  = 5;

TaskHandle_t rpcTaskHandle;

Button button_service;
Device device_service;
Wifi wifi_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(button_service);
    server.RegisterService(device_service);
    server.RegisterService(wifi_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

} // namespace

void Init()
{
    PigweedLogger::init();

    ESP_LOGI(TAG, "----------- esp32-rpc-service starting -----------");

    xTaskCreate(RunRpcService, "RPC", kRpcStackSizeBytes / sizeof(StackType_t), nullptr, kRpcTaskPriority, &rpcTaskHandle);
}

} // namespace rpc
} // namespace chip

#endif // CONFIG_ENABLE_PW_RPC
