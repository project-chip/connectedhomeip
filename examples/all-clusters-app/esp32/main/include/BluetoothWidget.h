/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class BluetoothWidget
{
public:
    void Init();
    void Set(bool state);
    void SetVLED(int id);

private:
    int mVLED;
    bool mState;
};
