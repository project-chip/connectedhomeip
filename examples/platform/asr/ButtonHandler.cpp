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
#elif !defined CFG_PLF_DUET
#define duet_gpio_dev_t lega_gpio_dev_t
#define duet_gpio_init lega_gpio_init
#define duet_gpio_input_get lega_gpio_input_get
#define DUET_INPUT_PULL_UP LEGA_INPUT_PULL_UP
#endif

TaskHandle_t sGpioTaskHandle;
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
    // generic switch2 button
    switch2_btn.port   = SWITCH2_BUTTON;
    switch2_btn.config = DUET_INPUT_PULL_UP;
    switch2_btn.priv   = NULL;
    duet_gpio_init(&switch2_btn);
}

static uint32_t btn1Value   = 1;
static uint8_t btn1_trigger = 0;
static uint32_t btn2Value   = 1;
static uint8_t btn2_trigger = 0;
void GpioTaskMain(void * pvParameter)
{
    ASR_LOG("GPIO Task started");
    uint32_t btnValue;
    uint8_t buttonevent = 0;
    while (true)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        // switch button 1
        duet_gpio_input_get(&switch1_btn, &btnValue);
        if (btnValue != btn1Value)
        {
            if (btn1_trigger)
            {
                btn1Value   = btnValue;
                buttonevent = (uint8_t) btnValue;
                GetAppTask().ButtonEventHandler(SWITCH1_BUTTON, (buttonevent) ? BUTTON_RELEASED : BUTTON_PRESSED);
                btn1_trigger = 0;
            }
            else
            {
                btn1_trigger = 1;
            }
        }
        else
        {
            btn1_trigger = 0;
        }
        // switch button 2
        duet_gpio_input_get(&switch2_btn, &btnValue);
        if (btnValue != btn2Value)
        {
            if (btn2_trigger)
            {
                btn2Value   = btnValue;
                buttonevent = (uint8_t) btnValue;
                GetAppTask().ButtonEventHandler(SWITCH2_BUTTON, (buttonevent) ? BUTTON_RELEASED : BUTTON_PRESSED);
                btn2_trigger = 0;
            }
            else
            {
                btn2_trigger = 1;
            }
        }
        else
        {
            btn2_trigger = 0;
        }
    }
}
