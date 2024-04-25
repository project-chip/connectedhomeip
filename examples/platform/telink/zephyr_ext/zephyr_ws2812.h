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

#ifndef ZEPHYR_WS2812_H
#define ZEPHYR_WS2812_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>

/* Defines */

#ifndef PERMILLE_MAX
#define PERMILLE_MAX 1000
#endif /* PERMILLE_MAX */

/* Data types */

enum ws2812_led_state
{
    WS2812_LED_OFF = 0,
    WS2812_LED_ON,
    WS2812_LED_FIXED
};

struct ws2812_led_data
{
    const struct device * led;
    const size_t led_len;
    struct led_rgb * pix;
    void * aux;
};

/*
 * Declare struct ws2812_led_data variable base on data from .dts.
 * The name of variable should correspond to .dts node name.
 * .dts fragment example:
 * name: ws2812 {
 *     compatible = "worldsemi,ws2812-gpio";
 *     chain-length = <1>;
 *     color-mapping = <LED_COLOR_ID_GREEN
 *                      LED_COLOR_ID_RED
 *                      LED_COLOR_ID_BLUE>;
 *     in-gpios = <&gpiob 4 0>;
 * };
 */
#define WS2812_LED_DEFINE(name)                                                                                                    \
    struct ws2812_led_data name = {                                                                                                \
        .led     = COND_CODE_1(DT_NODE_HAS_COMPAT(DT_NODELABEL(name), worldsemi_ws2812_gpio), (DEVICE_DT_GET(DT_NODELABEL(name))), \
                               (NULL)),                                                                                            \
        .led_len = COND_CODE_1(DT_NODE_HAS_COMPAT(DT_NODELABEL(name), worldsemi_ws2812_gpio),                                      \
                               (DT_PROP(DT_NODELABEL(name), chain_length)), (0)),                                                  \
        .pix     = (struct led_rgb[COND_CODE_1(DT_NODE_HAS_COMPAT(DT_NODELABEL(name), worldsemi_ws2812_gpio),                      \
                                               (DT_PROP(DT_NODELABEL(name), chain_length)), (0))]){},                              \
        .aux     = (struct led_rgb[COND_CODE_1(DT_NODE_HAS_COMPAT(DT_NODELABEL(name), worldsemi_ws2812_gpio),                      \
                                               (DT_PROP(DT_NODELABEL(name), chain_length)), (0))]){},                              \
    }

/* Public APIs */

bool ws2812_led_init(struct ws2812_led_data * ws2812_led);
bool ws2812_led_set(struct ws2812_led_data * ws2812_led, size_t id, enum ws2812_led_state state, ...);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_WS2812_H */
