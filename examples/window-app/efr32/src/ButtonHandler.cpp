/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <AppConfig.h>
#include <AppTask.h>
#include <ButtonHandler.h>

#include <bsp.h>
#include <hal-config-board.h>

static const ButtonHandler::ButtonConfig_t sButtonsConfig[BSP_BUTTON_COUNT] = BSP_BUTTON_INIT;

ButtonHandler ButtonHandler::sInstance;

ButtonHandler & ButtonHandler::Instance()
{
    return sInstance;
}

ButtonHandler::ButtonHandler() : mButtonUp(kButton_Up, "BTN:UP"), mButtonDown(kButton_Down, "BTN:DOWN") {}

void ButtonHandler::Init()
{
    GPIOINT_Init();
    mButtonUp.Init(sButtonsConfig[0]);
    mButtonDown.Init(sButtonsConfig[1]);

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);
}

bool ButtonHandler::IsPressed(ButtonId_t button)
{
    if (kButton_Up == button)
    {
        return mButtonUp.mIsPressed;
    }
    else if (kButton_Down == button)
    {
        return mButtonDown.mIsPressed;
    }
    return false;
}

ButtonHandler::Button::Button(ButtonId_t id, const char * tag) : mId(id), mName(tag), mIsPressed(false) {}

void ButtonHandler::Button::Init(const ButtonConfig_t & config)
{
    // GPIO
    mPort = config.port;
    mPin  = config.pin;
    GPIO_PinModeSet(mPort, mPin, gpioModeInputPull, 1);
    GPIO_IntConfig(mPort, mPin, true, true, true);
    GPIOINT_CallbackRegister(mPin, ButtonISR);
    // Timer
    mTimer.Init(this, mName, APP_BUTTON_DEBOUNCE_PERIOD_MS, ButtonTimerCallback);
}

void ButtonHandler::ButtonISR(uint8_t pin)
{
    Button & btn   = (sInstance.mButtonUp.mPin == pin) ? sInstance.mButtonUp : sInstance.mButtonDown;
    btn.mIsPressed = !GPIO_PinInGet(btn.mPort, btn.mPin);
    btn.mTimer.IsrStart();
}

void ButtonHandler::ButtonTimerCallback(AppTimer & timer, void * context)
{
    Button * btn = (Button *) context;
    if (btn)
    {
        btn->mIsPressed           = !GPIO_PinInGet(btn->mPort, btn->mPin);
        AppEvent::EventType event = btn->mIsPressed ? AppEvent::EventType::ButtonPressed : AppEvent::EventType::ButtonReleased;
        AppTask::Instance().PostEvent(AppEvent(event, btn));
    }
}
