/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ButtonManager);

#include <ButtonManager.h>

ButtonManager ButtonManager::sInstance;

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
static struct gpio_callback button_cb_data;
void button_pressed(const struct device * dev, struct gpio_callback * cb, uint32_t pins);
#endif

void Button::Configure(const struct device * port, gpio_pin_t outPin, gpio_pin_t inPin, void (*callback)(void))
{
    if (!device_is_ready(port))
    {
        LOG_ERR("%s is not ready\n", port->name);
    }

    mPort     = port;
    mOutPin   = outPin;
    mInPin    = inPin;
    mCallback = callback;
}

int Button::Init(void)
{
    int ret = 0;

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    ret = gpio_pin_configure(mPort, mInPin, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure(mPort, mInPin, GPIO_INT_EDGE_FALLING);
    if (ret < 0)
    {
        LOG_ERR("Config irq pin err: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_pressed, mInPin);
    ret = gpio_add_callback(mPort, &button_cb_data);
    if (ret < 0)
    {
        LOG_ERR("Config gpio_init_callback err: %d", ret);
        return ret;
    }
#else

    ret = gpio_pin_configure(mPort, mOutPin, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Config out pin err: %d", ret);
        return ret;
    }

    ret = gpio_pin_configure(mPort, mInPin, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
#endif

    return ret;
}

int Button::Deinit(void)
{
    int ret = 0;

    /* Reconfigure output key pin to input */
    ret = gpio_pin_configure(mPort, mOutPin, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Reconfig out pin err: %d", ret);
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

    if (ret == STATE_HIGH && ret != mPreviousState)
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

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
void button_pressed(const struct device * dev, struct gpio_callback * cb, uint32_t pins)
{
    ButtonManager & sInstance = ButtonManagerInst();
    sInstance.PollIRQ();
}

void ButtonManager::PollIRQ(void)
{
    for (unsigned int i = 0; i < mButtons.size(); i++)
    {
        mButtons[i].PollIRQ();
    }
}

void Button::PollIRQ()
{
    int ret = gpio_pin_get(mPort, mInPin);
    if (ret == STATE_LOW)
    {
        if (mCallback != NULL)
        {
            mCallback();
        }
    }
}

void Button::Configure(const struct device * port, gpio_pin_t inPin, void (*callback)(void))
{
    if (!device_is_ready(port))
    {
        LOG_ERR("%s is not ready\n", port->name);
    }

    mPort     = port;
    mInPin    = inPin;
    mCallback = callback;

    Init();
}

#else
K_THREAD_DEFINE(buttonThread, 512, ButtonEntry, NULL, NULL, NULL, K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1), 0, 0);
#endif
