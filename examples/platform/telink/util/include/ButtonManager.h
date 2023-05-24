/*
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

#pragma once

#include <vector>
#include <zephyr/device.h>

#define STATE_HIGH 1
#define STATE_LOW 0

class Button
{
public:
    void Configure(const gpio_dt_spec * input_button_dt, const gpio_dt_spec * output_button_dt, void (*callback)(void));
    void Configure(const gpio_dt_spec * input_button_dt, void (*callback)(void));
    void Poll(Button * previous);
    void PollIRQ(const struct device * dev, uint32_t pins);
    void SetCallback(void (*callback)(void));

private:
    int Init(void);
    int Deinit(void);

    const struct gpio_dt_spec * mInput_button;
    const struct gpio_dt_spec * mOutput_matrix_pin;
    int mPreviousState = STATE_LOW;
    struct gpio_callback mButton_cb_data;
    void (*mCallback)(void) = NULL;
};

class ButtonManager
{
public:
    void Init(void);
    void Poll(void);
    void PollIRQ(const struct device * dev, uint32_t pins);
    void AddButton(Button & button);
    void SetCallback(unsigned int index, void (*callback)(void));

private:
    std::vector<Button> mButtons;

    friend ButtonManager & ButtonManagerInst(void);

    static ButtonManager sInstance;
};

/**
 * Returns the KeyManager singleton object.
 */
inline ButtonManager & ButtonManagerInst(void)
{
    return ButtonManager::sInstance;
}
