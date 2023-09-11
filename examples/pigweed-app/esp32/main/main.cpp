/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

const char * TAG = "chip-pigweed-app";

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
