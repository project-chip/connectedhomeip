/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LEDWidget.h"

#include <array>

// A lightweight wrapper for unused LEDs
template <uint8_t size>
class FactoryResetLEDsWrapper
{
public:
    using Gpio = uint32_t;
    using Leds = std::array<std::pair<Gpio, LEDWidget>, size>;

    explicit FactoryResetLEDsWrapper(std::array<Gpio, size> aLeds)
    {
        auto idx{ 0 };
        for (const auto & led : aLeds)
            mLeds[idx++] = std::make_pair(led, LEDWidget());
        Init();
    }
    void Set(bool aState)
    {
        for (auto & led : mLeds)
            led.second.Set(aState);
    }
    void Blink(uint32_t aRate)
    {
        for (auto & led : mLeds)
            led.second.Blink(aRate);
    }

private:
    void Init()
    {
        LEDWidget::InitGpio();
        for (auto & led : mLeds)
            led.second.Init(led.first);
    }
    Leds mLeds;
};
