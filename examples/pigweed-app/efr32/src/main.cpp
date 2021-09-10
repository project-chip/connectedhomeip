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

#include "AppConfig.h"
#include "LEDWidget.h"
#include "init_efrPlatform.h"
#include "sl_system_kernel.h"

#include "pw_rpc/echo_service_nanopb.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_efr32/init.h"
#include "uart.h"

#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
#include <FreeRTOS.h>
#include <task.h>

static LEDWidget sStatusLED;

namespace {
using std::byte;

#define RPC_TASK_STACK_SIZE 4096
#define RPC_TASK_PRIORITY 2
static TaskHandle_t sRpcTaskHandle;

pw::rpc::EchoService echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

int main(void)
{
    init_efrPlatform();
    EFR32_LOG("***** CHIP EFR32 pigweed example *****\r\n");

    pw_sys_io_Init();
    // Initialize LEDs
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
    sStatusLED.Set(true);

    xTaskCreate(RunRpcService, "RPC_Task", RPC_TASK_STACK_SIZE / sizeof(StackType_t), nullptr, RPC_TASK_PRIORITY, &sRpcTaskHandle);
    sl_system_kernel_start();
}
