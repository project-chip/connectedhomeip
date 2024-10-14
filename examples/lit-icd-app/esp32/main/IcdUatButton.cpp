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

#include "IcdUatButton.h"

#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include <cstdint>

#define ESP_INTR_FLAG_DEFAULT 0

static const char TAG[]              = "Button";
QueueHandle_t UatButton::sEventQueue = nullptr;
TaskHandle_t UatButton::sTaskHandle  = nullptr;

static void IRAM_ATTR gpio_isr_handler(void * arg)
{
    if (UatButton::sEventQueue)
    {
        UatButton * button = (UatButton *) arg;
        button->GpioIntrEnable(false);
        xQueueSendFromISR(UatButton::sEventQueue, &button, NULL);
    }
}

void UatButton::RunEventLoop(void * arg)
{
    bool eventDone     = true;
    UatButton * button = nullptr;

    for (;;)
    {
        if (xQueueReceive(sEventQueue, &button, portMAX_DELAY) == pdTRUE && button)
        {
            button->GpioIntrEnable(false);
            eventDone = false;
        }
        while (!eventDone)
        {
            // GPIO Pull up is enabled so the button is pressed when this value is false.
            bool value = gpio_get_level(button->mGpioNum);
            switch (button->mState)
            {
            case ButtonState::kIdle:
                button->mState = value == false ? ButtonState::kPressed : ButtonState::kIdle;
                break;
            case ButtonState::kPressed:
                button->mState = value == false ? ButtonState::kPressed : ButtonState::kReleased;
                break;
            case ButtonState::kReleased:
                button->mState = ButtonState::kIdle;
                if (button->mUatButtonPressCallback)
                {
                    button->mUatButtonPressCallback(button);
                }
                break;
            default:
                break;
            }
            if (button->mState == ButtonState::kIdle)
            {
                button->GpioIntrEnable(true);
                eventDone = true;
                break;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void UatButton::GpioIntrEnable(bool enable)
{
    if (enable)
    {
        gpio_intr_enable(mGpioNum);
    }
    else
    {
        gpio_intr_disable(mGpioNum);
    }
}

void UatButton::Init(gpio_num_t gpioNum, esp_sleep_ext1_wakeup_mode_t wakeupMode)
{
    mGpioNum              = gpioNum;
    mState                = ButtonState::kIdle;
    gpio_config_t io_conf = {};
    io_conf.intr_type     = GPIO_INTR_LOW_LEVEL;
    io_conf.pin_bit_mask  = (1ULL << static_cast<uint8_t>(mGpioNum));
    io_conf.mode          = GPIO_MODE_INPUT;
    io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en    = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    if (!sEventQueue)
    {
        // create a queue to handle gpio event from isr
        sEventQueue = xQueueCreate(10, sizeof(UatButton *));
        if (!sEventQueue)
        {
            ESP_LOGE(TAG, "Failed to create GPIO EventQueue");
            return;
        }
    }
    if (!sTaskHandle)
    {
        // start gpio task
        xTaskCreate(RunEventLoop, "UatButton", 4096, nullptr, 10, &sTaskHandle);
        if (!sTaskHandle)
        {
            ESP_LOGE(TAG, "Failed to create GPIO Task");
            return;
        }
    }
    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(mGpioNum, gpio_isr_handler, (void *) this);
    ESP_LOGI(TAG, "UAT Button initialized..");
    // Configure RTC IO wake up
    esp_sleep_enable_ext1_wakeup(1ULL << static_cast<uint8_t>(mGpioNum), wakeupMode);
#if SOC_RTCIO_INPUT_OUTPUT_SUPPORTED
    rtc_gpio_pulldown_dis(mGpioNum);
    rtc_gpio_pullup_en(mGpioNum);
#else
    gpio_pulldown_dis(mGpioNum);
    gpio_pullup_en(mGpioNum);
#endif
}
