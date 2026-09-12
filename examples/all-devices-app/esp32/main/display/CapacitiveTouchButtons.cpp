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

#include "CapacitiveTouchButtons.h"

#include "Display.h"
#include "ScreenManager.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <platform/PlatformManager.h>

// No dedicated task/stack here on purpose: unlike a GPIO-interrupt button, a
// polled touch controller needs *something* to wake up periodically, but a
// software timer piggybacks on the existing FreeRTOS Timer Service task
// (already running for Button.cpp's own debounce timers on other boards)
// instead of paying for a new task stack. 20 Hz is plenty for menu
// navigation and comfortably above this touch controller family's own scan
// rate ceiling, so this doesn't spin the I2C bus or the CPU any harder than
// needed.

namespace CapacitiveTouchButtons {

namespace {

constexpr const char TAG[] = "TouchButtons";

// 7-bit I2C slave address and touch-point register layout shared by the
// FT6206/FT6236/FT6336/FT5x06 family. TD_STATUS (number of active touch
// points) is immediately followed by the first touch point's XH/XL/YH/YL
// registers, so both are read in one burst starting at kRegTdStatus.
constexpr uint8_t kTouchAddress = 0x38;
constexpr uint8_t kRegTdStatus  = 0x02;

// Only the bottom band of the screen acts as the button bar; touches above it
// are assumed to be interacting with the list/menu content instead.
constexpr uint16_t kButtonBarHeightPercent = 20;

constexpr TickType_t kPollInterval = pdMS_TO_TICKS(50);
// Minimum time between two accepted presses, to absorb the noisy point-count
// blips the controller can report for a couple of scans as a finger lands.
constexpr TickType_t kPressDebounce = pdMS_TO_TICKS(150);

TimerHandle_t gPollTimer = nullptr;
i2c_port_t gI2CPort      = I2C_NUM_0;
bool gWasPressed         = false;
TickType_t gLastPressed  = 0;

esp_err_t ReadTouchRegs(uint8_t startReg, uint8_t * data, size_t len)
{
    return i2c_master_write_read_device(gI2CPort, kTouchAddress, &startReg, 1, data, len, pdMS_TO_TICKS(50));
}

// Reads the first touch point, if any. Returns true and fills x/y (in raw
// panel coordinates) if a finger is currently down.
//
// NOTE: if button zones come out mirrored or vertically flipped on real
// hardware, that means this board's digitizer isn't screen-aligned the way
// the reference Core2 is; swap x/y below, or flip the button-bar comparison.
bool ReadTouchPoint(uint16_t & x, uint16_t & y)
{
    uint8_t regs[5];
    if (ReadTouchRegs(kRegTdStatus, regs, sizeof(regs)) != ESP_OK || (regs[0] & 0x0F) == 0)
    {
        return false;
    }

    x = static_cast<uint16_t>(((regs[1] & 0x0F) << 8) | regs[2]);
    y = static_cast<uint16_t>(((regs[3] & 0x0F) << 8) | regs[4]);
    return true;
}

// Maps a horizontal touch position to one of the 3 virtual buttons, using the
// same left-to-right ID convention as the physical M5Stack Basic/Gray buttons
// (id 1 = leftmost).
int ButtonIdForX(uint16_t x)
{
    if (DisplayWidth == 0)
    {
        return 1;
    }
    uint32_t third = (static_cast<uint32_t>(x) * 3) / DisplayWidth;
    if (third >= 3)
    {
        third = 2;
    }
    return static_cast<int>(third) + 1;
}

void PollTimerCallback(TimerHandle_t)
{
    const uint16_t barTop = static_cast<uint16_t>(DisplayHeight - (DisplayHeight * kButtonBarHeightPercent) / 100);

    uint16_t x = 0, y = 0;
    bool isPressed = ReadTouchPoint(x, y) && y >= barTop;

    if (isPressed && !gWasPressed)
    {
        TickType_t now = xTaskGetTickCount();
        if (now - gLastPressed >= kPressDebounce)
        {
            gLastPressed = now;
            int buttonId = ButtonIdForX(x);
            ESP_LOGD(TAG, "Touch at (%u, %u) -> button %d", x, y, buttonId);
            WakeDisplay();
            LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(
                [](intptr_t arg) { ScreenManager::ButtonPressed(static_cast<int>(arg)); }, static_cast<intptr_t>(buttonId)));
        }
    }
    gWasPressed = isPressed;
}

} // namespace

esp_err_t Init(i2c_port_t i2cPort)
{
    gI2CPort   = i2cPort;
    gPollTimer = xTimerCreate("TouchButtons", kPollInterval, pdTRUE, nullptr, PollTimerCallback);
    if (gPollTimer == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create touch polling timer");
        return ESP_FAIL;
    }
    if (xTimerStart(gPollTimer, 0) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to start touch polling timer");
        return ESP_FAIL;
    }
    return ESP_OK;
}

} // namespace CapacitiveTouchButtons
