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

#include "Core2Power.h"

#if CONFIG_DEVICE_TYPE_M5STACK_CORE2

#include "AXP192.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Core2Power {

namespace {

constexpr const char TAG[] = "Core2Power";

constexpr gpio_num_t kSDA = GPIO_NUM_21;
constexpr gpio_num_t kSCL = GPIO_NUM_22;

// Runs one step of the power-on sequence, logging and returning on the first failure.
#define CORE2_POWER_STEP(expr, what)                                                                                             \
    do                                                                                                                           \
    {                                                                                                                            \
        esp_err_t _err = (expr);                                                                                                 \
        if (_err != ESP_OK)                                                                                                      \
        {                                                                                                                        \
            ESP_LOGE(TAG, "%s failed: %s", what, esp_err_to_name(_err));                                                         \
            return _err;                                                                                                        \
        }                                                                                                                        \
    } while (0)

} // namespace

esp_err_t Init()
{
    CORE2_POWER_STEP(AXP192::Init(kInternalI2CPort, kSDA, kSCL), "AXP192 init");

    // LCD backlight rail (DCDC3): 2.8V, matches the panel's rated backlight voltage.
    CORE2_POWER_STEP(AXP192::SetDCDC3Voltage(2800), "Set DCDC3 (backlight) voltage");
    // LCD logic rail (LDO2): 3.3V.
    CORE2_POWER_STEP(AXP192::SetLDOVoltage(2, 3300), "Set LDO2 (LCD logic) voltage");

    // Enable both rails.
    CORE2_POWER_STEP(AXP192::SetLDOEnable(2, true), "Enable LDO2");
    CORE2_POWER_STEP(AXP192::SetDCDC3Enable(true), "Enable DCDC3");

    // GPIO4 drives the LCD's RESET pin; configure it as a push-pull output.
    CORE2_POWER_STEP(AXP192::SetGPIO4PushPullOutput(), "Configure GPIO4");

    // Pulse the LCD reset line: hold low, then release.
    CORE2_POWER_STEP(AXP192::SetGPIO4Level(false), "Assert LCD reset");
    vTaskDelay(pdMS_TO_TICKS(100));
    CORE2_POWER_STEP(AXP192::SetGPIO4Level(true), "Release LCD reset");
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "AXP192 LCD power sequence complete");
    return ESP_OK;
}

#undef CORE2_POWER_STEP

void SetBacklight(bool on)
{
    esp_err_t err = AXP192::SetDCDC3Enable(on);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to %s backlight: %s", on ? "enable" : "disable", esp_err_to_name(err));
    }
}

} // namespace Core2Power

#endif // CONFIG_DEVICE_TYPE_M5STACK_CORE2
