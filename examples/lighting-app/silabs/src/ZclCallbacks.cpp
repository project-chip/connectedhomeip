/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"
#include "ColorFormat.h"
#include "LightingManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef DIC_ENABLE
#include "dic.h"
#endif // DIC_ENABLE

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    EndpointId endpoint     = attributePath.mEndpointId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
#ifdef DIC_ENABLE
        dic_sendmsg("light/state", (const char *) (value ? (*value ? "on" : "off") : "invalid"));
#endif // DIC_ENABLE
        LightMgr().InitiateAction(AppEvent::kEventType_Light, *value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (clusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
// WIP Apply attribute change to Light
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
        /* XY color space */
        if (attributeId == ColorControl::Attributes::CurrentX::Id || attributeId == ColorControl::Attributes::CurrentY::Id)
        {
            XyColor_t xy;
            if (size != sizeof(uint16_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
                return;
            }
            Protocols::InteractionModel::Status status_x = ColorControl::Attributes::CurrentY::Get(endpoint, &xy.y);
            assert(status_x == Protocols::InteractionModel::Status::Success);
            Protocols::InteractionModel::Status status_y = ColorControl::Attributes::CurrentX::Get(endpoint, &xy.x);
            assert(status_y == Protocols::InteractionModel::Status::Success);
            if (attributeId == ColorControl::Attributes::CurrentX::Id)
            {
                xy.x = *reinterpret_cast<uint16_t *>(value);
                // get Y from cluster value storage
            }
            if (attributeId == ColorControl::Attributes::CurrentY::Id)
            {
                xy.y = *reinterpret_cast<uint16_t *>(value);
            }
            ChipLogProgress(Zcl, "New XY color: %u|%u", xy.x, xy.y);
            LightMgr().InitiateLightAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_XY, sizeof(xy),
                                           (uint8_t *) &xy);
        }
        /* HSV color space */
        else if (attributeId == ColorControl::Attributes::CurrentHue::Id ||
                 attributeId == ColorControl::Attributes::CurrentSaturation::Id ||
                 attributeId == ColorControl::Attributes::EnhancedCurrentHue::Id)
        {
            HsvColor_t hsv                                = {};
            Protocols::InteractionModel::Status statusHue = ColorControl::Attributes::CurrentHue::Get(endpoint, &hsv.h);
            assert(statusHue == Protocols::InteractionModel::Status::Success);
            Protocols::InteractionModel::Status statusSat = ColorControl::Attributes::CurrentSaturation::Get(endpoint, &hsv.s);
            ChipLogProgress(Zcl, "New HSV color: %u|%u", hsv.h, hsv.s);
            assert(statusSat == Protocols::InteractionModel::Status::Success);
            LightMgr().InitiateLightAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_HSV, sizeof(hsv),
                                           (uint8_t *) &hsv);
        }

        else if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
        {
            if (size != sizeof(uint16_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
                return;
            }
            CtColor_t ct;
            ct.ctMireds = *(uint16_t *) value;
            ChipLogProgress(Zcl, "New Temperature Mireds color = %u", *(uint16_t *) value);
            ChipLogProgress(Zcl, "New ColorTemperatureMireds: : %u", ct.ctMireds);
            LightMgr().InitiateLightAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_CT, sizeof(ct),
                                           (uint8_t *) &ct);
        }

#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    }
    else if (clusterId == Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}
