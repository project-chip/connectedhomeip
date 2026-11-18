/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lvgl.h>

/**
 * Design tokens and shared building blocks for the LVGL renderer. Sizes come in a
 * large variant for 480-class panels and a small one for 320x240, chosen at run time
 * from the display resolution.
 */
namespace Ui {

inline lv_color_t Background()
{
    return lv_color_hex(0x0B1020);
}
inline lv_color_t BackgroundEnd()
{
    return lv_color_hex(0x182046);
}
inline lv_color_t Surface()
{
    return lv_color_hex(0x1C2446);
}
inline lv_color_t SurfaceRaised()
{
    return lv_color_hex(0x283260);
}
inline lv_color_t Outline()
{
    return lv_color_hex(0x35427E);
}
inline lv_color_t Accent()
{
    return lv_color_hex(0x4F7DFF);
}
inline lv_color_t AccentAlt()
{
    return lv_color_hex(0x8B5CF6);
}
inline lv_color_t Success()
{
    return lv_color_hex(0x34D399);
}
inline lv_color_t Danger()
{
    return lv_color_hex(0xFF5C6C);
}
inline lv_color_t Warning()
{
    return lv_color_hex(0xFBBF24);
}
inline lv_color_t Text()
{
    return lv_color_hex(0xEEF1FA);
}
inline lv_color_t TextMuted()
{
    return lv_color_hex(0x9AA6CF);
}

bool IsLarge();
int32_t Px(int32_t large, int32_t small);

const lv_font_t * TitleFont();
const lv_font_t * HeadingFont();
const lv_font_t * BodyFont();
const lv_font_t * CaptionFont();

int32_t CardRadius();
int32_t Gap();

/// Column container with no chrome of its own, for laying out cards inside a screen.
void LayoutColumn(lv_obj_t * parent);

/// Circle with a symbol in it.
lv_obj_t * CreateIconBadge(lv_obj_t * parent, const char * symbol, lv_color_t color, int32_t size);

/// Full-width menu entry: icon badge, title, subtitle and a chevron.
lv_obj_t * CreateTile(lv_obj_t * parent, const char * symbol, lv_color_t accent, const char * title, const char * subtitle,
                      lv_event_cb_t onClick, void * userData = nullptr);

/// Rounded label with a tinted background.
lv_obj_t * CreatePill(lv_obj_t * parent, const char * text, lv_color_t tint);

lv_obj_t * CreateHeading(lv_obj_t * parent, const char * text);
lv_obj_t * CreateCaption(lv_obj_t * parent, const char * text);

/// Slides the object in from the side while it appears.
void AnimateEntrance(lv_obj_t * obj, bool fromRight);

/// Breathing opacity until StopPulse() or the object is deleted.
void StartPulse(lv_obj_t * obj);
void StopPulse(lv_obj_t * obj);

/// Grows the object from nothing to its size with a slight overshoot.
void PopIn(lv_obj_t * obj);

} // namespace Ui
