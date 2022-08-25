/*
 *
 *    Copyright (c) 2019 Google LLC.
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

// clang-format off
#include "app-common/app-common/zap-generated/enums.h"
#include "app-common/app-common/zap-generated/cluster-enums.h"
// clang-format on

#include <lcd.h>

#include <stdint.h>

enum class LcdIcon
{
    None = 0,
    One,
    Two,
    Lift,
    Tilt
};

class PixelPainter
{
public:
    PixelPainter(uint16_t lift, uint16_t tilt);
    virtual ~PixelPainter()                       = default;
    virtual uint8_t Color(uint32_t x, uint32_t y) = 0;

protected:
    uint16_t mLift;
    uint16_t mTilt;
};

class CompositePainter : public PixelPainter
{
public:
    CompositePainter(uint16_t lift, uint16_t tilt, PixelPainter * painter1, PixelPainter * painter2, PixelPainter * painter3);
    uint8_t Color(uint32_t x, uint32_t y);

private:
    PixelPainter * mPainter1;
    PixelPainter * mPainter2;
    PixelPainter * mPainter3;
};

class FramePainter : public PixelPainter
{
    using PixelPainter::PixelPainter;
    uint8_t Color(uint32_t x, uint32_t y);
};

class IconPainter : public PixelPainter
{
public:
    IconPainter(uint16_t lift, uint16_t tilt, LcdIcon icon);
    uint8_t Color(uint32_t x, uint32_t y);

private:
    uint8_t mIconSize;
    uint8_t mIconOffset;
    LcdIcon mIcon;
};

class VerticalShadePainter : public PixelPainter
{
public:
    using PixelPainter::PixelPainter;
    uint8_t Color(uint32_t x, uint32_t y);
};

class HorizontalShadePainter : public PixelPainter
{
public:
    using PixelPainter::PixelPainter;
    uint8_t Color(uint32_t x, uint32_t y);
};

class VerticalBlindPainter : public PixelPainter
{
public:
    VerticalBlindPainter(uint16_t lift, uint16_t tilt);
    uint8_t Color(uint32_t x, uint32_t y);

private:
    constexpr static uint8_t sBandCount = 12;
    uint8_t mBandSize;
};

class LcdPainter
{
public:
    static void Paint(SilabsLCD & lcd, chip::app::Clusters::WindowCovering::Type type, uint16_t lift, uint16_t tilt, LcdIcon icon);

private:
    static PixelPainter * GetCoverPainter(chip::app::Clusters::WindowCovering::Type type, uint16_t lift, uint16_t tilt);
};
