/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#ifndef ZEPHYR_LED_POOL_H
#define ZEPHYR_LED_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* Data types */

enum led_state
{
    LED_OFF = 0,
    LED_ON,
    LED_BLINK
};

struct led_pool_data
{
    const struct gpio_dt_spec * out;
    const size_t out_len;
    void * aux;
    struct k_work_delayable work;
};

/*
 * Declare struct led_pool_data variable base on data from .dts.
 * The name of variable should correspond to .dts node name.
 * .dts fragment example:
 * name {
 *     compatible = "gpio-leds";
 *     out {
 *         gpios = <&gpioe 6 GPIO_ACTIVE_HIGH>,
 *                 <&gpiod 0 GPIO_ACTIVE_HIGH>;
 *     };
 * };
 */
#define LED_POOL_DEFINE(name)                                                                                                      \
    struct led_pool_data name = {                                                                                                  \
        .out     = (const struct gpio_dt_spec[]){ COND_CODE_1(                                                                     \
            DT_NODE_HAS_PROP(DT_PATH_INTERNAL(DT_CHILD(name, out)), gpios),                                                    \
            (DT_FOREACH_PROP_ELEM_SEP(DT_PATH_INTERNAL(DT_CHILD(name, out)), gpios, GPIO_DT_SPEC_GET_BY_IDX, (, ))), ()) },    \
        .out_len = COND_CODE_1(DT_NODE_HAS_PROP(DT_PATH_INTERNAL(DT_CHILD(name, out)), gpios),                                     \
                               (DT_PROP_LEN(DT_PATH_INTERNAL(DT_CHILD(name, out)), gpios)), (0)),                                  \
    }

/* Public APIs */

bool led_pool_init(struct led_pool_data * led_pool);
bool led_pool_set(struct led_pool_data * led_pool, size_t led, enum led_state state, ...);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KEY_POOL_H */
