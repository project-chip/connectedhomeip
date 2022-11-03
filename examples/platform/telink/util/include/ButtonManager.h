/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>
#include <zephyr/device.h>

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
