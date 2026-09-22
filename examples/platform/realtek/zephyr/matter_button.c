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

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
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

#define DEBOUNCE_MS DT_PROP_OR(DT_PATH(buttons), debounce_interval_ms, 30)

struct button_state
{
    struct k_work_delayable work;
    uint8_t index;
};
static struct button_state button_state[NUMBER_OF_BUTTONS];
static struct gpio_callback button_cb_data;
static button_handler_t button_handler_cb;

static void button_work_handler(struct k_work * work)
{
    struct k_work_delayable * dwork = k_work_delayable_from_work(work);
    struct button_state * state     = CONTAINER_OF(dwork, struct button_state, work);
    uint8_t i                       = state->index;
    int pressed;

    pressed = gpio_pin_get_dt(&buttons[i]);

    if (pressed > 0)
    {
        /* PRESSED */
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_LEVEL_INACTIVE);
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_MODE_ENABLE_ONLY);

        if (button_handler_cb != NULL)
        {
            button_handler_cb(i, MATTER_BUTTON_STATE_PRESS);
        }
    }
    else
    {
        /* RELEASED */
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_LEVEL_ACTIVE);
        gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_MODE_ENABLE_ONLY);

        if (button_handler_cb != NULL)
        {
            button_handler_cb(i, MATTER_BUTTON_STATE_RELEASE);
        }
    }
}

void button_pressed(const struct device * dev, struct gpio_callback * cb, uint32_t pins)
{
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        if (pins == BIT(buttons[i].pin))
        {
            break;
        }
    }

    if (i == ARRAY_SIZE(buttons))
    {
        return;
    }

    gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_MODE_DISABLE_ONLY);
    k_work_reschedule(&button_state[i].work, K_MSEC(DEBOUNCE_MS));
}

int matter_button_init(button_handler_t button_handler)
{
    uint32_t pin_mask = 0;
    int err;

    button_handler_cb = button_handler;

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        button_state[i].index = i;
        k_work_init_delayable(&button_state[i].work, button_work_handler);

        if (!gpio_is_ready_dt(&buttons[i]))
        {
            LOG_ERR("button device %s is not ready", buttons[i].port->name);
            return -EIO;
        }

        err = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
        if (err)
        {
            LOG_ERR("Error %d: failed to configure %s pin %d", err, buttons[i].port->name, buttons[i].pin);
            return err;
        }

        err = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_LEVEL_ACTIVE);
        if (err != 0)
        {
            LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", err, buttons[i].port->name, buttons[i].pin);
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
