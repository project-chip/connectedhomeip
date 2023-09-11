/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

struct RgbColor_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct HsvColor_t
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
};

struct XyColor_t
{
    uint16_t x;
    uint16_t y;
};

struct CtColor_t
{
    uint16_t ctMireds;
};

RgbColor_t XYToRgb(uint8_t Level, uint16_t currentX, uint16_t currentY);
RgbColor_t HsvToRgb(HsvColor_t hsv);
RgbColor_t CTToRgb(CtColor_t ct);
