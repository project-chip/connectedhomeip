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

#ifndef ZEPHYR_KEY_POOL_H
#define ZEPHYR_KEY_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* Data types */

typedef void (*key_pool_on_button_change_t)(size_t button, bool pressed, void * context);

struct key_pool_data
{
    const struct gpio_dt_spec * inp;
    const size_t inp_len;
    uint8_t * buttons;
    void * aux;
    key_pool_on_button_change_t on_button_change;
    void * context;
    struct k_work_delayable work;
};

/*
 * Declare struct key_pool_data variable base on data from .dts.
 * The name of variable should correspond to .dts node name.
 * .dts fragment example:
 * name {
 *     compatible = "gpio-keys";
 *     inp {
 *         gpios = <&gpioc 3 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>,
 *                 <&gpioc 1 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
 *     };
 * };
 */
#define KEY_POOL_DEFINE(name)                                                                                                      \
    struct key_pool_data name = {                                                                                                  \
        .inp     = (const struct gpio_dt_spec[]){ COND_CODE_1(                                                                     \
            DT_NODE_HAS_PROP(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios),                                                    \
            (DT_FOREACH_PROP_ELEM_SEP(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios, GPIO_DT_SPEC_GET_BY_IDX, (, ))), ()) },    \
        .inp_len = COND_CODE_1(DT_NODE_HAS_PROP(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios),                                     \
                               (DT_PROP_LEN(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios)), (0)),                                  \
        .buttons = (uint8_t[COND_CODE_1(DT_NODE_HAS_PROP(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios),                            \
                                        (DIV_ROUND_UP(DT_PROP_LEN(DT_PATH_INTERNAL(DT_CHILD(name, inp)), gpios), 8)), (0))]){},    \
    }

/* Public APIs */

bool key_pool_init(struct key_pool_data * key_pool);
void key_pool_set_callback(struct key_pool_data * key_pool, key_pool_on_button_change_t on_button_change, void * context);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KEY_POOL_H */
