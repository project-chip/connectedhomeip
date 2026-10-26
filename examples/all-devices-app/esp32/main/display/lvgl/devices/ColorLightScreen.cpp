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

#include "ColorLightScreen.h"
#include "DeviceHeader.h"
#include "clusters/ColorControlClusterWidget.h"
#include "clusters/LevelControlClusterWidget.h"
#include "clusters/OnOffClusterWidget.h"

namespace chip::app {

namespace {

void SetupScreen(lv_obj_t * parent, const char * title, ColorLight & device)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_hor(parent, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(parent, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 4, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    CreateDeviceHeader(parent, title, device.GetEndpointId());
    CreateOnOffClusterWidget(parent, device.GetOnOffCluster());
    CreateLevelControlClusterWidget(parent, device.GetLevelControlCluster());
}

} // namespace

void ShowColorTemperatureLightScreen(lv_obj_t * parent, ColorLight & device)
{
    SetupScreen(parent, "Color Temperature Light", device);
    CreateColorControlClusterWidget(parent, device.GetColorControlCluster());
}

void ShowExtendedColorLightScreen(lv_obj_t * parent, ColorLight & device)
{
    SetupScreen(parent, "Extended Color Light", device);
    CreateColorControlClusterWidget(parent, device.GetColorControlCluster());
}

} // namespace chip::app
