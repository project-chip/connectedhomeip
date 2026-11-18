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

#include "Ui.h"

namespace Ui {
namespace {

constexpr int32_t kLargeMinWidth = 400;

void SetTranslateX(void * obj, int32_t value)
{
    lv_obj_set_style_translate_x(static_cast<lv_obj_t *>(obj), value, LV_PART_MAIN);
}

void SetOpa(void * obj, int32_t value)
{
    lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), static_cast<lv_opa_t>(value), LV_PART_MAIN);
}

void SetScale(void * obj, int32_t value)
{
    lv_obj_set_style_transform_scale(static_cast<lv_obj_t *>(obj), value, LV_PART_MAIN);
}

} // namespace

bool IsLarge()
{
    lv_display_t * display = lv_display_get_default();
    return display != nullptr && lv_display_get_horizontal_resolution(display) >= kLargeMinWidth;
}

int32_t Px(int32_t large, int32_t small)
{
    return IsLarge() ? large : small;
}

const lv_font_t * TitleFont()
{
#if LV_FONT_MONTSERRAT_32
    if (IsLarge())
    {
        return &lv_font_montserrat_32;
    }
#endif
    return HeadingFont();
}

const lv_font_t * HeadingFont()
{
#if LV_FONT_MONTSERRAT_24
    if (IsLarge())
    {
        return &lv_font_montserrat_24;
    }
#endif
    return LV_FONT_DEFAULT;
}

const lv_font_t * BodyFont()
{
    return LV_FONT_DEFAULT;
}

const lv_font_t * CaptionFont()
{
#if LV_FONT_MONTSERRAT_14
    return &lv_font_montserrat_14;
#else
    return LV_FONT_DEFAULT;
#endif
}

int32_t CardRadius()
{
    return Px(22, 10);
}

int32_t Gap()
{
    return Px(14, 8);
}

void LayoutColumn(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, Gap(), LV_PART_MAIN);
}

lv_obj_t * CreateIconBadge(lv_obj_t * parent, const char * symbol, lv_color_t color, int32_t size)
{
    lv_obj_t * badge = lv_obj_create(parent);
    lv_obj_set_size(badge, size, size);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(badge, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(badge, 0, LV_PART_MAIN);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * icon = lv_label_create(badge);
    lv_label_set_text_static(icon, symbol);
    lv_obj_set_style_text_color(icon, lv_color_lighten(color, 60), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, size >= 48 ? HeadingFont() : BodyFont(), LV_PART_MAIN);
    lv_obj_center(icon);
    return badge;
}

lv_obj_t * CreateTile(lv_obj_t * parent, const char * symbol, lv_color_t accent, const char * title, const char * subtitle,
                      lv_event_cb_t onClick, void * userData)
{
    lv_obj_t * tile = lv_button_create(parent);
    lv_obj_set_width(tile, LV_PCT(100));
    lv_obj_set_height(tile, Px(92, 52));
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(tile, Px(18, 10), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(tile, Px(12, 6), LV_PART_MAIN);
    lv_obj_set_style_pad_column(tile, Px(16, 8), LV_PART_MAIN);
    lv_obj_set_style_radius(tile, CardRadius(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tile, Surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(tile, SurfaceRaised(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(tile, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_border_width(tile, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(tile, Outline(), LV_PART_MAIN);
    lv_obj_set_style_border_color(tile, accent, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(tile, Px(24, 0), LV_PART_MAIN);
    lv_obj_set_style_shadow_color(tile, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(tile, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(tile, Px(8, 0), LV_PART_MAIN);

    CreateIconBadge(tile, symbol, accent, Px(56, 32));

    lv_obj_t * text = lv_obj_create(tile);
    lv_obj_remove_style_all(text);
    lv_obj_set_flex_grow(text, 1);
    lv_obj_set_height(text, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(text, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(text, Px(4, 1), LV_PART_MAIN);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * titleLabel = lv_label_create(text);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_font(titleLabel, HeadingFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(titleLabel, Text(), LV_PART_MAIN);
    lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_width(titleLabel, LV_PCT(100));

    if (subtitle != nullptr)
    {
        lv_obj_t * subtitleLabel = lv_label_create(text);
        lv_label_set_text(subtitleLabel, subtitle);
        lv_obj_set_style_text_font(subtitleLabel, CaptionFont(), LV_PART_MAIN);
        lv_obj_set_style_text_color(subtitleLabel, TextMuted(), LV_PART_MAIN);
        lv_label_set_long_mode(subtitleLabel, LV_LABEL_LONG_DOT);
        lv_obj_set_width(subtitleLabel, LV_PCT(100));
    }

    lv_obj_t * chevron = lv_label_create(tile);
    lv_label_set_text_static(chevron, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(chevron, TextMuted(), LV_PART_MAIN);

    if (onClick != nullptr)
    {
        lv_obj_add_event_cb(tile, onClick, LV_EVENT_CLICKED, userData);
    }
    return tile;
}

lv_obj_t * CreatePill(lv_obj_t * parent, const char * text, lv_color_t tint)
{
    lv_obj_t * pill = lv_label_create(parent);
    lv_label_set_text(pill, text);
    lv_obj_set_style_text_font(pill, CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(pill, lv_color_lighten(tint, 70), LV_PART_MAIN);
    lv_obj_set_style_bg_color(pill, tint, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pill, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(pill, Px(12, 6), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(pill, Px(5, 2), LV_PART_MAIN);
    return pill;
}

lv_obj_t * CreateHeading(lv_obj_t * parent, const char * text)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, HeadingFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, Text(), LV_PART_MAIN);
    return label;
}

lv_obj_t * CreateCaption(lv_obj_t * parent, const char * text)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, TextMuted(), LV_PART_MAIN);
    return label;
}

void AnimateEntrance(lv_obj_t * obj, bool fromRight)
{
    const int32_t distance = Px(48, 24) * (fromRight ? 1 : -1);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, SetTranslateX);
    lv_anim_set_values(&anim, distance, 0);
    lv_anim_set_duration(&anim, 260);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    lv_anim_start(&anim);
}

void StartPulse(lv_obj_t * obj)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, SetOpa);
    lv_anim_set_values(&anim, LV_OPA_COVER, LV_OPA_40);
    lv_anim_set_duration(&anim, 900);
    lv_anim_set_reverse_duration(&anim, 900);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

void StopPulse(lv_obj_t * obj)
{
    lv_anim_delete(obj, SetOpa);
    lv_obj_set_style_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
}

void PopIn(lv_obj_t * obj)
{
    lv_obj_set_style_transform_pivot_x(obj, LV_PCT(50), LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(obj, LV_PCT(50), LV_PART_MAIN);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, SetScale);
    lv_anim_set_values(&anim, 0, LV_SCALE_NONE);
    lv_anim_set_duration(&anim, 420);
    lv_anim_set_path_cb(&anim, lv_anim_path_overshoot);
    lv_anim_start(&anim);
}

} // namespace Ui
