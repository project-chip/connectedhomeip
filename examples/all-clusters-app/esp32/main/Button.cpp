/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 * Implements a Button tied to a GPIO and provides debouncing and polling
 *
 **/

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "Button.h"
#include "ScreenManager.h"
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

static const char * TAG = "Button.cpp";

static xQueueHandle button_evt_queue = NULL;

static void IRAM_ATTR button_isr_handler(void * arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

static void button_task(void * arg)
{
    uint32_t io_num;

    while (1)
    {
        if (xQueueReceive(button_evt_queue, &io_num, portMAX_DELAY))
        {
            vTaskDelay(50 / portTICK_PERIOD_MS);
#if CONFIG_DEVICE_TYPE_M5STACK
            int level = gpio_get_level((gpio_num_t) io_num);
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d, pressed\n", io_num, level);
            if (level == 0)
            {
                bool woken = false;
                if (!woken)
                {
                    woken = WakeDisplay();
                }
                if (woken)
                {
                    continue;
                }

                ScreenManager::ButtonPressed(40 - io_num);
            }
#endif
        }
    }
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
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_isr_handler_add failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (button_evt_queue == NULL)
    {
        // create a queue to handle gpio event from isr
        button_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        // start gpio task
        xTaskCreate(button_task, "button_task", 3500, NULL, 10, NULL);
    }

    return ESP_OK;
}
