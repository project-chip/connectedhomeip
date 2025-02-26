/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include "support/CodeUtils.h"

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
#include "pigweed/rpc_services/BooleanState.h"
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
#if CONFIG_DEVICE_TYPE_M5STACK
#include "ScreenManager.h"
#endif // CONFIG_DEVICE_TYPE_M5STACK
#include "pigweed/rpc_services/Button.h"
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
#include "pigweed/rpc_services/Descriptor.h"
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
#include "pigweed/rpc_services/Device.h"
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE
#include "pigweed/rpc_services/Event.h"
#endif // defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
#include "pigweed/rpc_services/Lighting.h"
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
#include "pigweed/rpc_services/Locking.h"
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

#if defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE
#include "NetworkCommissioningDriver.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "pigweed/rpc_services/Wifi.h"
#include "pw_status/try.h"
#endif // defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
#define PW_TRACE_BUFFER_SIZE_BYTES 1024
#include "pw_trace/trace.h"
#include "pw_trace_tokenized/trace_rpc_service_nanopb.h"

// Define trace time for pw_trace
PW_TRACE_TIME_TYPE pw_trace_GetTraceTime()
{
    return (PW_TRACE_TIME_TYPE) chip::System::SystemClock().GetMonotonicMicroseconds64().count();
}
// Microsecond time source
size_t pw_trace_GetTraceTimeTicksPerSecond()
{
    return 1000000;
}

#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

namespace chip {
namespace rpc {
namespace {

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
class Esp32Button final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
#if CONFIG_DEVICE_TYPE_M5STACK
        if (request.pushed)
        {
            ScreenManager::ButtonPressed(1 + request.idx);
            WakeDisplay();
        }
        return pw::OkStatus();
#else  // CONFIG_DEVICE_TYPE_M5STACK
        return pw::Status::Unimplemented();
#endif // CONFIG_DEVICE_TYPE_M5STACK
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class Esp32Device final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response) override
    {
        TickType_t delayMs = kRebootTimerPeriodMs;
        if (request.delay_ms != 0)
        {
            delayMs = request.delay_ms;
        }
        else
        {
            ChipLogProgress(NotSpecified, "Did not receive a reboot delay. Defaulting to %d ms",
                            static_cast<int>(kRebootTimerPeriodMs));
        }
        mRebootTimer = xTimerCreateStatic("Reboot", pdMS_TO_TICKS(delayMs), false, nullptr, RebootHandler, &mRebootTimerBuffer);

        xTimerStart(mRebootTimer, 0);
        return pw::OkStatus();
    }

private:
    static constexpr uint32_t kRebootTimerPeriodMs = 1000;
    TimerHandle_t mRebootTimer;
    StaticTimer_t mRebootTimerBuffer;

    static void RebootHandler(TimerHandle_t) { esp_restart(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE

class Esp32WiFi final : public WiFi
{
public:
    pw::Status GetChannel(const pw_protobuf_Empty & request, chip_rpc_Channel & response) override
    {
        uint8_t channel = 0;
        wifi_second_chan_t second;
        PW_TRY(EspToPwStatus(esp_wifi_get_channel(&channel, &second)));
        response.channel = channel;
        return pw::OkStatus();
    }

    pw::Status GetSsid(const pw_protobuf_Empty & request, chip_rpc_Ssid & response) override
    {
        wifi_config_t config;
        PW_TRY(EspToPwStatus(esp_wifi_get_config(WIFI_IF_STA, &config)));
        size_t size = std::min(sizeof(response.ssid.bytes), sizeof(config.sta.ssid));
        memcpy(response.ssid.bytes, config.sta.ssid, size);
        response.ssid.size = size;
        return pw::OkStatus();
    }

    pw::Status GetState(const pw_protobuf_Empty & request, chip_rpc_State & response) override
    {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        PW_TRY(EspToPwStatus(err));
        response.connected = (err != ESP_ERR_WIFI_NOT_CONNECT);
        return pw::OkStatus();
    }

    pw::Status GetMacAddress(const pw_protobuf_Empty & request, chip_rpc_MacAddress & response) override
    {
        uint8_t mac[6];
        PW_TRY(EspToPwStatus(esp_wifi_get_mac(WIFI_IF_STA, mac)));
        snprintf(response.mac_address, sizeof(response.mac_address), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2],
                 mac[3], mac[4], mac[5]);
        return pw::OkStatus();
    }

    pw::Status GetWiFiInterface(const pw_protobuf_Empty & request, chip_rpc_WiFiInterface & response) override
    {
        wifi_ap_record_t ap_info;
        PW_TRY(EspToPwStatus(esp_wifi_sta_get_ap_info(&ap_info)));
        snprintf(response.interface, sizeof(response.interface), "STA");
        return pw::OkStatus();
    }

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
    pw::Status GetIP4Address(const pw_protobuf_Empty & request, chip_rpc_IP4Address & response) override
    {
        esp_netif_ip_info_t ip_info;
        PW_TRY(EspToPwStatus(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info)));
        snprintf(response.address, sizeof(response.address), IPSTR, IP2STR(&ip_info.ip));
        return pw::OkStatus();
    }
#endif

    pw::Status GetIP6Address(const pw_protobuf_Empty & request, chip_rpc_IP6Address & response) override
    {
        esp_ip6_addr_t ip6;
        PW_TRY(EspToPwStatus(esp_netif_get_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip6)));
        snprintf(response.address, sizeof(response.address), IPV6STR, IPV62STR(ip6));
        return pw::OkStatus();
    }

