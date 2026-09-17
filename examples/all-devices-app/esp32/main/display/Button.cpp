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

#include "Button.h"
#include "Display.h"
#include "ScreenManager.h"
#include <esp_log.h>
#include <platform/PlatformManager.h>

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

static const char TAG[] = "Button";

void IRAM_ATTR button_isr_handler(void * arg)
{
    auto * button = static_cast<Button *>(arg);
    if (button != nullptr && button->mButtonTimer != nullptr)
    {
        xTimerStartFromISR(button->mButtonTimer, nullptr);
    }
}

void Button::TimerCallback(TimerHandle_t xTimer)
{
    auto * button = static_cast<Button *>(pvTimerGetTimerID(xTimer));
    if (button == nullptr)
    {
        return;
    }

    int state = gpio_get_level(button->mGPIONum);
    if (state == APP_BUTTON_PRESSED)
    {
        WakeDisplay();
        int buttonId = 40 - button->mGPIONum;
        LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t arg) { ScreenManager::ButtonPressed(static_cast<int>(arg)); }, static_cast<intptr_t>(buttonId)));
    }
}

esp_err_t Button::Init()
{
    if (mGPIONum == GPIO_NUM_NC)
    {
        return ESP_FAIL;
    }
    return Init(mGPIONum);
}

esp_err_t Button::Init(gpio_num_t gpioNum)
{
    mGPIONum = gpioNum;

    gpio_config_t io_conf = {};
    io_conf.intr_type     = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask  = (1ULL << gpioNum);
    io_conf.mode          = GPIO_MODE_INPUT;
    io_conf.pull_up_en    = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_config failed for pin %d: %s", gpioNum, esp_err_to_name(err));
        return err;
    }

    mButtonTimer = xTimerCreate("BtnTmr", pdMS_TO_TICKS(50), pdFALSE, this, TimerCallback);
    if (mButtonTimer == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create debounce timer for button on pin %d", gpioNum);
        return ESP_FAIL;
    }

    err = gpio_isr_handler_add(gpioNum, button_isr_handler, this);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio_isr_handler_add failed for pin %d: %s", gpioNum, esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
