/*
 *    Copyright (c) 2025 Project CHIP Authors
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

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include "AppTaskZephyr.h"

#include "CHIPDeviceManager.h"
#include <app/clusters/identify-server/identify-server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_CHIP_WIFI
#include <platform/Zephyr/wifi/ZephyrWifiDriver.h>
#endif

#ifdef ENABLE_CHIP_SHELL
#include "AppCLIZephyr.h"
#endif

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

constexpr size_t kAppEventQueueSize = 10;
K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

/* -------------------------------------------------------------------------- */
/*                            Class implementation                            */
/* -------------------------------------------------------------------------- */

#if defined(CONFIG_CHIP_WIFI)
chip::DeviceLayer::NetworkCommissioning::WiFiDriver * chip::Zephyr::App::AppTaskZephyr::GetWifiDriverInstance()
{
    return static_cast<chip::DeviceLayer::NetworkCommissioning::WiFiDriver *>(
        &(NetworkCommissioning::ZephyrWifiDriver::Instance()));
}
#endif

CHIP_ERROR chip::Zephyr::App::AppTaskZephyr::AppMatter_Register()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#ifdef ENABLE_CHIP_SHELL
    /* Register Matter CLI cmds */
    err = chip::Zephyr::App::GetAppCLI().Init();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Error during CLI init"));
    AppMatter_RegisterCustomCliCommands();
#endif
    return err;
}

void chip::Zephyr::App::AppTaskZephyr::Start()
{
    PreInitMatterStack();
    if (CHIP_NO_ERROR != Init())
    {
        ChipLogError(DeviceLayer, "AppTaskZephyr::Start() failed to initialize");
        return;
    }
    PostInitMatterStack();

    AppEvent event{};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(event);
    }
    // This point should never be reached, but if it is, we return an error.
    ChipLogError(DeviceLayer, "AppTaskZephyr::Start() should never return");
}

void chip::Zephyr::App::AppTaskZephyr::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void chip::Zephyr::App::AppTaskZephyr::DispatchEvent(const AppEvent & event)
{
    if (event.Handler)
    {
        event.Handler(event);
    }
    else
    {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}
