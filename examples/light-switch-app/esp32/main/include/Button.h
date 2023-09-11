/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver/gpio.h"
#include "esp_log.h"

class Button
{
public:
    typedef void (*ButtonPressCallback)(void);

    void Init(void);
    void SetButtonPressCallback(ButtonPressCallback button_callback);
};
