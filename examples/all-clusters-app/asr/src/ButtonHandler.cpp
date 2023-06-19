/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ButtonHandler.h"
#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>

#ifdef CFG_PLF_RV32
#define duet_gpio_dev_t asr_gpio_dev_t
#define duet_gpio_init asr_gpio_init
#define duet_gpio_input_get asr_gpio_input_get
#define DUET_INPUT_PULL_UP ASR_INPUT_PULL_UP
#endif

TaskHandle_t sGpioTaskHandle;
ButtonHandler::KeyInformation ButtonHandler::keyInfo = { 0 };

static void GpioTaskMain(void * pvParameter);

void ButtonHandler::Init(void)
{
    GpioInit();

    xTaskCreate(GpioTaskMain, GPIO_TASK_NAME, GPIO_TASK_STACK_SIZE, 0, 2, &sGpioTaskHandle);
}

// port pin
duet_gpio_dev_t switch1_btn;
duet_gpio_dev_t switch2_btn;

void ButtonHandler::GpioInit(void)
{
    // light switch1 button
    switch1_btn.port   = SWITCH1_BUTTON;
    switch1_btn.config = DUET_INPUT_PULL_UP;
    switch1_btn.priv   = NULL;
    duet_gpio_init(&switch1_btn);

    switch2_btn.port   = SWITCH2_BUTTON;
    switch2_btn.config = DUET_INPUT_PULL_UP;
    switch2_btn.priv   = NULL;
    duet_gpio_init(&switch2_btn);
}

void GpioTaskMain(void * pvParameter)
{
    ASR_LOG("GPIO Task started");
    uint32_t btnValue;
    uint8_t currentKeys = 0;

    for (;;)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);

        duet_gpio_input_get(&switch1_btn, &btnValue);

        if (!btnValue)
        {
            currentKeys |= 0x01;
        }
        else
        {
            currentKeys &= ~0x01;
        }

        duet_gpio_input_get(&switch2_btn, &btnValue);

        if (!btnValue)
        {
            currentKeys |= 0x02;
        }
        else
        {
            currentKeys &= ~0x02;
        }

        ButtonHandler::keyInfo.keyImPulse      = (ButtonHandler::keyInfo.keyDown ^ currentKeys) & ~ButtonHandler::keyInfo.keyDown;
        ButtonHandler::keyInfo.keyReleasePulse = (ButtonHandler::keyInfo.keyDown ^ currentKeys) & ButtonHandler::keyInfo.keyDown;
        ButtonHandler::keyInfo.keyDown         = currentKeys;

        if (ButtonHandler::keyInfo.keyImPulse & 0x01)
        {
            GetAppTask().PostButtonEvent(SWITCH1_BUTTON, APP_BUTTON_PRESSED);
        }

        if (ButtonHandler::keyInfo.keyImPulse & 0x02)
        {
            GetAppTask().PostButtonEvent(SWITCH2_BUTTON, APP_BUTTON_PRESSED);
        }

        if (ButtonHandler::keyInfo.keyReleasePulse & 0x01)
        {
            GetAppTask().PostButtonEvent(SWITCH1_BUTTON, APP_BUTTON_RELEASED);
        }

        if (ButtonHandler::keyInfo.keyReleasePulse & 0x02)
        {
            GetAppTask().PostButtonEvent(SWITCH2_BUTTON, APP_BUTTON_RELEASED);
        }
    }
}
