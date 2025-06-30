/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "matter_button.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(matter_button, CONFIG_CHIP_APP_LOG_LEVEL);

#define BUTTONS_NODE DT_PATH(buttons)
#define INCREMENT_BY_ONE(button) +1
#define NUMBER_OF_BUTTONS (0 DT_FOREACH_CHILD(BUTTONS_NODE, INCREMENT_BY_ONE))
#define GPIO_SPEC_AND_COMMA(button) GPIO_DT_SPEC_GET(button, gpios),

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_EXISTS(BUTTONS_NODE)
	DT_FOREACH_CHILD(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};
static uint8_t key_trigger_level[NUMBER_OF_BUTTONS];
static struct gpio_callback button_cb_data;
static button_handler_t button_handler_cb;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		            uint32_t pins)
{
    uint8_t state;
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        if(pins == BIT(buttons[i].pin))
        {
            break;
        }
    }

    if(i == ARRAY_SIZE(buttons))
    {
        return;
    }

    if (key_trigger_level[i] == GPIO_ACTIVE_HIGH)
    {
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_INACTIVE);
        key_trigger_level[i] = GPIO_ACTIVE_LOW;
        state = MATTER_BUTTON_STATE_PRESS;
    }
    else
    {
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
        key_trigger_level[i] = GPIO_ACTIVE_HIGH;
        state = MATTER_BUTTON_STATE_RELEASE;
    }

    if (button_handler_cb != NULL)
    {
        button_handler_cb(i, state);
    }
}

int matter_button_init(button_handler_t button_handler)
{
    uint32_t pin_mask = 0;
    int err;

    button_handler_cb = button_handler;

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        key_trigger_level[i] = GPIO_ACTIVE_HIGH;

        if (!gpio_is_ready_dt(&buttons[i]))
        {
            LOG_ERR("button device %s is not ready", buttons[i].port->name);
            return -EIO;
	    }

		err = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (err)
        {
			LOG_ERR("Error %d: failed to configure %s pin %d",
		            err, buttons[i].port->name, buttons[i].pin);
			return err;
		}

        err = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
        if (err != 0)
        {
            LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
                    err, buttons[i].port->name, buttons[i].pin);
            return err;
        }

        pin_mask |= BIT(buttons[i].pin);
	}

    gpio_init_callback(&button_cb_data, button_pressed, pin_mask);

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        err = gpio_add_callback(buttons[i].port, &button_cb_data);
        if (err)
        {
            LOG_ERR("Cannot add callback");
            return err;
        }
	}

    return err;
}
