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

#include <support/logging/CHIPLogging.h>

#include "af.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"
#include <app/util/af-types.h>

#include "AppTask.h"
#include "LightingManager.h"

using namespace chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    ChipLogProgress(Zcl, "Cluster callback: %d", clusterId);

    if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
            return;
        }

        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION,
                                     AppEvent::kEventType_Lighting, size, value);
    }
    else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        if (attributeId != ZCL_MOVE_TO_LEVEL_COMMAND_ID)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
            return;
        }

        ChipLogProgress(Zcl, "Value: %u, length %u", *value, size);
        if (size == 1)
        {
            LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, AppEvent::kEventType_Lighting, size, value);
        }
        else
        {
            ChipLogError(Zcl, "wrong length for level: %d", size);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %d", clusterId);
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
