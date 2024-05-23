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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pw_log/log.h"
#include "pw_rpc/echo_service_nanopb.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"

#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include <lib/support/logging/CHIPLogging.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

extern const char TAG[] = "chip-pigweed-app";

namespace {
using std::byte;

constexpr size_t kRpcStackSizeBytes = (4 * 1024);
constexpr uint8_t kRpcTaskPriority  = 5;

TaskHandle_t rpcTaskHandle;

pw::rpc::EchoService echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

void RunRpcService(void *)
{
    ::chip::rpc::Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

extern "C" void app_main()
{
    PigweedLogger::init();

    ESP_LOGI(TAG, "----------- chip-esp32-pigweed-example starting -----------");

    xTaskCreate(RunRpcService, "RPC", kRpcStackSizeBytes / sizeof(StackType_t), nullptr, kRpcTaskPriority, &rpcTaskHandle);
}
