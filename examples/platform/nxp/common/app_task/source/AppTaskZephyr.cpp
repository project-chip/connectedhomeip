/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright 2024 NXP
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
#include <platform/nxp/zephyr/wifi/NxpWifiDriver.h>
#endif

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nxp/common/factory_data/FactoryDataProvider.h>
#else
#include <platform/nxp/zephyr/DeviceInstanceInfoProviderImpl.h>
#endif

#include "AppFactoryData.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

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
chip::DeviceLayer::NetworkCommissioning::WiFiDriver * chip::NXP::App::AppTaskZephyr::GetWifiDriverInstance()
{
    return static_cast<chip::DeviceLayer::NetworkCommissioning::WiFiDriver *>(&(NetworkCommissioning::NxpWifiDriver::Instance()));
}
#endif // CONFIG_CHIP_WIFI

CHIP_ERROR chip::NXP::App::AppTaskZephyr::Start()
{

    ReturnErrorOnFailure(Init());

    AppEvent event{};

    while (true)
    {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(event);
    }

    return CHIP_NO_ERROR;
}

void chip::NXP::App::AppTaskZephyr::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT) != 0)
    {
        LOG_INF("Failed to post event to app task event queue");
    }
}

void chip::NXP::App::AppTaskZephyr::DispatchEvent(const AppEvent & event)
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
