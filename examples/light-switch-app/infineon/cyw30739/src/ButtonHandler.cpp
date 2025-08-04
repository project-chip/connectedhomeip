/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <ButtonHandler.h>
#include <LEDWidget.h>
#include <LightSwitch.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cycfg_pins.h>
#include <platform/CHIPDeviceLayer.h>
#include <wiced_button_manager.h>
#include <wiced_platform.h>

#define APP_MAX_BUTTON_DEF 1

void app_button_event_handler(const button_manager_button_t * button_mgr, button_manager_event_t event,
                              button_manager_button_state_t state);

using namespace chip::app::Clusters;

static wiced_button_manager_configuration_t app_button_manager_configuration = {
    .short_hold_duration     = 500, /* msec */
    .medium_hold_duration    = 700,
    .long_hold_duration      = 1500,
    .very_long_hold_duration = 2500,
    .debounce_duration       = 150, /* typically a click takes around ~150-200 ms */
    .double_click_interval   = 250,
    .continuous_hold_detect  = WICED_TRUE,
    .event_handler           = app_button_event_handler,
};

static wiced_button_configuration_t app_button_configurations[APP_MAX_BUTTON_DEF];
static button_manager_button_t app_buttons[APP_MAX_BUTTON_DEF];
static button_manager_t app_button_manager;

wiced_result_t app_button_init(void)
{
    wiced_result_t result = WICED_ERROR;

    memset(app_button_configurations, 0, (sizeof(wiced_button_configuration_t) * APP_MAX_BUTTON_DEF));
    memset(app_buttons, 0, (sizeof(button_manager_button_t) * APP_MAX_BUTTON_DEF));

    app_button_configurations[ON_OFF_BUTTON].gpio              = PLATFORM_BUTTON_USER;
    app_button_configurations[ON_OFF_BUTTON].button_event_mask = BUTTON_CLICK_EVENT | BUTTON_HOLDING_EVENT;
    app_buttons[ON_OFF_BUTTON].configuration                   = &app_button_configurations[ON_OFF_BUTTON];

    result = wiced_button_manager_init(&app_button_manager, &app_button_manager_configuration, app_buttons, 1);

    if (result != WICED_SUCCESS)
    {
        printf("button_manager_init failed (%d)\n", result);
    }
    return result;
}

void app_button_event_handler(const button_manager_button_t * button_mgr, button_manager_event_t event,
                              button_manager_button_state_t state)
{
    if (button_mgr[0].configuration->gpio == PLATFORM_BUTTON_USER)
    {
        if (event == BUTTON_CLICK_EVENT && state == BUTTON_STATE_RELEASED)
        {
            LightSwitch::GetInstance().InitiateActionSwitch(LightSwitch::Action::Toggle);
        }
        else if (event == BUTTON_HOLDING_EVENT)
        {
            printf("Button Performing factory reset ...\r\n");
            chip::DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        }
    }
}
