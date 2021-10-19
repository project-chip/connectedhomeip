/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "AppTask.h"
#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include "rpc_services/Button.h"
#include "rpc_services/Device.h"
#include "rpc_services/Locking.h"

#include <lib/support/logging/CHIPLogging.h>

const char * TAG = "RPC";

using chip::DeviceLayer::ConfigurationMgr;

static bool uartInitialised;

namespace chip {
namespace rpc {

class Esp32Button final : public Button
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        GetAppTask().ButtonEventHandler(request.idx, request.pushed);
        return pw::OkStatus();
    }
};

class Esp32Device final : public Device
{
public:
    pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response) override
    {
        esp_restart();
        // WILL NOT RETURN
        return pw::OkStatus();
    }
};

constexpr size_t kRpcStackSizeBytes = (4 * 1024);
constexpr uint8_t kRpcTaskPriority  = 5;

TaskHandle_t rpcTaskHandle;

Esp32Button button_service;
Locking locking_service;
Device device_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(locking_service);
    server.RegisterService(button_service);
    server.RegisterService(device_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

void Init()
{
    PigweedLogger::init();
    uartInitialised = true;

    ESP_LOGI(TAG, "----------- esp32-rpc-service starting -----------");

    xTaskCreate(RunRpcService, "RPC", kRpcStackSizeBytes / sizeof(StackType_t), nullptr, kRpcTaskPriority, &rpcTaskHandle);
}

} // namespace rpc
} // namespace chip

#endif
