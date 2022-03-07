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

#include "AppTask.h"

#include <system/SystemError.h>

#include <logging/log.h>

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

#ifdef CONFIG_USB_DEVICE_STACK
#include <usb/usb_device.h>
#endif

LOG_MODULE_REGISTER(app);

using namespace ::chip;

int main()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef CONFIG_CHIP_PW_RPC
    rpc::Init();
#endif

#ifdef CONFIG_USB_DEVICE_STACK
    err = System::MapErrorZephyr(usb_enable(nullptr));
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to initialize USB device");
    }
#endif

    if (err == CHIP_NO_ERROR)
    {
        err = GetAppTask().StartApp();
    }

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
