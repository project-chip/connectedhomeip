/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <LcdPainter.h>
#include <display.h>
#include <lcd.h>

constexpr uint32_t sTiltIcon[] = {
    0xffffffff, 0xffffffff, 0xc0000003, 0xc0000003, 0xc0000003, 0xc0000003, 0xc1ffffc3, 0xc1ffffc3,
    0xc1ffffc3, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003,
    0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003, 0xc003c003,
    0xc003c003, 0xc003c003, 0xc0000003, 0xc0000003, 0xc0000003, 0xc0000003, 0xffffffff, 0xffffffff,
};

constexpr uint32_t sLiftIcon[] = {
    0xffffffff, 0xffffffff, 0xc0000003, 0xc0000003, 0xc0000003, 0xc0000003, 0xc0f00003, 0xc0f00003,
    0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003,
    0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0f00003, 0xc0ffffc3,
    0xc0ffffc3, 0xc0ffffc3, 0xc0000003, 0xc0000003, 0xc0000003, 0xc0000003, 0xffffffff, 0xffffffff,
};

constexpr uint32_t sOneIcon[] = {
    0xffffffff, 0xffffffff, 0xc0000003, 0xc0000003, 0xc0000003, 0xc00fe003, 0xc01fe003, 0xc03fe003,
    0xc07fe003, 0xc0fbe003, 0xc1f3e003, 0xc3e3e003, 0xc003e003, 0xc003e003, 0xc003e003, 0xc003e003,
    0xc003e003, 0xc003e003, 0xc003e003, 0xc003e003, 0xc003e003, 0xc003e003, 0xc003e003, 0xc3ffffc3,
    0xc3ffffc3, 0xc3ffffc3, 0xc3ffffc3, 0xc0000003, 0xc0000003, 0xc0000003, 0xffffffff, 0xffffffff,
};

constexpr uint32_t sTwoIcon[] = {
    0xffffffff, 0xffffffff, 0xc0000003, 0xc0000003, 0xc0000003, 0xc07ffe03, 0xc0ffff03, 0xc1ffff83,
    0xc3ffffc3, 0xc3f00fc3, 0xc3e007c3, 0xc3e007c3, 0xc00007c3, 0xc00007c3, 0xc0000fc3, 0xc00fff83,
    0xc03fff03, 0xc07ffe03, 0xc0fc0003, 0xc1f80003, 0xc1f00003, 0xc3e00003, 0xc3e00003, 0xc3ffffc3,
    0xc3ffffc3, 0xc3ffffc3, 0xc3ffffc3, 0xc0000003, 0xc0000003, 0xc0000003, 0xffffffff, 0xffffffff,
};

PixelPainter::PixelPainter(uint8_t lift, uint8_t tilt) : mLift(lift), mTilt(tilt) {}

CompositePainter::CompositePainter(uint8_t lift, uint8_t tilt, PixelPainter * painter1, PixelPainter * painter2,
                                   PixelPainter * painter3) :
    PixelPainter(lift, tilt),
    mPainter1(painter1), mPainter2(painter2), mPainter3(painter3)
{}

uint8_t CompositePainter::Color(uint32_t x, uint32_t y)
{
    int8_t pixel = -1;
    if (mPainter1)
    {
        pixel = mPainter1->Color(x, y);
        if (pixel >= 0)
        {
            return pixel;
        }
    }
    if (mPainter2)
    {
        pixel = mPainter2->Color(x, y);
        if (pixel >= 0)
        {
            return pixel;
        }
    }
    if (mPainter3)
    {
        pixel = mPainter3->Color(x, y);
        if (pixel >= 0)
        {
            return pixel;
        }
    }
    return pixel;
}

uint8_t FramePainter::Color(uint32_t x, uint32_t y)
{
    if (x < LCD_FRAME_SIZE || x >= (LCD_SIZE - LCD_FRAME_SIZE) || y < LCD_FRAME_SIZE || y >= (LCD_SIZE - LCD_FRAME_SIZE))
    {
        if (x < LCD_MARGIN_SIZE || x >= (LCD_SIZE - LCD_MARGIN_SIZE) || y < LCD_MARGIN_SIZE || y >= (LCD_SIZE - LCD_MARGIN_SIZE))
        {
            return 0;
        }
        else
        {
            return (x < (LCD_MARGIN_SIZE + LCD_BORDER_SIZE)) || (x >= (LCD_SIZE - LCD_BORDER_SIZE - LCD_MARGIN_SIZE)) ||
                (y < (LCD_MARGIN_SIZE + LCD_BORDER_SIZE)) || (y >= (LCD_SIZE - LCD_BORDER_SIZE - LCD_MARGIN_SIZE));
        }
    }
    return -1;
}

