/*
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
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
