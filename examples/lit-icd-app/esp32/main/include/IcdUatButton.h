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

#pragma once

#include <driver/gpio.h>
#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class UatButton
{
public:
    using UatButtonPressCallback = void (*)(UatButton *);

    enum class ButtonState : uint8_t
    {
        kIdle = 0,
        kPressed,
        kReleased,
    };

    void Init(gpio_num_t gpioNum, esp_sleep_ext1_wakeup_mode_t wakeMode);
    void SetUatButtonPressCallback(UatButtonPressCallback buttonCallback) { mUatButtonPressCallback = buttonCallback; }
    void GpioIntrEnable(bool enable);

    static void RunEventLoop(void * arg);
    static TaskHandle_t sTaskHandle;
    static QueueHandle_t sEventQueue;

private:
    gpio_num_t mGpioNum;
    ButtonState mState;
    UatButtonPressCallback mUatButtonPressCallback;
};