    pw::Status Connect(const chip_rpc_ConnectionData & request, chip_rpc_ConnectionResult & response) override
    {
        char ssid[sizeof(wifi_config_t().sta.ssid)];
        char password[sizeof(wifi_config_t().sta.password)];
        size_t ssid_size = std::min(sizeof(ssid) - 1, static_cast<size_t>(request.ssid.size));
        memcpy(ssid, request.ssid.bytes, ssid_size);
        ssid[ssid_size]      = '\0';
        size_t password_size = std::min(sizeof(password) - 1, static_cast<size_t>(request.secret.size));
        memcpy(password, request.secret.bytes, password_size);
        password[password_size] = '\0';
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        CHIP_ERROR error = chip::DeviceLayer::NetworkCommissioning::ESPWiFiDriver::GetInstance().ConnectWiFiNetwork(
            ssid, strlen(ssid), password, strlen(password));
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        if (error != CHIP_NO_ERROR)
        {
            return pw::Status::Internal();
        }
        return pw::OkStatus();
    }

    pw::Status Disconnect(const pw_protobuf_Empty & request, pw_protobuf_Empty & response) override
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        chip::DeviceLayer::ConnectivityMgr().ClearWiFiStationProvision();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
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

#endif // defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE

#define RPC_TASK_STACK_SIZE (8 * 1024)
#define RPC_TASK_PRIORITY 5
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
Actions actions_service;
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
BooleanState boolean_state_service;
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
Esp32Button button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
Descriptor descriptor_service;
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
Esp32Device device_service;
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE
Event event_service;
#endif // defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
Lighting lighting_service;
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
Locking locking;
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
pw::trace::TraceService trace_service(pw::trace::GetTokenizedTracer());
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

#if defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE
Esp32WiFi wifi_service;
#endif // defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
    server.RegisterService(actions_service);
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
    server.RegisterService(attributes_service);
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
    server.RegisterService(boolean_state_service);
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
    server.RegisterService(button_service);
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
    server.RegisterService(descriptor_service);
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
    server.RegisterService(device_service);
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE
    server.RegisterService(event_service);
#endif // defined(PW_RPC_EVENT_SERVICE) && PW_RPC_EVENT_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
    server.RegisterService(lighting_service);
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
    server.RegisterService(locking);
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
    server.RegisterService(trace_service);
    PW_TRACE_SET_ENABLED(true);
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

#if defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE
    server.RegisterService(wifi_service);
#endif // defined(PW_RPC_WIFI_SERVICE) && PW_RPC_WIFI_SERVICE
}

} // namespace

#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
void SubscribeActions(RpcActionsSubscribeCallback subscriber)
{
    actions_service.SubscribeActions(subscriber);
}
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

void Init()
{
    PigweedLogger::init();

    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", MATTER_ARRAY_SIZE(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
}

} // namespace rpc
} // namespace chip

#endif // CONFIG_ENABLE_PW_RPC
