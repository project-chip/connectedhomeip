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

#include "Rpc.h"
#include <DFUManager.h>
#include <LEDWidget.h>

#include "rtos/Thread.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/mbed/Logging.h>

using namespace ::chip::rpc;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging::Platform;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

int main()
{
    int ret                  = 0;
    CHIP_ERROR err           = CHIP_NO_ERROR;
    rtos::Thread * rpcThread = nullptr;

    mbed_logging_init();

    ChipLogProgress(NotSpecified, "Mbed pigweed-app example application start");

    sStatusLED.Set(true);

    err = GetDFUManager().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "DFU manager initialization failed: %s", err.AsString());
        ret = EXIT_FAILURE;
        goto exit;
    }

    rpcThread = chip::rpc::Init();
    if (rpcThread == NULL)
    {
        ChipLogError(NotSpecified, "RPC service initialization and run failed");
        ret = EXIT_FAILURE;
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Mbed pigweed-app example application run");

    rpcThread->join();

exit:
    ChipLogProgress(NotSpecified, "Exited with code %d", ret);
    return ret;
}
