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
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
#include "RGBLEDWidget.h"
#endif //(defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
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
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
#ifdef DIC_ENABLE
        dic_sendmsg("light/state", (const char *) (value ? (*value ? "on" : "off") : "invalid"));
#endif // DIC_ENABLE
        LightMgr().InitiateAction(AppEvent::kEventType_Light, *value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION,
                                  value);
    }
    // WIP Apply attribute change to Light
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        if (attributeId == LevelControl::Attributes::CurrentLevel::Id && value != nullptr)
        {
            LightMgr().InitiateAction(AppEvent::kEventType_Light, LightingManager::LEVEL_ACTION, value);
        }
    }
#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    // WIP Apply attribute change to Light
    if (clusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        if (clusterId == ColorControl::Id && attributeId == ColorControl::Attributes::CurrentX::Id)
        {
            ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                            ChipLogValueMEI(attributeId), type, *value, size);

            LightMgr().InitiateLightCtrlAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_XY, attributeId, value);
        }
        else if (clusterId == ColorControl::Id && attributeId == ColorControl::Attributes::CurrentY::Id)
        {
            ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                            ChipLogValueMEI(attributeId), type, *value, size);
            LightMgr().InitiateLightCtrlAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_XY, attributeId, value);
        }
        if (clusterId == ColorControl::Id && attributeId == ColorControl::Attributes::CurrentHue::Id)
        {
            ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                            ChipLogValueMEI(attributeId), type, *value, size);
            LightMgr().InitiateLightCtrlAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_HSV, attributeId, value);
        }
        else if (clusterId == ColorControl::Id && attributeId == ColorControl::Attributes::CurrentSaturation::Id)
        {
            ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                            ChipLogValueMEI(attributeId), type, *value, size);
            LightMgr().InitiateLightCtrlAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_HSV, attributeId, value);
        }
        else if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
        {
            if (size != sizeof(uint16_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
                return;
            }
            LightMgr().InitiateLightCtrlAction(AppEvent::kEventType_Light, LightingManager::COLOR_ACTION_CT, attributeId, value);
        }
    }
#endif // (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
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
