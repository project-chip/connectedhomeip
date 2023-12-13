/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "Button.h"

#define GPIO_INPUT_IO_0 CONFIG_EXAMPLE_BOARD_BUTTON_GPIO

#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

static const char TAG[] = "Button";

static Button::ButtonPressCallback button_press_handler = nullptr;

static void gpio_isr_handler(void * arg)
{
    if (button_press_handler != nullptr)
    {
        button_press_handler();
    }
}

void Button::Init()
{
    /* Initialize button interrupt*/
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(static_cast<gpio_num_t>(GPIO_INPUT_IO_0), gpio_isr_handler, (void *) GPIO_INPUT_IO_0);

    ESP_LOGI(TAG, "Button initialized..");
}

void Button::SetButtonPressCallback(ButtonPressCallback button_callback)
{
    if (button_callback != nullptr)
    {
        button_press_handler = button_callback;
    }
}
