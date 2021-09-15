/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "LightingManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

using namespace ::chip;
using namespace chip::app::Clusters;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, size, value);
    }
    else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        if (attributeId != ZCL_CURRENT_LEVEL_ATTRIBUTE_ID)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        if (size == 1)
        {
            ChipLogProgress(Zcl, "New level: %u", *value);
            LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, 0, size, value);
        }
        else
        {
            ChipLogError(Zcl, "wrong length for level: %d", size);
        }
    }
    else if (clusterId == ZCL_COLOR_CONTROL_CLUSTER_ID)
    {
        /* ignore several attributes that are currently not processed */
        if ((attributeId == ZCL_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE_ID) ||
            (attributeId == ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE_ID) ||
            (attributeId == ZCL_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID))
        {
            return;
        }

        if ((attributeId != ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID) &&
            (attributeId != ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID) &&
            (attributeId != ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID) &&
            (attributeId != ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID))
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %" PRIx32, attributeId);
            return;
        }

        if (size == sizeof(uint16_t))
        {
            XyColor_t xy;
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID)
            {
                xy.x = *reinterpret_cast<uint16_t *>(value);
                // get Y from cluster value storage
                EmberAfStatus status = ColorControl::Attributes::GetCurrentY(endpoint, &xy.y);
                assert(status == EMBER_ZCL_STATUS_SUCCESS);
            }
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID)
            {
                xy.y = *reinterpret_cast<uint16_t *>(value);
                // get X from cluster value storage
                EmberAfStatus status = ColorControl::Attributes::GetCurrentX(endpoint, &xy.x);
                assert(status == EMBER_ZCL_STATUS_SUCCESS);
            }
            ChipLogProgress(Zcl, "New XY color: %u|%u", xy.x, xy.y);
            LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_XY, 0, sizeof(xy), (uint8_t *) &xy);
        }
        else if (size == sizeof(uint8_t))
        {
            HsvColor_t hsv;
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
            {
                hsv.h = *value;
                // get saturation from cluster value storage
                EmberAfStatus status = ColorControl::Attributes::GetCurrentSaturation(endpoint, &hsv.s);
                assert(status == EMBER_ZCL_STATUS_SUCCESS);
            }
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID)
            {
                hsv.s = *value;
                // get hue from cluster value storage
                EmberAfStatus status = ColorControl::Attributes::GetCurrentHue(endpoint, &hsv.h);
                assert(status == EMBER_ZCL_STATUS_SUCCESS);
            }
            ChipLogProgress(Zcl, "New HSV color: %u|%u", hsv.h, hsv.s);
            LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_HSV, 0, sizeof(hsv), (uint8_t *) &hsv);
        }
        else
        {
            ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %" PRIx32, clusterId);
        return;
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
    GetAppTask().UpdateClusterState();
}
