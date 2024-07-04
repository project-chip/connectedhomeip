/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

#include "AppTask.h"
#include "ColorFormat.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    static HsvColor_t hsv;
    static XyColor_t xy;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        ChipLogDetail(Zcl, "Cluster OnOff: attribute OnOff set to %u", *value);
        GetAppTask().SetInitiateAction(*value ? AppTask::ON_ACTION : AppTask::OFF_ACTION,
                                       static_cast<int32_t>(AppEvent::kEventType_DeviceAction), value);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        if (GetAppTask().IsTurnedOn())
        {
            ChipLogDetail(Zcl, "Cluster LevelControl: attribute CurrentLevel set to %u", *value);
            GetAppTask().SetInitiateAction(AppTask::LEVEL_ACTION, static_cast<int32_t>(AppEvent::kEventType_DeviceAction), value);
        }
        else
        {
            ChipLogDetail(Zcl, "LED is off. Try to use move-to-level-with-on-off instead of move-to-level");
        }
    }
    else if (clusterId == ColorControl::Id)
    {
        /* Ignore several attributes that are currently not processed */
        if ((attributeId == ColorControl::Attributes::RemainingTime::Id) ||
            (attributeId == ColorControl::Attributes::EnhancedColorMode::Id) ||
            (attributeId == ColorControl::Attributes::ColorMode::Id))
        {
            return;
        }

        /* XY color space */
        if (attributeId == ColorControl::Attributes::CurrentX::Id || attributeId == ColorControl::Attributes::CurrentY::Id)
        {
            if (attributeId == ColorControl::Attributes::CurrentX::Id)
            {
                xy.x = *reinterpret_cast<uint16_t *>(value);
            }
            else if (attributeId == ColorControl::Attributes::CurrentY::Id)
            {
                xy.y = *reinterpret_cast<uint16_t *>(value);
            }

            ChipLogDetail(Zcl, "New XY color: %u|%u", xy.x, xy.y);
            GetAppTask().SetInitiateAction(AppTask::COLOR_ACTION_XY, static_cast<int32_t>(AppEvent::kEventType_DeviceAction),
                                           (uint8_t *) &xy);
        }
        /* HSV color space */
        else if (attributeId == ColorControl::Attributes::CurrentHue::Id ||
                 attributeId == ColorControl::Attributes::CurrentSaturation::Id ||
                 attributeId == ColorControl::Attributes::EnhancedCurrentHue::Id)
        {
            if (attributeId == ColorControl::Attributes::EnhancedCurrentHue::Id)
            {
                hsv.h = (uint8_t) (((*reinterpret_cast<uint16_t *>(value)) & 0xFF00) >> 8);
                hsv.s = (uint8_t) ((*reinterpret_cast<uint16_t *>(value)) & 0xFF);
            }
            else if (attributeId == ColorControl::Attributes::CurrentHue::Id)
            {
                hsv.h = *value;
            }
            else if (attributeId == ColorControl::Attributes::CurrentSaturation::Id)
            {
                hsv.s = *value;
            }
            ChipLogDetail(Zcl, "New HSV color: hue = %u| saturation = %u", hsv.h, hsv.s);
            GetAppTask().SetInitiateAction(AppTask::COLOR_ACTION_HSV, static_cast<int32_t>(AppEvent::kEventType_DeviceAction),
                                           (uint8_t *) &hsv);
        }
        /* Temperature Mireds color space */
        else if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
        {
            ChipLogDetail(Zcl, "New Temperature Mireds color = %u", *(uint16_t *) value);
            GetAppTask().SetInitiateAction(AppTask::COLOR_ACTION_CT, static_cast<int32_t>(AppEvent::kEventType_DeviceAction),
                                           value);
        }
        else
        {
            ChipLogDetail(Zcl, "Ignore ColorControl attribute (%u) that is not currently processed!", attributeId);
        }
    }
}
