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

/**
 * @file Button.cpp
 *
 * Implements a Button tied to a GPIO and provides debouncing
 *
 **/

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_system.h"

#include "AppTask.h"
#include "Button.h"
#include "Globals.h"
#include "ScreenManager.h"
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <vector>

static const char * TAG = "Button.cpp";

extern Button gButtons[BUTTON_NUMBER];

Button::Button() {}

Button::Button(gpio_num_t gpioNum)
{
    mGPIONum = gpioNum;
}

int32_t Find_Button_Via_Pin(gpio_num_t gpioNum)
{
    for (int i = 0; i < BUTTON_NUMBER; i++)
    {
        if (gButtons[i].GetGPIONum() == gpioNum)
        {
            return i;
        }
    }
    return -1;
}

void IRAM_ATTR button_isr_handler(void * arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    int32_t idx       = Find_Button_Via_Pin((gpio_num_t) gpio_num);
    if (idx == -1)
    {
        return;
    }
    BaseType_t taskWoken = pdFALSE;
    xTimerStartFromISR(gButtons[idx].mbuttonTimer,
                       &taskWoken); // If the timer had already been started ,restart it will reset its expiry time
}

esp_err_t Button::Init()
{
    return Init(mGPIONum);
}

esp_err_t Button::Init(gpio_num_t gpioNum)
{
    esp_err_t ret = ESP_OK;

    mGPIONum = gpioNum;
    //  zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // interrupt of falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL << gpioNum;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&io_conf);

    // hook isr handler for specific gpio pin
    ret = gpio_isr_handler_add(gpioNum, button_isr_handler, (void *) gpioNum);
    ESP_RETURN_ON_ERROR(ret, TAG, "gpio_isr_handler_add failed: %s", esp_err_to_name(ret));

    mbuttonTimer = xTimerCreate("BtnTmr",               // Just a text name, not used by the RTOS kernel
                                pdMS_TO_TICKS(50),      // timer period
                                false,                  // no timer reload (==one-shot)
                                (void *) (int) gpioNum, // init timer id = gpioNum index
                                TimerCallback           // timer callback handler (all buttons use
                                                        // the same timer cn function)
    );

    return ESP_OK;
}
void Button::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event Handler.
    uint32_t gpio_num = (uint32_t) pvTimerGetTimerID(xTimer);
    GetAppTask().ButtonEventHandler(gpio_num, APP_BUTTON_PRESSED);
}
