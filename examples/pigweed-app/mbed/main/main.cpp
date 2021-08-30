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

#include "Rpc.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_mbed/init.h"

//#include <platform/mbed/Logging.h>
#include <support/logging/CHIPLogging.h>

#include "rtos/Mutex.h"
#include "rtos/Thread.h"

using namespace chip::rpc;

#define RPC_THREAD_NAME "RPC"
#define RPC_STACK_SIZE (4 * 1024)

rtos::Thread rpcThread{ osPriorityNormal, RPC_STACK_SIZE, /* memory provided */ nullptr, RPC_THREAD_NAME };

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

int main()
{
    pw_sys_io_Init();

    // mbed_logging_init();

    ChipLogProgress(NotSpecified, "==================================================\r\n");
    ChipLogProgress(NotSpecified, "chip-mbed-pigweed-example starting\r\n");
    ChipLogProgress(NotSpecified, "==================================================\r\n");

    auto error = rpcThread.start(RunRpcService);
    if (error != osOK)
    {
        ChipLogError(NotSpecified, "Run RPC service failed[%d]", error);
        return 1;
    }

    rpcThread.join();
    return 0;
}
