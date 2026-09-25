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

#include "ColorControlClusterWidget.h"
#include "CommandStatusLog.h"
#include "DisplayNotificationHub.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

using Clusters::ColorControl::EnhancedColorModeEnum;

// Range the simulated lamp supports, set in ColorLight.cpp (153 mireds = 6500 K,
// 370 mireds = 2700 K). The cluster keeps its CTConfig private, so the values are repeated here.
constexpr uint16_t kMinMireds = 153;
constexpr uint16_t kMaxMireds = 370;

// Hue and saturation are 8 bit attributes; 254 is the maximum the spec allows.
constexpr uint8_t kMaxHue        = 254;
constexpr uint8_t kMaxSaturation = 254;

// CurrentX / CurrentY are expressed in units of 1/65536, constrained to 0xFEFF.
constexpr uint16_t kMaxXY = 0xFEFF;

// Values used to seed the controls of a mode the cluster is not currently in. The cluster's live
// accessors return 0 outside their own mode instead of converting, so there is no current colour to
// carry over; the seeds stand in until the user moves a slider. xy is the D65 white point.
constexpr uint16_t kSeedMireds    = kMaxMireds;
constexpr uint8_t kSeedHue        = 0;
constexpr uint8_t kSeedSaturation = kMaxSaturation;
constexpr uint16_t kSeedX         = 20493; // 0.3127
constexpr uint16_t kSeedY         = 21562; // 0.3290

// The colour modes this widget can drive. Both hue/saturation flavours of the cluster (plain and
// enhanced) share one set of controls.
enum class ColorMode : uint8_t
{
    kHueSaturation = 0,
    kXy            = 1,
    kTemperature   = 2,
};

constexpr size_t kModeCount = 3;
constexpr uint8_t kNoButton = 0xFF;

constexpr size_t Index(ColorMode mode)
{
    return static_cast<size_t>(mode);
}

ColorMode ModeOf(EnhancedColorModeEnum clusterMode)
{
    switch (clusterMode)
    {
    case EnhancedColorModeEnum::kCurrentXAndCurrentY:
        return ColorMode::kXy;
    case EnhancedColorModeEnum::kColorTemperatureMireds:
        return ColorMode::kTemperature;
    default:
        return ColorMode::kHueSaturation;
    }
}

// Sliders of the hidden modes hold the value a mode switch will command, so the widget needs no
// separate storage for them.
struct ColorControlContext
{
    Clusters::ColorControlCluster * cluster;

    lv_obj_t * selector;                      // null when a single mode is supported
    const char * selectorMap[kModeCount + 1]; // referenced by the button matrix for its lifetime
    ColorMode buttonMode[kModeCount];         // button index -> mode
    uint8_t buttonIndex[kModeCount];          // mode -> button index, kNoButton when absent

    lv_obj_t * group[kModeCount]; // null for unsupported modes, only the active one is visible

    lv_obj_t * hueSlider;
    lv_obj_t * hueLabel;
    lv_obj_t * saturationSlider;
    lv_obj_t * saturationLabel;
    lv_obj_t * swatch;

    lv_obj_t * xSlider;
    lv_obj_t * xLabel;
    lv_obj_t * ySlider;
    lv_obj_t * yLabel;

    lv_obj_t * temperatureSlider;
    lv_obj_t * temperatureLabel;

    ColorMode shown;
};

void SetHueLabel(lv_obj_t * label, uint8_t hue)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "Hue %u", static_cast<unsigned int>(hue));
    lv_label_set_text(label, buf);
}

void SetSaturationLabel(lv_obj_t * label, uint8_t saturation)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "Saturation %u", static_cast<unsigned int>(saturation));
    lv_label_set_text(label, buf);
}

void SetSwatch(lv_obj_t * swatch, uint8_t hue, uint8_t saturation)
{
    const uint32_t hueDegrees       = (static_cast<uint32_t>(hue) * 360) / (kMaxHue + 1);
    const uint8_t saturationPercent = static_cast<uint8_t>((static_cast<uint32_t>(saturation) * 100) / kMaxSaturation);

    lv_obj_set_style_bg_color(swatch, lv_color_hsv_to_rgb(static_cast<uint16_t>(hueDegrees), saturationPercent, 100), LV_PART_MAIN);
}