IconPainter::IconPainter(uint8_t lift, uint8_t tilt, LcdIcon icon) : PixelPainter(lift, tilt), mIcon(icon)
{
    mIconSize   = sizeof(sTiltIcon) / sizeof(uint32_t);
    mIconOffset = (LCD_SIZE - mIconSize) / 2;
}

uint8_t IconPainter::Color(uint32_t x, uint32_t y)
{
    const uint32_t * icon = nullptr;
    switch (mIcon)
    {
    case LcdIcon::One:
        icon = sOneIcon;
        break;
    case LcdIcon::Two:
        icon = sTwoIcon;
        break;
    case LcdIcon::Lift:
        icon = sLiftIcon;
        break;
    case LcdIcon::Tilt:
        icon = sTiltIcon;
        break;
    default:
        return -1;
    }
    if (x >= mIconOffset && x < (mIconOffset + mIconSize) && y >= mIconOffset && y < (mIconOffset + mIconSize))
    {
        uint32_t x0 = x - mIconOffset;
        uint32_t y0 = y - mIconOffset;
        return (icon[y0] & (1 << (31 - x0))) > 0;
    }
    return -1;
}

uint8_t VerticalShadePainter::Color(uint32_t x, uint32_t y)
{
    return (y % 2) && y < (uint32_t)(LCD_FRAME_SIZE + mLift);
}

uint8_t HorizontalShadePainter::Color(uint32_t x, uint32_t y)
{
    return (x % 2) && x < (uint32_t)(LCD_FRAME_SIZE + mLift);
}

VerticalBlindPainter::VerticalBlindPainter(uint8_t lift, uint8_t tilt) : PixelPainter(lift, tilt)
{
    mBandSize = (LCD_COVER_SIZE / sBandCount);
}

uint8_t VerticalBlindPainter::Color(uint32_t x, uint32_t y)
{
    if (x % 2)
    {
        return 0;
    }
    else
    {
        uint32_t closedCount = (mLift + 1) / mBandSize;
        uint32_t bandCount   = (y - LCD_FRAME_SIZE) / mBandSize;
        // ChipLogProgress(Zcl, "BLIND: ccount:%u, ccount:%u", clusterId);
        if (bandCount <= closedCount)
        {
            return y <= (LCD_FRAME_SIZE + mBandSize * bandCount + mTilt);
        }
        else
        {
            return 0;
        }
    }
}

PixelPainter * LcdPainter::GetCoverPainter(EmberAfWcType type, uint8_t lift, uint8_t tilt)
{
    switch (type)
    {
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE:
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE2_MOTOR:
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE_EXTERIOR:
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE_EXTERIOR2_MOTOR:
        return new VerticalShadePainter(lift, tilt);

    case EMBER_ZCL_WC_TYPE_DRAPERY:
    case EMBER_ZCL_WC_TYPE_AWNING:
        return new HorizontalShadePainter(lift, tilt);

    case EMBER_ZCL_WC_TYPE_SHUTTER:
    case EMBER_ZCL_WC_TYPE_TILT_BLIND_TILT_ONLY:
    case EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT:
        return new VerticalBlindPainter(lift, tilt);

    case EMBER_ZCL_WC_TYPE_PROJECTOR_SCREEN:
    case EMBER_ZCL_WC_TYPE_UNKNOWN:
    default:
        return new VerticalShadePainter(lift, tilt);
    }
}

void LcdPainter::Paint(EmberAfWcType type, uint8_t lift, uint8_t tilt, LcdIcon icon)
{
    FramePainter framePaint         = FramePainter(lift, tilt);
    IconPainter iconPaint           = IconPainter(lift, tilt, icon);
    PixelPainter * coverPaint       = GetCoverPainter(type, lift, tilt);
    CompositePainter compositePaint = CompositePainter(lift, tilt, &framePaint, &iconPaint, coverPaint);
    void * context                  = LCDContext();

    LCD_clear(context);

    for (int i = 0; i < LCD_SIZE; i++)
    {
        for (int j = 0; j < LCD_SIZE; j++)
        {
            if (compositePaint.Color(i, j))
            {
                LCD_drawPixel(context, i, j);
            }
        }
    }
    LCD_update();
    delete coverPaint;
}
