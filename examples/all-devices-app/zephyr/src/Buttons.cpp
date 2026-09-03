/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "Buttons.h"

#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

namespace chip::app::AllDevices::Button {
namespace {

constexpr uint16_t kFactoryResetHoldMs = 5000;

#if ALL_DEVICES_HAS_FACTORY_RESET_SW
const struct gpio_dt_spec sFactoryResetSw = GPIO_DT_SPEC_GET(ALL_DEVICES_FACTORY_RESET_SW_NODE, gpios);
struct gpio_callback sFactoryResetSwCallback;
int64_t sFactoryResetSwPressedAtMs = 0;

void OnFactoryResetSwPressed(const struct device *, struct gpio_callback *, gpio_port_pins_t)
{
    if (gpio_pin_get_dt(&sFactoryResetSw) > 0)
    {
        sFactoryResetSwPressedAtMs = k_uptime_get();
        ChipLogProgress(DeviceLayer, "Factory reset button pressed, hold for %dms to reset.", kFactoryResetHoldMs);
    }
    else if (sFactoryResetSwPressedAtMs != 0 && (k_uptime_get() - sFactoryResetSwPressedAtMs) >= kFactoryResetHoldMs)
    {
        ChipLogProgress(DeviceLayer, "Factory reset button held; resetting");
        Server::GetInstance().ScheduleFactoryReset();
    }
    else
    {
        sFactoryResetSwPressedAtMs = 0;
    }
}

CHIP_ERROR InitButtonGpio(const struct gpio_dt_spec & sw, struct gpio_callback & callback, gpio_callback_handler_t handler)
{
    VerifyOrReturnError(gpio_is_ready_dt(&sw), CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "Button GPIO not ready"));
    VerifyOrReturnError(gpio_pin_configure_dt(&sw, GPIO_INPUT) == 0, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Button GPIO configure failed"));
    VerifyOrReturnError(gpio_pin_interrupt_configure_dt(&sw, GPIO_INT_EDGE_BOTH) == 0, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Button GPIO interrupt configure failed"));

    gpio_init_callback(&callback, handler, BIT(sw.pin));
    VerifyOrReturnError(gpio_add_callback(sw.port, &callback) == 0, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Button GPIO add callback failed"));

    return CHIP_NO_ERROR;
}
#endif // ALL_DEVICES_HAS_FACTORY_RESET_SW

} // namespace

CHIP_ERROR Init()
{
#if ALL_DEVICES_HAS_FACTORY_RESET_SW
    ReturnErrorOnFailure(InitButtonGpio(sFactoryResetSw, sFactoryResetSwCallback, OnFactoryResetSwPressed));
#endif // ALL_DEVICES_HAS_FACTORY_RESET_SW

    return CHIP_NO_ERROR;
}

} // namespace chip::app::AllDevices::Button
