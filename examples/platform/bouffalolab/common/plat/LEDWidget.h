/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

class LEDWidget
{
public:
    void Init(void);
    void SetOnoff(bool state);
    bool GetOnoff(void);
    void Toggle(void);

    uint8_t mOnoff;
    uint8_t mPin;
};

class DimmableLEDWidget : public LEDWidget
{
public:
    void Init(void);
    void Toggle(void);
    void SetOnoff(bool state);
    void SetLevel(uint8_t level);

    uint8_t light_v;
};

class ColorLEDWidget : public DimmableLEDWidget
{
public:
    void Init(void);
    void Toggle(void);
    void SetOnoff(bool state);
    void SetLevel(uint8_t level);
    void SetColor(uint8_t level, uint8_t hue, uint8_t sat);

    uint8_t light_h;
    uint8_t light_s;
};
