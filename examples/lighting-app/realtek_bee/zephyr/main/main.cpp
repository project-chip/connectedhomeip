/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <system/SystemError.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include "AppTask.h"

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;

#include <openthread/instance.h>
#include <openthread/platform/time.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread-system.h>
#include "soc.h"
#include "rtl_wdt.h"

extern "C"
{
// replace misc.c

extern void WDG_SystemReset(WDTMode_TypeDef wdt_mode, int reset_reason);
void __wrap_otPlatReset(otInstance *aInstance)
{
 	ARG_UNUSED(aInstance);
	WDG_SystemReset(RESET_ALL, 0xff);
}
}

int main()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef CONFIG_CHIP_PW_RPC
    rpc::Init();
#endif

    if (err == CHIP_NO_ERROR)
    {
        err = AppTask::Instance().StartApp();
    }

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
