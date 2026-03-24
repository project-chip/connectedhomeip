
/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "InitMatterMultipan.h"
#include <lib/support/CodeUtils.h>
#include <openthread/cli.h>
#include <openthread/instance.h>

extern "C" {
#include "platform-efr32.h"
#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
#include "sl_ot_custom_cli.h"
void otAppCliInit(otInstance * aInstance);
#endif // CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
}

// Global variable to store the secondary OpenThread instance
static otInstance * myOtInstance = nullptr;

/**
 * @brief Initialize Matter Multi-PAN support with internal app initialization
 *
 * This file initializes a secondary OpenThread instance for Multi-PAN support
 * and initializes the OpenThread CLI to the secondary Thread instance.
 */
extern "C" void InternalAppInitSecondInstance()
{
    myOtInstance = otInstanceInitMultiple(1);
    if (myOtInstance == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to initialize secondary OpenThread instance for Multi-PAN");
    }
}

extern "C" void InternalAppInitCli()
{
    /* Move CLI to the secondary Thread instance */
    if (myOtInstance == nullptr)
    {
        ChipLogError(NotSpecified, "Cannot initialize CLI: secondary OpenThread instance is null");
        return;
    }
    otAppCliInit(myOtInstance);
    sl_ot_custom_cli_init();
}
