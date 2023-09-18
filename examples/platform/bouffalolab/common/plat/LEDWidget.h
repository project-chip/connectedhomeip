/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdint.h>

class LEDWidget
{
public:
    virtual void Init(void)           = 0;
    virtual void SetOnoff(bool state) = 0;
    bool GetOnoff(void);
    virtual void Toggle(void) = 0;
    virtual ~LEDWidget(void){};

    uint8_t mOnoff;
    uint8_t mPin;
};

class DimmableLEDWidget : public LEDWidget
{
public:
    void Init(void) override;
    void Toggle(void) override;
    void SetOnoff(bool state) override;
    void SetLevel(uint8_t level);
    ~DimmableLEDWidget(void) {}

    uint8_t light_v;
};

class ColorLEDWidget : public DimmableLEDWidget
{
public:
    void Init(void) override;
    void Toggle(void) override;
    void SetOnoff(bool state) override;
    void SetLevel(uint8_t level);
    void SetColor(uint8_t level, uint8_t hue, uint8_t sat);
    ~ColorLEDWidget(void) {}

    uint8_t light_h;
    uint8_t light_s;
};
