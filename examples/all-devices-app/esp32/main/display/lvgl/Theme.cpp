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

#include "Theme.h"
#include "Ui.h"

// lv_theme_t is opaque in the public API; copying the default theme needs its layout.
#include <lvgl_private.h>

namespace {

lv_theme_t sTheme;

lv_style_t sScreen;
lv_style_t sCard;
lv_style_t sButton;
lv_style_t sButtonPressed;
lv_style_t sSliderMain;
lv_style_t sSliderIndicator;
lv_style_t sSliderKnob;
lv_style_t sSwitchMain;
lv_style_t sSwitchIndicatorChecked;
lv_style_t sSwitchKnob;
lv_style_t sMsgbox;
lv_style_t sBackdrop;
lv_style_t sTableItems;
lv_style_t sSpinnerMain;
lv_style_t sSpinnerIndicator;
lv_style_t sDropdown;
lv_style_transition_dsc_t sPressTransition;

const lv_style_prop_t kPressProps[] = { LV_STYLE_TRANSFORM_SCALE_X, LV_STYLE_TRANSFORM_SCALE_Y, LV_STYLE_BG_COLOR,
                                        LV_STYLE_BORDER_COLOR, LV_STYLE_PROP_INV };

void InitStyles()
{
    lv_style_init(&sScreen);
    lv_style_set_bg_color(&sScreen, Ui::Background());
    lv_style_set_bg_grad_color(&sScreen, Ui::BackgroundEnd());
    lv_style_set_bg_grad_dir(&sScreen, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&sScreen, LV_OPA_COVER);
    lv_style_set_text_color(&sScreen, Ui::Text());

    lv_style_init(&sCard);
    lv_style_set_bg_color(&sCard, Ui::Surface());
    lv_style_set_bg_opa(&sCard, LV_OPA_90);
    lv_style_set_radius(&sCard, Ui::CardRadius());
    lv_style_set_border_width(&sCard, 1);
    lv_style_set_border_color(&sCard, Ui::Outline());
    lv_style_set_border_opa(&sCard, LV_OPA_70);
    lv_style_set_shadow_width(&sCard, Ui::Px(20, 0));
    lv_style_set_shadow_color(&sCard, lv_color_black());
    lv_style_set_shadow_opa(&sCard, LV_OPA_30);
    lv_style_set_shadow_offset_y(&sCard, Ui::Px(6, 0));
    lv_style_set_pad_all(&sCard, Ui::Px(14, 8));
    lv_style_set_text_color(&sCard, Ui::Text());

    lv_style_transition_dsc_init(&sPressTransition, kPressProps, lv_anim_path_ease_out, 120, 0, nullptr);

    lv_style_init(&sButton);
    lv_style_set_bg_color(&sButton, Ui::Accent());
    lv_style_set_bg_opa(&sButton, LV_OPA_COVER);
    lv_style_set_radius(&sButton, Ui::Px(18, 8));
    lv_style_set_border_width(&sButton, 0);
    lv_style_set_shadow_width(&sButton, 0);
    lv_style_set_pad_hor(&sButton, Ui::Px(16, 8));
    lv_style_set_pad_ver(&sButton, Ui::Px(10, 5));
    lv_style_set_text_color(&sButton, lv_color_white());
    lv_style_set_transform_pivot_x(&sButton, LV_PCT(50));
    lv_style_set_transform_pivot_y(&sButton, LV_PCT(50));
    lv_style_set_transition(&sButton, &sPressTransition);

    lv_style_init(&sButtonPressed);
    lv_style_set_transform_scale(&sButtonPressed, 244);
    lv_style_set_bg_color(&sButtonPressed, lv_color_darken(Ui::Accent(), 40));
    lv_style_set_transition(&sButtonPressed, &sPressTransition);

    lv_style_init(&sSliderMain);
    lv_style_set_bg_color(&sSliderMain, Ui::SurfaceRaised());
    lv_style_set_bg_opa(&sSliderMain, LV_OPA_COVER);
    lv_style_set_radius(&sSliderMain, LV_RADIUS_CIRCLE);
    lv_style_set_height(&sSliderMain, Ui::Px(16, 8));

    lv_style_init(&sSliderIndicator);
    lv_style_set_bg_color(&sSliderIndicator, Ui::Accent());
    lv_style_set_bg_grad_color(&sSliderIndicator, Ui::AccentAlt());
    lv_style_set_bg_grad_dir(&sSliderIndicator, LV_GRAD_DIR_HOR);
    lv_style_set_bg_opa(&sSliderIndicator, LV_OPA_COVER);
    lv_style_set_radius(&sSliderIndicator, LV_RADIUS_CIRCLE);

    lv_style_init(&sSliderKnob);
    lv_style_set_bg_color(&sSliderKnob, lv_color_white());
    lv_style_set_bg_opa(&sSliderKnob, LV_OPA_COVER);
    lv_style_set_radius(&sSliderKnob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&sSliderKnob, Ui::Px(8, 4));
    lv_style_set_shadow_width(&sSliderKnob, Ui::Px(14, 6));
    lv_style_set_shadow_color(&sSliderKnob, Ui::Accent());
    lv_style_set_shadow_opa(&sSliderKnob, LV_OPA_60);

    lv_style_init(&sSwitchMain);
    lv_style_set_bg_color(&sSwitchMain, Ui::SurfaceRaised());
    lv_style_set_bg_opa(&sSwitchMain, LV_OPA_COVER);
    lv_style_set_radius(&sSwitchMain, LV_RADIUS_CIRCLE);

    lv_style_init(&sSwitchIndicatorChecked);
    lv_style_set_bg_color(&sSwitchIndicatorChecked, Ui::Success());
    lv_style_set_bg_opa(&sSwitchIndicatorChecked, LV_OPA_COVER);

    lv_style_init(&sSwitchKnob);
    lv_style_set_bg_color(&sSwitchKnob, lv_color_white());
    lv_style_set_bg_opa(&sSwitchKnob, LV_OPA_COVER);
    lv_style_set_radius(&sSwitchKnob, LV_RADIUS_CIRCLE);

    lv_style_init(&sMsgbox);
    lv_style_set_bg_color(&sMsgbox, Ui::Surface());
    lv_style_set_bg_opa(&sMsgbox, LV_OPA_COVER);
    lv_style_set_radius(&sMsgbox, Ui::Px(26, 12));
    lv_style_set_border_width(&sMsgbox, 1);
    lv_style_set_border_color(&sMsgbox, Ui::Outline());
    lv_style_set_shadow_width(&sMsgbox, Ui::Px(40, 12));
    lv_style_set_shadow_color(&sMsgbox, lv_color_black());
    lv_style_set_shadow_opa(&sMsgbox, LV_OPA_60);
    lv_style_set_pad_all(&sMsgbox, Ui::Px(18, 10));
    lv_style_set_text_color(&sMsgbox, Ui::Text());
    lv_style_set_min_width(&sMsgbox, Ui::Px(360, 240));

    lv_style_init(&sBackdrop);
    lv_style_set_bg_color(&sBackdrop, lv_color_black());
    lv_style_set_bg_opa(&sBackdrop, LV_OPA_60);

    lv_style_init(&sTableItems);
    lv_style_set_bg_opa(&sTableItems, LV_OPA_TRANSP);
    lv_style_set_border_color(&sTableItems, Ui::Outline());
    lv_style_set_border_opa(&sTableItems, LV_OPA_60);
    lv_style_set_border_side(&sTableItems, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_text_color(&sTableItems, Ui::Text());
    lv_style_set_pad_ver(&sTableItems, Ui::Px(16, 10));
    lv_style_set_pad_hor(&sTableItems, Ui::Px(14, 8));

    lv_style_init(&sSpinnerMain);
    lv_style_set_arc_color(&sSpinnerMain, Ui::Outline());
    lv_style_set_arc_width(&sSpinnerMain, Ui::Px(5, 3));

    lv_style_init(&sSpinnerIndicator);
    lv_style_set_arc_color(&sSpinnerIndicator, Ui::Accent());
    lv_style_set_arc_width(&sSpinnerIndicator, Ui::Px(5, 3));

    lv_style_init(&sDropdown);
    lv_style_set_bg_color(&sDropdown, Ui::SurfaceRaised());
    lv_style_set_bg_opa(&sDropdown, LV_OPA_COVER);
    lv_style_set_radius(&sDropdown, Ui::Px(14, 6));
    lv_style_set_border_width(&sDropdown, 1);
    lv_style_set_border_color(&sDropdown, Ui::Outline());
    lv_style_set_text_color(&sDropdown, Ui::Text());
}

void ApplyTheme(lv_theme_t *, lv_obj_t * obj)
{
    if (lv_obj_get_parent(obj) == nullptr)
    {
        lv_obj_add_style(obj, &sScreen, LV_PART_MAIN);
        return;
    }

    if (lv_obj_check_type(obj, &lv_obj_class))
    {
        lv_obj_add_style(obj, &sCard, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_button_class))
    {
        lv_obj_add_style(obj, &sButton, LV_PART_MAIN);
        lv_obj_add_style(obj, &sButtonPressed, LV_PART_MAIN | LV_STATE_PRESSED);
    }
    else if (lv_obj_check_type(obj, &lv_slider_class))
    {
        lv_obj_add_style(obj, &sSliderMain, LV_PART_MAIN);
        lv_obj_add_style(obj, &sSliderIndicator, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &sSliderKnob, LV_PART_KNOB);
    }
    else if (lv_obj_check_type(obj, &lv_switch_class))
    {
        lv_obj_add_style(obj, &sSwitchMain, LV_PART_MAIN);
        lv_obj_add_style(obj, &sSwitchIndicatorChecked, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &sSwitchKnob, LV_PART_KNOB);
    }
    else if (lv_obj_check_type(obj, &lv_msgbox_class))
    {
        lv_obj_add_style(obj, &sMsgbox, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_msgbox_backdrop_class))
    {
        lv_obj_add_style(obj, &sBackdrop, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_table_class))
    {
        lv_obj_add_style(obj, &sCard, LV_PART_MAIN);
        lv_obj_add_style(obj, &sTableItems, LV_PART_ITEMS);
    }
    else if (lv_obj_check_type(obj, &lv_spinner_class))
    {
        lv_obj_add_style(obj, &sSpinnerMain, LV_PART_MAIN);
        lv_obj_add_style(obj, &sSpinnerIndicator, LV_PART_INDICATOR);
    }
    else if (lv_obj_check_type(obj, &lv_dropdown_class))
    {
        lv_obj_add_style(obj, &sDropdown, LV_PART_MAIN);
    }
}

} // namespace

void InitTheme(lv_display_t * display)
{
    InitStyles();

    lv_theme_t * base = lv_theme_default_init(display, Ui::Accent(), Ui::AccentAlt(), /* dark = */ true, LV_FONT_DEFAULT);
    sTheme            = *base;
    lv_theme_set_parent(&sTheme, base);
    lv_theme_set_apply_cb(&sTheme, ApplyTheme);
    lv_display_set_theme(display, &sTheme);
}
