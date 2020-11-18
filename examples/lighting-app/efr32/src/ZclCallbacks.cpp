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
#include "DataModelHandler.h"
#include "LightingManager.h"

#include "gen/attribute-id.h"
#include "gen/cluster-id.h"

#include <app/util/af-types.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        EFR32_LOG("Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        EFR32_LOG("Unknown attribute ID: %d", attributeId);
        return;
    }

    if (*value)
    {
        LightMgr().InitiateAction(AppEvent::kEventType_Light, LightingManager::ON_ACTION);
    }
    else
    {
        LightMgr().InitiateAction(AppEvent::kEventType_Light, LightingManager::OFF_ACTION);
    }
}

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 *
 * TODO Issue #3841
 * emberAfClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfClusterInitCallback(EndpointId endpoint, ClusterId clusterId)
{
    // TODO: implement any additional Cluster Server init actions
}
