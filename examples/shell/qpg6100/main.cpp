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

#include "FreeRTOS.h"
#include "task.h"

#include <lib/shell/shell.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>

#include "qvCHIP.h"

#if CHIP_ENABLE_OPENTHREAD
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}
#endif // CHIP_ENABLE_OPENTHREAD

using namespace chip;
using namespace chip::Shell;

namespace {

const size_t kShellTaskStackSize = 2048;
const int kShellTaskPriority     = 1;
TaskHandle_t sShellTaskHandle;

void ShellCLIMain(void * pvParameter)
{
    // Initialize the default streamer that was linked.
    const int rc = streamer_init(streamer_get());

    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return;
    }

    ChipLogDetail(Shell, "Initializing CHIP shell commands: %d", rc);

    cmd_device_init();
    cmd_base64_init();
    cmd_misc_init();
    cmd_btp_init();
    cmd_otcli_init();

    ChipLogDetail(Shell, "Run CHIP shell Task: %d", rc);

    shell_task(nullptr);
}

} // namespace

int StartShellTask(void)
{
    int ret = 0;

    // Start Shell task.
    if (xTaskCreate(ShellCLIMain, "SHELL", kShellTaskStackSize / sizeof(StackType_t), NULL, kShellTaskPriority,
                    &sShellTaskHandle) != pdPASS)
    {
        ret = -1;
    }

    return ret;
}

int main(void)
{
    /* Initialize platform */
    qvCHIP_init();

    /* Launch shell task */
    StartShellTask();

    /* Start FreeRTOS */
    vTaskStartScheduler();
    return 0;
}