// CurrentX / CurrentY are fractions of 65536 and always below 1, printed as 0.nnnn. Fixed point
// keeps this off the floating point printf, which the nano formatting on this target lacks.
void SetXyLabel(lv_obj_t * label, char axis, uint16_t value)
{
    const unsigned int fraction = static_cast<unsigned int>((static_cast<uint32_t>(value) * 10000u + 32768u) / 65536u);

    char buf[16];
    snprintf(buf, sizeof(buf), "%c 0.%04u", axis, fraction);
    lv_label_set_text(label, buf);
}

void SetTemperatureLabel(lv_obj_t * label, uint16_t mireds)
{
    const unsigned int kelvin = (mireds > 0) ? (1000000u / mireds) : 0;

    char buf[48];
    snprintf(buf, sizeof(buf), "Color temperature: %u mireds (%u K)", static_cast<unsigned int>(mireds), kelvin);
    lv_label_set_text(label, buf);
}

// Container for one mode's controls: transparent and border free so the card reads as a single
// surface, with enough row spacing to tell the sliders apart.
lv_obj_t * CreateGroup(lv_obj_t * card)
{
    lv_obj_t * group = lv_obj_create(card);
    lv_obj_set_width(group, LV_PCT(100));
    lv_obj_set_height(group, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(group, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(group, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(group, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(group, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(group, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(group, LV_OBJ_FLAG_HIDDEN);

    return group;
}

lv_obj_t * CreateSlider(lv_obj_t * group, int32_t min, int32_t max, int32_t value)
{
    lv_obj_t * slider = lv_slider_create(group);
    lv_obj_set_width(slider, LV_PCT(96));
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);

    return slider;
}

void ShowMode(ColorControlContext * context, ColorMode mode)
{
    for (size_t i = 0; i < kModeCount; i++)
    {
        if (context->group[i] == nullptr)
        {
            continue;
        }

        if (i == Index(mode))
        {
            lv_obj_clear_flag(context->group[i], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(context->group[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (context->selector != nullptr && context->buttonIndex[Index(mode)] != kNoButton)
    {
        lv_buttonmatrix_clear_button_ctrl_all(context->selector, LV_BUTTONMATRIX_CTRL_CHECKED);
        lv_buttonmatrix_set_button_ctrl(context->selector, context->buttonIndex[Index(mode)], LV_BUTTONMATRIX_CTRL_CHECKED);
    }

    context->shown = mode;
}

// The cluster has no "set mode" API: the mode follows whichever colour command ran last, so
// switching means re-sending the target mode's current controls.
void SendMode(ColorControlContext * context, ColorMode mode)
{
    Clusters::ColorControlCluster * cluster = context->cluster;

    switch (mode)
    {
    case ColorMode::kHueSaturation: {
        const auto hue        = static_cast<uint16_t>(lv_slider_get_value(context->hueSlider));
        const auto saturation = static_cast<uint8_t>(lv_slider_get_value(context->saturationSlider));
        DeviceLayer::SystemLayer().ScheduleLambda([cluster, hue, saturation]() {
            LogCommandFailure("MoveToHueAndSaturation", cluster->MoveToHueAndSaturation(hue, saturation, 0, /*isEnhanced=*/false));
        });
        break;
    }
    case ColorMode::kXy: {
        const auto x = static_cast<uint16_t>(lv_slider_get_value(context->xSlider));
        const auto y = static_cast<uint16_t>(lv_slider_get_value(context->ySlider));
        DeviceLayer::SystemLayer().ScheduleLambda(
            [cluster, x, y]() { LogCommandFailure("MoveToColor", cluster->MoveToColor(x, y, 0)); });
        break;
    }
    case ColorMode::kTemperature: {
        const auto mireds = static_cast<uint16_t>(lv_slider_get_value(context->temperatureSlider));
        DeviceLayer::SystemLayer().ScheduleLambda(
            [cluster, mireds]() { LogCommandFailure("MoveToColorTemp", cluster->MoveToColorTemp(mireds, 0)); });
        break;
    }
    }
}

// Label-only updates, driven either by a finger on a slider or by a report from the cluster.
void UpdateHueSaturationLabels(ColorControlContext * context)
{
    const auto hue        = static_cast<uint8_t>(lv_slider_get_value(context->hueSlider));
    const auto saturation = static_cast<uint8_t>(lv_slider_get_value(context->saturationSlider));

    SetHueLabel(context->hueLabel, hue);
    SetSaturationLabel(context->saturationLabel, saturation);
    SetSwatch(context->swatch, hue, saturation);
}

void UpdateXyLabels(ColorControlContext * context)
{
    SetXyLabel(context->xLabel, 'x', static_cast<uint16_t>(lv_slider_get_value(context->xSlider)));
    SetXyLabel(context->yLabel, 'y', static_cast<uint16_t>(lv_slider_get_value(context->ySlider)));
}

void UpdateTemperatureLabel(ColorControlContext * context)
{
    SetTemperatureLabel(context->temperatureLabel, static_cast<uint16_t>(lv_slider_get_value(context->temperatureSlider)));
}

// Pulls the active mode's values back out of the cluster. Sliders being dragged keep the value the
// finger is on; their labels follow the slider so the two never disagree.
void RefreshFromCluster(ColorControlContext * context)
{
    const ColorMode mode = ModeOf(context->cluster->GetEnhancedColorMode());

    if (mode != context->shown && context->group[Index(mode)] != nullptr)
    {
        ShowMode(context, mode);
    }

    switch (context->shown)
    {
    case ColorMode::kHueSaturation: {
        if (!lv_slider_is_dragged(context->hueSlider))
        {
            lv_slider_set_value(context->hueSlider, context->cluster->CurrentHue(), LV_ANIM_OFF);
        }
        if (!lv_slider_is_dragged(context->saturationSlider))
        {
            lv_slider_set_value(context->saturationSlider, context->cluster->Saturation(), LV_ANIM_OFF);
        }

        UpdateHueSaturationLabels(context);
        break;
    }
    case ColorMode::kXy: {
        if (!lv_slider_is_dragged(context->xSlider))
        {
            lv_slider_set_value(context->xSlider, context->cluster->CurrentX(), LV_ANIM_OFF);
        }
        if (!lv_slider_is_dragged(context->ySlider))
        {
            lv_slider_set_value(context->ySlider, context->cluster->CurrentY(), LV_ANIM_OFF);
        }

        UpdateXyLabels(context);
        break;
    }
    case ColorMode::kTemperature: {
        if (!lv_slider_is_dragged(context->temperatureSlider))
        {
            lv_slider_set_value(context->temperatureSlider, context->cluster->ColorTempMireds(), LV_ANIM_OFF);
        }

        UpdateTemperatureLabel(context);
        break;
    }
    }
}

// Dragging a slider only moves its labels. The command is sent when the finger lifts, so a drag
// across the track costs one command instead of one per pixel.
void OnHueSaturationChanged(lv_event_t * event)
{
    UpdateHueSaturationLabels(static_cast<ColorControlContext *>(lv_event_get_user_data(event)));
}

void OnHueSaturationReleased(lv_event_t * event)
{
    SendMode(static_cast<ColorControlContext *>(lv_event_get_user_data(event)), ColorMode::kHueSaturation);
}

void OnXyChanged(lv_event_t * event)
{
    UpdateXyLabels(static_cast<ColorControlContext *>(lv_event_get_user_data(event)));
}

void OnXyReleased(lv_event_t * event)
{
    SendMode(static_cast<ColorControlContext *>(lv_event_get_user_data(event)), ColorMode::kXy);
}

void OnTemperatureChanged(lv_event_t * event)
{
    UpdateTemperatureLabel(static_cast<ColorControlContext *>(lv_event_get_user_data(event)));
}

void OnTemperatureReleased(lv_event_t * event)
{
    SendMode(static_cast<ColorControlContext *>(lv_event_get_user_data(event)), ColorMode::kTemperature);
}

void OnModeSelected(lv_event_t * event)
{
    auto * context          = static_cast<ColorControlContext *>(lv_event_get_user_data(event));
    auto * selector         = static_cast<lv_obj_t *>(lv_event_get_target(event));
    const uint32_t buttonId = lv_buttonmatrix_get_selected_button(selector);
    VerifyOrReturn(buttonId < kModeCount);

    const ColorMode mode = context->buttonMode[buttonId];

    // Shown immediately: a command refused while the light is off produces no report to reconcile
    // against, and the selector would otherwise snap back for no visible reason.
    ShowMode(context, mode);
    SendMode(context, mode);
}

void CreateHueSaturationGroup(ColorControlContext * context, lv_obj_t * card, bool active)
{
    lv_obj_t * group                                 = CreateGroup(card);
    context->group[Index(ColorMode::kHueSaturation)] = group;

    lv_obj_t * headerRow = lv_obj_create(group);
    lv_obj_set_width(headerRow, LV_PCT(100));
    lv_obj_set_height(headerRow, 30);
    lv_obj_set_flex_flow(headerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(headerRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(headerRow, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(headerRow, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(headerRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(headerRow, LV_OBJ_FLAG_SCROLLABLE);

    context->hueLabel = lv_label_create(headerRow);

    context->swatch = lv_obj_create(headerRow);
    lv_obj_set_size(context->swatch, 34, 26);
    lv_obj_set_style_radius(context->swatch, 4, LV_PART_MAIN);
    lv_obj_clear_flag(context->swatch, LV_OBJ_FLAG_SCROLLABLE);

    const uint8_t hue        = active ? context->cluster->CurrentHue() : kSeedHue;
    const uint8_t saturation = active ? context->cluster->Saturation() : kSeedSaturation;

    context->hueSlider = CreateSlider(group, 0, kMaxHue, hue);

    context->saturationLabel  = lv_label_create(group);
    context->saturationSlider = CreateSlider(group, 0, kMaxSaturation, saturation);

    SetHueLabel(context->hueLabel, hue);
    SetSaturationLabel(context->saturationLabel, saturation);
    SetSwatch(context->swatch, hue, saturation);

    lv_obj_add_event_cb(context->hueSlider, OnHueSaturationChanged, LV_EVENT_VALUE_CHANGED, context);
    lv_obj_add_event_cb(context->saturationSlider, OnHueSaturationChanged, LV_EVENT_VALUE_CHANGED, context);
    lv_obj_add_event_cb(context->hueSlider, OnHueSaturationReleased, LV_EVENT_RELEASED, context);
    lv_obj_add_event_cb(context->saturationSlider, OnHueSaturationReleased, LV_EVENT_RELEASED, context);
}

void CreateXyGroup(ColorControlContext * context, lv_obj_t * card, bool active)
{
    lv_obj_t * group                      = CreateGroup(card);
    context->group[Index(ColorMode::kXy)] = group;

    const uint16_t x = active ? context->cluster->CurrentX() : kSeedX;
    const uint16_t y = active ? context->cluster->CurrentY() : kSeedY;

    context->xLabel  = lv_label_create(group);
    context->xSlider = CreateSlider(group, 0, kMaxXY, x);

    context->yLabel  = lv_label_create(group);
    context->ySlider = CreateSlider(group, 0, kMaxXY, y);

    SetXyLabel(context->xLabel, 'x', x);
    SetXyLabel(context->yLabel, 'y', y);

    lv_obj_add_event_cb(context->xSlider, OnXyChanged, LV_EVENT_VALUE_CHANGED, context);
    lv_obj_add_event_cb(context->ySlider, OnXyChanged, LV_EVENT_VALUE_CHANGED, context);
    lv_obj_add_event_cb(context->xSlider, OnXyReleased, LV_EVENT_RELEASED, context);
    lv_obj_add_event_cb(context->ySlider, OnXyReleased, LV_EVENT_RELEASED, context);
}

void CreateTemperatureGroup(ColorControlContext * context, lv_obj_t * card, bool active)
{
    lv_obj_t * group                               = CreateGroup(card);
    context->group[Index(ColorMode::kTemperature)] = group;

    const uint16_t mireds = active ? context->cluster->ColorTempMireds() : kSeedMireds;

    context->temperatureLabel  = lv_label_create(group);
    context->temperatureSlider = CreateSlider(group, kMinMireds, kMaxMireds, mireds);

    SetTemperatureLabel(context->temperatureLabel, mireds);

    lv_obj_add_event_cb(context->temperatureSlider, OnTemperatureChanged, LV_EVENT_VALUE_CHANGED, context);
    lv_obj_add_event_cb(context->temperatureSlider, OnTemperatureReleased, LV_EVENT_RELEASED, context);
}

void CreateSelector(ColorControlContext * context, lv_obj_t * card)
{
    context->selector = lv_buttonmatrix_create(card);
    lv_obj_set_width(context->selector, LV_PCT(100));
    lv_obj_set_height(context->selector, 36);
    lv_obj_set_style_pad_all(context->selector, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(context->selector, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(context->selector, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_buttonmatrix_set_map(context->selector, context->selectorMap);
    lv_buttonmatrix_set_button_ctrl_all(context->selector, LV_BUTTONMATRIX_CTRL_CHECKABLE);
    lv_buttonmatrix_set_one_checked(context->selector, true);

    lv_obj_add_event_cb(context->selector, OnModeSelected, LV_EVENT_VALUE_CHANGED, context);
}

} // namespace

lv_obj_t * CreateColorControlClusterWidget(lv_obj_t * parent, Clusters::ColorControlCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 8, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    auto * context = static_cast<ColorControlContext *>(lv_malloc(sizeof(ColorControlContext)));
    VerifyOrReturnValue(context != nullptr, card);

    *context         = {};
    context->cluster = &cluster;
    for (size_t i = 0; i < kModeCount; i++)
    {
        context->buttonIndex[i] = kNoButton;
    }

    // Owned by the card, which outlives every handler that reads it.
    lv_obj_add_event_cb(
        card, [](lv_event_t * event) { lv_free(lv_event_get_user_data(event)); }, LV_EVENT_DELETE, context);

    const bool supportsHueSaturation = cluster.SupportsMode(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation) ||
        cluster.SupportsMode(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);
    const bool supportsXy          = cluster.SupportsMode(EnhancedColorModeEnum::kCurrentXAndCurrentY);
    const bool supportsTemperature = cluster.SupportsMode(EnhancedColorModeEnum::kColorTemperatureMireds);

    const ColorMode active = ModeOf(cluster.GetEnhancedColorMode());

    uint8_t buttonCount = 0;
    auto addButton      = [context, &buttonCount](ColorMode mode, const char * text) {
        context->selectorMap[buttonCount] = text;
        context->buttonMode[buttonCount]  = mode;
        context->buttonIndex[Index(mode)] = buttonCount;
        buttonCount++;
    };

    if (supportsHueSaturation)
    {
        CreateHueSaturationGroup(context, card, active == ColorMode::kHueSaturation);
        addButton(ColorMode::kHueSaturation, "Hue/Sat");
    }
    if (supportsXy)
    {
        CreateXyGroup(context, card, active == ColorMode::kXy);
        addButton(ColorMode::kXy, "xy");
    }
    if (supportsTemperature)
    {
        CreateTemperatureGroup(context, card, active == ColorMode::kTemperature);
        addButton(ColorMode::kTemperature, "Temp");
    }
    context->selectorMap[buttonCount] = nullptr;

    // A single mode needs no selector; the group below it is the whole card.
    if (buttonCount > 1)
    {
        CreateSelector(context, card);
        lv_obj_move_to_index(context->selector, 0);
    }

    // Without a single supported mode there are no controls to refresh, and RefreshFromCluster
    // would reach for sliders that were never created.
    VerifyOrReturnValue(buttonCount != 0, card);

    // The reported mode can name controls that were never built, because a cluster may report
    // a mode outside the set its feature map advertises. Falling back to the first mode that
    // was built keeps the card from rendering empty. Buttons are added alongside groups, so
    // buttonMode[0] is that mode.
    const ColorMode initial = (context->group[Index(active)] == nullptr) ? context->buttonMode[0] : active;

    ShowMode(context, initial);

    // Subscribing with 'card' automatically unregisters when 'card' is deleted. Every colour
    // attribute is of interest: the cluster never reports ColorMode itself, so a mode switch is
    // only visible as a change of the new mode's axes.
    DisplayNotificationHub::Instance().Subscribe(card, cluster.GetPaths()[0].mEndpointId, Clusters::ColorControl::Id,
                                                 [context](const ConcreteAttributePath &) { RefreshFromCluster(context); });

    return card;
}

} // namespace chip::app
