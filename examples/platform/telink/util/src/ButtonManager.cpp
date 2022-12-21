/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <assert.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_REGISTER(ButtonManager);

#include <ButtonManager.h>

ButtonManager ButtonManager::sInstance;

void Button::Configure(const struct device * port, gpio_pin_t outPin, gpio_pin_t inPin, bool intBothLevel, void (*callback)(void))
{
    __ASSERT(device_is_ready(port), "%s is not ready\n", port->name);

    mPort         = port;
    mOutPin       = outPin;
    mInPin        = inPin;
    mIntBothLevel = intBothLevel;
    mCallback     = callback;
}

int Button::Init(void)
{
    int ret = 0;

    ret = gpio_pin_configure(mPort, mOutPin, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Configure out pin - fail. Status %d", ret);
        return ret;
    }

    ret = gpio_pin_configure(mPort, mInPin, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Configure in pin - fail. Status %d", ret);
        return ret;
    }

    return ret;
}

int Button::Deinit(void)
{
    int ret = 0;

    /* Reconfigure output key pin to input */
    ret = gpio_pin_configure(mPort, mOutPin, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Reconfigure out pin - fail. Status %d", ret);
        return ret;
    }

    return ret;
}

void Button::Poll(Button * previous)
{
    int ret = 0;

    if (previous != NULL)
    {
        ret = previous->Deinit();
        assert(ret >= 0);
    }

    ret = Init();
    assert(ret >= 0);

    ret = gpio_pin_get(mPort, mInPin);
    assert(ret >= 0);

    if ((mIntBothLevel && ret != mPreviousState) || (!mIntBothLevel && ret == STATE_HIGH && ret != mPreviousState))
    {
        if (mCallback != NULL)
        {
            mCallback();
        }
    }

    mPreviousState = ret;

    k_msleep(10);
}

void Button::SetCallback(void (*callback)(void))
{
    mCallback = callback;
}

void ButtonManager::AddButton(Button & button)
{
    mButtons.push_back(button);
}

void ButtonManager::SetCallback(unsigned int index, void (*callback)(void))
{
    if (mButtons.size() <= index)
    {
        LOG_ERR("Wrong button index");
    }

    mButtons[index].SetCallback(callback);
}

void ButtonManager::Poll(void)
{
    static Button * previous = NULL;

    for (unsigned int i = 0; i < mButtons.size(); i++)
    {
        mButtons[i].Poll(previous);
        previous = &mButtons[i];
    }

    k_msleep(10);
}

void ButtonEntry(void * param1, void * param2, void * param3)
{
    ButtonManager & sInstance = ButtonManagerInst();

    while (true)
    {
        sInstance.Poll();
    }
}

K_THREAD_DEFINE(buttonThread, 512, ButtonEntry, NULL, NULL, NULL, K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1), 0, 0);
