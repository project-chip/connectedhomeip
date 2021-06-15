/*
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

#pragma once

#include <device.h>
#include <vector>

#define STATE_HIGH 1
#define STATE_LOW 0

class Button
{
public:
    void Configure(const struct device * port, gpio_pin_t outPin, gpio_pin_t inPin, void (*callback)(void));
    void Poll(Button * previous);
    void SetCallback(void (*callback)(void));

private:
    int Init(void);
    int Deinit(void);

    const struct device * mPort;
    gpio_pin_t mOutPin;
    gpio_pin_t mInPin;
    int mPreviousState      = STATE_LOW;
    void (*mCallback)(void) = NULL;
};

class ButtonManager
{
public:
    void Init(void);
    void Poll(void);
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
