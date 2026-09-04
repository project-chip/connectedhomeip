/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AppTask.h"

#include <lib/core/CHIPError.h>
#include <zephyr/kernel.h>

#include <cstdlib>

int main()
{
    const CHIP_ERROR err = chip::app::AllDevices::GetAppTask().Run();
    if (err != CHIP_NO_ERROR)
    {
        printk("AppTask::Run() failed: %" CHIP_ERROR_FORMAT "\n", err.Format());
        return EXIT_FAILURE;
    }

    while (true)
    {
        k_sleep(K_FOREVER);
    }
}
