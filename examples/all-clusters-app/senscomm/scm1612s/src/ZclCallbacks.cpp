/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
#include <lib/support/logging/CHIPLogging.h>

#include "ClusterManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    EndpointId endpoint     = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    P6_LOG("MatterPostAttributeChangeCallback - Cluster ID: " ChipLogFormatMEI
           ", EndPoint ID: '0x%02x', Attribute ID: " ChipLogFormatMEI,
           ChipLogValueMEI(clusterId), endpoint, ChipLogValueMEI(attributeId));

    switch (clusterId)
    {
    case OnOff::Id:
        ClusterMgr().OnOnOffPostAttributeChangeCallback(endpoint, attributeId, value);
        break;

    case Identify::Id:
        ClusterMgr().OnIdentifyPostAttributeChangeCallback(endpoint, attributeId, value);
        break;

    case LevelControl::Id:
        ClusterMgr().OnLevelControlAttributeChangeCallback(endpoint, attributeId, value);
        break;

    case ColorControl::Id:
        ClusterMgr().OnColorControlAttributeChangeCallback(endpoint, attributeId, value);
        break;
    default:
        P6_LOG("Unhandled cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
        break;
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
