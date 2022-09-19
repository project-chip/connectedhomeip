/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
#include <LockManager.h>
#include <stdio.h>
#include <wiced.h>
#include <wiced_button_manager.h>
#include <wiced_platform.h>

/******************************************************
 *                      Macros
 ******************************************************/
void app_button_event_handler(const button_manager_button_t * button, button_manager_event_t event,
                              button_manager_button_state_t state);

static wiced_button_manager_configuration_t app_button_manager_configuration = {
    .short_hold_duration     = 500, /*msec*/
    .medium_hold_duration    = 700,
    .long_hold_duration      = 1500,
    .very_long_hold_duration = 2500,
    .debounce_duration       = 150, /* typically a click takes around ~150-200 ms */
    .double_click_interval   = 250,
    .continuous_hold_detect  = WICED_TRUE,
    .event_handler           = app_button_event_handler,
};

/* Static button configuration */
static wiced_button_configuration_t app_button_configurations[APP_MAX_BUTTON_DEF];
/* Button objects for the button manager */
static button_manager_button_t app_buttons[APP_MAX_BUTTON_DEF];
static button_manager_t app_button_manager;

wiced_result_t app_button_init(void)
{
    wiced_result_t result = WICED_ERROR;

    memset(app_button_configurations, 0, (sizeof(wiced_button_configuration_t) * APP_MAX_BUTTON_DEF));
    memset(app_buttons, 0, (sizeof(button_manager_button_t) * APP_MAX_BUTTON_DEF));

    app_button_configurations[ON_OFF_BUTTON].button            = PLATFORM_BUTTON_1;
    app_button_configurations[ON_OFF_BUTTON].button_event_mask = BUTTON_CLICK_EVENT;
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
    // printf("app_button_event_handler. button=%d, event=%d, state=%d\n", button_mgr[ON_OFF_BUTTON].configuration->button, event,
    // state);
    bool initiated = false;
    LockManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (button_mgr[0].configuration->button == PLATFORM_BUTTON_1 && event == BUTTON_CLICK_EVENT && state == BUTTON_STATE_RELEASED)
    {
        if (LockMgr().NextState() == true)
        {
            action = LockManager::LOCK_ACTION;
        }
        else
        {
            action = LockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = CHIP_ERROR_UNEXPECTED_EVENT;
    }
    if (err == CHIP_NO_ERROR)
    {
        initiated = LockMgr().InitiateAction(AppEvent::kEventType_Button, action);

        if (!initiated)
        {
            printf("Action is already in progress or active.");
        }
    }
}
