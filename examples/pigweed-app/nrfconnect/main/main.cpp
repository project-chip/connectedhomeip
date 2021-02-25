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

#include "AppConfig.h"
#include "LEDWidget.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
#include <kernel.h>

#include "pw_rpc/echo_service_nanopb.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_nrfconnect/init.h"

#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(app);

static LEDWidget sStatusLED;

namespace {
#define RPC_STACK_SIZE (8 * 1024)
#define RPC_PRIORITY 7

K_THREAD_STACK_DEFINE(rpc_stack_area, RPC_STACK_SIZE);
struct k_thread rpc_thread_data;

pw::rpc::EchoService echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

void RunRpcService(void *, void *, void *)
{
    Start(RegisterServices, &::chip::rpc::logger_mutex);
}

} // namespace

int main()
{
    pw_sys_io_Init();

    LOG_INF("==================================================");
    LOG_INF("chip-nrf52840-pigweed-example starting");
#if BUILD_RELEASE
    LOG_INF("*** PSEUDO-RELEASE BUILD ***");
#else
    LOG_INF("*** DEVELOPMENT BUILD ***");
#endif
    LOG_INF("==================================================");

    // Light up the LED as a visual feedback that the flash was
    // successful.
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
    sStatusLED.Set(true);

    k_thread_create(&rpc_thread_data, rpc_stack_area, K_THREAD_STACK_SIZEOF(rpc_stack_area), RunRpcService, NULL, NULL, NULL,
                    RPC_PRIORITY, 0, K_NO_WAIT);
    k_thread_join(&rpc_thread_data, K_FOREVER);
    return 0;
}
