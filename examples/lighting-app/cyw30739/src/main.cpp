/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#include <AppShellCommands.h>
#include <ButtonHandler.h>
#include <ChipShellCollection.h>
#include <LightingManager.h>
#include <app/server/Server.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/platform.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <sparcommon.h>
#include <stdio.h>
#include <wiced_led_manager.h>
#include <wiced_memory.h>
#include <wiced_platform.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Shell;

static void EventHandler(const ChipDeviceEvent * event, intptr_t arg);
static void HandleThreadStateChangeEvent(const ChipDeviceEvent * event);
static void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t value);

static wiced_led_config_t chip_lighting_led_config = {
    .led    = PLATFORM_LED_1,
    .bright = 50,
};

APPLICATION_START()
{
    CHIP_ERROR err;
    wiced_result_t result;

    printf("\nChipLighting App starting\n");

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR MemoryInit %ld\n", err.AsInteger());
    }

    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        printf("ERROR app_button_init %d\n", result);
    }

    /* Init. LED Manager. */
    result = wiced_led_manager_init(&chip_lighting_led_config);
    if (result != WICED_SUCCESS)
        printf("wiced_led_manager_init fail (%d)\n", result);

    printf("Initializing CHIP\n");
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR InitChipStack %ld\n", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Initializing OpenThread stack\n");
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR InitThreadStack %ld\n", err.AsInteger());
    }
#endif

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else  // !CHIP_DEVICE_CONFIG_THREAD_FTD
    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif // CHIP_DEVICE_CONFIG_THREAD_FTD
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR SetThreadDeviceType %ld\n", err.AsInteger());
    }

    printf("Starting event loop task\n");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR StartEventLoopTask %ld\n", err.AsInteger());
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    printf("Starting thread task\n");
    err = ThreadStackMgr().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR StartThreadTask %ld\n", err.AsInteger());
    }
#endif

    PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    LightMgr().Init();
    LightMgr().SetCallbacks(LightManagerCallback, NULL);

    /* Start CHIP datamodel server */
    chip::Server::GetInstance().Init();

    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ConfigurationMgr().LogDeviceConfig();

    const int ret = Engine::Root().Init();
    if (!chip::ChipError::IsSuccess(ret))
    {
        printf("ERROR Shell Init %d\n", ret);
    }
    cmd_ping_init();
    RegisterAppShellCommands();
    Engine::Root().RunMainLoop();

    assert(!wiced_rtos_check_for_stack_overflow());
}

void EventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kThreadStateChange:
        HandleThreadStateChangeEvent(event);
        break;
    default:
        break;
    }
}

void HandleThreadStateChangeEvent(const ChipDeviceEvent * event)
{
#if CHIP_BYPASS_RENDEZVOUS
    if (event->ThreadStateChange.NetDataChanged && !ConnectivityMgr().IsThreadProvisioned())
    {
        ThreadStackMgr().JoinerStart();
    }
#endif /* CHIP_BYPASS_RENDEZVOUS */
}

void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t level)
{
    if (action == LightingManager::ON_ACTION)
    {
        printf("Turning light ON\n");
        wiced_led_manager_enable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        printf("Turning light OFF\n");
        wiced_led_manager_disable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::LEVEL_ACTION)
    {
        printf("Set light level = %d\n", level);
        chip_lighting_led_config.bright = (uint16_t) level * 100 / 0xfe;
        wiced_led_manager_reconfig_led(&chip_lighting_led_config);
    }
}
