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

#include "LEDWidget.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"

#include "pw_rpc/echo_service_nanopb.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_mbed/init.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/mbed/Logging.h>

#include "rtos/Mutex.h"
#include "rtos/Thread.h"

using namespace ::chip::rpc;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

namespace {

#define RPC_THREAD_NAME "RPC"
#define RPC_STACK_SIZE (4 * 1024)

rtos::Thread rpcThread{ osPriorityNormal, RPC_STACK_SIZE, /* memory provided */ nullptr, RPC_THREAD_NAME };

pw::rpc::EchoService echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

void RunRpcService()
{
    Start(RegisterServices, &logger_mutex);
}

} // namespace

int main()
{
    int ret = 0;

    mbed_logging_init();

    ChipLogProgress(NotSpecified, "Mbed pigweed-app example application start");

    pw_sys_io_Init();

    sStatusLED.Set(true);

    auto error = rpcThread.start(RunRpcService);
    if (error != osOK)
    {
        ChipLogError(NotSpecified, "Run RPC thread failed [%d]", (int) error);
        ret = EXIT_FAILURE;
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Mbed pigweed-app example application run");

    rpcThread.join();

exit:
    ChipLogProgress(NotSpecified, "Exited with code %d", ret);
    return ret;
}
