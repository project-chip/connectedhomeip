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
#include <support/logging/CHIPLogging.h>

#include "LightingManager.h"

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af-types.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
        {
            ChipLogError(Zcl, "ON OFF attribute ID: 0x%" PRIx32 " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                         attributeId, type, *value, size);
            return;
        }

        LightMgr().InitiateAction(AppEvent::kEventType_Light, *value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
    else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: 0x%" PRIx32 " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        attributeId, type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (clusterId == ZCL_COLOR_CONTROL_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: 0x%" PRIx32 " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        attributeId, type, *value, size);

        // WIP Apply attribute change to Light
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown Cluster ID: 0x%" PRIx32, clusterId);
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
